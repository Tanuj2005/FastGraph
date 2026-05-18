#pragma once
#include <vector>
#include <string>
#include <tuple>

struct CSRGraph {
    int node_count = 0;
    int edge_count = 0;

    // Maps node id → array index (since ids may not be contiguous)
    std::vector<int> id_to_idx;   // id_to_idx[id] = index, -1 if absent
    std::vector<int> idx_to_id;   // idx_to_id[index] = id

    // Core CSR arrays
    std::vector<int>   offsets;   // size = node_count + 1
    std::vector<int>   targets;   // size = edge_count
    std::vector<float> weights;   // size = edge_count

    // Node metadata parallel arrays (SoA)
    std::vector<std::string> labels;
    std::vector<std::string> props;

    // Edge metadata
    std::vector<std::string> rel_types;

    std::pair<int,int> neighbor_range(int idx) const {
        return {offsets[idx], offsets[idx + 1]};
    }

    int degree(int idx) const {
        return offsets[idx + 1] - offsets[idx];
    }

    // -1 if id not in graph
    int to_idx(int id) const {
        if (id < 0 || id >= (int)id_to_idx.size()) return -1;
        return id_to_idx[id];
    }
};