#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cassert>
#include <algorithm>

#include "client.h"
#include "server.h"
#include "socket_api.h"
#include "http/http_response_plain.h"
#include "http/http_response_chunked.h"
#include "http/http_response_bodyless.h"
//#include "http/http_response_closed_connection.h"

const std::string BAD_REQUEST = "HTTP/1.1 400 Bad Request\r\nServer: proxy\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 164\r\nConnection: close\r\n\r\n<html>\r\n<head><title>400 Bad Request</title></head>\r\n<body bgcolor=\"white\">\r\n<center><h1>400 Bad Request</h1></center>\r\n<hr><center>proxy</center>\r\n</body>\r\n</html>";


server_t::server_t(int fd):
        peer_t(fd),
        client(nullptr),
        request(nullptr),
        response(nullptr)
{}

server_t::server_t(sockaddr addr):
        peer_t(socket_api::create_server_socket(addr)),
        client(nullptr),
        request(nullptr),
        response(nullptr)
{}

void server_t::bind(client_t* new_client) {
    client = new_client;
}

bool server_t::is_request_passed() const noexcept {
    assert(request);
    return request->is_passed();
}

http_response const* server_t::get_response() {
//    assert(response);
    return response.get();
}

void server_t::set_request(http_request* new_request) {
    request.reset(new_request);
}

std::string server_t::get_host() const noexcept {
    return host;
}


int server_t::get_client_fd() {
    return client->get_fd();
}

void server_t::write_request() {
    assert(request);
    assert(!request->is_passed());

    if (request->has_data_to_send()) {
        std::string next = request->get_next_data_to_send();
        size_t delta = write(next);
        request->move_offset(delta);
    }
}

void server_t::read_response() {
    int length = read();
    std::cout << "read {" << length << "} bytes" << std::endl;
//    std::cout << "read response" << std::endl;

    if (!response) {
        size_t i = buffer.find("\r\n\r\n");
        if (i == std::string::npos) return;

        std::string lower_header = buffer.substr(0, i);
        std::transform(lower_header.begin(), lower_header.end(), lower_header.begin(), ::tolower);

        i = lower_header.find("content-length: ") ;
        if (i != std::string::npos) {
            buffer[i] = 'C';
            buffer[i + 8] = 'L';
//            std::cout << "new plain response" << std::endl;
            response.reset(new http_response_plain());
        }

        i = lower_header.find("transfer-encoding: chunked") ;
        if (i != std::string::npos) {
//            std::cout << "new chunked response" << std::endl;
            response.reset(new http_response_chunked());
        }

        i = lower_header.find("connection: close");
        if (!response && i != std::string::npos && length == 0) {
            std::cout << "new response with Connection: close" << std::endl;
            std::string prefix = buffer.substr(0, i);
            std::string suffix = buffer.substr(i);
            buffer.clear();
            buffer.append(prefix);
            // skip Connection: close \r\n\r\n
            buffer.append("Content-Length: " + std::to_string((int)suffix.size() - 17 - 4) + "\r\n");
            buffer.append(suffix);
            response.reset(new http_response_plain());
        }

        // keep-alive and bodyless
        if (!response && i == std::string::npos) {
            std::cout << "new bodyless response" << std::endl;
            response.reset(new http_response_bodyless);
        }
    }

//    std::cout << "{" << buffer  << "}\n" << std::endl;
    if (response) {
        if (length == 0) {
            response->finish_connection();
        }
        response->append_data(get_buffer());
        buffer.clear();
    }
}

void server_t::move_response_offset(size_t delta) {
    response->move_offset(delta);
}
