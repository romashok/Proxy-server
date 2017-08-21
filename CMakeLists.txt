cmake_minimum_required(VERSION 3.5)
project(proxy-server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SRC_LIST
"main.cpp"
"proxy/exceptions.h"
"proxy/sockets.h"
"proxy/sockets.cpp"
"proxy/client.h"
"proxy/client.cpp"
"proxy/proxy_server.h"
"proxy/proxy_server.cpp"
"proxy/event_queue.h"
"proxy/event_queue.cpp"
"proxy/http_request.h"
"proxy/http_request.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

