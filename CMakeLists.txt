cmake_minimum_required(VERSION 3.5)
project(proxy-server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SRC_LIST
"proxy"
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

