#include "kv/ttl.hpp"
#include <algorithm>

Ms now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() ;
}

void TTLManager::set_expiry( const std::string& key, Ms ttl_ms ) {
    Ms deadline = now_ms() + ttl_ms ;
    auto it = index_.find(key) ;
    if ( it != index_.end() ) {
        size_t idx = it->second ;
        heap_[idx].expires_at = deadline ;
        sift_up( idx ) ;
        sift_down( idx ) ;
        return ;
    }
    size_t idx = heap_.size() ;
    heap_.push_back( {deadline, key, idx} ) ;
    index_[key] = idx ;
    sift_up( idx ) ;
}


void TTLManager::remove_expiry( const std::string& key ) {
    auto it = index_.find( key ) ;
    if ( it == index_.end() ) return ;
    size_t idx = it->second ;
    index_.erase( it ) ;
    if ( idx != heap_.size() - 1 ) {
        heap_[idx] = std::move( heap_.back() ) ;
        heap_[idx].heap_idx = idx ;
        index_[heap_[idx].key] = idx ;
        heap_.pop_back() ;
        sift_up( idx ) ;
        sift_down( idx ) ;
    } else {
        heap_.pop_back() ;
    }
}

Ms TTLManager::get_expiry( const std::string& key ) const {
    auto it = index_.find( key ) ;
    if ( it == index_.end() ) return -1 ;
    return heap_[it->second].expires_at ;
}

std::vector<std::string> TTLManager::evict_expired() {
    std::vector<std::string> expired ;
    Ms now = now_ms() ;
    while ( !heap_.empty() && heap_[0].expires_at <= now ) {
        expired.push_back(heap_[0].key) ;
        remove_expiry(heap_[0].key) ;
    }
    return expired ;
}

Ms TTLManager::next_expiry_in() const {
    if (heap_.empty()) return -1 ;
    Ms delta = heap_[0].expires_at - now_ms() ;
    return delta < 0 ? 0 : delta ;
}

void TTLManager::sift_up( size_t i ) {
    while ( i > 0 ) {
        size_t parent = ( i - 1 ) / 2;
        if ( heap_[parent].expires_at <= heap_[i].expires_at ) break ;
        swap_entries( parent, i ) ;
        i = parent ;
    }
}

void TTLManager::sift_down(size_t i) {
    size_t n = heap_.size();
    while (true) {
        size_t smallest = i;
        size_t left  = 2 * i + 1;
        size_t right = 2 * i + 2;
        if (left  < n && heap_[left].expires_at  < heap_[smallest].expires_at) smallest = left;
        if (right < n && heap_[right].expires_at < heap_[smallest].expires_at) smallest = right;
        if (smallest == i) break;
        swap_entries(i, smallest);
        i = smallest;
    }
}

void TTLManager::swap_entries(size_t a, size_t b) {
    std::swap(heap_[a], heap_[b]);
    heap_[a].heap_idx = a;
    heap_[b].heap_idx = b;
    index_[heap_[a].key] = a;
    index_[heap_[b].key] = b;
}