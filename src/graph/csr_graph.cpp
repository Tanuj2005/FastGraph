#include "graph/csr_graph.hpp"
#include "graph/dynamic_graph.hpp"
#include <algorithm>

// Build CSR from DynamicGraph
CSRGraph build_csr(const DynamicGraph& g) {
    CSRGraph csr;

    // Collect all node ids and sort them
    std::vector<int> ids;
    for (auto& [id, _] : g.all_meta()) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    csr.node_count = (int)ids.size();

    // Build id <-> index maps
    int max_id = ids.empty() ? 0 : ids.back();
    csr.id_to_idx.assign(max_id + 1, -1);
    csr.idx_to_id.resize(csr.node_count);
    for (int i = 0; i < csr.node_count; i++) {
        csr.id_to_idx[ids[i]] = i;
        csr.idx_to_id[i]      = ids[i];
    }

    // Count degrees
    std::vector<int> degree(csr.node_count, 0);
    for (auto& [id, edges] : g.all_adj()) {
        int idx = csr.to_idx(id);
        if (idx >= 0) degree[idx] = (int)edges.size();
    }

    // Prefix sum → offsets
    csr.offsets.resize(csr.node_count + 1);
    csr.offsets[0] = 0;
    for (int i = 0; i < csr.node_count; i++)
        csr.offsets[i + 1] = csr.offsets[i] + degree[i];

    csr.edge_count = csr.offsets[csr.node_count];
    csr.targets.resize(csr.edge_count);
    csr.weights.resize(csr.edge_count);
    csr.rel_types.resize(csr.edge_count);

    // Fill edges
    std::vector<int> cursor = csr.offsets;
    for (auto& [id, edges] : g.all_adj()) {
        int from_idx = csr.to_idx(id);
        if (from_idx < 0) continue;
        for (auto& e : edges) {
            int to_idx = csr.to_idx(e.to);
            if (to_idx < 0) continue;
            int pos = cursor[from_idx]++;
            csr.targets[pos]   = to_idx;
            csr.weights[pos]   = e.weight;
            csr.rel_types[pos] = e.rel_type;
        }
    }

    // Fill node metadata
    csr.labels.resize(csr.node_count);
    csr.props.resize(csr.node_count);
    for (auto& [id, meta] : g.all_meta()) {
        int idx = csr.to_idx(id);
        if (idx < 0) continue;
        csr.labels[idx] = std::string(meta.label_ptr);
        csr.props[idx]  = meta.props;
    }

    return csr;
}