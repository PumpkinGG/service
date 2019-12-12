#ifndef _LOGGER_LOGGER_UTILS_H_
#define _LOGGER_LOGGER_UTILS_H_

#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>

namespace logger {

const size_t kSmallBlockSize = 4 * 1024;
const size_t kLargeBlockSize = 1 * 1024 * 1024;
const size_t kReservePrependSize = 128;

enum class LogLevel {
    kDebug = 1,
    kInfo  = 2,
    kError = 3,
    kOff   = 4
};

namespace utils {

// Print current system time
std::string get_current_system_time();
// Transfer log level to str
const char* level_to_str(LogLevel level);
const char* level_to_str(int level);

}

class BufferBlock 
{
public:
    BufferBlock();
    explicit BufferBlock(const size_t block_size);
    
    ~BufferBlock();

    // Unreadable data
    const char* data();

    // Length of buffered data
    size_t length();

    // Delete buffered data from front
    // Return really deleted bytes
    // Not really delete, just move read_index_
    void delete_data(const size_t len);

    // Read int32 from buffer.
    int read_int32();

    // Writable space
    size_t writable_bytes();

    // Prependable space
    size_t prependable_bytes();

    // Prepend int32 to buffer. For prepending level tag and length
    void prepend(const int num);

    // Append str to buffer
    void append(const char* str, const size_t len);

    // Append str to buffer
    void append(const char* str);

    // Append str to buffer
    void append(const std::string& str);

    // Append format str to buffer, C-Style
    void append_fmt(const char* fmt, ...);
    void append_fmt(const char* fmt, va_list args);

    // Append '\n' to the end
    void append_new_line();

    // Reset buffer
    void reset();

private:
    char* buffer_;
    const size_t reserve_size_;
    size_t read_index_;
    size_t write_index_;
    size_t capacity_;
};


class LogFile
{
public:
    typedef std::unique_ptr<BufferBlock> BufferBlockPtr;

public:
    LogFile(const std::string& file_name);
    ~LogFile();

    // Append a string to the file
    void append(const char* str, const size_t len);

    // Append BufferBlock to the file
    void append(BufferBlockPtr& buffer);

    // Flush cache to file
    void flush();

private:
    std::string file_name_;
    std::ofstream file_;
    size_t written_bytes_;
};

} // namespace logger

#endif // _LOGGER_LOGGER_UTILS_H_
