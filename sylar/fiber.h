#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <memory>
#include <ucontext.h>
#include <functional>
#include "thread.h"

namespace sylar{
class Fiber : public std::enable_shared_from_this<Fiber>{
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State{
        INIT,
        HOLD,//暂停
        EXEC,//执行中
        TERM,//结束
        READY,//可执行
        EXCEPT//异常
    };
public:
//无参，每个线程第一个协程
    Fiber();
public:
//是否在主线程
    Fiber(std::function<void()>cb, size_t stacksize=0, bool use_caller=false);
    ~Fiber();
    //重置线程执行函数
    void reset(std::function<void()>cb);
    //协程切换
    void swapIn();
    void swapOut();
    
    static void SetThis(Fiber* f);
    static Fiber::ptr GetThis();
    static void YieldToHold();
    static void YieldToReady();
    static uint64_t TotalFibers();
    //执行函数，并返回主线程
    static void MainFunc();
    static uint64_t GetFiberId();
    uint64_t getId() const { return m_id; }
    State getState() const { return m_state; }
private:
    uint64_t m_id=0;
    uint32_t m_stacksize=0;
    State m_state=INIT;
    ucontext_t m_ctx;//协程上下文
    void* m_stack=nullptr;//协程栈
    std::function<void()> m_cb;


};


}
#endif