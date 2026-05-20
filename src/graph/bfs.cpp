#include "graph/bfs.hpp"
#include <queue>
#include <unordered_map>
#include <algorithm>

PathResult bfs_path(const DynamicGraph& g, int source, int target,
                    const std::string& rel_filter, Arena* arena) {
    if (source == target) return {{source}, {}};

    int max_node_id = g.max_node_id();

    int* parent = arena 
        ? arena->alloc_array<int>(max_node_id + 1)
        : new int[max_node_id + 1];

    const std::string** via_rel = arena 
        ? arena->alloc_array<const std::string*>(max_node_id + 1)
        : new const std::string*[max_node_id + 1];

    bool* visited = arena
        ? arena->alloc_array<bool>(max_node_id + 1)
        : new bool[max_node_id + 1]();

    if (arena) {
        std::fill(visited, visited + max_node_id + 1, false);
    } else {
        std::fill(visited, visited + max_node_id + 1, false);
    }
    
    // Custom queue using arena
    int* my_queue = arena 
        ? arena->alloc_array<int>(max_node_id + 1)
        : new int[max_node_id + 1];
    
    int head = 0, tail = 0;

    my_queue[tail++] = source;
    visited[source] = true;
    parent[source]  = -1;

    PathResult path;

    while (head < tail) {
        int node = my_queue[head++];

        for (auto& edge : g.neighbors(node)) {
            if (!rel_filter.empty() && edge.rel_type != rel_filter) continue;
            if (visited[edge.to]) continue;
            visited[edge.to] = true;
            parent[edge.to]  = node;
            via_rel[edge.to] = &edge.rel_type;

            if (edge.to == target) {
                for (int cur = target; cur != -1; cur = parent[cur]) {
                    path.nodes.push_back(cur);
                    if (parent[cur] != -1)
                        path.rels.push_back(*via_rel[cur]);
                }
                std::reverse(path.nodes.begin(), path.nodes.end());
                std::reverse(path.rels.begin(),  path.rels.end());
                break;
            }
            my_queue[tail++] = edge.to;
        }
        if (!path.empty()) break;
    }

    if (!arena) {
        delete[] parent;
        delete[] via_rel;
        delete[] visited;
        delete[] my_queue;
    }

    return path;
}

std::vector<HopResult> bfs_neighborhood(const DynamicGraph& g, int source,
                                         int max_hops,
                                         const std::string& rel_filter) {
    std::vector<HopResult>       result;
    std::unordered_map<int,int>  visited;
    std::queue<std::pair<int,int>> queue;

    queue.push({source, 0});
    visited[source] = 0;

    while (!queue.empty()) {
        auto [node, hop] = queue.front(); queue.pop();
        if (hop >= max_hops) continue;

        for (auto& edge : g.neighbors(node)) {
            if (!rel_filter.empty() && edge.rel_type != rel_filter) continue;
            if (visited.count(edge.to)) continue;
            visited[edge.to] = hop + 1;
            result.push_back({edge.to, hop + 1, node, edge.rel_type});
            queue.push({edge.to, hop + 1});
        }
    }
    return result;
}

std::vector<int> bfs_component(const DynamicGraph& g, int source) {
    std::vector<int>             component;
    std::unordered_map<int,bool> visited;
    std::queue<int>              queue;

    queue.push(source);
    visited[source] = true;

    while (!queue.empty()) {
        int node = queue.front(); queue.pop();
        component.push_back(node);
        for (auto& e : g.neighbors(node)) {
            if (!visited[e.to]) {
                visited[e.to] = true;
                queue.push(e.to);
            }
        }
    }
    return component;
}

PathResult bfs_path_csr(const CSRGraph& csr, int source_id, int target_id) {
    int src = csr.to_idx(source_id);
    int dst = csr.to_idx(target_id);
    if (src < 0 || dst < 0) return {};
    if (src == dst) return {{source_id}, {}};

    int n = csr.node_count;
    std::vector<int>  parent(n, -1);
    std::vector<bool> visited(n, false);
    std::vector<int>  queue(n);
    int head = 0, tail = 0;

    queue[tail++] = src;
    visited[src]  = true;

    while (head < tail) {
        int node = queue[head++];
        auto [begin, end] = csr.neighbor_range(node);
        for (int e = begin; e < end; e++) {
            int nb = csr.targets[e];
            if (visited[nb]) continue;
            visited[nb] = true;
            parent[nb]  = node;
            if (nb == dst) {
                PathResult path;
                for (int cur = dst; cur != -1; cur = parent[cur])
                    path.nodes.push_back(csr.idx_to_id[cur]);
                std::reverse(path.nodes.begin(), path.nodes.end());
                return path;
            }
            queue[tail++] = nb;
        }
    }
    return {};
}