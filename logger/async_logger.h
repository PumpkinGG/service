/*
 * @Description: Infrastructure of async multi-thread logger.
 * @Company: YUSUR
 * @Author: Kaihua Guo
 * @Date: 2019-11-30 14:28:00
 * @LastEditors: Kaihua Guo
 * @LastEditTime: 2019-12-03 11:23:27
 */
#ifndef _CORE_LOGGER_ASYNC_LOGGER_H_
#define _CORE_LOGGER_ASYNC_LOGGER_H_

#include "logger/logger_utils.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace doe {

// Notify logging thread every [kLogInterval] seconds.
const std::chrono::seconds kLogInterval = std::chrono::seconds(3);

class AsyncLogger
{
public:
    typedef std::unique_ptr<BufferBlock> BufferBlockPtr;
    typedef std::vector<std::unique_ptr<BufferBlock>> BufferBlockPtrVector;
    typedef std::unordered_map<std::string, std::unique_ptr<LogFile>> FileMap;

public:
    AsyncLogger(const char* base_name);
    ~AsyncLogger();

    void set_logfile_path(const char* log_path);

    void start();

    void stop();

    // FrontEnd interface.
    void append(const char* line, const size_t len);

private:
    // BackEnd interface.
    void logging_thread_loop();

    // Flush buffer to files
    void flush_to_file(BufferBlockPtr& buffer);

    // Pack file name
    std::string pack_file_name(LogLevel level, int tag);

private:
    std::string file_path_;
    std::string base_name_;
    std::atomic<bool> running_;
    std::shared_ptr<std::thread> thread_;
    std::mutex mtx_;
    std::condition_variable cond_;
    BufferBlockPtr curr_buffer_;
    BufferBlockPtr next_buffer_;
    // Save fully filled buffers
    BufferBlockPtrVector buffers_;
    // File name to LogFile map
    FileMap files_;
};

} // namespace doe

#endif // _CORE_LOGGER_ASYNC_LOGGER_H_
