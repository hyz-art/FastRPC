#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <memory>
#include <string>
#include <list>
#include <stdint.h>

namespace sylar
{
    class Logger;
    class LogEvent;
    // 日志级别
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOW = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

    private:
    };

    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();

    private:
        const char *m_file = nullptr;
        int32_t m_line = 0;
        uint32_t m_elapse = 0; // 程序启动到现在的毫秒数
        uint32_t m_threadId = 0;
        uint32_t m_filerId = 0;
        uint64_t m_time = 0;
        std::string m_content;
    };

    // 事件包装器
    class LogEventWrap
    {
    public:
    private:
    };

    // 日志格式器
    class LogFormatter
    {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string format(LogEvent::ptr event);
    private:
        std::string m_pattern; // 日志格式化模式
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        void log(LogLevel::Level Level, const LogEvent::ptr event);
        virtual ~LogAppender(){}
    private:
        LogLevel::Level m_level; // 日志级别
        LogFormatter::ptr m_formatter; // 日志格式器
    };

    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string &name = "root");
        // 设置日志级别
        void log(LogLevel::Level level, const LogEvent::ptr &event);

        // 写入不同级别日志
        void debug(const LogEvent::ptr event);
        void info(const LogEvent::ptr event);
        void warn(const LogEvent::ptr event);
        void error(const LogEvent::ptr event);
        void fatal(const LogEvent::ptr event);

        // 编辑日志目标（输出位置）
        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();

        // 设置日志格式
        void setFormatter(LogFormatter::ptr formatter);
        LogFormatter::ptr getFormatter() const;

        // 获取日志器信息        
        const std::string &getName() const { return m_name; }
        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level level) { m_level = level; }
        
        // 日志器配置转换为YAML格式
        std::string toYamlString() const;

    private:
        std::string m_name;
        LogLevel::Level m_level;
        std::list<LogAppender::ptr> m_appenders; // 日志输出地列表 
        LogFormatter::ptr m_formatter;
    };

    

    class StdoutLogAppender : public LogAppender
    {
    public:
        StdoutLogAppender();

    private:
    };

    class FileLogAppender : public LogAppender
    {
    public:
        FileLogAppender(const std::string &filename);

    private:
    };

    // 日志器管理类
    class LogManager
    {
    };

}

#endif