#include <cassert>
#include <cstdio>
#include "graph/graph_engine.hpp"
#include "graph/bfs.hpp"
#include "graph/dfs.hpp"
#include "graph/dijkstra.hpp"

void build_test_graph(GraphEngine& g) {
    // 1-Person --KNOWS--> 2-Person --KNOWS--> 4-Person
    // 1-Person --KNOWS--> 3-Person --KNOWS--> 4-Person
    // weights: 1->2=1, 1->3=5, 2->4=1, 3->4=1
    g.add_node(1, "Person", R"({"name":"Alice"})");
    g.add_node(2, "Person", R"({"name":"Bob"})");
    g.add_node(3, "Person", R"({"name":"Carol"})");
    g.add_node(4, "Person", R"({"name":"Dave"})");
    g.add_node(5, "Company", R"({"name":"Acme"})");
    g.add_edge(1, 2, "KNOWS",    1.0f);
    g.add_edge(1, 3, "KNOWS",    5.0f);
    g.add_edge(2, 4, "KNOWS",    1.0f);
    g.add_edge(3, 4, "KNOWS",    1.0f);
    g.add_edge(1, 5, "WORKS_AT", 1.0f);
}

void test_bfs() {
    GraphEngine g;
    build_test_graph(g);

    // Path 1->4 should be 1->2->4 (fewest hops)
    auto path = bfs_path(g.graph_ref(), 1, 4);
    assert(!path.empty());
    assert(path.nodes.front() == 1);
    assert(path.nodes.back()  == 4);
    assert(path.nodes.size()  == 3);
    assert(path.nodes[1]      == 2);

    // Filtered — only KNOWS edges
    auto fpath = bfs_path(g.graph_ref(), 1, 4, "KNOWS");
    assert(!fpath.empty());
    assert(fpath.nodes.size() == 3);

    // No path via WORKS_AT to node 4
    auto npath = bfs_path(g.graph_ref(), 1, 4, "WORKS_AT");
    assert(npath.empty());

    // Neighborhood — 2 hops from 1
    auto hood = bfs_neighborhood(g.graph_ref(), 1, 2);
    assert(hood.size() >= 3);  // 2,3,5 at hop1; 4 at hop2

    // Component
    auto comp = bfs_component(g.graph_ref(), 1);
    assert(comp.size() == 5);

    // CSR path
    auto cpath = bfs_path_csr(g.csr(), 1, 4);
    assert(!cpath.empty());
    assert(cpath.nodes.front() == 1 && cpath.nodes.back() == 4);

    printf("bfs: OK\n");
}

void test_dfs() {
    GraphEngine g;
    build_test_graph(g);

    auto comp = dfs_component(g.graph_ref(), 1);
    assert(comp.size() == 5);

    assert(!dfs_has_cycle(g.graph_ref(), 5));

    // Add a cycle
    g.add_edge(4, 1, "KNOWS");
    assert(dfs_has_cycle(g.graph_ref(), 5));

    printf("dfs: OK\n");
}

void test_dijkstra() {
    GraphEngine g;
    build_test_graph(g);

    // Weighted path 1->4: should be 1->2->4 (cost=2) not 1->3->4 (cost=6)
    auto path = dijkstra_path(g.graph_ref(), 1, 4);
    assert(!path.empty());
    assert(path.nodes.front() == 1);
    assert(path.nodes.back()  == 4);
    assert(path.cost == 2.0f);
    assert(path.nodes[1] == 2);

    // All distances from 1
    auto dmap = dijkstra_all(g.graph_ref(), 1);
    assert(dmap.dist[2] == 1.0f);
    assert(dmap.dist[3] == 5.0f);
    assert(dmap.dist[4] == 2.0f);

    // CSR variant
    auto cpath = dijkstra_path_csr(g.csr(), 1, 4);
    assert(!cpath.empty());
    assert(cpath.cost == 2.0f);

    printf("dijkstra: OK\n");
}

int main() {
    test_bfs();
    test_dfs();
    test_dijkstra();
    printf("Phase 6: all algorithm tests passed\n");
    return 0;
}