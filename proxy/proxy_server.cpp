#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "exceptions.h"
#include "proxy_server.h"
#include "http_request.h"

#include "socket_api.h"
#include "utils.h"

proxy_server::proxy_server(uint32_t port):
    proxy_socket(socket_api::create_main_socket(port)),
    queue(),
    resolver(),
    is_working(true)
{
    std::cout << "Main socket established on fd[" << proxy_socket.get_fd() << "]." << std::endl;

    queue.add_event([this](struct epoll_event& ev) {
        this->connect_client();
    }, proxy_socket.get_fd(), EPOLLIN);
}


void proxy_server::run() {
    std::cout << "Start proxy server!" << std::endl;

    try{
        while(is_working) {
            int amount = queue.get_events_amount();

            if (amount == -1) {
                perror("Error occured during getting new epoll events!");
                continue;
            }

            queue.handle_events(amount);
        }
        std::cout << "proxy server stoped" << std::endl;
        std::exit(0);
    } catch (custom_exception& e) {
        std::cout << e.reason();
    }
}

void proxy_server::connect_client() {
    std::cout << "===================================================================\nConnect client! ";
    client_t * new_client = new client_t(socket_api::accept(proxy_socket.get_fd()));
    clients[new_client->get_fd()] = std::move(std::unique_ptr<client_t>(new_client));
    std::cout << "Client fd [" << new_client->get_fd() << "], total amount now is " << clients.size() << std::endl;

    queue.add_event([this](struct epoll_event& ev) {
        this->read_from_client(ev);
    }, new_client->get_fd(), EPOLLIN);
}

void proxy_server::disconnect_client(struct epoll_event& ev) {
    std::cout << "Disconnect client [" << ev.data.fd << "]" << std::endl;
    queue.delete_event(ev);
    clients.erase(ev.data.fd);
}

void proxy_server::read_from_client(struct epoll_event& ev) {
    struct client_t* client = clients.at(ev.data.fd).get();

    int new_chunk_size = client->read(); // TODO 1) should read only available data
    if (new_chunk_size)
        std::cout << "\nClient ["<< client->get_fd() <<"] data (new " << new_chunk_size << "):[\n" << client->get_buffer() << "]" << std::endl;
    else
        disconnect_client(ev);

    if (http_request::is_complete_request(client->get_buffer())) {
        std::cout << "Got complited request!" << std::endl;
        {
            std::cout << "delete event{write to server} " << std::endl;
            struct epoll_event ev;
            ev.data.fd = client->get_fd();
            ev.events = EPOLLIN;

            queue.delete_event(ev);
        }

        std::unique_ptr<http_request> request(new (std::nothrow) http_request(client->get_buffer()));

        if (!request) {
            std::cout << "Allocation problems for request!";
            return;
        }

        if (client->has_server()) {
            std::cout << "HAS SERVER" << std::endl;
            if (client->get_request_host() == request->get_host()) {
                std::cout << "SAME SERVER" << std::endl;
                // todo write to the server
            } else {
                std::cout << "OTHER SERVER" << std::endl;
                // todo unbind the server
            }
        } else {
           // todo bind new server
            std::cout << "NO SERVER" << std::endl;
            request->set_client_fd(client->get_fd());
            resolve(request.get());
            if (request->is_resolved()) {
                // todo write to server
                std::cout << "Successfuly resovled!" << std::endl;
                client->send_msg_to_server();
                queue.add_event([this](struct epoll_event& ev) {
                    this->write_to_server(ev);
                }, client->get_server_fd(), EPOLLOUT);
                std::cout << "Add server task." << std::endl;
            } else {
                // todo handle resolve error
                std::cout << "Resovle error!" << std::endl;
            }
        }

    }
}


void proxy_server::resolve(http_request* request) {
    std::cout << "Resolving host{" << request->get_host() << "}" << std::endl;
    size_t i = request->get_host().find(":");
    std::string port = "80";
    if (i != std::string::npos) {
        port = request->get_host().substr(i + 1);
    }
    std::string new_host_name = request->get_host().substr(0, i);

    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // AF_INET for ipv4 //AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(new_host_name.c_str(), port.c_str(), &hints, &server_info) != 0) {
        std::cout << "RESOLVER: getaddrinfo error!" << std::endl;
        request->set_resolved(false);
        return;
    }

    sockaddr server_addr = *server_info->ai_addr;
    freeaddrinfo(server_info);
    request->set_server_addr(server_addr);

    // after resolve. bind server and client
    struct client_t* client = clients.at(request->get_client_fd()).get();
    server_addr = std::move(request->get_server_addr());
    struct server_t* server;

    try {
        server = new struct server_t(server_addr);
    } catch (...) {
        std::cout << strerror(errno) << "\nError occured! Can't create server socket." << std::endl;
    }

    servers[server->get_fd()] = server;
    client->bind(server);
    server->bind(client);

    request->set_resolved(true);
}

void proxy_server::write_to_server(epoll_event& ev) {
    std::cout << "Writing to server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    server->write();
    if (server->is_empty_buffer()) {
        // todo delete event{write to server}
        {
            std::cout << "delete event{write to server} " << std::endl;
            struct epoll_event ev;
            ev.data.fd = server->get_fd();
            ev.events = EPOLLOUT;

            queue.delete_event(ev);
            queue.invalidate_event(server->get_fd(), EPOLLOUT);
        }

        queue.add_event([this](struct epoll_event& ev) {
            this->read_from_server(ev);
        }, server->get_fd(), EPOLLIN);
    }
}


void proxy_server::read_from_server(struct epoll_event& ev) {
//    std::cout << "Reading from server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    server->read();

    if (server->get_buffer().find("\r\n0\r\n\r\n") != std::string::npos) {
        {
            std::cout << "delete event{read from server} " << std::endl;
            struct epoll_event ev;
            ev.data.fd = server->get_fd();
            ev.events = EPOLLIN;

            queue.delete_event(ev);
        }

        std::cout << "ANSWER :{\n" << server->get_buffer() << "}" << std::endl;
        server->sent_msg_to_client();

        queue.add_event([this](struct epoll_event& ev) {
            this->write_to_client(ev);
        }, server->get_client_fd(), EPOLLOUT);
    }
}


void proxy_server::write_to_client(struct epoll_event& ev) {
    std::cout << "Writing to client" << std::endl;
    struct client_t* client = clients.at(ev.data.fd).get();
    client->write();

    if (client->get_buffer().empty()) {
        std::cout << "delete event{write to client} " << std::endl;
        struct epoll_event ev;
        ev.data.fd = client->get_fd();
        ev.events = EPOLLOUT;

        queue.delete_event(ev);
        queue.invalidate_event(client->get_fd(), EPOLLOUT);
//        std::exit(0);
    }
}
