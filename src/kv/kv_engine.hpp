#pragma once
#include "kv/hashmap.hpp"
#include "kv/ttl.hpp"
#include <string>
#include <optional>
#include <tuple>

class KVEngine {
public:
    // Returns false if key updated, true if inserted
    bool set(const std::string& key, const std::string& val,
             Ms ttl_ms = -1);

    std::optional<std::string> get(const std::string& key);
    bool        del(const std::string& key);
    bool        exists(const std::string& key);
    bool        expire(const std::string& key, Ms ttl_ms);
    bool        persist(const std::string& key);
    Ms          ttl(const std::string& key);   // ms remaining, -1=none, -2=missing
    size_t      size() const { return map_.size(); }

    std::vector<std::tuple<std::string,std::string,int64_t>>
    all_entries() const;

    // Call from event loop — evicts expired keys
    void        tick();
    Ms          next_expiry_in() const { return ttl_.next_expiry_in(); }

private:
    RobinHoodMap map_;
    TTLManager   ttl_;
};