#include "util.h"

namespace sylar{
    pid_t GetThreadId(){
        return syscall(SYS_gettid);
    }
    uint32_t GetFiberId(){
        //return sylar::GetFiberId();
        return 0;
    }
}