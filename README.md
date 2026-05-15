# FastGraph

An in-memory knowledge graph database with a Redis-compatible protocol.
Combines key-value storage with graph traversal — nodes, edges, BFS/DFS/Dijkstra, 
range queries, TTL expiry, and persistence. Speak to it with any Redis client.

## Status
🚧 Work in progress

## Architecture
- TCP server with epoll event loop (single-threaded, non-blocking)
- RESP protocol — compatible with redis-cli and any Redis client library
- Robin Hood hash table for O(1) KV operations
- Adjacency list + CSR graph for traversal
- Skip list property index for range queries
- RDB-style persistence via fork + mmap

## Build
Requirements: Linux, GCC 11+, CMake 3.16+

git clone https://github.com/Tanuj2005/FastGraph.git
cd fastgraph
mkdir build && cd build
cmake ..
make -j$(nproc)
./fastgraph

## Usage
redis-cli -p 6379 ping
redis-cli -p 6379 set foo bar
redis-cli -p 6379 get foo
redis-cli -p 6379 graph.add_node 1 Person '{"name":"Alice","age":30}'
redis-cli -p 6379 graph.path 1 5

## Commands
### KV
SET key value [EX seconds]
GET key
DEL key
EXISTS key
EXPIRE key seconds
TTL key
PERSIST key

### Graph
GRAPH.ADD_NODE id label properties
GRAPH.ADD_EDGE from to rel_type
GRAPH.PATH src dst [rel_type]
GRAPH.WPATH src dst [rel_type]
GRAPH.NEIGHBORHOOD src hops [rel_type]
GRAPH.COMPONENT src

### Sorted Sets
ZADD key score member
ZRANGE key start stop
ZRANGEBYSCORE key min max
ZSCORE key member

## License
MIT