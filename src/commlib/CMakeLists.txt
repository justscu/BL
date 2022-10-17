cmake_minimum_required(VERSION 2.8)

PROJECT(CommxProject)

# 打印消息
message("PROJECT_SOURCE_DIR=" ${PROJECT_SOURCE_DIR})
# set(CMAKE_VERBOSE_MAKEFILEON ON)

# 编译选项
add_definitions(-g -O2 -Wall -std=c++11 -fPIC -mavx2 -Wno-deprecated ${CMAKE_CXX_FLAGS})

include_directories(
${PROJECT_SOURCE_DIR}/utils/
${PROJECT_SOURCE_DIR}/log/
${PROJECT_SOURCE_DIR}/tinyini/
${PROJECT_SOURCE_DIR}/third/fmt/include/
)

aux_source_directory(${PROJECT_SOURCE_DIR}/utils/   SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/log/     SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/tinyini/ SRC_LIST)

#拷贝头文件
file(COPY ${PROJECT_SOURCE_DIR}/utils/   DESTINATION ${PROJECT_SOURCE_DIR}/commx/ FILES_MATCHING PATTERN "*.h")
file(COPY ${PROJECT_SOURCE_DIR}/log/     DESTINATION ${PROJECT_SOURCE_DIR}/commx/ FILES_MATCHING PATTERN "*.h")
file(COPY ${PROJECT_SOURCE_DIR}/tinyini/ DESTINATION ${PROJECT_SOURCE_DIR}/commx/ FILES_MATCHING PATTERN "*.h")
file(COPY ${PROJECT_SOURCE_DIR}/third/fmt/include/fmt/ DESTINATION ${PROJECT_SOURCE_DIR}/commx/fmt/ FILES_MATCHING PATTERN "*.h")

link_directories(
/usr/local/lib/
${PROJECT_SOURCE_DIR}/third/fmt/debug/
)

# 输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/commx/)


# 生成 libcommx.a
add_library(commx STATIC ${SRC_LIST})
target_link_libraries(commx
"-Wl, --no-whole-archive" # 之后的库不使用--whole-archive
libfmt.a
)
# 生成 libcommx.so
#add_library(commx_2 SHARED fmt ${SRC_LIST})
#set_target_properties(commx_2 PROPERTIES OUTPUT_NAME "commx") #修改输出名字
