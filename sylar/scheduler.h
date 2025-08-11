#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__
#include "log.h"
#include "macro.h"
#include "thread.h"
#include "fiber.h"
#include "mutex.h"
namespace sylar{
class Scheduler {
public:
typedef std::shared_ptr<Scheduler> ptr;
typedef Mutex MutexType;
    Scheduler(size_t threads=1,bool use_caller=true,const std::string& name="");
    virtual ~Scheduler();
    const std::string& getName() const{ return m_name;}
    static Scheduler* GetThis();
    //当前协程调度器的调度协程
    static Fiber* GetMainFiber();
    void start();
    void stop();

    template <class FiberOrcb>
    void schedule(FiberOrcb fc,int thread=-1){
        bool need_tickle=false;
        MutexType::Lock lock(m_mutex);
        need_tickle=scheduleNoLock(fc,thread);
        if(need_tickle){
            tickle();
        }
    }

    template<class InputIterator>
    void schedule(InputIterator begin,InputIterator end){
        bool need_tickle=false;
        MutexType::Lock lock(m_mutex);
        while(end!=begin){
            need_tickle=scheduleNoLock(&*begin,-1) || need_tickle;
            ++begin;
        }
        if(need_tickle){
            tickle();
        }
    }

    void switchTo(int thread=-1);
    std::ostream& dump(std::ostream& os);
protected:
    //通知调度器有任务了
    virtual void tickle();
    //协程调度函数
    void run();
    //是否可以停止
    virtual bool stopping();
    //没有任务时，停止调度，执行idle协程
    virtual void idle();
    //设置当前协程
    void setThis();
    //是否有空闲线程
    bool hasIdleThreads() { return m_idleThreadCount>0; }
private:
    template<class FiberOrcb>
    bool scheduleNoLock(FiberOrcb fc, int thread){
        bool need_tickle=m_fibers.empty();//返回加入新任务之前队列的状态
        FiberAndThread ft(fc,thread);
        if(ft.fiber||ft.cb){
            m_fibers.push_back(ft);
        }
        return need_tickle;//队列为空的时候才需要唤醒调度
    }
    struct FiberAndThread{
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f,int thr):fiber(f),thread(thr){}
        FiberAndThread(Fiber::ptr* f,int thr):thread(thr){
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f,int thr):cb(f),thread(thr){}
        FiberAndThread(std::function<void()> *f,int thr):thread(thr){
            cb.swap(*f);
        }

        FiberAndThread():thread(-1){}
        
        /**
         * @brief 重置数据
         */
        void reset(){
            cb=nullptr;
            fiber=nullptr;
            thread=-1;
        }
    };

private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;//线程池
    std::list<FiberAndThread> m_fibers;//待执行的协程队列
    Fiber::ptr m_rootFiber;//调度协程
    std::string m_name;//调度器名称
protected:
    //协程下的线程id数
    std::vector<int> m_threadIds;
    size_t m_threadCount=0;
    // 工作线程数
    std::atomic<size_t> m_activeThreadCount={0};
    // 空闲线程数
    std::atomic<size_t> m_idleThreadCount={0};
    bool m_stopping=true;
    bool m_autoStop=false;
    // 主线程id
    int m_rootThread=0;
};


class SchedulerSwitcher:public Noncopyable{
public:
    SchedulerSwitcher(Scheduler* target=nullptr);
    ~SchedulerSwitcher();
private:
    Scheduler *m_caller;
};

}

#endif