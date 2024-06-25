cmake_minimum_required(VERSION 2.8)


######################################
# 工程名字
######################################
PROJECT(udp_pg)

######################################
# 添加编译选项
######################################
add_definitions(-g -O2 -std=c++11 -Wno-deprecated ${CMAKE_CXX_FLAGS})


######################################
# include
######################################
include_directories(
    ${PROJECT_SOURCE_DIR}/../third/
    ${PROJECT_SOURCE_DIR}/src/
)

######################################
# source files
######################################
file(GLOB SRC_LIST
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)


######################################
# 设置输出文件路径
######################################
SET(EXECUTABLE_OUTPUT_PATH ./)

######################################
# 添加库文件路径
######################################
link_directories(
    /usr/local/lib/
    /usr/lib/
    ${PROJECT_SOURCE_DIR}/../third/commx/        
    /home/ll/software/download/src/fmt/debug/
#    /home/ll/software/download/src/onload-7.1.2.141/build/gnu_x86_64/lib/ciul/
#    /home/ll/software/download/src/onload-7.1.0.265/build/gnu_x86_64/lib/ciul
    /home/ll/software/download/src/onload-8.1.3.40/onload-8.1.3.40/build/gnu_x86_64/lib/ciul
)

######################################
# 生成可执行文件，依赖于SRC_LIST文件
######################################
add_executable(udp_pg ${SRC_LIST} main_pg.cpp)

######################################
# 添加库文件
######################################
target_link_libraries(udp_pg libciul1.a libcommx.a libfmt.a pthread)



######################################
# 生成可执行文件，依赖于SRC_LIST文件
######################################
add_executable(udp_pg_efvi ${SRC_LIST} main_pg_efvi.cpp)

######################################
# 添加库文件
######################################
target_link_libraries(udp_pg_efvi libciul1.a libcommx.a libfmt.a pthread)

