#ifndef _LOGGER_LOGGER_H_
#define _LOGGER_LOGGER_H_

#include "logger/log_c_style.h"

namespace logger
{

inline void init_async_logger(const char* log_path = "./", 
    LogLevel log_level = LogLevel::kDebug,
    bool log_thread_id = false
) {
    LogCStyle::get_instance().init_logger(log_path, log_level, log_thread_id);
}

#define LOG_DEBUG(...) \
    LogCStyle::get_instance().logging(logger::LogLevel::kDebug, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_INFO(...) \
    LogCStyle::get_instance().logging(logger::LogLevel::kInfo, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_ERROR(...) \
    LogCStyle::get_instance().logging(logger::LogLevel::kError, __FILE__, __LINE__, __VA_ARGS__);
// Print to stdout
#define LOG_TEST(...) \
    ::printf_s(__VA_ARGS__);

} // namespace logger

#endif // _LOGGER_LOGGER_H_
