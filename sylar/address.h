#ifndef __SYLAR_ADDRESS_H__
#define __SYLAR_ADDRESS_H__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace sylar{
    class IPAddress;
class Address{
public:
    typedef std::shared_ptr<Address> ptr; 
    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
    //通过host地址返回对应条件的所有Address
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host, 
        int family=AF_INET, int type=0, int protocol=0);

    //通过host地址返回对应条件的任意Address
    static Address::ptr LookupAny(const std::string& host, int family=AF_INET, int type=0, int protocol=0);
    //通过host地址返回对应条件的指定IPAddress
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host, 
        int family=AF_INET, int type=0, int protocol=0);

    //返回本机所有网卡的<网卡名, 地址, 子网掩码位数>,map获取键值对
    static bool GetInterfaceAddresses(std::multimap<std::string,std::pair<Address::ptr,uint32_t>>& result,
        int family=AF_INET);
    //获取指定网卡的地址和子网掩码位数,iface 网卡名称
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr,uint32_t>>& result,
        const std::string& iface,int family=AF_INET);

    virtual ~Address(){}
    //获取数据
    int getFamily() const;
    virtual const sockaddr* getAddr() const=0;
    virtual sockaddr* getAddr()=0;
    virtual socklen_t getAddrLen() const=0;
    //可读性输出
    virtual std::ostream& insert(std::ostream& os) const=0;
    std::string toString() const;
    //比较操作符
    bool operator<=(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;    
};

class IPAddress : public Address{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    static IPAddress::ptr Create(const char* addr, uint16_t port=0);
    //获取广播地址、网段、掩码地址，传递掩码位数，成功都是返回地址指针
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len)=0;
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len)=0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len)=0;
    virtual uint32_t getPort() const=0;
    virtual void setPort(uint16_t v)=0;
};
class IPv4Address:public IPAddress{
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    static IPv4Address::ptr Create(const char* addr, uint16_t port=0);
    //sockaddr_in 构造
    IPv4Address(const sockaddr_in& address);
    //二进制地址构造
    IPv4Address(uint32_t address= INADDR_ANY, uint16_t port=0);

    //获取地址信息
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    //获取广播地址、网段、掩码地址，传递掩码位数，成功都是返回地址指针
    IPAddress::ptr broadcastAddress(uint32_t prefix_len);
    IPAddress::ptr networdAddress(uint32_t prefix_len);
    IPAddress::ptr subnetMask(uint32_t prefix_len);
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address:public IPAddress{
public:
    typedef std::shared_ptr<IPv6Address> ptr;
    static IPv6Address::ptr Create(const char* addr, uint16_t port=0);
    //无参构造
    IPv6Address();
    //sockaddr_in 构造
    IPv6Address(const sockaddr_in6& address);
    //二进制地址构造,与4结构不同
    IPv6Address(const uint8_t address[16], uint16_t port=0);

    //获取地址信息
    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    //获取广播地址、网段、掩码地址，传递掩码位数，成功都是返回地址指针
    IPAddress::ptr broadcastAddress(uint32_t prefix_len);
    IPAddress::ptr networdAddress(uint32_t prefix_len);
    IPAddress::ptr subnetMask(uint32_t prefix_len);
    uint32_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address{
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string& path);
    //继承
    const sockaddr* getAddr() const override;
    sockaddr*       getAddr() override;
    socklen_t       getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
    void setAddrLen(uint32_t v);
    std::string getPath() const;
private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    const sockaddr* getAddr() const override;
    sockaddr*       getAddr() override;
    socklen_t       getAddrLen() const override;
    std::ostream&   insert(std::ostream& os) const override;

private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);
}
#endif