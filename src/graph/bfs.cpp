#include "graph/bfs.hpp"
#include <queue>
#include <unordered_map>
#include <algorithm>

PathResult bfs_path(const DynamicGraph& g, int source, int target,
                    const std::string& rel_filter) {
    if (source == target) return {{source}, {}};

    std::unordered_map<int,int>         parent;
    std::unordered_map<int,std::string> via_rel;
    std::unordered_map<int,bool>        visited;
    std::queue<int> queue;

    queue.push(source);
    visited[source] = true;
    parent[source]  = -1;

    while (!queue.empty()) {
        int node = queue.front(); queue.pop();

        for (auto& edge : g.neighbors(node)) {
            if (!rel_filter.empty() && edge.rel_type != rel_filter) continue;
            if (visited[edge.to]) continue;
            visited[edge.to] = true;
            parent[edge.to]  = node;
            via_rel[edge.to] = edge.rel_type;

            if (edge.to == target) {
                PathResult path;
                for (int cur = target; cur != -1; cur = parent[cur]) {
                    path.nodes.push_back(cur);
                    if (parent[cur] != -1)
                        path.rels.push_back(via_rel[cur]);
                }
                std::reverse(path.nodes.begin(), path.nodes.end());
                std::reverse(path.rels.begin(),  path.rels.end());
                return path;
            }
            queue.push(edge.to);
        }
    }
    return {};
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