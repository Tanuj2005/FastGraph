#pragma once
#include "graph/dynamic_graph.hpp"
#include "graph/csr_graph.hpp"
#include "memory/arena.hpp"
#include <vector>
#include <string>
#include <unordered_map>

struct PathResult {
    std::vector<int>         nodes;
    std::vector<std::string> rels;
    bool empty() const { return nodes.empty(); }
};

struct HopResult {
    int         node;
    int         hop;
    int         via;
    std::string rel_type;
};

// Shortest path (unweighted) — early exit at target
PathResult bfs_path(const DynamicGraph& g, int source, int target,
                    const std::string& rel_filter = "",
                    Arena* arena = nullptr);

// All nodes within max_hops of source
std::vector<HopResult> bfs_neighborhood(const DynamicGraph& g, int source,
                                         int max_hops,
                                         const std::string& rel_filter = "");

// All reachable nodes from source
std::vector<int> bfs_component(const DynamicGraph& g, int source);

// CSR variant — faster for bulk traversal
PathResult bfs_path_csr(const CSRGraph& csr, int source_id, int target_id);