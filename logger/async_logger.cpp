/*
 * @Description: Copyright(c) All rights reserved.
 * @Company: YUSUR
 * @Author: Kaihua Guo
 * @Date: 2019-11-30 14:28:18
 * @LastEditors: Kaihua Guo
 * @LastEditTime: 2019-12-03 11:12:45
 */
#include "logger/async_logger.h"

namespace doe {

AsyncLogger::AsyncLogger(const char* base_name) :
    file_path_("./"),
    base_name_(base_name),
    running_(false),
    thread_(nullptr),
    curr_buffer_(new BufferBlock()),
    next_buffer_(new BufferBlock())
{
    buffers_.reserve(10);
}

AsyncLogger::~AsyncLogger()
{
    if (running_) {
        this->stop();
    }
}

void AsyncLogger::set_logfile_path(const char* log_path)
{
    file_path_ = log_path;
    if (file_path_.back() != '\\' && file_path_.back() != '/') {
#ifdef _WIN32
        file_path_.append("\\");
#else
        file_path_.append("/");
#endif
    }
}

void AsyncLogger::start()
{
    running_ = true;
    thread_.reset(new std::thread(&AsyncLogger::logging_thread_loop, this));
}

void AsyncLogger::stop()
{
    if (!running_) {
        return;
    }
    running_.store(false);
    cond_.notify_all();
    thread_->join();
}

void AsyncLogger::append(const char* line, const size_t len)
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (curr_buffer_->writable_bytes() > len) {
        curr_buffer_->append(line, len);
    }
    else {
        buffers_.emplace_back(curr_buffer_.release());
        if (next_buffer_) {
            curr_buffer_.reset(next_buffer_.release());
        }
        else {
            // Rarely happened, two buffers are all fully filled.
            curr_buffer_.reset(new BufferBlock());
        }
        curr_buffer_->append(line, len);
        cond_.notify_one();
    }
}

void AsyncLogger::logging_thread_loop()
{
    // BackEnd buffers
    BufferBlockPtr back_buffer1(new BufferBlock());
    BufferBlockPtr back_buffer2(new BufferBlock());
    BufferBlockPtrVector back_buffers;
    back_buffers.reserve(10);
    // Thread loop
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mtx_);
            if (buffers_.empty()) {
                // Wait for notifying by frontend or timeout.
                cond_.wait_for(lock, kLogInterval);
            }
            // If notified by timeout, curr_buffer_ is not empty
            buffers_.emplace_back(curr_buffer_.release());
            curr_buffer_.reset(back_buffer1.release());
            back_buffers.swap(buffers_);
            if (!next_buffer_) {
                next_buffer_.reset(back_buffer2.release());
            }
        }
        // Write buffers to file
        for (auto& b : back_buffers) {
            this->flush_to_file(b);
        }
        // Exchange back_buffer with empty buffers
        if (!back_buffer1) {
            back_buffer1.reset(back_buffers.back().release());
            back_buffer1->reset();
            back_buffers.pop_back();
        }
        if (!back_buffer2) {
            back_buffer2.reset(back_buffers.back().release());
            back_buffer2->reset();
            back_buffers.pop_back();
        }
        back_buffers.clear();
    }
    // Flush front buffers to file when exit
    // If logger stops when logging thread is writting log file,
    // there may be data in front buffers.
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (!buffers_.empty()) {
            for (auto& b : buffers_) {
                this->flush_to_file(b);
            }
        }
        if (curr_buffer_->length() > 0) {
            this->flush_to_file(curr_buffer_);
        }
    }
}

void AsyncLogger::flush_to_file(BufferBlockPtr& buffer)
{
    while (buffer->length() > 0) {
        LogLevel level = static_cast<LogLevel>(buffer->read_int32());
        int tag = buffer->read_int32();
        int len = buffer->read_int32();
        std::string file_name = pack_file_name(level, tag);
        // If file is not exist, create & open it
        if (files_.find(file_name) == files_.end()) {
            files_[file_name].reset(new LogFile(file_name));
        }
        files_[file_name]->append(buffer->data(), len);
        files_[file_name]->flush();
        buffer->delete_data(len);
    }
}

std::string AsyncLogger::pack_file_name(LogLevel level, int tag)
{
    std::string file_name = file_path_ + base_name_ + "-" + utils::level_to_str(level);
    if (tag >= 0) {
        file_name += "-DB" + std::to_string(tag);
    }
    else {
        file_name += "-DEFAULT";
    }
    return file_name;
}

} // namespace doe
