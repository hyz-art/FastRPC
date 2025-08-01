#include "thread.h"
#include "log.h"
#include "util.h"
namespace sylar{
static thread_local Thread* t_thread=nullptr;
static thread_local std::string t_thread_name="UNNAMED";

static sylar::Logger::ptr g_logger=SYLAR_LOG_NAME("system");

Thread* Thread::GetThis(){
    return t_thread;
}

const std::string& Thread::GetName(){
    return t_thread_name;
}

void Thread::setName(const std::string &name){
    //线程存在的话，线程对应的名也赋值
    if(t_thread){
        t_thread->m_name=name;
    }
    t_thread_name=name;
}

Thread::Thread(std::function<void()> cb, const std::string &name):m_cb(cb),m_name(name){
    if(name.empty()){
        m_name="UNNAMED";
    }
    int rt=pthread_create(&m_thread,nullptr,&Thread::run,this);
    if(rt){
        SYLAR_LOG_ERROR(g_logger)<<"pthread_create thread fail,rt="<<rt<<" name="<<name;
        throw std::logic_error("pthread_create error");
    }
}

Thread::~Thread(){
    if(m_thread)
        pthread_detach(m_thread);
}

void Thread::join(){
    if(m_thread){
        int rt=pthread_join(m_thread,nullptr);
        if(rt){
            SYLAR_LOG_ERROR(g_logger)<<"pthread_join thread fail, name="<<m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread=0;
    }
}

void* Thread::run(void* arg){
    Thread* thread=(Thread*)arg;
    t_thread=thread;//当前线程对象
    t_thread_name  = thread->m_name;
    thread->m_id=sylar::GetThreadId();//获取syscall(SYS_gettid);
    //pthread_self 是pthread_t的类型标识符
    pthread_setname_np(pthread_self(), thread->m_name.substr(0,15).c_str());
    std::function<void()> cb;
    cb.swap(thread->m_cb);//转移对象所有权，避免不必要的复制

   thread->m_semaphore.notify(); // 通知线程已经创建完成 
   
    cb();
    return 0;
}
}