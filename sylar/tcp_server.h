#ifndef __SYLAR_TCP_SERVER_H__
#define __SYLAR_TCP_SERVER_H__
#include <memory>
#include "iomanager.h"
#include "noncopyable.h"
#include "socket.h"
namespace sylar {

class TcpServer : public std::enable_shared_from_this<TcpServer> , Noncopyable{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(IOManager* worker = IOManager::GetThis(), IOManager* io_worker = IOManager::GetThis(), IOManager* accept_worker = IOManager::GetThis());
    virtual ~TcpServer();
    //单地址绑定
    virtual bool bind(Address::ptr addr, bool ssl =true);
    //系列地址处理,可连接和连接失败的地址
    virtual bool bind(std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails, bool ssl = true);
    virtual bool start();
    virtual void stop();
    std::vector<Socket::ptr> getSocks() const { return m_socks; }
    uint64_t getRecvTimeout() const { return m_recvTimeout; }
    std::string getName() const {return m_name; }
    bool isStop() const { return m_isStop; }
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
    virtual void setName(const std::string& v) { m_name = v; }
    //conf配置

    virtual std::string toString(const std::string& prefix = "");

protected:
    virtual void startAccept(Socket::ptr sock);
    virtual void handleClient(Socket::ptr sock);

private:
    //对象 
    std::vector<Socket::ptr> m_socks;  
    //管理调度
    IOManager* m_worker;
    IOManager* m_ioworker;
    IOManager* m_acceptworker;
    //名称·类型·接收时间
    uint64_t m_recvTimeout;
    std::string m_name;
    std::string m_type="tcp";
    bool m_isStop;

    
};
}

#endif