#include "sylar/sylar.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>

sylar::Logger::ptr g_logger=SYLAR_LOG_ROOT();
int sock=0;
void test_fiber(){
    SYLAR_LOG_INFO(g_logger)<<"test_fiber";
    sock=socket(AF_INET,SOCK_STREAM,0);
    fcntl(sock,F_SETFL,O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port=htons(80);//转网络字节序
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,"110.242.69.21",&addr.sin_addr.s_addr);
    if(!connect(sock,(const sockaddr*)&addr,sizeof(addr))){
    }else if(errno==EINPROGRESS){
        sylar::IOManager::GetThis()->addEvent(sock,sylar::IOManager::READ,[](){
            SYLAR_LOG_INFO(g_logger)<<"read caller";
        });
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, []() {
            SYLAR_LOG_INFO(g_logger) << "write callback";
            // close(sock);
            sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::READ);
            close(sock);
        });
    }

}
void test1(){
    std::cout << "EPOLLIN=" << EPOLLIN << " EPOLLOUT=" << EPOLLOUT << std::endl;
    sylar::IOManager iom;
    iom.schedule(&test_fiber);
}
int main(int argc,char** argv){
    test1();
}