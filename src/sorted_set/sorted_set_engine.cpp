#include "sorted_set/sorted_set_engine.hpp"


SortedSetEngine::~SortedSetEngine() {
    for ( auto& [key, sl] : sets_ ) delete sl ;
}

SkipList* SortedSetEngine::get_or_create( const std::string& key ) {
    auto it = sets_.find( key ) ;
    if ( it != sets_.end() ) return it->second ;
    auto* sl = new SkipList() ;
    sets_[key] = sl ;
    return sl ;
}

SkipList* SortedSetEngine::get(const std::string& key) const {
    auto it = sets_.find(key);
    return it != sets_.end() ? it->second : nullptr;
}

int SortedSetEngine::zadd(const std::string& key, double score,
                           const std::string& member) {
    auto* sl  = get_or_create(key);
    bool  is_new = !sl->score_of(member).has_value();
    sl->insert(score, member);
    return is_new ? 1 : 0;
}

bool SortedSetEngine::zrem(const std::string& key,
                            const std::string& member) {
    auto* sl = get(key);
    if (!sl) return false;
    auto sc = sl->score_of(member);
    if (!sc.has_value()) return false;
    return sl->remove(sc.value(), member);
}

std::optional<double> SortedSetEngine::zscore(const std::string& key,
                                                const std::string& member) {
    auto* sl = get(key);
    if (!sl) return std::nullopt;
    return sl->score_of(member);
}

std::vector<std::pair<double,std::string>>
SortedSetEngine::zrange(const std::string& key, int start, int stop,
                         bool with_scores) {
    auto* sl = get(key);
    if (!sl) return {};
    return sl->range_by_rank(start, stop);
}

std::vector<std::pair<double,std::string>>
SortedSetEngine::zrangebyscore(const std::string& key,
                                double min_score, double max_score) {
    auto* sl = get(key);
    if (!sl) return {};
    return sl->range_by_score(min_score, max_score);
}

long long SortedSetEngine::zcard(const std::string& key) {
    auto* sl = get(key);
    return sl ? (long long)sl->size() : 0;
}

long long SortedSetEngine::zrank(const std::string& key,
                                  const std::string& member) {
    auto* sl = get(key);
    if (!sl) return -1;
    auto sc = sl->score_of(member);
    if (!sc.has_value()) return -1;
    auto results = sl->range_by_rank(0, (int)sl->size());
    for (int i = 0; i < (int)results.size(); i++)
        if (results[i].second == member) return i;
    return -1;
}
