#pragma once
#include <cstdlib>
#include <vector>
#include <cassert>

template< typename T, size_t BlockSize = 4096 >
class PoolAllocator {
public:
    // Constructor
    PoolAllocator() {
        grow() ;
    }

    // Destructor
    ~PoolAllocator() {
        for ( auto* b : blocks_ )
            free( b ) ;
    }

    PoolAllocator( const PoolAllocator& ) = delete ;
    PoolAllocator& operator = ( const PoolAllocator& ) = delete ;

    T* allocate() {
        if ( !free_list_ ) 
            grow() ;

        Slot* slot = free_list_ ;
        free_list_ = slot->next ;
        return reinterpret_cast< T* >( slot ) ;
    }

    void deallocate( T* ptr ) {
        Slot* slot = reinterpret_cast< Slot* >( ptr ) ;
        slot->next = free_list_ ;
        free_list_ = slot ;
    }

    template< typename... Args >
    T* make( Args&&... args ) {
        T* p = allocate() ;
        new (p) T( std::forward<Args>(args)... ) ;
        return p ;
    }

    void destroy( T* p ) {
        p->~T() ;
        deallocate( p ) ;
    }

private:
    union Slot {
        T obj ;
        Slot* next ;
    };

    static_assert( sizeof( T ) >= sizeof( void* ), "T too small for pool allocator") ;

    void grow() {
        size_t count = BlockSize / sizeof( Slot ) ;
        assert( count > 0 ) ;
        Slot* block = static_cast< Slot* >( malloc( count * sizeof( Slot ) ) ) ;
        assert( block ) ;
        blocks_.push_back( block ) ;
        for ( size_t i = 0 ; i < count - 1 ; i++ )
            block[i].next = &block[i + 1] ;
        block[count - 1 ].next = free_list_ ;
        free_list_ = block ;
    }

    Slot* free_list_ = nullptr ;
    std::vector<void*> blocks_ ;
};