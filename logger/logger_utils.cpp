/*
 * @Description: Copyright(c) All rights reserved.
 * @Company: YUSUR
 * @Author: Kaihua Guo
 * @Date: 2019-11-29 16:06:09
 * @LastEditors: Kaihua Guo
 * @LastEditTime: 2019-12-03 11:15:19
 */
#include "logger/logger_utils.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <ctime>
#include <algorithm>

//#define LOGGER_DEBUG

namespace doe {

std::string utils::get_current_system_time()
{
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm ptm;
    localtime_s(&ptm, &tt);
    char date[60] = { 0 };
    sprintf_s(date, "%d-%02d-%02d %02d.%02d.%02d",
        (int)ptm.tm_year + 1900, (int)ptm.tm_mon + 1, (int)ptm.tm_mday,
        (int)ptm.tm_hour, (int)ptm.tm_min, (int)ptm.tm_sec);
    return std::string(date);
}

const char* utils::level_to_str(LogLevel level)
{
    switch (level)
    {
    case LogLevel::kDebug:
        return "DEBUG";
    case LogLevel::kInfo:
        return "INFO";
    case LogLevel::kError:
        return "ERROR";
    case LogLevel::kOff:
    default:
        return "OFF";
    }
}

const char* utils::level_to_str(int level)
{
    return level_to_str(static_cast<LogLevel>(level));
}

BufferBlock::BufferBlock() :
    reserve_size_(kReservePrependSize),
    read_index_(kReservePrependSize),
    write_index_(kReservePrependSize),
    capacity_(kSmallBlockSize + kReservePrependSize)
{
    buffer_ = new char[capacity_];
#ifdef LOGGER_DEBUG
    ::printf("%p constructs!\n", this);
#endif
}

BufferBlock::BufferBlock(const size_t block_size) :
    reserve_size_(kReservePrependSize),
    read_index_(kReservePrependSize),
    write_index_(kReservePrependSize),
    capacity_(block_size + kReservePrependSize)
{
    buffer_ = new char[capacity_];
#ifdef LOGGER_DEBUG
    ::printf("%p constructs!\n", this);
#endif
}

BufferBlock::~BufferBlock()
{
    delete[] buffer_;
#ifdef LOGGER_DEBUG
    ::printf("%p destructs!\n", this);
#endif
}

const char* BufferBlock::data()
{
    return buffer_ + read_index_;
}

size_t BufferBlock::length()
{
    return write_index_ - read_index_;
}

void BufferBlock::delete_data(const size_t len)
{
    if (len >= this->length()) {
        this->reset();
    }
    else {
        read_index_ += len;
    }
}

int BufferBlock::read_int32()
{
    if (this->length() < sizeof(int)) {
        return -1;
    }
    int res = 0;
    ::memcpy((void*)&res, buffer_ + read_index_, sizeof(int));
    read_index_ += sizeof(int);
    return res;
}

size_t BufferBlock::writable_bytes()
{
    return capacity_ - write_index_;
}

size_t BufferBlock::prependable_bytes()
{
    return read_index_;
}

void BufferBlock::prepend(const int num)
{
    if (this->prependable_bytes() < sizeof(int)) {
        return;
    }
    ::memcpy(buffer_ + read_index_ - sizeof(int), (void*)&num, sizeof(int));
    read_index_ -= sizeof(int);
}

void BufferBlock::append(const char* str, const size_t len)
{
    if (len > this->writable_bytes()) {
        return;
    }
    ::memcpy(buffer_ + write_index_, str, len);
    write_index_ += len;
}

void BufferBlock::append(const char* str)
{
    this->append(str, ::strlen(str));
}

void BufferBlock::append(const std::string& str)
{
    this->append(str.c_str(), str.length());
}

void BufferBlock::append_fmt(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    this->append_fmt(fmt, args);
    va_end(args);
}

void BufferBlock::append_fmt(const char* fmt, va_list args)
{
    int str_len = ::vsnprintf(nullptr, 0, fmt, args) + 1;
    if ((size_t)str_len > this->writable_bytes()) {
        return;
    }
    int ret = ::vsnprintf(buffer_ + write_index_, str_len, fmt, args);
    write_index_ += ret;
}

void BufferBlock::append_new_line()
{
    buffer_[write_index_++] = '\n';
}

void BufferBlock::reset()
{
    read_index_ = reserve_size_;
    write_index_ = reserve_size_;
}

LogFile::LogFile(const std::string& file_name) :
    file_name_(file_name),
    written_bytes_(0)
{
    file_name_.append(".txt");
    file_.open(file_name_, std::fstream::app);
}

LogFile::~LogFile()
{
    if (file_.is_open()) {
        file_.close();
    }
}

void LogFile::append(const char* str, const size_t len)
{
    if (!file_.is_open()) {
        ::printf("Log file is not opened!\n");
        return;
    }
    file_.write(str, len);
    written_bytes_ += len;
}

void LogFile::append(BufferBlockPtr& buffer)
{
    if (!file_.is_open()) {
        ::printf("Log file is not opened!\n");
        return;
    }
    file_.write(buffer->data(), buffer->length());
    written_bytes_ += buffer->length();
}

void LogFile::flush()
{
    file_.flush();
}

} // namespace doe