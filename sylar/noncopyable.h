#ifndef __SYLAR_NONCOPYABLE_H__
#define __SYLAR_NONCOPYABLE_H__

namespace sylar{
class Noncopyable{
public:
    Noncopyable()=default;
    ~Noncopyable()=default;
    Noncopyable(const Noncopyable&)=delete;//拷贝
    Noncopyable& operator=(const Noncopyable&)=delete;//赋值

};
}
#endif