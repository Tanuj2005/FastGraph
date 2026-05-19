#pragma once
#include "index/skiplist.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class PropertyIndex {

public:

    void add( const std::string& label, const std::string& prop, double value, int node_id ) ;

    void remove( const std::string& label, const std::string& prop, double value, int node_id ) ;

    void remove_node( const std::string& label, int node_id ) ;


    std::vector<int> range( const std::string& label, const std::string& prop, double min_val, double max_val ) const ;

    std::vector<int> exact( const std::string& label, const std::string& prop, double value ) const ;

    ~PropertyIndex() {
        for (auto& [key, list] : indices_) {
            delete list;
        }
    }

private:

    std::string make_key( const std::string& label, const std::string& prop ) const {
        return label + "." + prop ;
    }

    std::unordered_map<std::string, SkipList*> indices_ ;

    std::unordered_map<int, std::vector<std::pair<std::string,double>>> node_entries_ ;

};