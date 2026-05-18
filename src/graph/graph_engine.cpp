#include "graph/graph_engine.hpp"
#include <algorithm>

void GraphEngine::add_node(int id, const std::string& label,
                            const std::string& props) {
    graph_.add_node(id, label, props);
    label_index_[label].push_back(id);
}

void GraphEngine::remove_node(int id) {
    auto* meta = graph_.node_meta(id);
    if (meta) {
        auto& ids = label_index_[meta->label];
        ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
    }
    graph_.remove_node(id);
}

void GraphEngine::add_edge(int from, int to, const std::string& rel,
                            float weight) {
    graph_.add_edge(from, to, rel, weight);
}

void GraphEngine::remove_edge(int from, int to, const std::string& rel) {
    graph_.remove_edge(from, to, rel);
}

std::vector<int> GraphEngine::nodes_by_label(const std::string& label) const {
    auto it = label_index_.find(label);
    if (it == label_index_.end()) return {};
    return it->second;
}

const CSRGraph& GraphEngine::csr() {
    if (graph_.is_dirty()) compact();
    return csr_;
}

void GraphEngine::compact() {
    csr_ = build_csr(graph_);
    graph_.clear_dirty();
}