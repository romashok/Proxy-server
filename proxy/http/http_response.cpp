#include "http_response.h"

bool http_response::is_obtained() const noexcept {
    return is_header_obtained() && is_body_obtained();
}

bool http_response::is_passed() const noexcept {
    return passed;
}

bool http_response::is_header_obtained() const noexcept {
    return full_header;
}

bool http_response::is_body_obtained() const noexcept {
    return full_body;
}
