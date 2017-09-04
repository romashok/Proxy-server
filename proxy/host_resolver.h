#pragma once

#include "client.h"

struct client_t;

struct host_resolver {
    host_resolver();

    void resolve(client_t* client);
};
