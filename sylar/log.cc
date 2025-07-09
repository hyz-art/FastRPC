#include "log.h"
#include <sstream>
#include <iostream>

namespace sylar
{
    const char *LogLevel::ToString(LogLevel::Level Level)
    {
        switch (Level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;
            XX(DEBUG)
            XX(INFO)
            XX(WARN)
            XX(ERROR)
            XX(FATAL)
#undef XX
        default:
            return "UNKNOW";
        }
        return "UNKNOEW";
    }

    LogLevel::Level LogLevel::FromString(const std::string &str)
    {
#define XX(level, v)            \
    if (str == #v)              \
    {                           \
        return LogLevel::level; \
    }
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKNOW;
#undef XX
    }

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    class FilberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadNameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getThreadName();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof buf, m_format.c_str(), &tm);
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std::string& str) : m_string(str) {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        TabFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << "\t";
        }

    private:
        std::string m_string;
    };

    void Logger::log(LogLevel::Level level, const LogEvent::ptr &event)
    {
        if (level < m_level)
        {
            return; // 如果日志级别低于当前设置的级别，则不处理
        }
        else if (!event)
        {
            return; // 如果事件为空，则不处理
        }
        // 如果事件内容不为空，则处理日志
        for (auto &appender : m_appenders)
        {
            appender->log(level, event);
        }
    }

    // 日志事件
    void Logger::debug(const LogEvent::ptr event)
    {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(const LogEvent::ptr event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(const LogEvent::ptr event)
    {
        log(LogLevel::WARN, event);
    }
    void Logger::error(const LogEvent::ptr event)
    {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(const LogEvent::ptr event)
    {
        log(LogLevel::FATAL, event);
    }

    // 编辑日志目标（输出位置）
    void Logger::addAppender(LogAppender::ptr appender)
    {
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }
    void Logger::clearAppenders()
    {
        m_appenders.clear();
    }

    // 设置日志格式
    void Logger::setFormatter(LogFormatter::ptr formatter)
    {
    }
    LogFormatter::ptr Logger::getFormatter() const
    {
    }

    void StdoutLogAppender::log(LogLevel::Level level, const LogEvent::ptr event)
    {
        if (level >= m_level && event)
        {
            // 输出日志到标准输出
            m_formatter->format(event);
        }
    }

    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
        reopen(); // 尝试打开文件
    }
    bool FileLogAppender::reopen()
    {
        if (!m_filename.empty())
            m_filestream.close();
        m_filestream.open(m_filename, std::ios::app);
        return !m_filestream;
    }
    void FileLogAppender::log(LogLevel::Level level, const LogEvent::ptr event)
    {
        if (level >= m_level && event)
        {
            m_filestream << m_formatter->format(event);
        }
    }
    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern)
    {
        init(); // 初始化格式化项
    }
    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
            // i.format(ss,logger,level,event);
            return ss.str();
    }
    std::ostream &LogFormatter::format(std::ostream &ofs, std::shared_ptr<Logger> Logger, LogLevel::Level level, LogEvent::ptr event)
    {
        for (auto &i : m_items)
            // i.format(ofs,logger,level,event);
            return ofs;
    }
    void LogFormatter::init()
    {
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr += '%'; // 处理%%，输出%
                    continue;
                }
            }
            size_t n = i + 1;
            int fmt_status = 0;
            int fmt_begin = 0;
            std::string fmt;
            std::string str;
            while (n < m_pattern.size())
            {
                if (!fmt_status && m_pattern[n] != '{}' && m_pattern[n] != '}' && (!isalpha(m_pattern[n])))
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; // 开始格式化
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt_status = 0; // 格式化结束
                        fmt = m_pattern.substr(fmt_begin, n - fmt_begin + 1);
                        break;
                    }
                }
                ++n;
                if (n >= m_pattern.size() && str.empty())
                {
                    str = m_pattern.substr(i + 1);
                }
            }
            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, fmt, 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 0));
                i = n - 1; // 更新i到n-1位置
            }
            else if (fmt_status == 1)
            {
                // 如果格式化没有结束，说明有错误
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;

                m_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
                return;
            }
        }
        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
    }
}