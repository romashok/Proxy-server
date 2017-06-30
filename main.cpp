#include <iostream>
#include "proxy/proxy_server.h"

int main()
{
    proxy_server server(2017);
    server.run();

    return 0;
}

