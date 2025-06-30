#include "log.h"

namespace sylar
{
    void Logger::log(LogLevel::Level level, const LogEvent::ptr &event)
    {
        if(level < m_level){
            return; // 如果日志级别低于当前设置的级别，则不处理
        }else if(!event){
            return; // 如果事件为空，则不处理
        }
        // 如果事件内容不为空，则处理日志
        for(auto &appender : m_appenders){
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
        for(auto it=m_appenders.begin();it!=m_appenders.end();++it){
            if(*it==appender){
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
}