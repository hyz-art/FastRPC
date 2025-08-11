#include "sylar/sylar.h"
static sylar::Logger::ptr g_logger=SYLAR_LOG_ROOT();

void test_fiber(){
    SYLAR_LOG_INFO(g_logger)<<"test in fiber";
    sleep(1);
}
int main(int argc,char** argv){
    sylar::Scheduler sc;
    //(3,false,"test")
    sc.start();
    sc.schedule(&test_fiber);
    sc.stop();
    return 0;
}