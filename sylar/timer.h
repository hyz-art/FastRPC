#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include "thread.h"
#include <memory>
#include <set>
#include <vector>
namespace sylar{
class TimerManager;
class Timer:public std::enable_shared_from_this<Timer>{
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    //取消定时器
    bool cancel();
    //刷新设置时间
    bool refresh();
    //重置时间
    bool reset(uint64_t ms, bool from_now);

private:
    //构造通过TimerManager创建，私有
    /**
     * @brief 函数说明
     * @param[in] ms 定时器执行间隔事件
     * @param[in] cb 回调函数
     * @param[in] recurring 是否循环
     * @param[in] manager 定时器管理
     */
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);
    /*
    * @param[in] 执行的时间戳
    */
    Timer(uint64_t next);

private:
    bool m_recurring=false;//是否循环定时器   
    uint64_t m_ms=0;//执行周期
    uint64_t m_next=0;//精准的执行时间
    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;

    /**
     * @brief 定时器比较仿函数
     */
    struct Comparator{
        bool operator()(const Timer::ptr &lhs,const Timer::ptr &rhs)const;
    };
};

class TimerManager{
friend class Timer;
public:
    // typedef std::shared_ptr<TimerManager> ptr;
    typedef RWMutex RWMutexType;
    TimerManager();
    virtual ~TimerManager();
//管理方法
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring=false);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring=false);
    uint64_t getNextTime();
    //获取需要执行的回调函数列表
    void listExpiredCb(std::vector<std::function<void()>>& cbs);
    //判断有无定时器
    bool hasTimer();
protected:
    virtual void onTimerInsertedAtFront()=0;
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
private:
    //检查服务器时间是否置后
    bool detectClockRollover(uint64_t now_ms);
    //读写锁
    RWMutexType m_mutex;
    //排序的容器存定时器
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    bool m_tickled =false;
    uint64_t m_previouseTime = 0;
};
}
#endif