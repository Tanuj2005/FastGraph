#pragma once
#include "graph/dynamic_graph.hpp"
#include "graph/csr_graph.hpp"
#include <vector>
#include <string>
#include <unordered_map>

struct WeightedPath {
    std::vector<int>         nodes;
    std::vector<std::string> rels;
    float                    cost = 0.0f;
    bool empty() const { return nodes.empty(); }
};

struct DistanceMap {
    std::unordered_map<int,float> dist;
    std::unordered_map<int,int>   parent;
};

// Single target — early exit
WeightedPath dijkstra_path(const DynamicGraph& g, int source, int target,
                            const std::string& rel_filter = "");

// All distances from source
DistanceMap dijkstra_all(const DynamicGraph& g, int source);

// CSR variant
WeightedPath dijkstra_path_csr(const CSRGraph& csr,
                                int source_id, int target_id);