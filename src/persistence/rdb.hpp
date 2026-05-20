#pragma once
#include "kv/kv_engine.hpp"
#include "graph/graph_engine.hpp"
#include <string>

// Binary format:
// [MAGIC 8 bytes] [VERSION 4 bytes]
// [KV SECTION]
//   [KV_COUNT 8 bytes]
//   per entry: [key_len 4][key][val_len 4][val][expires_at 8]
// [GRAPH SECTION]
//   [NODE_COUNT 8 bytes]
//   per node: [id 4][label_len 4][label][props_len 4][props]
//   [EDGE_COUNT 8 bytes]
//   per edge: [from 4][to 4][rel_len 4][rel][weight 4]
// [EOF_MARKER 8 bytes]

class RDB {
public:
    static bool save(const std::string& path,
                     const KVEngine&    kv,
                     const GraphEngine& graph);

    static bool load(const std::string& path,
                     KVEngine&          kv,
                     GraphEngine&       graph);

private:
    static constexpr uint64_t MAGIC      = 0x464153544752FF00ULL;
    static constexpr uint32_t VERSION    = 1;
    static constexpr uint64_t EOF_MARKER = 0xFFFFFFFFFFFFFFFFULL;
};