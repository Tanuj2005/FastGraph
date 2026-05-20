#include <cstdio>
#include <chrono>
#include <string>
#include <vector>
#include <random>
#include <functional>
#include "kv/kv_engine.hpp"

using Clock = std::chrono::high_resolution_clock;

static double bench(const char* name, int n,
                    std::function<void()> fn) {
    auto t1 = Clock::now();
    for (int i = 0; i < n; i++) fn();
    auto t2   = Clock::now();
    double ms = std::chrono::duration<double,
                std::milli>(t2 - t1).count();
    double ops = (n / ms) * 1000.0;
    printf("%-25s %8.0f ops/sec  (%6.1f ms)\n", name, ops, ms);
    return ops;
}

int main() {
    const int N = 1000000;
    KVEngine kv;

    printf("=== KVEngine microbenchmarks (N=%d) ===\n\n", N);

    // Pre-generate keys
    std::vector<std::string> keys(N);
    for (int i = 0; i < N; i++)
        keys[i] = "key:" + std::to_string(i);

    // Sequential SET
    bench("SET sequential", N, [&](){
        static int i = 0;
        kv.set(keys[i % N], "value");
        i++;
    });

    // Sequential GET (all hits)
    bench("GET sequential (hit)", N, [&](){
        static int i = 0;
        kv.get(keys[i % N]);
        i++;
    });

    // Random GET
    std::mt19937 rng(42);
    bench("GET random (hit)", N, [&](){
        kv.get(keys[rng() % N]);
    });

    // GET miss
    bench("GET miss", N, [&](){
        static int i = 0;
        kv.get("missing:" + std::to_string(i++ % N));
    });

    // DEL
    bench("DEL", N / 10, [&](){
        static int i = 0;
        kv.del(keys[i++ % N]);
    });

    // SET with TTL
    bench("SET with TTL", N, [&](){
        static int i = 0;
        kv.set(keys[i % N], "value", 60000);
        i++;
    });

    printf("\nFinal map size: %zu\n", kv.size());
    return 0;
}