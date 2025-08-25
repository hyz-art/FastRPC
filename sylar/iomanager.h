#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__
#include "scheduler.h"
#include "timer.h"
namespace sylar{
class IOManager :public Scheduler,public TimerManager{
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<IOManager> ptr;
    enum Event{
        NONE=0x0,
        READ=0x1,
        WRITE=0x4
    };
private:
    struct  FdContext {
        typedef Mutex MutexType;
        struct EventContext{
            Scheduler* scheduler=nullptr;//事件执行的调度
            Fiber::ptr fiber; //执行的协程
            std::function<void()> cb; //事件 
        };
        //获取事件上下文
        EventContext& getContext(Event event);
        //重置上下文
        void resetContext(EventContext& ctx);
        //触发事件
        void triggerEvent(Event event);

        EventContext read;//读事件上下文
        EventContext write;//写事件
        int fd=0;
        Event events=NONE;
        MutexType mutex;
    };
    
public:
    IOManager(size_t threads=1,bool use_caller=true,const std::string& name="");
    ~IOManager();
    //void contextResize(size_t size);
    int addEvent(int fd,Event event, std::function<void()> cb=nullptr);
    bool delEvent(int fd,Event event);
    bool cancelEvent(int fd, Event event);
    bool cancelAll(int fd);

    static IOManager* GetThis();

protected:
    void tickle()override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;
    void contextResize(size_t size);
    //判断事件是否停止
    bool stopping(uint64_t& timeout);
private:
    int m_epfd=0;
    int m_tickleFds[2];
    std::atomic<size_t> m_pendingEventCount={0};
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;
};
}
#endif