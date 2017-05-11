cmake_minimum_required(VERSION 3.5)
project(proxy-server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SRC_LIST
"main.cpp"
"proxy/exceptions.h"
"proxy/sockets.h"
"proxy/sockets.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

