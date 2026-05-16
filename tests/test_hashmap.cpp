#include <cassert>
#include <cstdio>
#include <cstring>
#include "memory/arena.hpp"
#include "memory/pool_allocator.hpp"
#include "memory/slab.hpp"
#include "memory/string_pool.hpp"

struct Node { int val; char pad[24]; };

void test_arena() {
    Arena a(1024);
    int* x = a.alloc_one<int>();
    assert(x);
    *x = 42;
    assert(*x == 42);

    int* arr = a.alloc_array<int>(10);
    for (int i = 0; i < 10; i++) arr[i] = i;
    assert(arr[9] == 9);

    size_t used = a.used();
    a.reset();
    assert(a.used() == 0);
    assert(a.used() < used);
    printf("arena: OK\n");
}

void test_pool() {
    PoolAllocator<Node> pool;
    Node* a = pool.make();  a->val = 1;
    Node* b = pool.make();  b->val = 2;
    assert(a->val == 1 && b->val == 2);
    pool.destroy(a);
    Node* c = pool.make();  // should reuse a's slot
    c->val = 3;
    assert(c->val == 3);
    pool.destroy(b);
    pool.destroy(c);
    printf("pool: OK\n");
}

void test_slab() {
    SlabAllocator slab;
    void* p1 = slab.allocate(10);   // goes to 16-byte class
    void* p2 = slab.allocate(64);   // goes to 64-byte class
    void* p3 = slab.allocate(3000); // large — falls back to malloc
    assert(p1 && p2 && p3);
    slab.deallocate(p1, 10);
    slab.deallocate(p2, 64);
    slab.deallocate(p3, 3000);
    void* p4 = slab.allocate(10);   // should reuse p1's slot
    assert(p4 == p1);
    slab.deallocate(p4, 10);
    printf("slab: OK\n");
}

void test_string_pool() {
    StringPool sp;
    const char* a = sp.intern("hello");
    const char* b = sp.intern("world");
    const char* c = sp.intern("hello");  // same as a
    assert(a == c);         // pointer equality
    assert(a != b);
    assert(strcmp(a, "hello") == 0);
    printf("string_pool: OK\n");
}

int main() {
    test_arena();
    test_pool();
    test_slab();
    test_string_pool();
    printf("Phase 1: all memory tests passed\n");
    return 0;
}