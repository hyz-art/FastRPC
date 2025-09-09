#include "fiber.h"
#include "config.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"
#include <atomic>
namespace sylar{
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
//定义两个全局原子变量，用于总数计数
std::atomic<uint64_t> s_fiber_id{0};
std::atomic<uint64_t> s_fiber_count{0};

static thread_local Fiber* t_fiber=nullptr;// 当前线程的协程
static thread_local Fiber::ptr t_thread_fiber=nullptr;//主协程

//根据config写协程的栈大小
static ConfigVar<uint32_t>::ptr g_fiber_stack_size=
    Config::Lookup<uint32_t>("fiber.stack_size",1024*1024, "fiber stack size");

//测内存分配效率，创建/释放协程运行栈
class MallocStackAllocator{
public:
    static void* Alloc(size_t size){
        return malloc(size);
    }
    static void Dealloc(void* vp, size_t size){
        return free(vp);
    }
};

using StackAllocator=MallocStackAllocator;
//主协程构造
Fiber::Fiber(){
    m_state=EXEC;
    SetThis(this);
    if(getcontext(&m_ctx))
    {
        SYLAR_ASSERT2(false,"getcontext");
    }
    ++s_fiber_count;
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
}
//子协程构造
Fiber::Fiber(std::function<void()>cb, size_t stacksize, bool use_caller)
    :m_id(++s_fiber_id),m_cb(cb){
        ++s_fiber_count;
    m_stacksize=stacksize?stacksize:g_fiber_stack_size->getValue();
    m_stack=StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx))
    {
         SYLAR_ASSERT2(false,"getcontext");
    }
    m_ctx.uc_link=nullptr;//协程结束后不切换到其他协程
    m_ctx.uc_stack.ss_size=m_stacksize;
    m_ctx.uc_stack.ss_sp=m_stack;
    if(!use_caller){//如果不是调用者协程，指明该context入口函数
        makecontext(&m_ctx,&Fiber::MainFunc,0);
    }else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
     SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id = " << m_id;
}
Fiber::~Fiber(){
    --s_fiber_count;
    if(m_stack){
        SYLAR_ASSERT(m_state==TERM||m_state==EXCEPT|| m_state == INIT);
        StackAllocator::Dealloc(m_stack,m_stacksize);
    }else{
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state==EXEC);
        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id << " total=" << s_fiber_count;
}


//重置线程执行函数,优化内存的使用
void Fiber::reset(std::function<void()>cb){
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state==EXCEPT||m_state==INIT||m_state==TERM);
    m_cb=cb;
    if(getcontext(&m_ctx)){
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link=nullptr;
    m_ctx.uc_stack.ss_size=m_stacksize;
    m_ctx.uc_stack.ss_sp=m_stack;
    makecontext(&m_ctx,&Fiber::MainFunc,0);
    m_state=INIT;
}
//协程切换
void Fiber::swapIn(){
    SetThis(this);
    SYLAR_ASSERT(m_state!=EXEC);
    m_state=EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx,&m_ctx)){
        SYLAR_ASSERT2(false,"swapcontext");
    }
}
void Fiber::swapOut(){
    SetThis(Scheduler::GetMainFiber());//获取this
    if(swapcontext(&m_ctx,&Scheduler::GetMainFiber()->m_ctx)){
        SYLAR_ASSERT2(false,"swapcontext");
    }

}

//执行当前线程的主协程
void Fiber::call(){
    SetThis(this);//更新当前协程的指针，确保在切换之后能正确管理和追踪当前的协程状态
    m_state=EXEC;
    if(swapcontext(&t_thread_fiber->m_ctx, &m_ctx)){
        SYLAR_ASSERT2(false,"swapcontext");
    }
}
//返回线程的主协程
void Fiber::back(){
    SetThis(t_thread_fiber.get());
    if(swapcontext(&m_ctx,&t_thread_fiber->m_ctx)){
        SYLAR_ASSERT2(false, "swapcontext");
    }
}


void Fiber::SetThis(Fiber* f){
    t_fiber=f;
}
Fiber::ptr Fiber::GetThis(){
    if(t_fiber){
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    SYLAR_ASSERT(t_fiber==main_fiber.get());
    t_thread_fiber=main_fiber;
    return t_thread_fiber->shared_from_this();
}
// 协程切换到后台，并且设置为Hold状态
void Fiber::YieldToHold(){
    Fiber::ptr cur=GetThis();
    //cur->m_state=HOLD;
    cur->swapOut();
}
void Fiber::YieldToReady(){
    Fiber::ptr cur=GetThis();
    cur->m_state=READY;
    cur->swapOut();

}
uint64_t Fiber::TotalFibers(){
    return s_fiber_count;
}
//执行函数，并返回主线程
void Fiber::MainFunc(){
    Fiber::ptr cur=GetThis();
    SYLAR_ASSERT(cur);
    try{
        cur->m_cb();//执行函数
        cur->m_cb=nullptr;//执行完置为空
        cur->m_state=TERM;//设置终止
    }catch(std::exception & ex){
        cur->m_state=EXCEPT;//抛出异常设置
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << " fiber_id=" << cur->getId() << std::endl
                                  << sylar::BacktraceToString();
    }catch (...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
                                  << " fiber_id=" << cur->getId() << std::endl
                                  << sylar::BacktraceToString();
    }
    
    auto raw_ptr=cur.get();
    cur.reset();
    raw_ptr->swapOut();
    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb    = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << " fiber_id=" << cur->getId() << std::endl
                                  << sylar::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except"
                                  << " fiber_id=" << cur->getId() << std::endl
                                  << sylar::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

uint64_t Fiber::GetFiberId(){
    if(t_fiber){
        return t_fiber->getId();
    }
    return 0;
}

}