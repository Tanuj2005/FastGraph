#include "graph/dfs.hpp"
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <functional>

std::vector<int> dfs_component(const DynamicGraph& g, int source) {
    std::vector<int>             component;
    std::unordered_map<int,bool> visited;
    std::stack<int>              stack;

    stack.push(source);
    while (!stack.empty()) {
        int node = stack.top(); stack.pop();
        if (visited[node]) continue;
        visited[node] = true;
        component.push_back(node);
        for (auto& e : g.neighbors(node))
            if (!visited[e.to]) stack.push(e.to);
    }
    return component;
}

bool dfs_has_cycle(const DynamicGraph& g, int num_nodes) {
    // 0=white, 1=gray, 2=black
    std::unordered_map<int,int> color;

    std::function<bool(int)> visit = [&](int node) -> bool {
        color[node] = 1;
        for (auto& e : g.neighbors(node)) {
            if (color[e.to] == 1) return true;   // back edge
            if (color[e.to] == 0 && visit(e.to)) return true;
        }
        color[node] = 2;
        return false;
    };

    for (auto& [id, _] : g.all_meta()) {
        if (color[id] == 0)
            if (visit(id)) return true;
    }
    return false;
}

std::vector<int> dfs_topo_sort(const DynamicGraph& g,
                                const std::vector<int>& node_ids) {
    std::unordered_map<int,bool> visited;
    std::vector<int>             result;

    std::function<void(int)> visit = [&](int node) {
        visited[node] = true;
        for (auto& e : g.neighbors(node))
            if (!visited[e.to]) visit(e.to);
        result.push_back(node);
    };

    for (int id : node_ids)
        if (!visited[id]) visit(id);

    std::reverse(result.begin(), result.end());
    return result;
}