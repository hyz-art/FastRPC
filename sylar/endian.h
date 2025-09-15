#ifndef __SYLAR_ENDIAN_H__
#define __SYLAR_ENDIAN_H__

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENGIAN 2

#include <stdint.h>
#include <byteswap.h>
//#include <type_traits>
namespace sylar {
//8字节类型的字节序转化
template<class T>
typename std::enable_if<sizeof(T)==sizeof(uint64_t), T>::type byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}
//4字节类型的字节序转化
template<class T>
typename std::enable_if<sizeof(T)==sizeof(uint32_t), T>::type byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}
//2字节类型的字节序转化
template<class T>
typename std::enable_if<sizeof(T)==sizeof(uint16_t), T>::type byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER==BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENGIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER==SYLAR_BIG_ENGIAN
//如果在小端机器上，就 swap 一下，让数据变成大端（网络字节序）
template<class T>
T byteswapOnLittleEndian(T t){
    return t;
}
//当前为大端序机器，需要转换
template<class T>
T byteswapOnBigEndian(T t){
    return byteswap(t);
}

#else
template<class T>
T byteswapOnLittleEndian(T t){
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t){
    return t;
}
#endif

}
#endif