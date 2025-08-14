#include "iomanager.h"


#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

namespace sylar
{
static sylar::Logger::ptr g_logger=SYLAR_LOG_NAME("system");
enum EpollCtlOp{};

IOManager::FdContext::EventContext& IOManager::FdContext::getContext(IOManager::Event event){
    switch (event)
    {
        case IOManager::READ: return read;
        case IOManager::WRITE: return write;
        default: SYLAR_ASSERT2(false,"getContext");
            break;
    }
    throw std::invalid_argument("getContext invalid event");
}

void IOManager::FdContext::resetContext(EventContext& ctx){
    ctx.fiber.reset();//shared_ptr的重置
    ctx.cb=nullptr;
    ctx.scheduler=nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event){
    SYLAR_ASSERT(event&events);
    events=(Event)(events&~event);//更新事件
    EventContext& ctx=getContext(event);
    if(ctx.cb){
        ctx.scheduler->schedule(&ctx.cb);
    }else{
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler=nullptr;
    return;
}


IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
    :Scheduler(threads,use_caller,name){
    m_epfd=epoll_create(5000);
    SYLAR_ASSERT(m_epfd>0);

    int rt=pipe(m_tickleFds);
    SYLAR_ASSERT(!rt);

    epoll_event event;
    memset(&event,0,sizeof(epoll_event));
    event.data.fd=m_tickleFds[0];//读端
    event.events=EPOLLIN|EPOLLET;//事件类型

    rt=fcntl(m_tickleFds[0],F_SETFL,O_NONBLOCK);//非阻塞
    SYLAR_ASSERT(!rt);
    rt=epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickleFds[0],&event);//红黑树监听
    SYLAR_ASSERT(!rt);

    contextResize(32);//重置m_fdContexts
    start();
    
}
IOManager::~IOManager(){
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);
    for(size_t i=0;i<m_fdContexts.size();++i){
        if(m_fdContexts[i])
            delete m_fdContexts[i];
    }
}

void IOManager::contextResize(size_t size){
    m_fdContexts.resize(size);
    for(size_t i=0;i<m_fdContexts.size();i++){
        if(!m_fdContexts[i]){
            m_fdContexts[i]=new FdContext;
            m_fdContexts[i]->fd=i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb){
    //获取上下文
    FdContext* fd_ctx=nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()>fd){
        fd_ctx=m_fdContexts[fd];//描述符的上下文，包含相关事件信息、同步机制
        lock.unlock();
    }else{
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd*1.5);
        fd_ctx=m_fdContexts[fd];
    }
    //判断事件存在性
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(SYLAR_UNLIKELY(fd_ctx->events&event)){
        SYLAR_LOG_ERROR(g_logger)<<"addEvent assert fd="<<fd<<" event="<<(EPOLL_EVENTS)event
            <<" fd_ctx.event="<<(EPOLL_EVENTS)fd_ctx->events;
        SYLAR_ASSERT(!(fd_ctx->events&event));
    }
    //选择操作类型，构建epoll_event，进行ctl加入监听
    int op=fd_ctx->events?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events=EPOLLET| fd_ctx->events | event;
    epevent.data.ptr=fd_ctx;//传递结构体指针

    int rt=epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt){
        SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "<<op<<", "<<fd<<", "
        <<(EPOLL_EVENTS)epevent.events<<"): "<<rt<<" ("<<errno<<") ("<<strerror(errno)<<") "
        <<"fd_ctx->events="<<(EPOLL_EVENTS)fd_ctx->events;
        return -1;
    }

    //更新事件，获得对应EventContext
    ++m_pendingEventCount;
    fd_ctx->events=(Event)(fd_ctx->events|event);
    //单事件的上下文结构
    FdContext::EventContext &event_ctx=fd_ctx->getContext(event);
    SYLAR_ASSERT(!event_ctx.scheduler&&!event_ctx.fiber&&!event_ctx.cb);//三个值无效
    //绑定调度器
    event_ctx.scheduler=Scheduler::GetThis();
    if(cb){
        event_ctx.cb.swap(cb);
    }else{
        event_ctx.fiber=Fiber::GetThis();
        SYLAR_ASSERT2(event_ctx.fiber->getState()==Fiber::EXEC,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event event){
    RWMutexType::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd)
    {
        return false;
    }
    FdContext *fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if (SYLAR_UNLIKELY(!(fd_ctx->events & event))){
        return false;
    }
    //修改
    Event new_events = (Event)(fd_ctx->events&~event);
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;
    //更改内核态
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);//返回0成功
    if(rt){
        SYLAR_LOG_ERROR(g_logger)<<"epoll_ctl("<<m_epfd<<", "<<(EpollCtlOp)op<<", "<<fd<<", "
            <<(EPOLL_EVENTS)epevent.events<<"): "<<errno<<") ("<<strerror(errno)<<")";
        return false;
    }
    --m_pendingEventCount;
    fd_ctx->events=new_events;//修改用户态
    FdContext::EventContext& event_ctx=fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;    
}

bool IOManager::cancelEvent(int fd, Event event){
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()<=fd){
        return false;
    }
    FdContext* fd_ctx=m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(SYLAR_UNLIKELY(!(fd_ctx->events&event))){
        return false;
    }

    Event new_events=(Event)(fd_ctx->events&~event);
    int op=new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events=EPOLLET|new_events;
    epevent.data.ptr=fd_ctx;
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt){
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd << ", "
            << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    //激活事件，把事件协程回调主动取消监听
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd){
    RWMutexType::ReadLock lock(m_mutex);
    if((int)m_fdContexts.size()<=fd){
        return false;
    }
    FdContext* fd_ctx=m_fdContexts[fd];
    lock.unlock();

    RWMutexType::WriteLock lock2(m_mutex);
    if (!fd_ctx->events) {
        return false;
    }

    int op=EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events=0;
    epevent.data.ptr=fd_ctx;
    int rt=epoll_ctl(m_epfd,op,fd,&epevent);
    if(rt){
        SYLAR_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd << ", "
            << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    //分别处理事件
    if(fd_ctx->events&READ){
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if(fd_ctx->events&WRITE){
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }
    SYLAR_ASSERT(fd_ctx->events==0);//处理完成
    return true;

}

IOManager *IOManager::GetThis(){
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle(){
    if(hasIdleThreads()){ 
        return; 
    }
    int rt=write(m_tickleFds[1],"T",1);
    SYLAR_ASSERT(rt==1);
}

bool IOManager::stopping(uint64_t &timeout){
    return Scheduler::stopping()&& m_pendingEventCount==0;
}

bool IOManager::stopping(){
    uint64_t timeout=0;
    return stopping(timeout);
}

void IOManager::idle(){
    SYLAR_LOG_INFO(g_logger)<<"idle";
    const uint64_t MAX_EVENTS=256;
    epoll_event* events=new epoll_event[MAX_EVENTS]();
    std::shared_ptr<epoll_event> shared_events(events,[](epoll_event* ptr){delete[] ptr;});

    while (true)
    {
        //超时时间
        uint64_t next_timeout = 0;
        if (SYLAR_UNLIKELY(stopping(next_timeout))) {
            SYLAR_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
            break;
        }

        int rt=0;
        do{
                    static const int MAX_TIMEOUT=10;
            if(next_timeout!=~0ull){
                next_timeout=next_timeout>MAX_TIMEOUT? MAX_TIMEOUT:next_timeout;
            }else   next_timeout=MAX_TIMEOUT;

            rt=epoll_wait(m_epfd,events,MAX_EVENTS,(int)next_timeout);
            if(rt<0&&errno==EINTR){
            }else break;
        }while(true);

        //调度过期事件
        for(int i=0;i<rt;i++){
            epoll_event& event=events[i];
            if(event.data.fd==m_tickleFds[0]){
                uint8_t dummy[256];
                while(read(m_tickleFds[0],dummy,sizeof(dummy))>0);
                continue;
            }

            FdContext* fd_ctx=(FdContext*)event.data.ptr;
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if(fd_ctx->events&(EPOLLERR|EPOLLHUP)){
                event.events|=(EPOLLIN|EPOLLOUT)&fd_ctx->events;//如果包含在内，更新事件
            }
            int real_events=NONE;
            if(event.events & EPOLLIN){//有可读事件
                real_events|=READ;
            }
            if(event.events& EPOLLOUT){
                real_events|=WRITE;
            }
            if((fd_ctx->events&real_events)==NONE){
                continue;
            }

            // 计算剩余的事件，并准备修改 epoll 事件
            int left_events=(fd_ctx->events&~real_events);
            int op=left_events? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events=EPOLLET|left_events;//计算剩余的事件，并准备修改 epoll 事件

            int rt2=epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2){
                SYLAR_LOG_ERROR(g_logger)
                    << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd_ctx->fd << ", "
                    << (EPOLL_EVENTS)event.events << "):" << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            if(real_events&READ){
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if(real_events&WRITE){
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }            
        }

        //清除
        Fiber::ptr cur=Fiber::GetThis();
        auto raw_ptr=cur.get();
        cur.reset();
        raw_ptr->swapOut();
    }
    
    
}
};