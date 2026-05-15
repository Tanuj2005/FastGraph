#!/bin/bash
# run as: bash scaffold.sh

dirs=(
  src/net src/protocol src/dispatch
  src/kv src/graph src/index
  src/sorted_set src/memory src/persistence
  src/thread src/config
  tests bench client
)
for d in "${dirs[@]}"; do mkdir -p "$d"; done

# Helper: write header guard + pragma once
write_header() {
  cat > "$1" << EOF
#pragma once
EOF
}

write_cpp() {
  cat > "$1" << EOF
#include "${2}"
EOF
}

# net/
write_header src/net/reactor.hpp
write_header src/net/conn.hpp
write_header src/net/server.hpp
write_cpp   src/net/server.cpp   "net/server.hpp"

# protocol/
write_header src/protocol/resp.hpp
write_cpp   src/protocol/resp.cpp    "protocol/resp.hpp"
write_header src/protocol/encoder.hpp
write_cpp   src/protocol/encoder.cpp "protocol/encoder.hpp"

# dispatch/
write_header src/dispatch/dispatcher.hpp
write_cpp   src/dispatch/dispatcher.cpp "dispatch/dispatcher.hpp"

# kv/
write_header src/kv/hashmap.hpp
write_cpp   src/kv/hashmap.cpp   "kv/hashmap.hpp"
write_header src/kv/ttl.hpp
write_cpp   src/kv/ttl.cpp       "kv/ttl.hpp"
write_header src/kv/kv_engine.hpp
write_cpp   src/kv/kv_engine.cpp "kv/kv_engine.hpp"

# graph/
write_header src/graph/dynamic_graph.hpp
write_cpp   src/graph/dynamic_graph.cpp "graph/dynamic_graph.hpp"
write_header src/graph/csr_graph.hpp
write_cpp   src/graph/csr_graph.cpp     "graph/csr_graph.hpp"
write_header src/graph/bfs.hpp
write_cpp   src/graph/bfs.cpp           "graph/bfs.hpp"
write_header src/graph/dfs.hpp
write_cpp   src/graph/dfs.cpp           "graph/dfs.hpp"
write_header src/graph/dijkstra.hpp
write_cpp   src/graph/dijkstra.cpp      "graph/dijkstra.hpp"
write_header src/graph/graph_engine.hpp
write_cpp   src/graph/graph_engine.cpp  "graph/graph_engine.hpp"

# index/
write_header src/index/skiplist.hpp
write_cpp   src/index/skiplist.cpp       "index/skiplist.hpp"
write_header src/index/property_index.hpp
write_cpp   src/index/property_index.cpp "index/property_index.hpp"

# sorted_set/
write_header src/sorted_set/sorted_set_engine.hpp
write_cpp   src/sorted_set/sorted_set_engine.cpp "sorted_set/sorted_set_engine.hpp"

# memory/
write_header src/memory/pool_allocator.hpp
write_header src/memory/arena.hpp
write_header src/memory/slab.hpp
write_header src/memory/string_pool.hpp

# persistence/
write_header src/persistence/persistence.hpp
write_cpp   src/persistence/persistence.cpp "persistence/persistence.hpp"
write_header src/persistence/rdb.hpp
write_cpp   src/persistence/rdb.cpp          "persistence/rdb.hpp"

# thread/
write_header src/thread/thread_pool.hpp
write_cpp   src/thread/thread_pool.cpp "thread/thread_pool.hpp"

# config/
write_header src/config/config.hpp
write_cpp   src/config/config.cpp "config/config.hpp"

# main - Updated to fastgraph
cat > src/main.cpp << EOF
#include <cstdio>

int main() {
    printf("fastgraph starting...\n");
    return 0;
}
EOF

# tests — each includes a main so they compile standalone
for t in test_hashmap test_resp test_graph test_skiplist test_ttl test_integration; do
  cat > "tests/${t}.cpp" << EOF
#include <cstdio>
int main() {
    printf("${t}: no tests yet\n");
    return 0;
}
EOF
done

# bench
for b in bench_kv bench_graph; do
  cat > "bench/${b}.cpp" << EOF
#include <cstdio>
int main() { return 0; }
EOF
done

# client - Renamed to fastgraph-cli.py
cat > client/fastgraph-cli.py << 'EOF'
import redis
import sys

r = redis.Redis(host='localhost', port=6379)
print(r.ping())
EOF

# config
cat > config.conf << EOF
port 6379
threads 4
snapshot_interval 300
max_memory 512mb
EOF

echo "Scaffold complete."
