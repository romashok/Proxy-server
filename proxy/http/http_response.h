#pragma once

#include <string>

struct http_response {
    http_response()=default;

    virtual void append_data(std::string const& data)=0;
    virtual std::string get_next_data_to_send() const =0;
    virtual void move_offset(size_t delta)=0;

    virtual bool has_data_to_send() const noexcept =0 ;
    virtual bool is_obtained() const noexcept;
    virtual bool is_passed() const noexcept;

    virtual ~http_response()=default;

protected:
    // Plaint text of response
    std::string text;

    bool full_header, full_body;
    bool passed;

    virtual bool is_header_obtained() const noexcept;
    virtual bool is_body_obtained() const noexcept;
};
