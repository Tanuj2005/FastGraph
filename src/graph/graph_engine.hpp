#pragma once
#include "graph/dynamic_graph.hpp"
#include "graph/csr_graph.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include "index/property_index.hpp"

#include <tuple>

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

    
    const DynamicGraph& graph_ref() const { return graph_; }

    int node_count() const { return graph_.node_count(); }
    int edge_count() const { return graph_.edge_count(); }

    // Range query on numeric property
    std::vector<int> range_query(const std::string& label,
                                const std::string& prop,
                                double min_val, double max_val) const;

    std::vector<int> exact_query(const std::string& label,
                                const std::string& prop,
                                double value) const;

    // Index a numeric property manually
    void index_property(const std::string& label, const std::string& prop,
                        double value, int node_id);

    std::vector<std::tuple<int,std::string,std::string>> all_nodes() const;
    std::vector<std::tuple<int,int,std::string,float>>   all_edges() const;

private:
    void compact();

    DynamicGraph graph_;
    CSRGraph     csr_;

    // Label → node ids index
    std::unordered_map<std::string, std::vector<int>> label_index_;
    PropertyIndex prop_index_;
};