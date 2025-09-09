#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__

#include <thread>
#include <memory>
#include <functional>
#include <pthread.h>
#include "mutex.h"
namespace sylar{
class Thread : public Noncopyable{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string &name);
    ~Thread();
    //等待线程完成
    void join();
    //获取当前线程指针
    static Thread* GetThis();
    // 获取当前线程名
    static const std::string& GetName();
    // 设置线程名
    static void SetName(const std::string &name);
    // 获取线程id
    pid_t getId() const { return m_id;}
    // 获取线程名
    const std::string& getName()const { return m_name;}
private:
    static void* run(void* arg);
    pid_t m_id=0;
    pthread_t m_thread=0;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};
}

#endif