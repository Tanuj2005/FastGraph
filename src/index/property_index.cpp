#include "index/property_index.hpp"
#include <algorithm>

void PropertyIndex::add(const std::string& label, const std::string& prop,
                         double value, int node_id) {
    std::string key = make_key(label, prop);
    auto it = indices_.find(key);
    if (it == indices_.end()) {
        indices_[key] = new SkipList();
        it = indices_.find(key);
    }
    it->second->insert(value, std::to_string(node_id));
    node_entries_[node_id].push_back({key, value});
}

void PropertyIndex::remove(const std::string& label, const std::string& prop,
                             double value, int node_id) {
    std::string key = make_key(label, prop);
    auto it = indices_.find(key);
    if (it != indices_.end())
        it->second->remove(value, std::to_string(node_id));

    auto& entries = node_entries_[node_id];
    entries.erase(std::remove_if(entries.begin(), entries.end(),
        [&](const std::pair<std::string,double>& e) {
            return e.first == key && e.second == value;
        }), entries.end());
}

void PropertyIndex::remove_node(const std::string& label, int node_id) {
    auto it = node_entries_.find(node_id);
    if (it == node_entries_.end()) return;
    for (auto& [key, value] : it->second) {
        auto sit = indices_.find(key);
        if (sit != indices_.end())
            sit->second->remove(value, std::to_string(node_id));
    }
    node_entries_.erase(it);
}

std::vector<int> PropertyIndex::range(const std::string& label,
                                       const std::string& prop,
                                       double min_val,
                                       double max_val) const {
    std::string key = make_key(label, prop);
    auto it = indices_.find(key);
    if (it == indices_.end()) return {};
    auto results = it->second->range_by_score(min_val, max_val);
    std::vector<int> ids;
    ids.reserve(results.size());
    for (auto& [score, member] : results)
        ids.push_back(std::stoi(member));
    return ids;
}

std::vector<int> PropertyIndex::exact(const std::string& label,
                                       const std::string& prop,
                                       double value) const {
    return range(label, prop, value, value);
}