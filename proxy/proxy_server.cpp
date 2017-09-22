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
    } catch (std::runtime_error& e) {
        std::cout << e.what();
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
    queue.invalidate_event(ev.data.fd, ev.events);
    queue.delete_event(ev);
    clients.erase(ev.data.fd);
}

void proxy_server::disconnect_server(int fd) {
    // todo
    std::cout << "Disconnect server[" << fd << "]" << std::endl;
    servers.erase(fd);
}

void proxy_server::read_from_client(struct epoll_event& ev) {
    struct client_t* client = clients.at(ev.data.fd).get();
    client->read_request();

    if (client->is_bad_request()) {
        std::cout << "BAD_REQUEST" << std::endl;
        // todo send BAD_REQUEST to client
    }

    if (!client->is_ready_to_send()) return;

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
    if (client->get_request()->get_state() != RESOLVED) {
        std::cout << "INRESOLVED" << std::endl;
        disconnect_client(ev);
        return;
    }
    sockaddr server_addr = std::move(client->get_server_addr());
    struct server_t* server;

    try {
        server = new struct server_t(server_addr);
        servers[server->get_fd()] = server;
    } catch (...) {
        perror("create server");
        std::cout << "Can't create server socket." << std::endl;
    }

    client->bind(server);
    client->move_request_to_server();
    queue.add_event([this](struct epoll_event& ev) {
        this->write_to_server(ev);
    }, client->get_server_fd(), EPOLLOUT);

    {// todo DISABLE CLIENT READ how handle new client request when already has one
//        queue.invalidate_event(ev.data.fd, ev.events);
//        queue.delete_event(ev);
    }
    std::cout << "Add task WRITE_TO_SERVER" << std::endl;
}

void proxy_server::write_to_server(epoll_event& ev) {
    std::cout << "Writing to server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    server->write_request();
    std::cout << "is all?" << std::endl;
    if (server->is_sent_all_request()) {
        std::cout << "modify event for {reading from server} " << std::endl;
        queue.modify_event([this](struct epoll_event& ev) {
            this->read_from_server(ev);
        }, ev, EPOLLIN);
    }
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
    server_t* server = servers.at(ev.data.fd);

    server->read_response();
    if (server->get_response() && server->get_response()->is_obtained()) {
//        std::string chunk = server->get_response()->get_next_data_to_send();
//        std::cout << chunk.substr(0, 205) << chunk.size() << std::endl;
//        std::cout << server->get_buffer().substr(0, 210) << std::endl;

//        std::cout << "inner" << std::endl;
        //        {
//            std::cout << "delete event{read from server} " << std::endl;
//            ev.events = EPOLLIN;

//            queue.modify_event(ev);
//        }

//        server->write_to_client();

        {
            // delete server fd from epoll for future constistency


            queue.invalidate_event(ev.data.fd, ev.events);
            queue.delete_event(ev);

//            queue.modify_event([this](struct epoll_event& ev) {
//                this->write_to_client(ev);
//            }, ev, EPOLLOUT);
            queue.modify_event([this](struct epoll_event& ev) {
                this->write_to_client(ev);
            }, server->get_client_fd(), EPOLLIN, EPOLLOUT);
        }
    }
//    exit();
    std::cout << "readed form server" << std::endl;
}



void proxy_server::write_to_client(struct epoll_event& ev) {
    std::cout << "Writing to client" << std::endl;

    if (ev.events & EPOLLERR) {
        disconnect_client(ev);
        return;
    }
    if (ev.events & EPOLLHUP) {
        disconnect_client(ev);
        return;
    }


    struct client_t* client = clients.at(ev.data.fd).get();
    struct server_t* server = client->get_server();

    if (!server->get_response()->has_data_to_send()) {
        std::cout << "empty" << std::endl;
        return;
    }

    std::string chunk = server->get_response()->get_next_data_to_send();
    size_t delta = client->write_response(chunk);
//    std::cout << "chunk:{\n" << chunk << "}\n size: " <<  chunk.size() << "\ndelta: " << delta << std::endl;
    std::cout << "chunk size: " <<  chunk.size() << "\ndelta: " << delta << std::endl;

    //    std::cout << "move" << std::endl;
    server->move_response_offset(delta);

    std::cout << "check" << std::endl;

    if (server->get_response()->is_passed()) {
        std::cout << "response passed" << std::endl;

        queue.invalidate_event(ev.data.fd, ev.events);
        queue.delete_event(ev);

//        queue.add_event([this](struct epoll_event& ev) {
//            this->read_from_client(ev);
//        }, client->get_fd(), EPOLLIN);
        disconnect_server(server->get_fd());
        client->unbind();
        disconnect_client(ev);

    }

//    std::cout << "end" << std::endl;
}
