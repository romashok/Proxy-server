#include "http_response.h"

bool http_response::is_passed() {
    return passed;
}

bool http_response::is_header_obtained() {
    return full_header;
}

bool http_response::is_body_obtained() {
    return full_body;
}
