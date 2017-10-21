#include <iostream>
#include "proxy/proxy_server.h"
#include <algorithm>
#include <string>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


proxy_server server(2017);

void terminate_server(int sig) {
    std::cout << "terminating server by signal " << sig << std::endl;
    server.stop();
}

void handle_signal(int sig){
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = terminate_server;
    sigaction(sig, &action, NULL);
}

int main()
{
    try {
        handle_signal(SIGINT);
        server.run();
    } catch (std::exception& e) {
        std::cout << "Main: " << e.what() << std::endl;
    }

    return 0;
}

