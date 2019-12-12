#ifndef _LOGGER_LOG_C_STYLE_H_
#define _LOGGER_LOG_C_STYLE_H_

#include "logger/async_logger.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace logger
{

class LogCStyle
{
public:
    static LogCStyle& get_instance();

    void init_logger(const char* log_path, 
        LogLevel log_level,
        bool log_thread_id
    );

    void logging(const LogLevel level, 
        const char* file,
        const int line,
        const int tag,
        const char* fmt,
        ...
    );

private:
    LogCStyle();
    ~LogCStyle();

    LogCStyle(const LogCStyle&) = delete;
    LogCStyle& operator=(const LogCStyle&) = delete;

    // Split file name from __FILE__
    const char* split_source_file_name(const char* file);

    // Get ull thread id
    unsigned long long get_thread_id();

private:
    bool inited_;
    bool log_thread_id_;
    LogLevel log_level_;
    std::unique_ptr<AsyncLogger> logger_;
};

} // namespace logger

#endif // _LOGGER_LOG_C_STYLE_H_
