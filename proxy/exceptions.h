#pragma once

#include <exception>
#include <iostream>

struct custom_exception: std::exception {
    custom_exception(const std::string& msg):
        error_reason(msg)
    {}

    custom_exception(std::string&& msg):
        error_reason(msg)
    {}

    std::string reason() const {
        return error_reason;
    }

private:
    std::string error_reason;
};
