/*
 * @Description: Copyright(c) All rights reserved.
 * @Company: YUSUR
 * @Author: Kaihua Guo
 * @Date: 2019-11-30 14:29:50
 * @LastEditors: Kaihua Guo
 * @LastEditTime: 2019-12-05 20:44:36
 */
#include "logger/log_c_style.h"
#include "logger/logger_utils.h"

#include <cstdarg>
#include <cstdio>
#include <thread>
#include <sstream>

namespace doe {

LogCStyle& LogCStyle::get_instance()
{
    static LogCStyle instance;
    return instance;
}

void LogCStyle::init_logger(const char* log_path, LogLevel log_level, bool log_thread_id)
{
    if (inited_) {
        return;
    }

    log_level_ = log_level;
    log_thread_id_ = log_thread_id;
    logger_->set_logfile_path(log_path);
    logger_->start();
    
    inited_ = true;
}

void LogCStyle::logging(const LogLevel level, const char* file, const int line, const int tag, const char* fmt, ...)
{
    if (level < log_level_ || !inited_) {
        return;
    }

    // thread_local variables, only one instance in the same thread
    thread_local BufferBlock thread_local_buffer;
    thread_local unsigned long long tid = this->get_thread_id();

    // Pack log string
    thread_local_buffer.reset();
    if (log_thread_id_) {
        thread_local_buffer.append_fmt("[%s] [%s] [TID:%I64u] [%s:%d] ",
            utils::level_to_str(level),
            utils::get_current_system_time().c_str(),
            tid,
            this->split_source_file_name(file),
            line
        );
    }
    else {
        thread_local_buffer.append_fmt("[%s] [%s] [%s:%d] ",
            utils::level_to_str(level),
            utils::get_current_system_time().c_str(),
            this->split_source_file_name(file),
            line
        );
    }
    va_list args;
    va_start(args, fmt);
    thread_local_buffer.append_fmt(fmt, args);
    va_end(args);
    thread_local_buffer.append_new_line();

    // Prepend level tag & length to string
    thread_local_buffer.prepend(static_cast<int>(thread_local_buffer.length()));
    thread_local_buffer.prepend(tag);
    thread_local_buffer.prepend(static_cast<int>(level));

    /*std::string Level = utils::level_to_str(thread_local_buffer.read_int32());
    int Tag = thread_local_buffer.read_int32();
    int Len = thread_local_buffer.read_int32();
    printf("%s %d %d %d %s\n", 
        Level.c_str(), 
        Tag, 
        Len,
        (int)thread_local_buffer.length(),
        thread_local_buffer.data());*/

    // Append to backend logger, locked
    logger_->append(thread_local_buffer.data(), thread_local_buffer.length());
}

LogCStyle::LogCStyle() :
    inited_(false),
    log_thread_id_(false),
    log_level_(LogLevel::kDebug),
    logger_(new AsyncLogger("LOG"))
{}

LogCStyle::~LogCStyle()
{
    if (inited_) {
        logger_->stop();
    }
}

const char* LogCStyle::split_source_file_name(const char* file)
{
    const char* pos;
#ifdef _WIN32
    pos = ::strrchr(file, '\\');
#else
    pos = ::strrchr(file, '/');
#endif
    return (pos == nullptr) ? file : (pos + 1);
}

unsigned long long LogCStyle::get_thread_id()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return std::stoull(ss.str());
}

} // namespace doe
