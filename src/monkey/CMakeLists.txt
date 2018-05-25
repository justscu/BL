cmake_minimum_required(VERSION 2.6)

#工程名字
PROJECT(a.monkey)

# 输出平台
MESSAGE(STATUS "#### operation system is ${CMAKE_SYSTEM} ####project root dir is ${PROJECT_SOURCE_DIR}")
# 编译选项
ADD_DEFINITIONS(-g -o0 -D_FILE_OFFSET_BITS=64 ${CMAKE_CXX_FLAGS})

# 判断平台
IF(WIN32)
ELSEIF(UNIX)
ELSEIF(APPLE)
#ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
ELSE()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF(WIN32)

# 64bit
SET(PLATFORM X86_64)

# 添加头文件目录
INCLUDE_DIRECTORIES(
./
#./src/monitor_file_change
./src/hardware_info
)

# 源文件目录/文件
AUX_SOURCE_DIRECTORY(./  SRC_LIST_DIR)
#AUX_SOURCE_DIRECTORY(./src/monitor_file_change  SRC_LIST_DIR)

IF(WIN32)
SET(SRC_LIST_FILE
./main.cpp
#./src/hardware_info/machine_info_test.cpp 
#./src/hardware_info/machine_info_win.cpp
./src/udp_multicast/udp_multicast.cpp
)
ELSEIF(UNIX)
SET(SRC_LIST_FILE
./main.cpp
#./src/hardware_info/machine_info_test.cpp 
#./src/hardware_info/machine_info_linux.cpp
./src/udp_multicast/udp_multicast.cpp
)
ELSE()
ENDIF(WIN32)

# 链接文件路径
LINK_DIRECTORIES(
#/usr/lib
#/usr/local/lib
)

# output dir
SET(EXECUTABLE_OUTPUT_PATH ./)
#SET(LIBRARY_OUTPUT_PATH ./lib)

# 生成可执行文件
ADD_EXECUTABLE(a.monkey ${SRC_LIST_FILE} ${SRC_LIST_DIR})

# lib文件
TARGET_LINK_LIBRARIES(a.monkey)

