#include "sylar/sylar.h"
#include <vector>
#include <unistd.h>
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
int count = 0;
sylar::RWMutex s_mutex;
void fun1(){
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
                             << " this.name: " << sylar::Thread::GetThis()->getName() 
                             << " id: " << sylar::GetThreadId()
                             << " this.id: " << sylar::Thread::GetThis()->getId();
    for (int i = 0; i < 10000; ++i) {
        sylar::RWMutex::WriteLock lock(s_mutex);
        //sylar::Mutex::Lock lock(s_mutex);
        ++count;
    }
}
void fun2() {
    while (true) {
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while (true) {
        SYLAR_LOG_INFO(g_logger) << "========================================";
    }
}

int main(int argc,char** argv){
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    std::vector<sylar::Thread::ptr> threads;
    for(int i=0;i<5;i++){
        sylar::Thread::ptr thr(new sylar::Thread( &fun1,"name_"+std::to_string(i)));
        threads.push_back(thr);
    }
    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i]->join();
    }
    sleep(10);
    SYLAR_LOG_INFO(g_logger) << "thread test end";
    SYLAR_LOG_INFO(g_logger) << "count=" << count;
}
