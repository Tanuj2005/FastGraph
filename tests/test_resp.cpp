#include <cassert>
#include <cstdio>
#include "protocol/resp.hpp"
#include "protocol/encoder.hpp"

void test_parse_array() {
    RespParser parser;
    std::vector<std::string> args;

    // Full command
    std::string buf = "*3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n";
    auto res = parser.parse(buf, args);
    assert(res == ParseResult::Complete);
    assert(args[0] == "SET");
    assert(args[1] == "foo");
    assert(args[2] == "bar");
    assert(buf.empty());

    // Two commands in one buffer
    std::string buf2 = "*1\r\n$4\r\nPING\r\n*1\r\n$4\r\nPING\r\n";
    auto r1 = parser.parse(buf2, args);
    assert(r1 == ParseResult::Complete);
    assert(args[0] == "PING");
    auto r2 = parser.parse(buf2, args);
    assert(r2 == ParseResult::Complete);
    assert(args[0] == "PING");

    printf("parse_array: OK\n");
}

void test_partial_read() {
    RespParser parser;
    std::vector<std::string> args;

    // Split mid-message
    std::string buf = "*3\r\n$3\r\nSET\r\n";
    auto res = parser.parse(buf, args);
    assert(res == ParseResult::Incomplete);

    // Complete it
    buf += "$3\r\nfoo\r\n$3\r\nbar\r\n";
    res = parser.parse(buf, args);
    assert(res == ParseResult::Complete);
    assert(args[1] == "foo");

    printf("partial_read: OK\n");
}

void test_inline() {
    RespParser parser;
    std::vector<std::string> args;

    std::string buf = "PING\r\n";
    auto res = parser.parse(buf, args);
    assert(res == ParseResult::Complete);
    assert(args[0] == "PING");

    printf("inline: OK\n");
}

void test_encoder() {
    assert(RespEncoder::simple_string("OK")  == "+OK\r\n");
    assert(RespEncoder::error("bad cmd")     == "-ERR bad cmd\r\n");
    assert(RespEncoder::integer(42)          == ":42\r\n");
    assert(RespEncoder::null_bulk()          == "$-1\r\n");
    assert(RespEncoder::bulk_string("hello") == "$5\r\nhello\r\n");
    assert(RespEncoder::empty_array()        == "*0\r\n");

    std::vector<std::string> items = {"foo", "bar"};
    std::string arr = RespEncoder::array(items);
    assert(arr == "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n");

    printf("encoder: OK\n");
}

int main() {
    test_parse_array();
    test_partial_read();
    test_inline();
    test_encoder();
    printf("Phase 2: all protocol tests passed\n");
    return 0;
}