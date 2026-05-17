#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <chrono>

using Ms = int64_t;

Ms now_ms();

struct TTLEntry {
    Ms          expires_at;
    std::string key;
    size_t      heap_idx;
};

class TTLManager {
public:
    void set_expiry(const std::string& key, Ms ttl_ms);
    void remove_expiry(const std::string& key);
    Ms   get_expiry(const std::string& key) const;
    std::vector<std::string> evict_expired();
    Ms   next_expiry_in() const;
    size_t size() const { return heap_.size(); }

private:
    void sift_up(size_t i);
    void sift_down(size_t i);
    void swap_entries(size_t a, size_t b);

    std::vector<TTLEntry>                   heap_;
    std::unordered_map<std::string, size_t> index_;
};