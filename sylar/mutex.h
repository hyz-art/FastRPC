#ifndef __SYLAR_MUTEX_H__
#define __SYLAR_MUTEX_H__
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include "noncopyable.h"
namespace sylar{
class Semaphore:Noncopyable{
public:
    //根据信号量大小写构造函数
    Semaphore(uint32_t count=0);
    ~Semaphore();
    void wait();//获取信号量
    void notify();//释放信号量
private:
    sem_t m_semaphore;//信号量
};

//类，构造函数加锁，析构解锁
template<class T>
struct ScopedLockImpl{
public:
    ScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.lock();
        m_locked=true;
    }
    ~ScopedLockImpl(){
        unlock();
    }
    void lock(){
        if(m_locked){
            m_mutex.lock();
            m_locked=true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked=false;
        }
    }

private:
    T& m_mutex; //锁对象
    bool m_locked; //是否加锁

};

//局部读锁
template<class T>
struct ReadScopedLockImpl{
public:
    ReadScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.rdlock();
        m_locked=true;
    }
    ~ReadScopedLockImpl(){
        unlock();
    }
    void lock(){
        if(m_locked){
            m_mutex.rdlock();
            m_locked=true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked=false;
        }
    }

private:
    T& m_mutex; //锁对象
    bool m_locked; //是否加锁
};

// 局部写锁
template<class T>
struct WriteScopedLockImpl{
public:
    WriteScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.wrlock();
        m_locked=true;
    }
    ~WriteScopedLockImpl(){
        unlock();
    }
    void lock(){
        if(m_locked){
            m_mutex.wrlock();
            m_locked=true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked=false;
        }
    }

private:
    T& m_mutex; //锁对象
    bool m_locked; //是否加锁
};

/**
 * @brief 互斥量
 */
class Mutex : Noncopyable {
public:
    /// 局部锁
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex() { pthread_mutex_init(&m_mutex, nullptr); }

    ~Mutex() { pthread_mutex_destroy(&m_mutex); }

    void lock() { pthread_mutex_lock(&m_mutex); }

    void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};

/**
 * @brief 读写互斥量
 */
class RWMutex : Noncopyable{
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }
    ~RWMutex() { pthread_rwlock_destroy(&m_lock); }
    void rdlock() { pthread_rwlock_rdlock(&m_lock); }
    void wrlock() { pthread_rwlock_wrlock(&m_lock); }
    void unlock() { pthread_rwlock_unlock(&m_lock); }
private:
    pthread_rwlock_t m_lock;
};
}           
#endif