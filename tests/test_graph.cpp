#include <cassert>
#include <cstdio>
#include "graph/graph_engine.hpp"

void test_nodes() {
    GraphEngine g;
    g.add_node(1, "Person", R"({"name":"Alice"})");
    g.add_node(2, "Person", R"({"name":"Bob"})");
    g.add_node(3, "Company", R"({"name":"Acme"})");

    assert(g.has_node(1));
    assert(g.has_node(2));
    assert(!g.has_node(99));

    auto* m = g.node_meta(1);
    assert(m && m->label == "Person");

    auto persons = g.nodes_by_label("Person");
    assert(persons.size() == 2);

    g.remove_node(1);
    assert(!g.has_node(1));
    assert(g.nodes_by_label("Person").size() == 1);

    printf("nodes: OK\n");
}

void test_edges() {
    GraphEngine g;
    g.add_node(1, "Person", "{}");
    g.add_node(2, "Person", "{}");
    g.add_node(3, "Person", "{}");

    g.add_edge(1, 2, "KNOWS", 1.0f);
    g.add_edge(1, 3, "KNOWS", 2.0f);
    g.add_edge(2, 3, "WORKS_WITH", 1.0f);

    assert(g.has_edge(1, 2));
    assert(g.has_edge(1, 3));
    assert(!g.has_edge(3, 1));

    assert(g.neighbors(1).size() == 2);
    assert(g.reverse_neighbors(3).size() == 2);

    g.remove_edge(1, 2, "KNOWS");
    assert(!g.has_edge(1, 2));
    assert(g.neighbors(1).size() == 1);

    printf("edges: OK\n");
}

void test_csr() {
    GraphEngine g;
    g.add_node(1, "Person", "{}");
    g.add_node(2, "Person", "{}");
    g.add_node(3, "Person", "{}");
    g.add_edge(1, 2, "KNOWS");
    g.add_edge(1, 3, "KNOWS");
    g.add_edge(2, 3, "KNOWS");

    const CSRGraph& csr = g.csr();
    assert(csr.node_count == 3);
    assert(csr.edge_count == 3);

    int idx1 = csr.to_idx(1);
    assert(idx1 >= 0);
    assert(csr.degree(idx1) == 2);

    // Add more nodes — CSR should recompact
    g.add_node(4, "Person", "{}");
    g.add_edge(3, 4, "KNOWS");
    const CSRGraph& csr2 = g.csr();
    assert(csr2.node_count == 4);
    assert(csr2.edge_count == 4);

    printf("csr: OK\n");
}

int main() {
    test_nodes();
    test_edges();
    test_csr();
    printf("Phase 5: all graph storage tests passed\n");
    return 0;
}