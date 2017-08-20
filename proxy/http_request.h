#pragma once

#include<string>


struct http_request {
    http_request(std::string& str);
    http_request(const http_request& request);

    static bool is_complete_request(const std::string& str);
private:
    std::string header;
    std::string host;

    void parse();
    void modify_first_line();
    std::string get_relative_url();
};

