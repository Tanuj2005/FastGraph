#include <cassert>
#include <cstdio>
#include <thread>
#include <chrono>
#include "kv/kv_engine.hpp"

void test_hashmap() {
    RobinHoodMap map;
    map.set("foo", "bar");
    map.set("hello", "world");
    assert(*map.get("foo") == "bar");
    assert(*map.get("hello") == "world");
    assert(map.get("missing") == nullptr);

    map.set("foo", "updated");
    assert(*map.get("foo") == "updated");

    map.del("foo");
    assert(map.get("foo") == nullptr);

    // Stress — force multiple resizes
    for (int i = 0; i < 1000; i++)
        map.set("key" + std::to_string(i), "val" + std::to_string(i));
    for (int i = 0; i < 1000; i++)
        assert(*map.get("key" + std::to_string(i)) == "val" + std::to_string(i));

    printf("hashmap: OK\n");
}

void test_ttl() {
    TTLManager ttl;
    ttl.set_expiry("a", 100);
    ttl.set_expiry("b", 500);
    assert(ttl.get_expiry("a") > 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    auto expired = ttl.evict_expired();
    assert(expired.size() == 1 && expired[0] == "a");
    assert(ttl.get_expiry("b") > 0);

    ttl.remove_expiry("b");
    assert(ttl.get_expiry("b") == -1);
    printf("ttl: OK\n");
}

void test_kv_engine() {
    KVEngine kv;
    kv.set("name", "alice");
    assert(kv.get("name") == "alice");
    assert(kv.exists("name"));

    kv.set("temp", "value", 100);  // 100ms TTL
    assert(kv.get("temp").has_value());

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    kv.tick();
    assert(!kv.get("temp").has_value());

    kv.set("x", "y");
    kv.expire("x", 100);
    assert(kv.ttl("x") > 0);
    kv.persist("x");
    assert(kv.ttl("x") == -1);

    kv.del("name");
    assert(!kv.exists("name"));
    assert(kv.ttl("name") == -2);

    printf("kv_engine: OK\n");
}

int main() {
    test_hashmap();
    test_ttl();
    test_kv_engine();
    printf("Phase 3: all KV tests passed\n");
    return 0;
}