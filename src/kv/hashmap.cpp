#include "kv/hashmap.hpp"
#include <cstring>
#include <cassert>

RobinHoodMap::RobinHoodMap( size_t initial ) {
    capacity_ = next_pow2( initial ) ;
    mask_ = capacity_ - 1 ;
    entries_ = new Entry[capacity_]() ;
    clear_all() ;
}

RobinHoodMap::~RobinHoodMap() { delete[] entries_ ; }

bool RobinHoodMap::set( const std::string& key, const std::string& val ) {
    if ( size_ + 1 > capacity_ * MAX_LOAD ) grow() ;

    uint32_t h = hash( key ) ;
    uint32_t idx = h & mask_ ;

    Entry incoming ;
    incoming.hash = h ;
    incoming.dib = 0 ;
    incoming.key = key ;
    incoming.val = val ;

    while ( true ) {
        Entry& slot = entries_[idx] ;

        if ( slot.empty() ) {
            slot = std::move( incoming ) ;
            size_++ ;
            return true ;
        }

        if ( slot.hash == h && slot.key == incoming.key ) {
            slot.val = incoming.val ;
            return false ;
        }

        if ( incoming.dib > slot.dib ) {
            std::swap( incoming, slot ) ;
        }

        idx = ( idx + 1 ) & mask_ ;
        incoming.dib++ ;
    }
}

std::string* RobinHoodMap::get( const std::string& key ) {
    uint32_t h = hash( key ) ;
    uint32_t idx = h & mask_ ;
    uint32_t dib = 0 ;

    while ( true ) {
        Entry& slot = entries_[idx] ;
        if ( slot.empty() ) return nullptr ;
        if ( dib > slot.dib ) return nullptr ;
        if ( slot.hash == h && slot.key == key ) return &slot.val ;
        idx = ( idx + 1 ) & mask_ ;
        dib++ ;
    }
}

bool RobinHoodMap::del( const std::string& key ) {
    uint32_t h = hash( key ) ;
    uint32_t idx = h & mask_ ;
    uint32_t dib = 0 ;

    while ( true ) {
        Entry& slot = entries_[idx] ;
        if ( slot.empty() ) return false ;
        if ( dib > slot.dib ) return false ;
        if ( slot.hash == h && slot.key == key ) {
            erase_at( idx ) ;
            size_-- ;
            return true ;
        }
        idx = ( idx + 1 ) & mask_ ;
        dib++ ;
    }
    
}

void RobinHoodMap::erase_at(uint32_t idx) {
    while (true) {
        uint32_t next = (idx + 1) & mask_;
        Entry&   nx   = entries_[next];
        if (nx.empty() || nx.dib == 0) {
            entries_[idx] = Entry{};        // full reset — calls string destructors
            entries_[idx].hash = EMPTY;
            return;
        }
        entries_[idx] = std::move(nx);
        entries_[idx].dib--;
        idx = next;
    }
}

void RobinHoodMap::grow() {
    size_t old_cap = capacity_;
    Entry* old     = entries_;

    capacity_ = old_cap * 2;
    mask_     = capacity_ - 1;
    entries_  = new Entry[capacity_]();
    clear_all();
    size_ = 0;

    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].empty()) continue;

        // Inline insert — no load factor check, no recursive grow
        uint32_t h   = old[i].hash;
        uint32_t idx = h & mask_;

        Entry incoming;
        incoming.hash = h;
        incoming.dib  = 0;
        incoming.key  = std::move(old[i].key);
        incoming.val  = std::move(old[i].val);

        while (true) {
            Entry& slot = entries_[idx];
            if (slot.empty()) {
                slot     = std::move(incoming);
                size_++;
                break;
            }
            if (incoming.dib > slot.dib) {
                std::swap(incoming, slot);
            }
            idx = (idx + 1) & mask_;
            incoming.dib++;
        }
    }
    delete[] old;
}

void RobinHoodMap::clear_all() {
    for (size_t i = 0; i < capacity_; i++)
        entries_[i].hash = EMPTY;
}

uint32_t RobinHoodMap::hash(const std::string& key) {
    uint32_t h = 2166136261u;
    for (unsigned char c : key) { h ^= c; h *= 16777619u; }
    return h == EMPTY ? h - 1 : h;
}

size_t RobinHoodMap::next_pow2(size_t n) {
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4;
    n |= n >> 8; n |= n >> 16;
    return n + 1;
}