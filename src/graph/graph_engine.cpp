#include "graph/graph_engine.hpp"
#include <algorithm>
#include "index/property_index.hpp"

void GraphEngine::add_node(int id, const std::string& label,
                            const std::string& props) {
    const char* interned = label_pool_.intern(label);
    graph_.add_node(id, interned, props);
}

void GraphEngine::remove_node(int id) {
    auto* meta = graph_.node_meta(id);
    if (meta) {
        prop_index_.remove_node(meta->label_ptr, id);  // ← add this line
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
    const char* target_ptr = label_pool_.intern(label); 
    std::vector<int> result;
    for (const auto& [id, meta] : graph_.all_meta()) {
        if (meta.label_ptr == target_ptr) {
            result.push_back(id);
        }
    }
    return result;
}

const CSRGraph& GraphEngine::csr() {
    if (graph_.is_dirty()) compact();
    return csr_;
}

void GraphEngine::compact() {
    csr_ = build_csr(graph_);
    graph_.clear_dirty();
}

// Add to bottom of graph_engine.cpp

std::vector<int> GraphEngine::range_query(const std::string& label,
                                           const std::string& prop,
                                           double min_val,
                                           double max_val) const {
    return prop_index_.range(label, prop, min_val, max_val);
}

std::vector<int> GraphEngine::exact_query(const std::string& label,
                                           const std::string& prop,
                                           double value) const {
    return prop_index_.exact(label, prop, value);
}

void GraphEngine::index_property(const std::string& label,
                                  const std::string& prop,
                                  double value, int node_id) {
    prop_index_.add(label, prop, value, node_id);
}

std::vector<std::tuple<int,std::string,std::string>>
GraphEngine::all_nodes() const {
    std::vector<std::tuple<int,std::string,std::string>> result;
    for (auto& [id, meta] : graph_.all_meta())
        result.push_back({id, std::string(meta.label_ptr), meta.props});
    return result;
}

std::vector<std::tuple<int,int,std::string,float>>
GraphEngine::all_edges() const {
    std::vector<std::tuple<int,int,std::string,float>> result;
    for (auto& [from, edges] : graph_.all_adj())
        for (auto& e : edges)
            result.push_back({from, e.to, e.rel_type, e.weight});
    return result;
}