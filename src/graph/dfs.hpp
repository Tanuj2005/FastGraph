#pragma once
#include "graph/dynamic_graph.hpp"
#include <vector>

// All reachable nodes via DFS
std::vector<int> dfs_component(const DynamicGraph& g, int source);

// Cycle detection — true if graph has a cycle
bool dfs_has_cycle(const DynamicGraph& g, int num_nodes);

// Topological sort — returns empty if cycle exists
std::vector<int> dfs_topo_sort(const DynamicGraph& g,
                                const std::vector<int>& node_ids);