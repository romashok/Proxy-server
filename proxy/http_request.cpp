#include <iostream>

#include "http_request.h"


http_request::http_request(std::string& str):
    header(str),
    host()
{
    std::cout << "TODO parse http_request here!" << std::endl;
    parse();
    modify_first_line();
}


http_request::http_request(const http_request &request):
    header(request.header),
    host(request.host)
{}

//http_request::http_request(http_request &&request) {

bool http_request::is_complete_request(const std::string& str) {
    size_t i = str.find("\r\n\r\n");

    if (i == std::string::npos)
        return false;
    std::string method = str.substr(0, 4);
    std::cout <<  method << std::endl;
    if ( method != "POST")
        return true;
    return false;
}

void http_request::parse() {
    size_t i = header.find("Host:");
    if (i == std::string::npos) {
        std::cout << "Bad request! No host provided!" << std::endl;
        return;
    }

    i += 6;
    size_t j = header.find("\r\n", i);
    host = header.substr(i, j - i);
    std::cout << "Request host: " << host << std::endl;
}

void http_request::modify_first_line() {
    size_t i = header.find("\r\n");
    i += 2;

    std::string first_line = header.substr(0, i);
    header = header.substr(i);

    i = first_line.find(" ");

}

std::string http_request::get_relative_url() {
    return "revative header";
}


