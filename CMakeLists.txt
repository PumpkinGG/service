# CMakeList.txt: Service 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("Service")

aux_source_directory(./logger LOG_DIR)
aux_source_directory(./service SRV_DIR)

include_directories(
	./
)

# 将源代码添加到此项目的可执行文件。
add_executable (Service 
	main.cpp 
	${LOG_DIR}
	${SRV_DIR}
)

# TODO: 如有需要，请添加测试并安装目标。
