#pragma once
#include "kv/kv_engine.hpp"
#include <string>
#include <vector>

class Dispatcher {
public:
    explicit Dispatcher(KVEngine& kv) : kv_(kv) {}
    std::string dispatch(const std::vector<std::string>& args);

private:
    std::string handle_kv(const std::string& cmd,
                          const std::vector<std::string>& args);
    KVEngine& kv_;
};