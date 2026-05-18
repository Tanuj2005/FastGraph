#include "dispatch/dispatcher.hpp"
#include "protocol/encoder.hpp"
#include <algorithm>
#include <cctype>

static std::string to_upper(std::string s) {
    for (auto& c : s) c = toupper(c);
    return s;
}

std::string Dispatcher::dispatch(const std::vector<std::string>& args) {
    if (args.empty()) return RespEncoder::error("empty command");
    return handle_kv(to_upper(args[0]), args);
}

std::string Dispatcher::handle_kv(const std::string& cmd,
                                   const std::vector<std::string>& args) {
    if (cmd == "PING")
        return RespEncoder::simple_string(
            args.size() >= 2 ? args[1] : "PONG");

    if (cmd == "SET" && args.size() >= 3) {
        Ms ttl_ms = -1;
        for (size_t i = 3; i + 1 < args.size(); i++) {
            std::string opt = to_upper(args[i]);
            if (opt == "EX")  ttl_ms = std::stoll(args[i+1]) * 1000;
            if (opt == "PX")  ttl_ms = std::stoll(args[i+1]);
        }
        kv_.set(args[1], args[2], ttl_ms);
        return RespEncoder::simple_string("OK");
    }

    if (cmd == "GET" && args.size() >= 2) {
        auto v = kv_.get(args[1]);
        return v ? RespEncoder::bulk_string(*v) : RespEncoder::null_bulk();
    }

    if (cmd == "DEL" && args.size() >= 2) {
        return RespEncoder::integer(kv_.del(args[1]) ? 1 : 0);
    }

    if (cmd == "EXISTS" && args.size() >= 2) {
        return RespEncoder::integer(kv_.exists(args[1]) ? 1 : 0);
    }

    if (cmd == "EXPIRE" && args.size() >= 3) {
        bool ok = kv_.expire(args[1], std::stoll(args[2]) * 1000);
        return RespEncoder::integer(ok ? 1 : 0);
    }

    if (cmd == "PEXPIRE" && args.size() >= 3) {
        bool ok = kv_.expire(args[1], std::stoll(args[2]));
        return RespEncoder::integer(ok ? 1 : 0);
    }

    if (cmd == "TTL" && args.size() >= 2) {
        Ms ms = kv_.ttl(args[1]);
        if (ms == -2) return RespEncoder::integer(-2);
        if (ms == -1) return RespEncoder::integer(-1);
        return RespEncoder::integer((ms + 999) / 1000);
    }

    if (cmd == "PTTL" && args.size() >= 2) {
        return RespEncoder::integer(kv_.ttl(args[1]));
    }

    if (cmd == "PERSIST" && args.size() >= 2) {
        return RespEncoder::integer(kv_.persist(args[1]) ? 1 : 0);
    }

    if (cmd == "DBSIZE")
        return RespEncoder::integer((long long)kv_.size());

    if (cmd == "COMMAND")
        return RespEncoder::simple_string("OK");

    return RespEncoder::error("unknown command '" + args[0] + "'");
}