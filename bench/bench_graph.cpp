#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <string>
#include <vector>
#include <functional>

// Raw TCP client — sends RESP, reads response
struct Client {
    int fd;

    Client() {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(6379);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        connect(fd, (sockaddr*)&addr, sizeof(addr));
    }

    ~Client() { close(fd); }

    std::string send_cmd(const std::vector<std::string>& args) {
        std::string req = "*" + std::to_string(args.size()) + "\r\n";
        for (auto& a : args)
            req += "$" + std::to_string(a.size()) + "\r\n" + a + "\r\n";
        write(fd, req.data(), req.size());

        char buf[4096];
        ssize_t n = read(fd, buf, sizeof(buf));
        return n > 0 ? std::string(buf, n) : "";
    }
};

using Clock = std::chrono::steady_clock;

double bench(const std::string& name, int iterations,
             std::function<void(Client&, int)> fn) {
    Client c;
    auto start = Clock::now();
    for (int i = 0; i < iterations; i++) fn(c, i);
    double ms = std::chrono::duration_cast<std::chrono::microseconds>(
        Clock::now() - start).count() / 1000.0;
    double ops = iterations / (ms / 1000.0);
    printf("%-30s %7d ops  %8.1f ms  %10.0f ops/sec\n",
           name.c_str(), iterations, ms, ops);
    return ops;
}

int main() {
    int N = 10000;
    printf("FastGraph Benchmark — %d iterations each\n", N);
    printf("%-30s %7s  %10s  %12s\n", "operation", "ops", "time", "ops/sec");
    printf("%s\n", std::string(65, '-').c_str());

    // Pre-populate nodes for read benchmarks
    {
        Client c;
        for (int i = 0; i < N; i++)
            c.send_cmd({"GRAPH.ADD_NODE", std::to_string(i),
                        "Person", "{\"name\":\"user" + std::to_string(i) + "\"}"});
        // Linear edges: 0->1->2->...->N-1
        for (int i = 0; i < N - 1; i++)
            c.send_cmd({"GRAPH.ADD_EDGE", std::to_string(i),
                        std::to_string(i+1), "KNOWS"});
    }

    bench("GRAPH.ADD_NODE", N, [&](Client& c, int i) {
        c.send_cmd({"GRAPH.ADD_NODE",
                    std::to_string(N + i), "Company", "{}"});
    });

    bench("GRAPH.HAS_NODE (hit)", N, [&](Client& c, int i) {
        c.send_cmd({"GRAPH.HAS_NODE", std::to_string(i % N)});
    });

    bench("GRAPH.HAS_NODE (miss)", N, [&](Client& c, int i) {
        c.send_cmd({"GRAPH.HAS_NODE", std::to_string(N * 10 + i)});
    });

    bench("GRAPH.NEIGHBORS", N, [&](Client& c, int i) {
        c.send_cmd({"GRAPH.NEIGHBORS", std::to_string(i % N)});
    });

    bench("GRAPH.NODE (metadata)", N, [&](Client& c, int i) {
        c.send_cmd({"GRAPH.NODE", std::to_string(i % N)});
    });

    bench("GRAPH.PATH (short 5 hops)", 1000, [&](Client& c, int i) {
        int src = (i * 7) % (N - 10);
        c.send_cmd({"GRAPH.PATH",
                    std::to_string(src), std::to_string(src + 5)});
    });

    bench("GRAPH.WPATH (short 5 hops)", 1000, [&](Client& c, int i) {
        int src = (i * 7) % (N - 10);
        c.send_cmd({"GRAPH.WPATH",
                    std::to_string(src), std::to_string(src + 5)});
    });

    bench("GRAPH.NEIGHBORHOOD 2 hops", 1000, [&](Client& c, int i) {
        c.send_cmd({"GRAPH.NEIGHBORHOOD",
                    std::to_string(i % (N/2)), "2"});
    });

    bench("SET (KV baseline)", N, [&](Client& c, int i) {
        c.send_cmd({"SET", "key" + std::to_string(i), "val"});
    });

    bench("GET (KV baseline)", N, [&](Client& c, int i) {
        c.send_cmd({"GET", "key" + std::to_string(i)});
    });

    return 0;
}