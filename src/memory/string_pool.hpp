#pragma once
#include "memory/arena.hpp"
#include <unordered_map>
#include <string>
#include <cstring>

class StringPool {
public:
    explicit StringPool( size_t arena_size = 4 * 1024 * 1024 ) : arena_( arena_size ) {}

    const char* intern( const std::string& s ) {
        auto it = table_.find( s ) ;
        if ( it != table_.end() ) return it->second ;

        char* p = static_cast<char*>( arena_.alloc( s.size() + 1 ) ) ;
        if ( !p ) return nullptr ;
        memcpy( p, s.data(), s.size() + 1 ) ;
        table_[s] = p ;
        return p ;
    }

    bool equal( const char* a, const char* b ) const { return a == b ; }

    size_t size() const { return table_.size() ; }

private:
    Arena arena_ ;
    std::unordered_map<std::string, char*> table_ ;
};