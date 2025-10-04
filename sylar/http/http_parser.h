#ifndef __SYLAR_HTTP_PARSER_H__
#define __SYLAR_HTTP_PARSER_H__

#include "http.h"
#include "http11_parser.h"
#include "httpclient_parser.h"

namespace sylar{
namespace http{

class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser(/* args */);
    size_t execute(char* data, size_t len);
    bool isFinished();
    int hasError();
    HttpRequest::ptr getData() { return m_data; }
    const http_parser& getParser() const { return m_parser; }
    void setError(int v) { m_error = v; }
    uint64_t getContentLength();
    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    int m_error;// 错误码1000、1001、1002
};

class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();
    size_t execute(char* data, size_t len, bool chunck);
    bool isFinished();
    int hasError();
    HttpResponse::ptr getData() { return m_data; }
    const httpclient_parser& getParser() const { return m_parser; }
    void setError(int v) { m_error = v; }    
    uint64_t getContentLength();

    static uint64_t getHttpResponseBufferSize();
    static uint64_t getHttpResponseMaxBodySize();

private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_data;
    int m_error;
};


} // namespace http
} // namespace sylar

#endif