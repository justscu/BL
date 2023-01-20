cmake_minimum_required(VERSION 2.8)                                                                                            

# 工程名字
PROJECT(TestX)

# 添加编译选项
ADD_DEFINITIONS(-g -O0 -std=c++11 -Wno-deprecated -mavx2 ${CMAKE_CXX_FLAGS})


# 添加头文件路径
INCLUDE_DIRECTORIES(
/home/ll/software/work/BL/src/commlib/third/fmt/include
/home/ll/software/work/BL/src/commlib/utils/
/home/ll/software/work/BL/src/commlib/test/src/
)

#
AUX_SOURCE_DIRECTORY(/home/ll/software/work/BL/src/commlib/utils/  SRC_LIST)
AUX_SOURCE_DIRECTORY(/home/ll/software/work/BL/src/commlib/test/  SRC_LIST)



# 设置输出文件路径
SET(EXECUTABLE_OUTPUT_PATH ./)

# 添加库文件路径
LINK_DIRECTORIES(
/usr/local/lib/
/usr/lib/
/home/ll/software/download/src/fmt/debug/
)

# 生成可执行文件，依赖于SRC_LIST文件
ADD_EXECUTABLE(testx ${SRC_LIST})

# 添加库文件
TARGET_LINK_LIBRARIES(testx libfmt.a pthread)
