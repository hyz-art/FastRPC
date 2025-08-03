#include "sylar/sylar.h"
#include <assert.h>

sylar::Logger::ptr g_logger=SYLAR_LOG_ROOT();
void test_assert(){
    //int x=-1;
    //assert(x>0);测试条件是否成立
    SYLAR_LOG_INFO(g_logger)<<sylar::BacktraceToString(10);
}

int main(int argc,char** argv){
    test_assert();
    return 0;

}