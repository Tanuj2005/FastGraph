#pragma once
#include "kv/kv_engine.hpp"
#include "graph/graph_engine.hpp"
#include <string>
#include <atomic>

class Persistence {
public:
    Persistence(KVEngine& kv, GraphEngine& graph,
                const std::string& path = "fastgraph.rdb")
        : kv_(kv), graph_(graph), path_(path) {}

    // Blocking save — call from thread pool
    bool save();

    // Load on startup — blocking, returns false if no file
    bool load();

    // Fork-based background save — returns immediately
    bool bgsave();

    bool is_saving() const { return saving_.load(); }

private:
    KVEngine&    kv_;
    GraphEngine& graph_;
    std::string  path_;
    std::atomic<bool> saving_{false};
};