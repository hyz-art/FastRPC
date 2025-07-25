#include "mutex.h"
#include <stdexcept>
namespace sylar{
//根据信号量大小写构造函数
Semaphore::Semaphore(uint32_t count){
    if(sem_init(&m_semaphore,0,count)){
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore(){
    sem_destroy(&m_semaphore);
}

//获取信号量
void Semaphore::wait(){
    if(sem_wait(&m_semaphore))
        throw std::logic_error("sem_wait error");
}

//释放信号量        
void Semaphore::notify(){
    if(sem_post(&m_semaphore)){
        throw std::logic_error("sem_post error");
    }
} 
}
