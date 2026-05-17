#include "kv/kv_engine.hpp"

bool KVEngine::set(const std::string& key, const std::string& val, Ms ttl_ms) {
    bool inserted = map_.set(key, val);
    if (ttl_ms > 0)
        ttl_.set_expiry(key, ttl_ms);
    else
        ttl_.remove_expiry(key);
    return inserted;
}

std::optional<std::string> KVEngine::get(const std::string& key) {
    // Lazy expiry check
    Ms exp = ttl_.get_expiry(key);
    if (exp >= 0 && exp <= now_ms()) {
        map_.del(key);
        ttl_.remove_expiry(key);
        return std::nullopt;
    }
    auto* v = map_.get(key);
    if (!v) return std::nullopt;
    return *v;
}

bool KVEngine::del(const std::string& key) {
    ttl_.remove_expiry(key);
    return map_.del(key);
}

bool KVEngine::exists(const std::string& key) {
    return get(key).has_value();
}

bool KVEngine::expire(const std::string& key, Ms ttl_ms) {
    if (!map_.get(key)) return false;
    ttl_.set_expiry(key, ttl_ms);
    return true;
}

bool KVEngine::persist(const std::string& key) {
    if (ttl_.get_expiry(key) < 0) return false;
    ttl_.remove_expiry(key);
    return true;
}

Ms KVEngine::ttl(const std::string& key) {
    if (!map_.get(key)) return -2;
    Ms exp = ttl_.get_expiry(key);
    if (exp < 0) return -1;
    Ms remaining = exp - now_ms();
    return remaining < 0 ? 0 : remaining;
}

void KVEngine::tick() {
    for (auto& key : ttl_.evict_expired())
        map_.del(key);
}