#ifndef __SYLAR_SOCKET_STREAM_H__
#define __SYLAR_SOCKET_STREAM_H__
#include "sylar/stream.h"
#include "sylar/socket.h"
namespace sylar{
class SocketStream : public Stream {
public:
    typedef std::shared_ptr<Stream> ptr;
    SocketStream(Socket::ptr sock, bool owner =  true);
    ~SocketStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;
    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;
    // 返回socket类
    Socket::ptr getSocket() const { return m_socket; }
    //是否连接
    bool isConnected() const;
    //获取地址
    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();
    std::string getRemoteAddressString();
    std::string getLocalAddressString();

private:
    Socket::ptr m_socket;
    bool m_owner;
};
}

#endif