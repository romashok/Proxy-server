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
    // todo
    std::cout << "Disconnect client [" << ev.data.fd << "]" << std::endl;
    queue.delete_event(ev);
    clients.erase(ev.data.fd);
}

void proxy_server::read_from_client(struct epoll_event& ev) {
    struct client_t* client = clients.at(ev.data.fd).get();
    client->read_request();

    if (client->is_bad_request()) {
        std::cout << "BAD_REQUEST" << std::endl;
        // todo send BAD_REQUEST to client
    }

    if (!client->is_ready()) return;

    std::cout << "REQ:{\n" << client->get_request()->get_raw_text() << "}" << std::endl;

    if (client->has_right_server()) {
        std::cout << "send data to server" << std::endl;
        // todo send data to server
        // todo modify event instead of add???
        client->move_request_to_server();
        queue.add_event([this](struct epoll_event& ev) {
            this->write_to_server(ev);
        }, client->get_server_fd(), EPOLLOUT);
        return;
    } else {
        client->unbind();
    }


    resolver.resolve(client);

    // on resolve
    // assert (state == RESOLVED)
    sockaddr server_addr = std::move(client->get_server_addr());
    struct server_t* server;

    try {
        server = new struct server_t(server_addr);
    } catch (...) {
        perror("create server");
        std::cout << strerror(errno) << "\nError occured! Can't create server socket." << std::endl;
    }

    client->bind(server);
    client->move_request_to_server();
    queue.add_event([this](struct epoll_event& ev) {
        this->write_to_server(ev);
    }, client->get_server_fd(), EPOLLOUT);
    std::cout << "Add task WRITE_TO_SERVER" << std::endl;
}

void proxy_server::write_to_server(epoll_event& ev) {
    std::cout << "Writing to server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    server->write_request();
    if (server->is_sent_all()) {
        std::cout << "modify event for {reading from server} " << std::endl;
        queue.modify_event([this](struct epoll_event& ev) {
            this->read_from_server(ev);
        }, ev, EPOLLIN);
    }
/*
    if (server->get_buffer().empty()) {
        // todo delete event{write to server}
        {
            std::cout << "modify event{write to server} " << std::endl;
            queue.modify_event([this](struct epoll_event& ev) {
                this->read_from_server(ev);
            }, ev, EPOLLIN);
        }

//        queue.add_event([this](struct epoll_event& ev) {
//            this->read_from_server(ev);
//        }, server->get_fd(), EPOLLIN);
    }
*/
}

bool body_response(std::string& str) {
    size_t j = str.find("\r\n\r\n");
    if (j == std::string::npos) return false;

    size_t i = str.find("Content-Length: ");
    i += 16;
    int content_length = 0;
    while (str[i] != '\r') {
        content_length *= 10;
        content_length += (str[i++] - '0');
    }

    std::string body = str.substr(j + 4);
    return body.size() == content_length;
}

void proxy_server::read_from_server(struct epoll_event& ev) {
    std::cout << "Reading from server" << std::endl;
 /*   server_t* server = servers.at(ev.data.fd);

    size_t new_data = server->read();
    std::cout << "get " << new_data << std::endl;
    if (server->get_buffer().find("\r\n0\r\n\r\n") != std::string::npos || body_response(server->get_buffer())) {
//        {
//            std::cout << "delete event{read from server} " << std::endl;
//            ev.events = EPOLLIN;

//            queue.modify_event(ev);
//        }

        std::cout << "ANSWER :{\n" << server->get_buffer() << "}" << std::endl;
//        server->write_to_client();

        {
            // delete server fd from epoll for future constistency
            queue.delete_event(ev);

            struct epoll_event ev;
            ev.data.fd = server->get_client_fd();
            ev.events = EPOLLIN;

            queue.modify_event([this](struct epoll_event& ev) {
                this->write_to_client(ev);
            }, ev, EPOLLOUT);

        }
    }
*/
}



void proxy_server::write_to_client(struct epoll_event& ev) {
    std::cout << "Writing to client" << std::endl;
 /*
    struct client_t* client = clients.at(ev.data.fd).get();
    struct server_t* server = client->get_server();

    std::cout << "-----------------------------------------------------------------------------HAVE " << server->get_buffer().size() << std::endl;
    size_t length = client->write(server->get_buffer());
    std::cout << "-----------------------------------------------------------------------------PASS " << length << std::endl;
    server->get_buffer() = server->get_buffer().substr(length);

    if (server->get_buffer().empty()) {
        std::cout << "server buffer is empty" << std::endl;

        std::cout << "read from client again " << std::endl;

        queue.modify_event([this](struct epoll_event& ev) {
            this->read_from_client(ev);
        }, ev, EPOLLIN);
//        std::exit(0);
    }
*/
}
