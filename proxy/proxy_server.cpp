#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <tuple>

#include "proxy_server.h"
#include "http/http_request.h"

#include "socket_api.h"
#include "utils.h"
#include <sys/signalfd.h>
#include <signal.h>
#include <stdlib.h>


proxy_server::proxy_server(uint32_t port):
    proxy_socket(socket_api::create_main_socket(port)),
    queue(),
    resolver(),
    is_working(true)
{
    std::cout << "Main socket established on fd[" << proxy_socket.get_fd() << "]." << std::endl;

    if (!resolver.is_active()) {
        is_working = false;
        std::cout << "Shut down server" << std::endl;
        return;
    }
    std::cout << "EventFd on fd[" << resolver.get_eventfd() << "]." << std::endl;

    queue.create_events([this](struct epoll_event& ev) {
        this->on_host_resolved(ev);
    }, resolver.get_eventfd(), EPOLLIN);

    queue.create_events([this](struct epoll_event&) {
        this->connect_client();
    }, proxy_socket.get_fd(), EPOLLIN);
}


void proxy_server::run() {
    std::cout << "Start proxy server!" << std::endl;

    while(is_working) {
        int amount = queue.get_events_amount();

        if (amount == -1) {
            perror("Error occured during getting new epoll events!");
            continue;
        }

        queue.handle_events(amount);
    }
    std::cout << "Server is not working" << std::endl;
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
    std::cout << "Disconnect client [" << fd << "]" << std::endl;

    // todo delete server`s handlers if has server
    struct client_t* client = clients.at(fd).get();
    if (client->has_server()) {
        disconnect_server(client->get_server_fd());
    }

    clients.erase(fd);
}

void proxy_server::disconnect_server(int fd) {
    std::cout << "Disconnect server [" << fd << "]" << std::endl;
    servers.erase(fd);
}

void proxy_server::read_from_client(struct epoll_event& ev) {
//    std::cout << "Read from client" << std::endl;
    struct client_t* client = clients.at(ev.data.fd).get();

    if (ev.events & EPOLLERR || ev.events & EPOLLHUP) {
        std::cout << "Client read event error " << events_to_str(ev.events) << std::endl;
        queue.delete_fd_from_epoll(client->get_fd());
        disconnect_client(client->get_fd());
        return;
    }

    try {
        client->read_request();
    } catch (...) {
        std::cout << "Force client [" << client->get_fd() << "] disconnect" << std::endl;
        queue.delete_fd_from_epoll(client->get_fd());
        disconnect_client(client->get_fd());
        return;
    }

    // not full header
    if (!client->has_request()) {
//        std::cout << "not full header" << std::endl;
        if (client->get_buffer().empty()) {
            std::cout << "disconnect strange client with empty request" << std::endl;
            if (client->has_server()) {
                std::cout << "has server?" << std::endl;
                int server_fd = client->get_server()->get_fd();
                queue.delete_events_of_fd(server_fd);
                disconnect_server(server_fd);
            }
            queue.delete_events_of_fd(client->get_fd());
            disconnect_client(client->get_fd());
        }

//        std::cout << "{" << client->get_buffer() << "}" << std::endl;
        return;
    }

    if (client->is_bad_request()) {
        std::cout << "BAD_REQUEST" << std::endl;
        exit();
        // todo send BAD_REQUEST to client
    }

    if (!client->has_data_to_send()) {
//        std::cout << "no data to send" << std::endl;
        return;
    }

//    std::cout << "REQ:{\n" << client->get_request()->get_next_data_to_send()  << "}" << std::endl;

    // todo set host to server_t
    /*
    if (client->has_right_server()) {
        std::cout << "send data to server" << std::endl;
        client->move_request_to_server();
        // todo chould be in reading state
        return;
    } else {
        client->unbind();
    }
    */

    client->unbind();
    resolver.push_host(std::make_tuple(client->get_fd(), client->get_request_host()));
}

void proxy_server::on_host_resolved(struct epoll_event&) {
//    std::cout << "on_host_resolved" << std::endl;

    bool ok;
    int client_fd;
    std::string host;
    sockaddr server_addr;
    std::tie (ok, client_fd, host, server_addr) = resolver.pop_resolved_host();
//    std::cout << "client: " << client_fd << " host: " << host << std::endl;

    if (!clients.count(client_fd)) {
        std::cout << "Orphaned host: client was deleted" << std::endl;
        return;
    }

    struct client_t* client = clients.at(client_fd).get();

    if (!client->has_request() || client->get_request_host() != host) {
        std::cout << "Another host was expected" << std::endl;
        return;
    }

    if (client->has_server()) {
        std::cout << "Client already has server" << std::endl;
        return;
    }

    if (!ok) {
        std::cout << "Client disconnected because of resolve error" << std::endl;
        queue.delete_fd_from_epoll(client_fd);
        disconnect_client(client_fd);
        return;
    }

    struct server_t* server;
    try {
        server = new struct server_t(server_addr);
        servers[server->get_fd()] = server;
    } catch (...) {
        perror("Can't create server socket");
    }

    std::cout << " New server created" << std::endl;

    client->bind(server);
    client->move_request_to_server();
    queue.create_events([this](struct epoll_event& ev) {
        this->write_to_server(ev);
    }, client->get_server_fd(), EPOLLOUT);
}

void proxy_server::write_to_server(epoll_event& ev) {
//    std::cout << "Writing to server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    if (ev.events & EPOLLERR || ev.events & EPOLLHUP) {
        std::cout << "Server write event error " << events_to_str(ev.events) << std::endl;
        queue.delete_fd_from_epoll(server->get_fd());
        disconnect_client(server->get_fd());
        return;
    }

    server->write_request();
    if (server->is_request_passed()) {
//        std::cout << "request passed" << std::endl;
        queue.invalidate_event(server->get_fd(), EPOLLOUT);
        queue.reset_to_events([this](struct epoll_event& ev) {
            this->read_from_server(ev);
        }, server->get_fd(), EPOLLIN);
    } else {
//        std::cout << "want send more" << std::endl;
    }
}

void proxy_server::read_from_server(struct epoll_event& ev) {
//    std::cout << "Reading from server" << std::endl;
    server_t* server = servers.at(ev.data.fd);

    if (ev.events & EPOLLERR || ev.events & EPOLLHUP) {
        // todo write bad request to client
        std::cout << "Server read event error " << events_to_str(ev.events) << std::endl;
        queue.delete_fd_from_epoll(server->get_fd());
        queue.delete_fd_from_epoll(server->get_client_fd());
        disconnect_client(server->get_client_fd());
        return;
    }

    server->read_response();

//    if (server->get_response() && server->get_response()->has_data_to_send()) {
        if (server->get_response() && server->get_response()->is_obtained()) {
        if (server->get_response()->is_obtained()) {
            queue.invalidate_event(server->get_fd(), EPOLLIN);
            queue.delete_fd_from_epoll(server->get_fd());
        }

        queue.merge_events([this](struct epoll_event& ev) {
            this->write_to_client(ev);
        }, server->get_client_fd(), EPOLLOUT, EPOLLIN);
    } else {
//        std::cout << "no enough data of response" << std::endl;
//        std::cout << "{" << server->get_buffer()  << "}" << std::endl;
    }
}

void proxy_server::write_to_client(struct epoll_event& ev) {
//    std::cout << "Writing to client" << std::endl;

    struct client_t* client = clients.at(ev.data.fd).get();
    struct server_t* server = client->get_server();
    if (ev.events & EPOLLERR || ev.events & EPOLLHUP) {
        queue.invalidate_event(client->get_fd(),EPOLLOUT);

        queue.delete_events_of_fd(server->get_fd());
        queue.delete_events_of_fd(client->get_fd());
        disconnect_server(server->get_fd());
        disconnect_client(client->get_fd());
        return;
    }

    if (server->get_response()->is_passed()) {
//        std::cout << "response passed" << std::endl;
        queue.invalidate_event(client->get_fd(), EPOLLOUT);
        queue.reset_to_events([this](struct epoll_event& ev) {
            this->read_from_client(ev);
        }, client->get_fd(), EPOLLIN);

        queue.delete_events_of_fd(server->get_fd());
        queue.delete_events_of_fd(client->get_fd());
        disconnect_server(server->get_fd());
        disconnect_client(client->get_fd());
        return;
    }

    if (!server->get_response()->has_data_to_send()) {
//        std::cout << "empty response" << std::endl;
        return;
    }

//    std::cout << "response: has date to send" << std::endl;

    std::string chunk = server->get_response()->get_next_data_to_send();
    size_t delta = client->write_response(chunk);
    std::cout << "write to client[" << client->get_fd() << "] chunk size:{" <<  chunk.size() << "} delta:{" << delta << "}" << std::endl;
//    std::cout << "{\n" <<  chunk << "\n} " << delta << std::endl;
    server->move_response_offset(delta);

    if (server->get_response()->is_passed()) {
//        std::cout << "response passed" << std::endl;
        queue.invalidate_event(client->get_fd(), EPOLLOUT);
        queue.reset_to_events([this](struct epoll_event& ev) {
            this->read_from_client(ev);
        }, client->get_fd(), EPOLLIN);

        queue.delete_events_of_fd(server->get_fd());
        queue.delete_events_of_fd(client->get_fd());
        disconnect_server(server->get_fd());
        disconnect_client(client->get_fd());
    }
}

void proxy_server::stop() {
    is_working = false;
    std::cout << "Server stoped!" << std::endl;
}
