#include "tcp_server.h"
#include "config.h"

namespace sylar {
static sylar::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = Config::Lookup("tcp_server.read_timeout", (uint64_t)(6*1000*2), "tcp server read timeout");
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

TcpServer::TcpServer(IOManager* worker, IOManager* io_worker, IOManager* accept_worker) 
    :m_worker(worker), m_ioworker(io_worker), m_acceptworker(accept_worker), 
     m_recvTimeout(g_tcp_server_read_timeout->getValue()), m_name("syalr/1.0.0"), m_isStop(true) {}

TcpServer::~TcpServer() {
    //释放指针
    for(auto& i:m_socks) {
        i->close();
    }
    m_socks.clear();
}

//单地址绑定
bool TcpServer::bind(Address::ptr addr, bool ssl) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails, ssl);
}
//系列地址处理,可连接和连接失败的地址
bool TcpServer::bind(std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails, bool ssl) {
    for(auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if(!sock->bind(addr)) {
            SYLAR_LOG_ERROR(g_logger) << "bind fail errno=" << errno << " errstr=" << strerror(errno)
                << " add=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if(!sock->listen()) {
            SYLAR_LOG_ERROR(g_logger) << "listen fail errno=" << errno << " errstr=" <<strerror(errno)
                << " add=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    //如果有fails，释放全部
    if(!fails.empty()){
        m_socks.clear();
        return false;
    }
    //若全部连接成功
    for(auto& i : m_socks) {
        SYLAR_LOG_INFO(g_logger)<< "type=" <<m_type<< " name=" << m_name << " ssl="<<
            " server bind success: " << *i;
    }
    return true;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while(!m_isStop) {
        Socket::ptr client = sock->accept();
        //设计超时、提交到工作线程
        if(client){
            client->setRecvTimeout(m_recvTimeout);
            m_ioworker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
        } else {
            SYLAR_LOG_ERROR(g_logger) << " accept errno="<<errno<<strerror(errno);
        }
    }
}

bool TcpServer::start() {
    //检查是否已经开始
    if (!m_isStop) {
        return true;
    }
    m_isStop = false;
    for(auto& sock : m_socks) {
        m_acceptworker->schedule(std::bind(&TcpServer::startAccept,shared_from_this(),sock));
    }
    return true;
}
void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptworker->schedule([this, self](){
        for (auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}

void TcpServer::handleClient(Socket::ptr client) {
    SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
}

//conf配置

std::string TcpServer::toString(const std::string& prefix) {
    std::stringstream ss;
    ss << prefix << "[type="<<m_type<<" name="<<m_name<<" ssl="
        <<" work="<<(m_worker ? m_worker->getName():"")
        <<" accept="<<(m_acceptworker ? m_acceptworker->getName():"")
        <<" recv_timeout="<<m_recvTimeout<<"]"<<std::endl;
    std::string pfx = prefix.empty() ? " ":prefix;
    for (auto& i : m_socks) {
       ss << pfx << pfx << *i << std::endl;
    }
    return ss.str();
}
}