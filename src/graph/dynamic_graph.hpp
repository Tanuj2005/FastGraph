#pragma once
#include <string>
#include <vector>
#include <unordered_map>


struct Edge {
    int to ;
    std::string rel_type ;
    float weight ;
};


struct NodeMeta {
    std::string label ;
    std::string props ;   
};


class DynamicGraph {
public:
    void add_node( int id, const std::string& label, const std::string& props ) ;
    void remove_node( int id ) ;
    void add_edge( int from, int to, const std::string& rel, float weight = 1.0f ) ;
    void remove_edge( int from, int to , const std::string& rel ) ;

    const std::vector<Edge>& neighbors( int id ) const ;
    const std::vector<Edge>& reverse_neighbors( int id ) const ;
    const NodeMeta* node_meta( int id ) const ;
    bool has_node( int id ) const ;
    bool has_edge( int from, int to ) const ;

    int node_count() const { return (int)meta_.size() ; } 
    int edge_count() const { return edge_count_ ; }
    int is_dirty() const { return dirty_ ; }
    void clear_dirty() { dirty_ = false ; }

    const std::unordered_map<int, NodeMeta>& all_meta() const { return meta_ ; }
    const std::unordered_map<int, std::vector<Edge>>& all_adj() const { return adj_ ; }

private:

    std::unordered_map<int, NodeMeta>    meta_;
    std::unordered_map<int, std::vector<Edge>> adj_;   // forward
    std::unordered_map<int, std::vector<Edge>> radj_;  // reverse
    int  edge_count_ = 0;
    bool dirty_      = false;

    static const std::vector<Edge> kEmpty;
};
