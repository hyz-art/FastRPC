#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <cxxabi.h>
namespace sylar{
    pid_t GetThreadId();
    uint32_t GetFiberId();
    template <class T>
    const char* TypeToName() {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }
}
#endif