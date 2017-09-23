#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "proxy_server.h"
#include "http/http_request.h"

#include "socket_api.h"
#include "utils.h"

proxy_server::proxy_server(uint32_t port):
    proxy_socket(socket_api::create_main_socket(port)),
    queue(),
    resolver(),
    is_working(true)
{
    std::cout << "Main socket established on fd[" << proxy_socket.get_fd() << "]." << std::endl;

    queue.create_events([this](struct epoll_event& ev) {
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
    } catch (std::runtime_error& e) {
        std::cout << e.what();
    }
}

void proxy_server::connect_client() {
    std::cout << "===================================================================\nConnect client! ";
    client_t * new_client = new client_t(socket_api::accept(proxy_socket.get_fd()));
    clients[new_client->get_fd()] = std::move(std::unique_ptr<client_t>(new_client));
    std::cout << "Client fd [" << new_client->get_fd() << "], total amount now is " << clients.size() << std::endl;

    queue.create_events([this](struct epoll_event& ev) {
        this->read_from_client(ev);
    }, new_client->get_fd(), EPOLLIN);
}

void proxy_server::disconnect_client(int fd) {
    // todo
    std::cout << "Disconnect client [" << fd << "]" << std::endl;
    queue.delete_events_of_fd(fd);
    clients.erase(fd);
}

void proxy_server::disconnect_server(int fd) {
    // todo
    std::cout << "Disconnect server[" << fd << "]" << std::endl;
    queue.delete_events_of_fd(fd);
    servers.erase(fd);
}

void proxy_server::read_from_client(struct epoll_event& ev) {
    struct client_t* client = clients.at(ev.data.fd).get();
    client->read_request();

    if (!client->has_request()) return;

    if (client->is_bad_request()) {
        std::cout << "BAD_REQUEST" << std::endl;
        // todo send BAD_REQUEST to client
    }

    if (!client->has_data_to_send()) return;

    std::cout << "REQ:{\n" << client->get_request()->get_next_data_to_send()  << "}" << std::endl;

    if (client->has_right_server()) {
        std::cout << "send data to server" << std::endl;
        // todo send data to server
        client->move_request_to_server();
        // todo create -> merge
        queue.create_events([this](struct epoll_event& ev) {
            this->write_to_server(ev);
        }, client->get_server_fd(), EPOLLOUT);
        return;
    } else {
        client->unbind();
    }


    bool is_resolved = resolver.resolve(client);

    // on resolve
    if (!is_resolved) {
        std::cout << "INRESOLVED" << std::endl;
        disconnect_client(client->get_fd());
        return;
    }
    sockaddr server_addr = std::move(client->get_server_addr());
    struct server_t* server;

    try {
        server = new struct server_t(server_addr);
        servers[server->get_fd()] = server;
    } catch (...) {
        perror("Can't create server socket");
    }

    client->bind(server);
    client->move_request_to_server();
    queue.create_events([this](struct epoll_event& ev) {
        this->write_to_server(ev);
    }, client->get_server_fd(), EPOLLOUT);
}

void proxy_server::write_to_server(epoll_event& ev) {
    std::cout << "Writing to server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    server->write_request();
    if (server->is_request_passed()) {
        queue.invalidate_event(server->get_fd(), EPOLLOUT);
        queue.reset_to_events([this](struct epoll_event& ev) {
            this->read_from_server(ev);
        }, server->get_fd(), EPOLLIN);
    }
}

//bool body_response(std::string& str) {
//    size_t j = str.find("\r\n\r\n");
//    if (j == std::string::npos) return false;

//    size_t i = str.find("Content-Length: ");
//    i += 16;
//    int content_length = 0;
//    while (str[i] != '\r') {
//        content_length *= 10;
//        content_length += (str[i++] - '0');
//    }

//    std::string body = str.substr(j + 4);
//    return body.size() == content_length;
//}

void proxy_server::read_from_server(struct epoll_event& ev) {
    std::cout << "Reading from server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    server->read_response();
    if (server->get_response() && server->get_response()->is_obtained()) {
            queue.invalidate_event(server->get_fd(), EPOLLIN);
            queue.delete_events_of_fd(server->get_fd());

            queue.merge_events([this](struct epoll_event& ev) {
                this->write_to_client(ev);
            }, server->get_client_fd(), EPOLLOUT, EPOLLIN);
    }
}



void proxy_server::write_to_client(struct epoll_event& ev) {
    std::cout << "Writing to client" << std::endl;

    struct client_t* client = clients.at(ev.data.fd).get();
    struct server_t* server = client->get_server();

    if (ev.events & EPOLLERR || ev.events & EPOLLHUP) {
        std::cout << events_to_str(ev.events) << std::endl;
        disconnect_server(server->get_fd());
        disconnect_client(client->get_fd());
        return;
    }


    if (!server->get_response()->has_data_to_send()) {
        std::cout << "empty" << std::endl;
        return;
    }

    std::string chunk = server->get_response()->get_next_data_to_send();
    size_t delta = client->write_response(chunk);
    std::cout << "chunk size: " <<  chunk.size() << "\ndelta: " << delta << std::endl;
    server->move_response_offset(delta);

    if (server->get_response()->is_passed()) {
        std::cout << "response passed" << std::endl;
        queue.invalidate_event(client->get_fd(), EPOLLOUT);
        queue.reset_to_events([this](struct epoll_event& ev) {
            this->read_from_client(ev);
        }, client->get_fd(), EPOLLIN);

        disconnect_server(server->get_fd());
        disconnect_client(client->get_fd());
        client->unbind();
    }
}
