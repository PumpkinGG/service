## Async Logger
---
### 使用
- **初始化模块**
  - `include "core/logger/logger.h"`
  
  - 调用初始化函数配置Log路径及Log等级

    ```c++
    doe::init_async_logger(const char* log_path = "./", 
        LogLevel log_level = LogLevel::kDebug,
        bool log_thread_id = false // 是否打印线程id，默认不打印
    )
    ```
  
  - LogLevel分为kDebug、kInfo、kError三个等级，分别记录Debug信息，SQL执行信息，Error信息
  
- **需要使用Logger的模块**
  
  - `include "core/logger/logger.h"`
  - 使用宏LOG_DEBUG，LOG_INFO，LOG_ERROR
  - 宏采用C-Style printf风格，`LOG_DEBUG(int tag, const char* format, ...)`
  - 第一个参数指定对应数据库的tag，后面参数与`printf`函数参数相同，格式化一个log字符串

### 例子

```c++
#include "core/logger/logger.h"
using namespace doe;

int main()
{
    init_async_logger(".\\", LogLevel::INFO); // 只需要初始化模块调用，重复调用无效
    LOG_INFO(1, "Info message %d", 1);
    LOG_DEBUG(1, "Debug message %d", 2); // DEBUG等级低于INFO，不会打印到文件
    LOG_ERROR(1, "Error message %d", 3);
    return 0;
}
```

### 输出形式

```
[ERROR] [2019-12-11 15.29.35] [logger_ut.cpp:26] thread 2 log msg 1
[ERROR] [2019-12-11 15.29.35] [logger_ut.cpp:26] thread 2 log msg 2
[ERROR] [2019-12-11 15.29.35] [logger_ut.cpp:26] thread 2 log msg 3
[ERROR] [2019-12-11 15.29.35] [logger_ut.cpp:26] thread 2 log msg 4
[ERROR] [2019-12-11 15.29.35] [logger_ut.cpp:26] thread 2 log msg 5
```

