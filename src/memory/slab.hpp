#pragma once
#include <cstdlib>
#include <cstddef>

class SlabAllocator {
public:
    static constexpr size_t kClasses[] = {8,16,32,64,128,256,512,1024,2048} ;
    static constexpr int kNumClasses = 9 ;

    void* allocate( size_t size ) {
        int cls = size_class( size ) ;
        if ( cls < 0 ) return malloc( size ) ;
        return pools_[cls].get( kClasses[cls] ) ;
    }

    void deallocate( void* ptr, size_t size ) {
        int cls = size_class( size ) ;
        if ( cls < 0 ) {
            free( ptr ) ;
            return ;
        }
        pools_[cls].put( ptr ) ;
    }

private:
    int size_class( size_t size ) {
        for ( int i = 0 ; i < kNumClasses ; i++ )
            if ( size <= kClasses[i] ) return i ;
        return -1 ;
    }

    struct FreeList {
        struct Node {
            Node* next ;
        };

        Node* head = nullptr ;
        
        void* get( size_t sz ) {
            if ( head ) {
                void* p = head ;
                head = head->next ;
                return p ;
            }

            return malloc( sz ) ;
        }

        void put( void* p ) {
            auto n = static_cast<Node*>(p) ;
            n->next = head ;
            head = n ;
        }
    };

    FreeList pools_[kNumClasses] ;

};