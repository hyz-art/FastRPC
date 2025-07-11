#include <iostream>
#include "../sylar/log.h"
#include "../sylar/util.h"
int main(int argc, char* argv[]) {
    sylar::Logger::ptr logger(new sylar::Logger());
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender()));

    //增加文件输出平台
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);
    logger->addAppender(file_appender);

    //sylar::LogEvent::ptr event(new sylar::LogEvent(logger,sylar::LogLevel::DEBUG, __FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0),"main_thread"));

    //logger->log(sylar::LogLevel::DEBUG, event);

    std::cout << "hello sylar log" << std::endl;
    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";
    SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");

    auto l=sylar::LoggerMgr::GetInstance()->getLogger("xx");
    l->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender()));
    SYLAR_LOG_INFO(l)<<"xxx";
    return 0;
}