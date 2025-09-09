#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include <memory>
#include <vector>

#include "singleton.h"
#include "thread.h"

namespace sylar{
class FdCtx: public std::enable_shared_from_this<FdCtx>{
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx(int fd);
    ~FdCtx();

    bool isInit() const { return m_isInit; }
    bool isSocket() const { return m_isSocket; }
    int getFd() const { return m_fd; }

    bool isClosed() const { return m_isClosed; }
    void setClosed(bool val) { m_isClosed = val; }
    

    bool isSysNonblock() const { return m_sysNonblock; }
    void setSysNonblock(bool val) { m_sysNonblock = val; }
    bool getSysNonblock() const { return m_sysNonblock; }

    bool getUserNonblock() const { return m_userNonblock; }
    void setUserNonblock(bool val) { m_userNonblock = val; }

    uint64_t getTimeout(int type) ;
    void setTimeout(int type, uint64_t v);

private:
    //初始化
    bool init();
    bool m_isInit:1;
    bool m_isSocket:1;
    bool m_sysNonblock:1;// 是否hook非阻塞
    bool m_userNonblock:1;// 是否用户主动设置非阻塞
    bool m_isClosed:1;
    int m_fd;
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;

};


class FdManager{
public:
    typedef RWMutex RWMutexType;
    FdManager();
    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int fd);
private:
    RWMutexType m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

typedef Singleton<FdManager> FdMgr;
}

#endif