#include "index/skiplist.hpp"
#include <cstdlib>
#include <cassert>
#include <new>
#include <random>

SlabAllocator SkipNode::slab_;

SkipNode* SkipNode::make( int levels, double score, const std::string& member ) {
    size_t sz = sizeof(SkipNode) + sizeof( SkipNode* ) * (levels - 1 ) ;
    void* mem = SkipNode::slab_.allocate(sz);
    auto* n   = static_cast<SkipNode*>(mem);
    n->score = score ;
    n->level = levels ; // store actual level
    new ( &n->member ) std::string( member ) ;
    for ( int i = 0 ; i < levels ; i++ ) n->forward[i] = nullptr ;
    return n ;
}

void SkipNode::free( SkipNode* n ) {
    int actual_level = n->level;
    n->member.~basic_string() ;
    SkipNode::slab_.deallocate(n, sizeof(SkipNode) +
        sizeof(SkipNode*) * (actual_level - 1));
}

SkipList::SkipList() : level_(1), size_(0) {
    rng_.seed(42);
    head_ = SkipNode::make(SKIP_MAX_LEVEL, -1e18, "");
}

SkipList::~SkipList() {
    SkipNode* cur = head_;
    while (cur) {
        SkipNode* next = cur->forward[0];
        SkipNode::free(cur);
        cur = next;
    }
}

int SkipList::random_level() {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    int lvl = 1;
    while (dist(rng_) < 0.5f && lvl < SKIP_MAX_LEVEL) lvl++;
    return lvl;
}

void SkipList::insert(double score, const std::string& member) {
    SkipNode* update[SKIP_MAX_LEVEL];
    SkipNode* cur = head_;

    for (int i = level_ - 1; i >= 0; i--) {
        while (cur->forward[i] &&
               (cur->forward[i]->score < score ||
                (cur->forward[i]->score == score &&
                 cur->forward[i]->member < member)))
            cur = cur->forward[i];
        update[i] = cur;
    }

    // Check if exact (score, member) pair already exists — no-op
    SkipNode* next = update[0]->forward[0];
    if (next && next->member == member && next->score == score) return;

    // If member exists at different score — remove old entry first
    // Need to search for it separately
    auto old_score = score_of(member);
    if (old_score.has_value() && old_score.value() != score) {
        remove(old_score.value(), member);
        // Rebuild update array after removal
        cur = head_;
        for (int i = level_ - 1; i >= 0; i--) {
            while (cur->forward[i] &&
                   (cur->forward[i]->score < score ||
                    (cur->forward[i]->score == score &&
                     cur->forward[i]->member < member)))
                cur = cur->forward[i];
            update[i] = cur;
        }
    }

    int lvl = random_level();
    if (lvl > level_) {
        for (int i = level_; i < lvl; i++) update[i] = head_;
        level_ = lvl;
    }

    SkipNode* n = SkipNode::make(lvl, score, member);
    for (int i = 0; i < lvl; i++) {
        n->forward[i]         = update[i]->forward[i];
        update[i]->forward[i] = n;
    }
    member_scores_[member] = score;
    size_++;
}

bool SkipList::remove(double score, const std::string& member) {
    SkipNode* update[SKIP_MAX_LEVEL];
    SkipNode* cur = head_;

    for (int i = level_ - 1; i >= 0; i--) {
        while (cur->forward[i] &&
               (cur->forward[i]->score < score ||
                (cur->forward[i]->score == score &&
                 cur->forward[i]->member < member)))
            cur = cur->forward[i];
        update[i] = cur;
    }

    SkipNode* target = update[0]->forward[0];
    if (!target || target->score != score || target->member != member)
        return false;

    for (int i = 0; i < level_; i++) {
        if (update[i]->forward[i] != target) break;
        update[i]->forward[i] = target->forward[i];
    }
    SkipNode::free(target);

    while (level_ > 1 && !head_->forward[level_ - 1]) level_--;
    member_scores_.erase(member);
    size_--;
    return true;
}

std::optional<double> SkipList::score_of(const std::string& member) {
    auto it = member_scores_.find(member);
    if (it == member_scores_.end()) return std::nullopt;
    return it->second;
}

std::vector<std::pair<double,std::string>>
SkipList::range_by_score(double min_score, double max_score) const {
    std::vector<std::pair<double,std::string>> result;
    SkipNode* cur = head_;
    for (int i = level_ - 1; i >= 0; i--)
        while (cur->forward[i] && cur->forward[i]->score < min_score)
            cur = cur->forward[i];
    cur = cur->forward[0];
    while (cur && cur->score <= max_score) {
        result.push_back({cur->score, cur->member});
        cur = cur->forward[0];
    }
    return result;
}

std::vector<std::pair<double,std::string>>
SkipList::range_by_rank(int start, int stop) const {
    std::vector<std::pair<double,std::string>> result;
    SkipNode* cur = head_->forward[0];
    int idx = 0;
    while (cur && idx <= stop) {
        if (idx >= start) result.push_back({cur->score, cur->member});
        cur = cur->forward[0];
        idx++;
    }
    return result;
}