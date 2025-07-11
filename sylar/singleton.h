#ifndef __SYLAR_SINGLETION_H__
#define __SYLAR_SINGLETION_H__

#include <memory>
namespace sylar{
template <class T,class X=void,int N=0>
class Singleton{
public:
    //返回单例裸指针
    static T* GetInstance(){
        static T v;
        return &v;
    }
};
template <class T, class X = void, int N = 0>
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstance(){
        static std::shared_ptr<T> v(new T);
        return v;
    }
};
}
#endif