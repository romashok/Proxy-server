#include <iostream>

#include "http_request.h"


http_request::http_request(std::string& str):
    header(str),
    host(),
    path()
{
    parse();
}

http_request::http_request(const http_request &request):
    header(request.header),
    host(request.host),
    path(request.path)
{}

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
    if (!has_host()) return;

    extract_relative_url();
}

bool http_request::has_host() {
    size_t i = header.find("Host:");
    if (i == std::string::npos) {
        std::cout << "Bad request! No host provided!" << std::endl;
        return false;
    }

    i += 6;
    size_t j = header.find("\r\n", i);
    host = header.substr(i, j - i);
    std::cout << "Request host: " << host << std::endl;
    return true;
}

void http_request::extract_relative_url() {
    size_t i = header.find(" ");
    size_t j = header.find(" ", i + 1);

    std::string prefix = header.substr(0, i + 1);
    std::string url = header.substr(i + 1, j - i - 1);
    std::string suffix = header.substr(j);

    i = url.find(host);
    if (i != std::string::npos) {
        url = url.substr(i + host.size());
        path = url;
    } else {
        std::cout << "ERROR: no revative url path" << std::endl;
    }

    header = prefix + path + suffix;
    std::cout << "PATH ={" << url << "}" << std::endl;
    std::cout << "{" << header << "}" << std::endl;
}
