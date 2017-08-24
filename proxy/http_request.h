#pragma once

#include<string>


struct http_request {
    http_request(std::string& str);
    http_request(const http_request& request);

    static bool is_complete_request(const std::string& str);

    std::string get_host() const noexcept;
private:
    std::string header;
    std::string host;
    std::string path;

    void parse();
    bool has_host();
    void extract_relative_url();
};

