cmake_minimum_required(VERSION 2.8)
PROJECT(SHARK)

ADD_DEFINITIONS(-g -W -Wall -std=c++11 -Wformat -Wno-deprecated -DTHREADED ${CMAKE_CXX_FLAGS})

AUX_SOURCE_DIRECTORY(./ SRC_LIST)

INCLUDE_DIRECTORIES(
./
/Users/just-mac/software/u01/libevent-2.0.22-stable/include
)

SET(EXECUTABLE_OUTPUT_PATH ./)

LINK_DIRECTORIES(
/usr/lib/
/Users/just-mac/software/u01/libevent-2.0.22-stable/lib
)

ADD_EXECUTABLE(shark ${SRC_LIST})

TARGET_LINK_LIBRARIES(shark pthread event event_pthreads)
