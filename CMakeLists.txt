cmake_minimum_required(VERSION 3.5)
project(proxy-server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -Wextra -pedantic ")

set(SRC_LIST
"main.cpp"
"proxy/socket_util.h"
"proxy/socket_util.cpp"
"proxy/socket_api.h"
"proxy/socket_api.cpp"
"proxy/client.h"
"proxy/client.cpp"
"proxy/server.h"
"proxy/server.cpp"
"proxy/proxy_server.h"
"proxy/proxy_server.cpp"
"proxy/event_queue.h"
"proxy/event_queue.cpp"
"proxy/host_resolver.h"
"proxy/host_resolver.cpp"
"proxy/utils.h"
"proxy/utils.cpp"
"proxy/lru_cache.h"

"proxy/http/http_request.h"
"proxy/http/http_request.cpp"
"proxy/http/http_request_bodyless.h"
"proxy/http/http_request_bodyless.cpp"
"proxy/http/http_request_with_body.h"
"proxy/http/http_request_with_body.cpp"

"proxy/http/http_response.h"
"proxy/http/http_response.cpp"
"proxy/http/http_response_bodyless.h"
"proxy/http/http_response_bodyless.cpp"
"proxy/http/http_response_plain.h"
"proxy/http/http_response_plain.cpp"
"proxy/http/http_response_chunked.h"
"proxy/http/http_response_chunked.cpp"
#"proxy/http/http_response_closed_connection.h"
#"proxy/http/http_response_closed_connection.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_LIST})

