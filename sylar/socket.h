#ifndef __SYLAR_SOCKET_H__
#define __SYLAR_SOCKET_H__
#include "address.h"
#include "noncopyable.h"
namespace sylar{
class Socket:public std::enable_shared_from_this<Socket>, Noncopyable{
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;
    //枚举类型
    enum Type{
        TCP=SOCK_STREAM,
        UDP=SOCK_DGRAM
    };
    enum Family{
        IPv4=AF_INET,
        IPv6=AF_INET6,
        UNIX=AF_UNIX
    };

    static Socket::ptr CreateTCP(sylar::Address::ptr address);
    static Socket::ptr CreateUDP(sylar::Address::ptr address);
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();
    
    Socket(int family, int type, int protocol=0);
    virtual ~Socket();
    int getSendTimeout();
    void setSendTimeout(int64_t v);
    int getRecvTimeout();
    void setRecvTimeout(int64_t v);

    bool getOption(int level, int option, void* result, socklen_t* len);
    template<class T>
    bool getOption(int level, int option, T& result){
        socklen_t length=sizeof(T);
        //输入参数时传入地址
        return getOption(level, option, &result, &length);
    }

    bool setOption(int level, int option, const void* result, socklen_t len);
    template<class T>
    bool setOption(int level, int option, const T& value){
        return setOption(level, option, &value, sizeof(T));
    }

    virtual Socket::ptr accept();
    virtual bool bind(const Address::ptr addr);
    virtual bool connect(const Address::ptr addr, uint64_t timeout_ms=-1);
    virtual bool reconnect(const Address::ptr addr, uint64_t timeout_ms=-1);
    virtual bool listen(int backlog=SOMAXCONN);
    virtual bool close();

    virtual int send(const void* buffers, size_t length, int flags=0);
    virtual int send(const iovec* buffers, size_t length, int flags = 0);
    virtual int sendTo(const void* buffers, size_t length, const Address::ptr to, int flags=0);
    virtual int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);
    virtual int recv(void* buffers, size_t length, int flags=0);
    virtual int recv(iovec* buffers, size_t length, int flags=0);
    virtual int recvFrom(void* buffers, size_t length, Address::ptr from, int flags=0);
    virtual int recvFrom(iovec* buffers, size_t length, Address::ptr from, int flags=0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();

    int getFamily(){ return m_family; }
    int getType(){ return m_type; }
    int getProtocol(){ return m_protocol; }
    bool isConnected(){ return m_isConnected; }
    int getSocket(){ return m_sock; }

    bool isValid()const;
    int getError();

    //输出信息
    virtual std::ostream& dump(std::ostream& os)const;
    virtual std::string toString()const;

    //取消事件
    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();

protected:
    void initSock();
    void newSock();
    virtual bool init(int sock);

protected:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;
    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;
};
} // namespace sylar
#endif