#include "protocol/encoder.hpp"

std::string RespEncoder::simple_string(const std::string& s) {
    return "+" + s + "\r\n";
}

std::string RespEncoder::error(const std::string& msg) {
    return "-ERR " + msg + "\r\n";
}

std::string RespEncoder::integer(long long n) {
    return ":" + std::to_string(n) + "\r\n";
}

std::string RespEncoder::bulk_string(const std::string& s) {
    return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n";
}

std::string RespEncoder::null_bulk() {
    return "$-1\r\n";
}

std::string RespEncoder::array(const std::vector<std::string>& items) {
    std::string out = "*" + std::to_string(items.size()) + "\r\n";
    for (auto& item : items) out += bulk_string(item);
    return out;
}

std::string RespEncoder::empty_array() {
    return "*0\r\n";
}