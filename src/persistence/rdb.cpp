#include "persistence/rdb.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

// ── helpers ───────────────────────────────────────────────────────────────────

static bool write_u32(FILE* f, uint32_t v) {
    return fwrite(&v, 4, 1, f) == 1;
}
static bool write_u64(FILE* f, uint64_t v) {
    return fwrite(&v, 8, 1, f) == 1;
}
static bool write_f32(FILE* f, float v) {
    return fwrite(&v, 4, 1, f) == 1;
}
static bool write_str(FILE* f, const std::string& s) {
    uint32_t len = (uint32_t)s.size();
    return write_u32(f, len) &&
           (len == 0 || fwrite(s.data(), 1, len, f) == len);
}

static bool read_u32(FILE* f, uint32_t& v) {
    return fread(&v, 4, 1, f) == 1;
}
static bool read_u64(FILE* f, uint64_t& v) {
    return fread(&v, 8, 1, f) == 1;
}
static bool read_f32(FILE* f, float& v) {
    return fread(&v, 4, 1, f) == 1;
}
static bool read_str(FILE* f, std::string& s) {
    uint32_t len = 0;
    if (!read_u32(f, len)) return false;
    s.resize(len);
    return len == 0 || fread(s.data(), 1, len, f) == len;
}

// ── save ──────────────────────────────────────────────────────────────────────

bool RDB::save(const std::string& path,
               const KVEngine&    kv,
               const GraphEngine& graph) {
    // Write to tmp then rename — atomic on Linux
    std::string tmp = path + ".tmp";
    FILE* f = fopen(tmp.c_str(), "wb");
    if (!f) return false;

    // Header
    write_u64(f, MAGIC);
    write_u32(f, VERSION);

    // KV section
    auto entries = kv.all_entries();   // we'll add this accessor below
    write_u64(f, (uint64_t)entries.size());
    for (auto& [key, val, expires_at] : entries) {
        write_str(f, key);
        write_str(f, val);
        int64_t exp = expires_at;
        fwrite(&exp, 8, 1, f);
    }

    // Graph nodes
    auto nodes = graph.all_nodes();    // we'll add this accessor below
    write_u64(f, (uint64_t)nodes.size());
    for (auto& [id, label, props] : nodes) {
        write_u32(f, (uint32_t)id);
        write_str(f, label);
        write_str(f, props);
    }

    // Graph edges
    auto edges = graph.all_edges();    // we'll add this accessor below
    write_u64(f, (uint64_t)edges.size());
    for (auto& [from, to, rel, weight] : edges) {
        write_u32(f, (uint32_t)from);
        write_u32(f, (uint32_t)to);
        write_str(f, rel);
        write_f32(f, weight);
    }

    write_u64(f, EOF_MARKER);
    fclose(f);

    // Atomic rename
    return rename(tmp.c_str(), path.c_str()) == 0;
}

// ── load ──────────────────────────────────────────────────────────────────────

bool RDB::load(const std::string& path,
               KVEngine&          kv,
               GraphEngine&       graph) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    uint64_t magic = 0;
    uint32_t ver   = 0;
    if (!read_u64(f, magic) || magic != MAGIC) { fclose(f); return false; }
    if (!read_u32(f, ver)   || ver   != VERSION) { fclose(f); return false; }

    // KV entries
    uint64_t kv_count = 0;
    read_u64(f, kv_count);
    for (uint64_t i = 0; i < kv_count; i++) {
        std::string key, val;
        int64_t     expires_at = 0;
        read_str(f, key);
        read_str(f, val);
        fread(&expires_at, 8, 1, f);

        Ms ttl_ms = -1;
        if (expires_at > 0) {
            ttl_ms = expires_at - now_ms();
            if (ttl_ms <= 0) continue;  // already expired
        }
        kv.set(key, val, ttl_ms);
    }

    // Graph nodes
    uint64_t node_count = 0;
    read_u64(f, node_count);
    for (uint64_t i = 0; i < node_count; i++) {
        uint32_t    id = 0;
        std::string label, props;
        read_u32(f, id);
        read_str(f, label);
        read_str(f, props);
        graph.add_node((int)id, label, props);
    }

    // Graph edges
    uint64_t edge_count = 0;
    read_u64(f, edge_count);
    for (uint64_t i = 0; i < edge_count; i++) {
        uint32_t    from = 0, to = 0;
        std::string rel;
        float       weight = 1.0f;
        read_u32(f, from);
        read_u32(f, to);
        read_str(f, rel);
        read_f32(f, weight);
        graph.add_edge((int)from, (int)to, rel, weight);
    }

    uint64_t eof = 0;
    read_u64(f, eof);
    fclose(f);
    return eof == EOF_MARKER;
}