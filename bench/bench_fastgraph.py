# bench/bench_fastgraph.py
import time
import redis
import random

r = redis.Redis(host='localhost', port=6379, decode_responses=True)

def bench(name, n, fn):
    t1  = time.perf_counter()
    for _ in range(n): fn()
    t2  = time.perf_counter()
    ms  = (t2 - t1) * 1000
    ops = (n / ms) * 1000
    print(f"{name:<40} {ops:>8.0f} ops/sec  ({ms:>7.1f} ms)")

def build_graph(num_nodes, avg_degree):
    pipe = r.pipeline(transaction=False)
    for i in range(num_nodes):
        pipe.execute_command("GRAPH.ADD_NODE", i, "Person", "{}")
    pipe.execute()

    rng = random.Random(42)
    pipe = r.pipeline(transaction=False)
    for _ in range(num_nodes * avg_degree):
        src = rng.randint(0, num_nodes - 1)
        dst = rng.randint(0, num_nodes - 1)
        pipe.execute_command("GRAPH.ADD_EDGE", src, dst, "KNOWS")
    pipe.execute()

def run_suite(label, num_nodes, avg_degree, n_iter):
    print(f"\n-- {label} --")
    print(f"  Building graph...")
    build_graph(num_nodes, avg_degree)
    print(f"  Graph ready.\n")

    bench("BFS path",
          n_iter,
          lambda: r.execute_command("GRAPH.PATH", 0, num_nodes - 1))

    bench("Weighted path (Dijkstra)",
          n_iter,
          lambda: r.execute_command("GRAPH.WPATH", 0, num_nodes - 1))

    bench("Neighborhood hops=2",
          n_iter,
          lambda: r.execute_command("GRAPH.NEIGHBORHOOD", 0, 2))

    bench("Component",
          min(n_iter, 20),
          lambda: r.execute_command("GRAPH.COMPONENT", 0))

if __name__ == "__main__":
    print("=== FastGraph network benchmarks ===")
    run_suite("Small graph (1k nodes, deg=10)",   1000,  10, 100)
    run_suite("Medium graph (10k nodes, deg=10)", 10000, 10, 20)
    print("\nDone.")