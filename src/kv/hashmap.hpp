#pragma once
#include <string>
#include <cstdint>
#include <cstdlib>
#include <vector>

class RobinHoodMap {
public:
    static constexpr float MAX_LOAD = 0.75f ;
    static constexpr uint32_t EMPTY = UINT32_MAX ;

    struct Entry {
        uint32_t hash ;
        uint32_t dib ;
        std::string key ;
        std::string val ;
        bool empty() const { return hash == EMPTY ; }
    };

    explicit RobinHoodMap( size_t inital = 16 ) ;
    ~RobinHoodMap() ;

    RobinHoodMap( const RobinHoodMap& ) = delete ;
    RobinHoodMap& operator=( const RobinHoodMap& ) = delete ;

    bool set( const std::string& key, const std::string& val ) ;
    std::string* get( const std::string& key ) ;
    bool del( const std:: string& key ) ;
    size_t size() const { return size_ ; }
    size_t capacity() const { return capacity_ ; }

private:
    void grow() ;
    void erase_at( uint32_t idx ) ;
    void clear_all() ;
    uint32_t hash( const std::string& key ) ;

    static size_t next_pow2( size_t n ) ;

    Entry* entries_ = nullptr ;
    size_t size_ = 0 ;
    size_t capacity_ = 0 ;
    uint32_t mask_ = 0 ;
};