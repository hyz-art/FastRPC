#include "sylar/hook.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_sleep() {
    sylar::IOManager iom(1);
    iom.schedule([]() {
        sleep(2);
        SYLAR_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([]() {
        sleep(3);
        SYLAR_LOG_INFO(g_logger) << "sleep 3";
    });
    SYLAR_LOG_INFO(g_logger) << "test_sleep";
}

void test_sock(){
    int sock=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_port=htons(80);//转网络字节序
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,"110.242.69.21",&addr.sin_addr.s_addr);
    int rt=connect(sock,(const sockaddr*)&addr,sizeof(addr));
    if(rt){
        return;
    }
    SYLAR_LOG_INFO(g_logger)<<"connect success";
    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt=send(sock,data,sizeof(data),0);
    SYLAR_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;
    if(rt<=0){
        return;
    }
    std::string buff;
    buff.resize(4096);
    rt=recv(sock,&buff[0],sizeof(buff),0);
    buff.resize(rt);

    // SYLAR_LOG_INFO(g_logger) << buff;

    // ssize_t total_received = 0;
    // while ((rt = recv(sock, &buff[total_received], sizeof(buff) , 0)) > 0) {
    //     total_received += rt;
    //     if (total_received >= 4096) {
    //         break;
    //     }
    // }
    // if (rt == 0) {
    //     SYLAR_LOG_WARN(g_logger) << "Connection closed by server.";
    // } else if (rt < 0) {
    //     SYLAR_LOG_ERROR(g_logger) << "recv error: " << errno;
    // }
    SYLAR_LOG_INFO(g_logger) << buff;
}

int main(int argc, char** argv) {
    //test_sleep();
    sylar::IOManager iom;
    iom.schedule(test_sock);
    //test_sock();
    return 0;
}