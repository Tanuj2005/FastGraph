#include "graph/dijkstra.hpp"
#include <queue>
#include <algorithm>
#include <limits>

static constexpr float INF = std::numeric_limits<float>::infinity();
using Entry = std::pair<float,int>;
using PQ    = std::priority_queue<Entry, std::vector<Entry>,
                                  std::greater<Entry>>;

WeightedPath dijkstra_path(const DynamicGraph& g, int source, int target,
                            const std::string& rel_filter) {
    std::unordered_map<int,float>       dist;
    std::unordered_map<int,int>         parent;
    std::unordered_map<int,std::string> via_rel;
    PQ pq;

    dist[source]   = 0.0f;
    parent[source] = -1;
    pq.push({0.0f, source});

    while (!pq.empty()) {
        auto [cost, node] = pq.top(); pq.pop();

        if (node == target) {
            WeightedPath path;
            path.cost = cost;
            for (int cur = target; cur != -1; cur = parent[cur]) {
                path.nodes.push_back(cur);
                if (parent[cur] != -1)
                    path.rels.push_back(via_rel[cur]);
            }
            std::reverse(path.nodes.begin(), path.nodes.end());
            std::reverse(path.rels.begin(),  path.rels.end());
            return path;
        }

        auto it = dist.find(node);
        if (it != dist.end() && cost > it->second) continue;

        for (auto& edge : g.neighbors(node)) {
            if (!rel_filter.empty() && edge.rel_type != rel_filter) continue;
            float nc = cost + edge.weight;
            auto  dit = dist.find(edge.to);
            if (dit == dist.end() || nc < dit->second) {
                dist[edge.to]    = nc;
                parent[edge.to]  = node;
                via_rel[edge.to] = edge.rel_type;
                pq.push({nc, edge.to});
            }
        }
    }
    return {};
}

DistanceMap dijkstra_all(const DynamicGraph& g, int source) {
    DistanceMap result;
    PQ pq;

    result.dist[source]   = 0.0f;
    result.parent[source] = -1;
    pq.push({0.0f, source});

    while (!pq.empty()) {
        auto [cost, node] = pq.top(); pq.pop();
        if (cost > result.dist[node]) continue;

        for (auto& edge : g.neighbors(node)) {
            float nc  = cost + edge.weight;
            auto  dit = result.dist.find(edge.to);
            if (dit == result.dist.end() || nc < dit->second) {
                result.dist[edge.to]   = nc;
                result.parent[edge.to] = node;
                pq.push({nc, edge.to});
            }
        }
    }
    return result;
}

WeightedPath dijkstra_path_csr(const CSRGraph& csr,
                                int source_id, int target_id) {
    int src = csr.to_idx(source_id);
    int dst = csr.to_idx(target_id);
    if (src < 0 || dst < 0) return {};

    int n = csr.node_count;
    std::vector<float> dist(n, INF);
    std::vector<int>   parent(n, -1);
    std::vector<bool>  visited(n, false);
    PQ pq;

    dist[src] = 0.0f;
    pq.push({0.0f, src});

    while (!pq.empty()) {
        auto [cost, node] = pq.top(); pq.pop();
        if (node == dst) break;
        if (visited[node]) continue;
        visited[node] = true;

        auto [begin, end] = csr.neighbor_range(node);
        for (int e = begin; e < end; e++) {
            int   nb = csr.targets[e];
            float nc = cost + csr.weights[e];
            if (nc < dist[nb]) {
                dist[nb]   = nc;
                parent[nb] = node;
                pq.push({nc, nb});
            }
        }
    }

    if (dist[dst] == INF) return {};

    WeightedPath path;
    path.cost = dist[dst];
    for (int cur = dst; cur != -1; cur = parent[cur])
        path.nodes.push_back(csr.idx_to_id[cur]);
    std::reverse(path.nodes.begin(), path.nodes.end());
    return path;
}