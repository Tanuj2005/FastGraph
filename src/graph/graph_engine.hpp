#pragma once
#include "graph/dynamic_graph.hpp"
#include "graph/csr_graph.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class GraphEngine {
public:
    // Node operations
    void add_node(int id, const std::string& label, const std::string& props);
    void remove_node(int id);
    bool has_node(int id) const { return graph_.has_node(id); }
    const NodeMeta* node_meta(int id) const { return graph_.node_meta(id); }

    // Edge operations
    void add_edge(int from, int to, const std::string& rel,
                  float weight = 1.0f);
    void remove_edge(int from, int to, const std::string& rel);
    bool has_edge(int from, int to) const { return graph_.has_edge(from, to); }

    // Neighbor access
    const std::vector<Edge>& neighbors(int id) const {
        return graph_.neighbors(id);
    }
    const std::vector<Edge>& reverse_neighbors(int id) const {
        return graph_.reverse_neighbors(id);
    }

    // Label index
    std::vector<int> nodes_by_label(const std::string& label) const;

    // Get compacted CSR — compacts if dirty
    const CSRGraph& csr();

    int node_count() const { return graph_.node_count(); }
    int edge_count() const { return graph_.edge_count(); }

private:
    void compact();

    DynamicGraph graph_;
    CSRGraph     csr_;

    // Label → node ids index
    std::unordered_map<std::string, std::vector<int>> label_index_;
};