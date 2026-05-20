#pragma once
#include <string>
#include <vector>
#include <optional>
#include <random>
#include <unordered_map> 
#include "memory/slab.hpp"

static constexpr int SKIP_MAX_LEVEL = 16 ;

struct SkipNode {
    double score ;
    std::string member ;
    int level ; // Added level field
    SkipNode* forward[1] ;

    static SkipNode* make( int levels, double score, const std::string& member ) ;

    static void free( SkipNode* n ) ;
    
    static SlabAllocator slab_ ;
};

class SkipList {
public:
    SkipList() ;
    ~SkipList() ;

    SkipList( const SkipList& ) = delete ;
    SkipList& operator = (const SkipList&) = delete ;

    void insert( double score, const std::string& member ) ;
    bool remove( double score, const std::string& member ) ;
    std::optional<double> score_of( const std::string& member ) ;

    std::vector<std::pair<double, std::string>> range_by_score( double min_score, double max_score ) const ;

    std::vector<std::pair<double, std::string>> range_by_rank( int start, int stop ) const ;

    size_t size() const { return size_ ; }

private:
    int random_level() ;
    SkipNode* head_ ;
    int level_ ;
    size_t size_ ;
    std::mt19937 rng_ ;
    std::unordered_map<std::string, double> member_scores_;
};
