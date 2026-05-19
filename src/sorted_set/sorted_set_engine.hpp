#pragma once
#include "index/skiplist.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

class SortedSetEngine {
public:
    ~SortedSetEngine() ;

    int zadd( const std::string& key, double score, const std::string& member ) ;

    bool zrem( const std::string& key, const std::string& member ) ;

    std::optional<double> zscore( const std::string& key, const std::string& member ) ;

    std::vector<std::pair<double, std::string>> zrange( const std::string& key, int start, int stop, bool with_scores = false ) ;

    std::vector<std::pair<double, std::string>> zrangebyscore( const std::string& key, double min_score, double max_score ) ;

    long long zcard( const std::string& key ) ;
    long long zrank( const std::string& key, const std::string& member ) ;

    

private:
    SkipList* get_or_create( const std::string& key ) ;
    SkipList* get( const std::string& key ) const ;

    std::unordered_map<std::string, SkipList*> sets_ ;

};