#include "log.h"
#include <sstream>
#include <iostream>
#include <map>
#include <functional>
#include <cstdarg>
#include "config.h"
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

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e) {}

    LogEventWrap::~LogEventWrap()
    {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }

    std::stringstream &LogEventWrap::getSS()
    {
        return m_event->getSS();
    }

    void LogAppender::setFormatter(LogFormatter::ptr val)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
        if (m_formatter){
            m_hasFormatter = true;
        }else{
            m_hasFormatter = false;
        }
    }

    LogFormatter::ptr LogAppender::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
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
        {//logger->getName()
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

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str) : m_string(str) {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
        {
            os << "\t";
        }

    private:
        std::string m_string;
    };
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, uint64_t time,
                       const std::string &thread_name)
        : m_file(file),
          m_line(line),
          m_elapse(elapse),
          m_threadId(thread_id),
          m_fiberId(fiber_id),
          m_time(time),
          m_threadName(thread_name),
          m_logger(logger),
          m_level(level) {}

    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    } // 时间，线程id，线程名称，文件名，日志级别p，日志器名称c，文件名f，行号l，消息体m，换行符n

    void Logger::log(LogLevel::Level level, const LogEvent::ptr &event)
    {
        if (level >= m_level){
            auto self = shared_from_this();
            MutexType::Lock lock(m_mutex);
            // 如果事件内容不为空，则处理日志
            if(!m_appenders.empty()){
                for (auto &appender : m_appenders) {
                    appender->log(self, level, event);
                }
            }else if(m_root){
                m_root->log(level,event);
            }
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
        MutexType::Lock lock(m_mutex);
        if (!appender->getFormatter())
        {
            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
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
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    // 设置日志格式
    void Logger::setFormatter(LogFormatter::ptr formatter)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter=formatter;
        for (auto& i : m_appenders) {
            //MutexType::Lock ll(i->m_mutex);
            if (!i->m_hasFormatter) {
                i->m_formatter = m_formatter;
            }
        }
    }
    void Logger::setFormatter(const std::string& val){
        MutexType::Lock lock(m_mutex);
        sylar::LogFormatter::ptr new_val(new sylar::LogFormatter(val));
        if(new_val->isError()){
           std::cout <<"Logger setFormatter name="<<m_name<<" value="<<val<<" invalid formatter"<< std::endl;
            return;
        } 
        //m_formatter=new_val;
        setFormatter(new_val);
    }

    //设置toYamlString转换
    std::string Logger::toYamlString(){
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"]=m_name;
        if(m_level!=LogLevel::DEBUG){
            node["level"]=LogLevel::ToString(m_level);
        }
        if (m_formatter) {
            node["formatter"] = m_formatter->getPattern();
        }
        for(auto i:m_appenders){
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
    LogFormatter::ptr Logger::getFormatter()
    {
        return m_formatter;
    }

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level && event)
        {
            MutexType::Lock lock(m_mutex);
            // 输出日志到标准输出
            m_formatter->format(std::cout, logger, level, event);
        }
    }
    std::string StdoutLogAppender::toYamlString(){
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"]="StdoutLogAppender";
        if(m_level!=LogLevel::UNKNOW){
            node["level"]=LogLevel::ToString(m_level);
        }
        if(m_hasFormatter&&m_formatter){
            node["formatter"]=m_formatter->getPattern();
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
    FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
    {
        reopen(); // 尝试打开文件
    }
    bool FileLogAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);
        if (!m_filename.empty())
            m_filestream.close();
        m_filestream.open(m_filename, std::ios::app);
        return !m_filestream;
    }
    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        if (level >= m_level) {
            uint64_t now = event->getTime();
            if (now >= (m_lastTime + 3)) {
                reopen();
                m_lastTime = now;
            }
            MutexType::Lock lock(m_mutex);
            // if(!(m_filestream << m_formatter->format(logger, level, event))) {
            if (!m_formatter->format(m_filestream, logger, level, event))
            {
                std::cout << "error" << std::endl;
            }
        }
    }
    std::string FileLogAppender::toYamlString(){
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"]="FileLogAppender";
        node["file"]=m_filename;
        if(m_level!=LogLevel::UNKNOW){
            node["level"]=LogLevel::ToString(m_level);
        }
        if(m_hasFormatter&&m_formatter){
            node["formatter"]=m_formatter->getPattern();
        }
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern)
    {
        init(); // 初始化格式化项
    }
    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
            i->format(ss, logger, level, event);
        return ss.str();
    }
    std::ostream &LogFormatter::format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
    {
        for (auto &i : m_items)
            i->format(ofs, logger, level, event);
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
                if (!fmt_status && m_pattern[n] != '{' && m_pattern[n] != '}' && (!isalpha(m_pattern[n])))
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
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        ++n;
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
                vec.push_back(std::make_tuple(str, fmt, 1));
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
        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
        #str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); } \
    }
            XX(m, MessageFormatItem),
            XX(p, LevelFormatItem),
            XX(r, ElapseFormatItem),
            XX(c, NameFormatItem),
            XX(t, ThreadIdFormatItem),
            XX(n, NewLineFormatItem),
            XX(d, DateTimeFormatItem),
            XX(f, FilenameFormatItem),
            XX(l, LineFormatItem),
            XX(T, TabFormatItem),
            XX(F, FilberIdFormatItem),
            XX(N, ThreadNameFormatItem),
#undef XX
        };
        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0)
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
        // std::cout << m_items.size() << std::endl;
    }

    struct LogAppenderDefine{
        int type=0;//1:Stdout,2:File
        LogLevel::Level level=LogLevel::UNKNOW;
        std::string formatter;
        std::string file;
        bool operator==(const LogAppenderDefine& oth)const{
            return type==oth.type&& level==oth.level &&formatter==oth.formatter&& file==oth.file;
        }
    };
    struct LogDefine{
        std::string name;
        LogLevel::Level level=LogLevel::UNKNOW;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;//日志输出
        bool operator==(const LogDefine& oth)const{
            return name==oth.name && level==oth.level && formatter==oth.formatter && appenders==oth.appenders;
        }
        bool operator<(const LogDefine& oth) const { return name < oth.name; }
        bool isValid() const { return !name.empty(); }
    };
    
    //定义日志类型的配置元素
    template<>
    class LexicalCast<std::string,LogDefine>{
    public:
        LogDefine operator()(const std::string &v){
            YAML::Node node=YAML::Load(v);
            LogDefine ld;
            if(!node["name"].IsDefined() || !node["name"].IsScalar()){
                std::cout<<"log config error: name is null or not scalar, value="<<v<<std::endl;
                throw std::logic_error("log config name is null");
            }
            ld.name=node["name"].as<std::string>();
            ld.level=LogLevel::FromString(node["level"].IsDefined()?node["level"].as<std::string>():"");
            if(node["formatter"].IsDefined()){
                ld.formatter=node["formatter"].as<std::string>();
            }
            if(node["appenders"].IsDefined()){
                for(size_t i=0;i<node["appenders"].size();++i){
                    auto a=node["appenders"][i];
                    if(!a["type"].IsDefined()){
                        std::cout<< "log config error: appender type is null, " << a << std::endl;
                        continue;
                    }
                    std::string type=node["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if(type=="FileLogAppender"){
                        lad.type=1;
                        if (!a["file"].IsDefined()) {
                            std::cout << "log config error: fileappender file is null, " << a << std::endl;
                            continue;
                        }
                        lad.file=a["file"].as<std::string>();
                        if (a["formatter"].IsDefined()) {
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    }else if(type=="StdoutLogAppender"){
                        lad.type=2;
                        if (a["formatter"].IsDefined()) {
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    }else {
                        std::cout << "log config error: appender type is invalid, " << a << std::endl;
                        continue;
                    }
                    ld.appenders.push_back(lad);
                }                
            }
            return ld;
        }
    };

    template <>
    class LexicalCast<LogDefine, std::string> {
    public:
        std::string operator()(const LogDefine& i) {
            YAML::Node n;
            n["name"] = i.name;
            if (i.level != LogLevel::UNKNOW) {
                n["level"] = LogLevel::ToString(i.level);
            }
            if (!i.formatter.empty()) {
                n["formatter"] = i.formatter;
            }
            // 输出目标
            for (auto& a : i.appenders) {
                YAML::Node na;
                if (a.type == 1) {
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                } else if (a.type == 2) {
                    na["type"] = "StdoutLogAppender";
                }
                if (a.level != LogLevel::UNKNOW) {
                    na["level"] = LogLevel::ToString(a.level);
                }

                if (!a.formatter.empty()) {
                    na["formatter"] = a.formatter;
                }
                // 添加到列表
                n["appenders"].push_back(na);
            }
            std::stringstream ss;
            ss << n;
            return ss.str();
        }
    };

    //专项处理
    template <>
    class LexicalCast<std::set<LogDefine>, std::string> {
    public:
        std::string operator()(const std::set<LogDefine>& logs) {
            YAML::Node node;
            for (const auto& i : logs) {
                YAML::Node n;
                n["name"] = i.name;
                if (i.level != LogLevel::UNKNOW) {
                    n["level"] = LogLevel::ToString(i.level);
                }
                if (!i.formatter.empty()) {
                    n["formatter"] = i.formatter;
                }
                // 输出目标
                for (auto& a : i.appenders) {
                    YAML::Node na;
                    if (a.type == 1) {
                        na["type"] = "FileLogAppender";
                        na["file"] = a.file;
                    } else if (a.type == 2) {
                        na["type"] = "StdoutLogAppender";
                    }
                    if (a.level != LogLevel::UNKNOW) {
                        na["level"] = LogLevel::ToString(a.level);
                    }

                    if (!a.formatter.empty()) {
                        na["formatter"] = a.formatter;
                    }
                    // 添加到列表
                    n["appenders"].push_back(na);
                }
                node.push_back(n); // 添加到 YAML 序列
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template <>
    class LexicalCast<std::string, std::set<LogDefine>> {
    public:
        std::set<LogDefine> operator()(const std::string &v) {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> logDefines;
            for (size_t i = 0; i < node.size(); ++i) {
                LogDefine ld;

                // 解析 LogDefine
                auto n = node[i];
                if (!n["name"].IsDefined()) {
                    std::cout << "log config error: name is null or not scalar, value=" << v << std::endl;
                    throw std::logic_error("log config name is null");
                }
                ld.name = n["name"].as<std::string>();
                ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
                if (n["formatter"].IsDefined()) {
                    ld.formatter = n["formatter"].as<std::string>();
                }
                if (n["appenders"].IsDefined()) {
                    for (size_t j = 0; j < n["appenders"].size(); ++j) {
                        auto a = n["appenders"][j];
                        if (!a["type"].IsDefined()) {
                            std::cout << "log config error: appender type is null, " << a << std::endl;
                            continue;
                        }
                        std::string type = a["type"].as<std::string>();
                        LogAppenderDefine lad;
                        if (type == "FileLogAppender") {
                            lad.type = 1;
                            if (!a["file"].IsDefined()) {
                                std::cout << "log config error: fileappender file is null, " << a << std::endl;
                                continue;
                            }
                            lad.file = a["file"].as<std::string>();
                            if (a["formatter"].IsDefined()) {
                                lad.formatter = a["formatter"].as<std::string>();
                            }
                        } else if (type == "StdoutLogAppender") {
                            lad.type = 2;
                            if (a["formatter"].IsDefined()) {
                                lad.formatter = a["formatter"].as<std::string>();
                            }
                        } else {
                            std::cout << "log config error: appender type is invalid, " << a << std::endl;
                            continue;
                        }
                        ld.appenders.push_back(lad);
                    }
                }
                logDefines.insert(ld); // 添加到 set 中
            }
            return logDefines;
        }
    };

    sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines=
        sylar::Config::Lookup("logs",std::set<LogDefine>(),"logs config");

    struct LogIniter {
        LogIniter() {
            g_log_defines->addListener([](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
                for (auto& i : new_value) {
                    auto it = old_value.find(i);
                    sylar::Logger::ptr logger;
                    if (it == old_value.end()) {
                        // 新增logger
                        logger = SYLAR_LOG_NAME(i.name);//获取日志器
                    } else {
                        if (!(i == *it)) {
                            // 修改的logger
                            logger = SYLAR_LOG_NAME(i.name);
                        } else {
                            continue;
                        }
                    }
                    logger->setLevel(i.level);
                    if (!i.formatter.empty()) {
                        logger->setFormatter(i.formatter);//设置日志格式
                    }

                    logger->clearAppenders();//重新添加目标
                    for (auto& a : i.appenders) {
                        sylar::LogAppender::ptr ap;
                        if (a.type == 1) {
                            ap.reset(new FileLogAppender(a.file));
                        } else if (a.type == 2) {
                            // if (!sylar::EnvMgr::GetInstance()->has("d")) {
                            //     ap.reset(new StdoutLogAppender);
                            // } else {
                            //     continue;
                            // }
                            ap.reset(new StdoutLogAppender);
                        }
                        ap->setLevel(a.level);//目标级的设置
                        if (!a.formatter.empty()) {
                            LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                            if (!fmt->isError()) {
                                ap->setFormatter(fmt);
                            } else {
                                std::cout << "log.name=" << i.name << " appender type=" << a.type
                                        << " formatter=" << a.formatter << " is invalid" << std::endl;
                            }
                        }
                        logger->addAppender(ap);
                    }
                }

                for (auto& i : old_value) {
                    auto it = new_value.find(i);
                    if (it == new_value.end()) {
                        // 删除logger
                        auto logger = SYLAR_LOG_NAME(i.name);
                        logger->setLevel((LogLevel::Level)0);
                        logger->clearAppenders();
                    }
                }
            });
        }
    };

    static LogIniter __log_init;
    
    void LoggerManager::init() {}
    
    LoggerManager::LoggerManager() {
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

        m_loggers[m_root->m_name] = m_root;

        init();
    }

    Logger::ptr LoggerManager::getLogger(const std::string& name) {
        MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end()) {
            return it->second;
        }

        Logger::ptr logger(new Logger(name));
        logger->m_root  = m_root;
        m_loggers[name] = logger;
        return logger;
    }

    std::string LoggerManager::toYamlString(){
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        for(auto& i:m_loggers){
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
}

