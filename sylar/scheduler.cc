#include "scheduler.h"
#include "log.h"

namespace sylar{

static sylar::Logger::ptr g_logger=SYLAR_LOG_NAME("system");
static thread_local Scheduler* t_scheduler=nullptr;
static thread_local Fiber* t_scheduler_fiber=nullptr;

Scheduler::Scheduler(size_t threads,bool use_caller,const std::string& name):m_name(name){
    SYLAR_ASSERT(threads>0);
    if(use_caller){
        sylar::Fiber::GetThis();
        //当前线程作为调度器管理，成为“主协程”线程，管理的线程数-1
        --threads;
        //确认当前线程没有协程在执行
        SYLAR_ASSERT(GetThis()==nullptr);
        t_scheduler=this;
        //主协程 是创建 协程调度器 的线程上第一个执行的协程，负责调度其他协程的执行
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this),0,true));
        sylar::Thread::SetName(m_name);

        t_scheduler_fiber=m_rootFiber.get();
        m_rootThread=sylar::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    }else{
        m_rootThread=-1;
    }
    m_threadCount=threads;
}

Scheduler::~Scheduler(){
    //如果是停止状态，指针为空
    SYLAR_ASSERT(m_stopping);
    if(GetThis()==this){
        t_scheduler=nullptr;
    }
}

Scheduler* Scheduler::GetThis(){
    return t_scheduler;
}
//当前协程调度器的调度协程
Fiber* Scheduler::GetMainFiber(){
    return t_scheduler_fiber;
}

void Scheduler::start(){
    MutexType::Lock lock(m_mutex);
    if(!m_stopping){ return; }//不是停止状态返回
    m_stopping=false;
    SYLAR_ASSERT(m_threads.empty());
    //重置大小，执行函数，存储threadId
    m_threads.resize(m_threadCount);
    for(size_t i=0;i<m_threadCount;i++){
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run,this),m_name+std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}
/**
 * @brief 停止调度器的运行，停止调度、等待线程结束、清理资源
 */
void Scheduler::stop(){
    m_autoStop=true;
    //判断停止条件，线程数归零、状态非运行、如果满足表示以及处于停止条件
    if(m_rootFiber&&m_threadCount==0&&(m_rootFiber->getState()==Fiber::TERM||m_rootFiber->getState()==Fiber::INIT)){
        SYLAR_LOG_INFO(g_logger)<<this<<" stopped";
        m_stopping=true;
        if(stopping()){
            return;
        }
    }

    //等于-1表示当前线程是调度器线程
    if(m_rootThread!=-1){ SYLAR_ASSERT(GetThis()==this); } 
    else { SYLAR_ASSERT(GetThis()!=this); }
    m_stopping=true;

    //向所有线程发送停止讯号
    for(size_t i=0;i<m_threadCount;i++){
        tickle();
    }
    //向主协程发送停止信号
    if(m_rootFiber){
        tickle();
    }
    //如果没有停止，调用CALL执行
    if(m_rootFiber){
        if(!stopping())
            m_rootFiber->call();//不唤醒，就会在老的协程上切换
    }
    //清理线程资源、等待完成
    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }
    for(auto& i:thrs){
        i->join();
    }

}

void Scheduler::setThis(){
    t_scheduler=this;
}
void Scheduler::run(){
    SYLAR_LOG_DEBUG(g_logger) << m_name << " run";
    //设置当前环境
    //set_hook_enable(true);
    setThis();
    //如果不是调度主协程，获取对应主协程
    if(sylar::GetThreadId()!=m_rootThread){
        t_scheduler_fiber=Fiber::GetThis().get();
    }
    //准备两个协程：空闲协程备用执行，复用对象执行普通任务
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true){
        //每一轮需要重置状态
        ft.reset();
        bool tickle_me=false;//是否需要唤醒其他线程
        bool is_active=false;//是否调度了任务
        {
            MutexType::Lock lock(m_mutex);
            auto it=m_fibers.begin();
            while (it!=m_fibers.end())
            {   //设置了thread，且不是当前线程跳过
                if(it->thread!=-1&&it->thread!=sylar::GetThreadId()){
                    ++it;
                    tickle_me=true;//需要唤醒其他线程
                    continue;
                }
                SYLAR_ASSERT(it->fiber||it->cb);
                //正在运行的跳过
                if(it->fiber&&it->fiber->getState()==Fiber::EXEC){
                    ++it;
                    continue;
                }
                //找到合适的任务。赋值到ft，从m_fiber中移除，活跃线程计数加一
                ft=*it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active=true;
                break;
            }            
        }
        if(tickle_me){
            tickle();
        }
        //执行任务
        //没有结束或异常,切入运行
        if(ft.fiber&& (ft.fiber->getState()!=Fiber::TERM&&ft.fiber->getState()!=Fiber::EXCEPT)){
            ft.fiber->swapIn();
            --m_activeThreadCount;
            //执行完判断状态
            //为准备状态，协程主动让出执行权,READY→调度，TERM/EXCEPT→丢弃，其他→HOLD
            if(ft.fiber->getState()==Fiber::READY){
                schedule(ft.fiber);
            }else if(ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT){
                ft.fiber->setState(Fiber::HOLD);
            }
            ft.reset();
        }else if(ft.cb){
            //cb_fiber复用或新建一个Fiber
            if(cb_fiber){
                cb_fiber->reset(ft.cb);
            }else{
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            //READY → 重新调度；TERM / EXCEPT → 清空；HOLD → 保留。
            if(cb_fiber->getState()==Fiber::READY){
                schedule(cb_fiber);
            }else if (cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {  // if(cb_fiber->getState() != Fiber::TERM) {
                ft.fiber->setState(Fiber::HOLD);
                cb_fiber.reset();
            }
        }else {//没有可执行的任务
            if(is_active){
                --m_activeThreadCount;
                continue;
            }
            //idle 被终止了（如调用了stop()），调度器就 break 退出循环
            if (idle_fiber->getState() == Fiber::TERM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
}

void Scheduler::tickle(){
    SYLAR_LOG_INFO(g_logger)<<"tickle";
}
bool Scheduler::stopping(){
    return m_stopping&&m_autoStop&& m_fibers.empty()&&m_activeThreadCount==0;
}
//执行转入后台，状态改hold
void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle";
    while (!stopping()) {
        sylar::Fiber::YieldToHold();
    }
}
}