#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include <memory>
#include <string>
#include <list>
#include <stdint.h>
#include <fstream>
#include <vector>
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
        static const char* ToString(LogLevel::Level level); //将日志级别转成文本输出
        static LogLevel::Level FromString(const std::string& str); //将文本转换成日志级别
    };

    // 日志事件
    class LogEvent
    {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();
        const char* getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        uint64_t getTime() const { return m_time; }
        const std::string& getThreadName() const { return m_threadName; }
        std::string getContent() const { return m_ss.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }
        std::stringstream& getSS() { return m_ss; }
    private:
        const char *m_file = nullptr;
        int32_t m_line = 0;
        uint32_t m_elapse = 0; // 程序启动到现在的毫秒数
        uint32_t m_threadId = 0;
        uint32_t m_fiberId = 0;
        uint64_t m_time = 0;
        std::string m_content;
        /// 线程名称
        std::string m_threadName;
        /// 日志内容流
        std::stringstream m_ss;
        /// 日志器
        std::shared_ptr<Logger> m_logger;
        /// 日志等级
        LogLevel::Level m_level;
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
        LogFormatter(const std::string &pattern);
        std::string format(LogEvent::ptr event);
        std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level ,LogEvent::ptr event);
        std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> Logger, LogLevel::Level level, LogEvent::ptr event);        
        void init();
        bool isError() const { return m_error;}

    public:
        class FormatItem{
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)=0;
        };
    private:
        std::string m_pattern; // 日志格式化模式
        bool m_error = false; // 是否有错误
        std::vector<FormatItem::ptr> m_items;
    };

    // 日志输出地
    class LogAppender
    {
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual void log(LogLevel::Level Level, const LogEvent::ptr event);
        virtual ~LogAppender(){}
    protected:
        LogLevel::Level m_level = LogLevel::DEBUG; // 日志级别
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
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(LogLevel::Level level, const LogEvent::ptr event) override;
    private:
    };

    class FileLogAppender : public LogAppender
    {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        FileLogAppender(const std::string &filename);
        bool reopen();
        void log(LogLevel::Level level, const LogEvent::ptr event) override;
    private:
        std::string m_filename;
        std::ofstream m_filestream; // 文件流
        uint64_t m_lastTime = 0; // 上次重开文件的时间
    };

    // 日志器管理类
    class LogManager
    {
    };

}

#endif