cmake_minimum_required(VERSION 2.8)



# 工程名字
PROJECT(pkg_reassemble)

MESSAGE(STATUS "Project Directory: ${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOG_FILE_NAME=__FILENAME__")

# 添加编译选项
ADD_DEFINITIONS(-g -O0 -std=c++11 -fPIC -Wno-deprecated ${CMAKE_CXX_FLAGS})

# 添加源文件路径
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/ SRC_LIST)

# 添加头文件路径
INCLUDE_DIRECTORIES(
${PROJECT_SOURCE_DIR}/src/
)

# 设置输出文件路径
SET(EXECUTABLE_OUTPUT_PATH ./)

# 添加库文件路径
LINK_DIRECTORIES(
/usr/local/lib/
/usr/lib/
)

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/ SRC_LIST)
ADD_EXECUTABLE(pkg_reassemble ${SRC_LIST})
