cmake_minimum_required(VERSION 2.8)                                                                                            

# 工程名字
PROJECT(TestX)

# 添加编译选项
add_definitions(-std=c++11 -g -O0 -Wno-deprecated -mavx2 ${CMAKE_CXX_FLAGS})


# include
include_directories(
    ${PROJECT_SOURCE_DIR}/../debug/commx/
    # ${PROJECT_SOURCE_DIR}/../third/fmt/include/
    /home/ll/software/download/src/fmt/include
    ${PROJECT_SOURCE_DIR}/src/
)

# source files
file(GLOB SRC_LIST
    ${PROJECT_SOURCE_DIR}/../utils/*-test.cpp
    ${PROJECT_SOURCE_DIR}/../log/*-test.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp
)


# 设置输出文件路径
SET(EXECUTABLE_OUTPUT_PATH ./)

# 添加库文件路径
link_directories(
    /usr/local/lib/
    /usr/lib/
    ${PROJECT_SOURCE_DIR}/../debug/commx/
   # ${PROJECT_SOURCE_DIR}/../third/fmt/debug/
    /home/ll/software/download/src/fmt/debug/
)


# 生成可执行文件，依赖于SRC_LIST文件
add_executable(testx ${SRC_LIST})

# 为testx添加编译和链接选项
#target_compile_options(testx PRIVATE -pg)
#target_link_options(testx PRIVATE -pg)

# 添加库文件
target_link_libraries(testx libcommx.a libfmt.a pthread)
