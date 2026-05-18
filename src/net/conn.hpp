#pragma once
#include <string>
#include "protocol/resp.hpp"

enum class ConnState { Reading, Writing, Closing };

struct Conn {
    int        fd;
    ConnState  state = ConnState::Reading;
    std::string read_buf;
    std::string write_buf;
    RespParser  parser;

    explicit Conn(int fd) : fd(fd) {}
};