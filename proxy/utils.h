#pragma once

#include <string>
#include <sys/epoll.h>
#include <iostream>

std::string events_to_str(uint32_t events);

void print_flags();

void print_flags_extended();

void exit();
