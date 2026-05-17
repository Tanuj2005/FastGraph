#pragma once
#include <cstddef>
#include <cstdlib>
#include <cassert>

class Arena {
public:

    // Constructor

    explicit Arena( size_t capacity ) : buf_( static_cast<char*>( malloc( capacity ) ) ), cap_( capacity ), used_( 0 ) {
        assert( buf_ ) ;
    }

    // Destructor

    ~Arena() {
        free( buf_ ) ;
    }

    // Making sure the Arena object can not be coppied by disabling the copy constructor

    Arena( const Arena& ) = delete ;
    Arena& operator = ( const Arena& ) = delete ;

    // Handles the memory allocation logic

    void* alloc( size_t size, size_t align = alignof( std::max_align_t ) ) {
        size_t offset = ( used_ + align - 1 ) & ~( align - 1 ) ;
        if ( offset + size > cap_ ) return nullptr ;
        used_ = offset + size ;
        return buf_ + offset ;
    }

    template< typename T >
    T* alloc_one() {
        return static_cast< T* >( alloc( sizeof( T ), alignof( T ) ) ) ;
    }

    template< typename T >
    T* alloc_array( size_t n ) {
        return static_cast< T* >( alloc( sizeof( T ) * n, alignof( T ) ) ) ;
    }

    void reset() {
        used_ = 0 ;
    }

    size_t used() const { return used_ ; }
    size_t capacity() const { return cap_ ; }

private:
    char* buf_ ;
    size_t cap_ ;
    size_t used_ ;
};

