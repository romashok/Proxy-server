#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "host_resolver.h"
#include "client.h"

host_resolver::host_resolver() {}

bool host_resolver::resolve(client_t* client) {
    http_request* request = client->request.get();
    std::cout << "Resolving host{" << request->get_host() << "}" << std::endl;

    size_t i = request->get_host().find(":");
    std::string port = "80";
    if (i != std::string::npos) {
        port = request->get_host().substr(i + 1);
    }
    std::string new_host_name = request->get_host().substr(0, i);

    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(new_host_name.c_str(), port.c_str(), &hints, &server_info) != 0) {
        std::cout << "RESOLVER: getaddrinfo error!" << std::endl;
        return false;
    }

    sockaddr server_addr = *server_info->ai_addr;
    freeaddrinfo(server_info);
    request->set_server_addr(server_addr);
    return true;
}
