#include <cstdio>
#include <chrono>
#include <functional>
#include <string>
#include "graph/graph_engine.hpp"
#include "graph/bfs.hpp"
#include "graph/dijkstra.hpp"
#include "graph/dfs.hpp"

using Clock = std::chrono::high_resolution_clock;

static double bench(const char* name, int n,
                    std::function<void()> fn) {
    auto t1 = Clock::now();
    for (int i = 0; i < n; i++) fn();
    auto t2   = Clock::now();
    double ms = std::chrono::duration<double,
                std::milli>(t2 - t1).count();
    double ops = (n / ms) * 1000.0;
    printf("%-35s %8.0f ops/sec  (%6.1f ms)\n", name, ops, ms);
    return ops;
}

// Build a random graph with num_nodes nodes and avg_degree edges per node
void build_random_graph(GraphEngine& g, int num_nodes, int avg_degree) {
    for (int i = 0; i < num_nodes; i++)
        g.add_node(i, "Person", "{}");

    std::mt19937 rng(42);
    int edges = num_nodes * avg_degree;
    for (int i = 0; i < edges; i++) {
        int from   = rng() % num_nodes;
        int to     = rng() % num_nodes;
        float w    = 1.0f + (rng() % 10);
        g.add_edge(from, to, "KNOWS", w);
    }
}

int main() {
    printf("=== GraphEngine microbenchmarks ===\n\n");

    // Small graph — 1k nodes, degree 10
    {
        printf("-- Small graph (1k nodes, deg=10) --\n");
        GraphEngine g;
        build_random_graph(g, 1000, 10);

        bench("add_node x1000", 1, [&](){
            // Already done above — just measure CSR compaction
            g.csr();
        });

        bench("BFS path (random src/dst)", 1000, [&](){
            bfs_path(g.graph_ref(), 0, 999);
        });

        bench("Dijkstra path (random)", 1000, [&](){
            dijkstra_path(g.graph_ref(), 0, 999);
        });

        bench("BFS neighborhood hops=2", 1000, [&](){
            bfs_neighborhood(g.graph_ref(), 0, 2);
        });

        bench("DFS component", 1000, [&](){
            dfs_component(g.graph_ref(), 0);
        });

        bench("CSR BFS path", 1000, [&](){
            bfs_path_csr(g.csr(), 0, 999);
        });

        bench("CSR Dijkstra path", 1000, [&](){
            dijkstra_path_csr(g.csr(), 0, 999);
        });

        printf("\n");
    }

    // Medium graph — 10k nodes, degree 10
    {
        printf("-- Medium graph (10k nodes, deg=10) --\n");
        GraphEngine g;
        build_random_graph(g, 10000, 10);
        g.csr();  // pre-compact

        bench("BFS path", 100, [&](){
            bfs_path(g.graph_ref(), 0, 9999);
        });

        bench("Dijkstra path", 100, [&](){
            dijkstra_path(g.graph_ref(), 0, 9999);
        });

        bench("CSR BFS path", 100, [&](){
            bfs_path_csr(g.csr(), 0, 9999);
        });

        bench("CSR Dijkstra path", 100, [&](){
            dijkstra_path_csr(g.csr(), 0, 9999);
        });

        bench("BFS neighborhood hops=3", 100, [&](){
            bfs_neighborhood(g.graph_ref(), 0, 3);
        });

        printf("\n");
    }

    // Large graph — 100k nodes, degree 5
    {
        printf("-- Large graph (100k nodes, deg=5) --\n");
        GraphEngine g;
        build_random_graph(g, 100000, 5);
        g.csr();

        bench("CSR BFS path", 10, [&](){
            bfs_path_csr(g.csr(), 0, 99999);
        });

        bench("CSR Dijkstra path", 10, [&](){
            dijkstra_path_csr(g.csr(), 0, 99999);
        });

        bench("BFS neighborhood hops=2", 10, [&](){
            bfs_neighborhood(g.graph_ref(), 0, 2);
        });

        printf("\n");
    }

    return 0;
}