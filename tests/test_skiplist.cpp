#include <cassert>
#include <cstdio>
#include "index/skiplist.hpp"
#include "index/property_index.hpp"

void test_skiplist() {
    SkipList sl;
    sl.insert(10.0, "alice");
    sl.insert(30.0, "bob");
    sl.insert(20.0, "carol");
    sl.insert(25.0, "dave");

    assert(sl.size() == 4);
    assert(sl.score_of("alice") == 10.0);
    assert(sl.score_of("bob")   == 30.0);
    assert(!sl.score_of("nobody").has_value());

    auto r1 = sl.range_by_score(15.0, 28.0);
    assert(r1.size() == 2);
    assert(r1[0].second == "carol");
    assert(r1[1].second == "dave");

    auto r2 = sl.range_by_rank(0, 1);
    assert(r2.size() == 2);
    assert(r2[0].second == "alice");

    sl.remove(10.0, "alice");
    assert(sl.size() == 3);
    assert(!sl.score_of("alice").has_value());

    // In test_skiplist(), the update score test:
    sl.insert(5.0, "bob");
    assert(sl.score_of("bob") == 5.0);   // already correct
    assert(sl.size() == 3);              // size should stay 3, not grow

    printf("skiplist: OK\n");
}

void test_property_index() {
    PropertyIndex idx;
    idx.add("Person", "age", 30.0, 1);
    idx.add("Person", "age", 25.0, 2);
    idx.add("Person", "age", 35.0, 3);
    idx.add("Person", "age", 22.0, 4);
    idx.add("Company", "size", 100.0, 5);

    // Range query
    auto r1 = idx.range("Person", "age", 25.0, 32.0);
    assert(r1.size() == 2);

    // Exact match
    auto r2 = idx.exact("Person", "age", 30.0);
    assert(r2.size() == 1 && r2[0] == 1);

    // Different label — no cross-contamination
    auto r3 = idx.range("Company", "age", 0.0, 100.0);
    assert(r3.empty());

    // Remove node
    idx.remove_node("Person", 1);
    auto r4 = idx.exact("Person", "age", 30.0);
    assert(r4.empty());

    printf("property_index: OK\n");
}

int main() {
    test_skiplist();
    test_property_index();
    printf("Phase 8: all index tests passed\n");
    return 0;
}