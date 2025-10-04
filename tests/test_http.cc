#include "sylar/http/http.h"
#include "sylar/log.h"
void test_request() {
    //创建request对象
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    //设置head、body
    req->setHeader("host", "www.syalr.top");
    req->setBody("hello sylar");
    //输出
    req->dump(std::cout) << std::endl;
}
void test_response() {
    sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello sylar");
    rsp->setStatus((sylar::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}
int main(int argc, char** argv) {
    test_request();
    test_response();
    return 0;
}
