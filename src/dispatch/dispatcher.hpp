#pragma once
#include "kv/kv_engine.hpp"
#include "graph/graph_engine.hpp"
#include "sorted_set/sorted_set_engine.hpp"
#include <string>
#include <vector>

class Dispatcher {
public:
    explicit Dispatcher(KVEngine& kv, GraphEngine& graph, SortedSetEngine& zsets)
        : kv_(kv), graph_(graph), zsets_(zsets) {}
    std::string dispatch(const std::vector<std::string>& args);

private:
    std::string handle_kv(const std::string& cmd,
                          const std::vector<std::string>& args);

    std::string handle_graph(const std::string& cmd,
                              const std::vector<std::string>& args);
    
    std::string handle_zset(const std::string& cmd,
                         const std::vector<std::string>& args);

    KVEngine& kv_;
    GraphEngine& graph_;
    SortedSetEngine& zsets_;
};