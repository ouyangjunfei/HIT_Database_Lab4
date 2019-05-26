cmake_minimum_required(VERSION 3.5)
project(SqlQueryAlgorithm C)

set(CMAKE_C_STANDARD 99)

#AUX_SOURCE_DIRECTORY(src DIR_SRCS)
#指定编译目录
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)
#添加子目录,这样进入源码文件src目录可以继续构建
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
