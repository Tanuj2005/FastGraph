#include "graph/dynamic_graph.hpp"
#include <algorithm>

const std::vector<Edge> DynamicGraph::kEmpty ;

void DynamicGraph::add_node( int id, const std::string& label, const std::string& props ) {

    meta_[id] = { label, props } ;
    if ( adj_.find( id ) == adj_.end() ) adj_[id] = {} ;
    if ( radj_.find( id ) == radj_.end() ) radj_[id] = {} ;
    dirty_ = true ;

}

void DynamicGraph::remove_node( int id ) {
    auto it = adj_.find( id ) ;
    if( it != adj_.end() ) {
        for ( auto& e : it->second ) {
            auto& rv = radj_[e.to] ;
            rv.erase( std::remove_if( rv.begin(), rv.end(), [id]( const Edge& e ){ return e.to == id ; }), rv.end()) ;
            edge_count_-- ;

        }

        adj_.erase( it ) ;
    }

    auto rit = radj_.find( id ) ;
    if ( rit != radj_.end() ) {
        for ( auto & e : rit->second ) {
            auto& fv = adj_[ e.to ] ;
            fv.erase( std::remove_if( fv.begin(), fv.end(), [id](const Edge& e){ return e.to == id ; }), fv.end()) ;
            edge_count_-- ;
        }

        radj_.erase( rit ) ;
    }

    meta_.erase( id ) ;
    dirty_ = true ;
}

void DynamicGraph::add_edge(int from, int to, const std::string& rel,
                             float weight) {
    adj_[from].push_back({to,   rel, weight});
    radj_[to].push_back( {from, rel, weight});
    edge_count_++;
    dirty_ = true;
}

void DynamicGraph::remove_edge(int from, int to, const std::string& rel) {
    auto remove_from = [&](std::vector<Edge>& v, int target,
                           const std::string& r) {
        auto it = std::remove_if(v.begin(), v.end(),
            [&](const Edge& e){ return e.to == target && e.rel_type == r; });
        if (it != v.end()) { v.erase(it, v.end()); edge_count_--; }
    };
    remove_from(adj_[from],  to,   rel);
    remove_from(radj_[to],   from, rel);
    dirty_ = true;
}

const std::vector<Edge>& DynamicGraph::neighbors(int id) const {
    auto it = adj_.find(id);
    return it != adj_.end() ? it->second : kEmpty;
}

const std::vector<Edge>& DynamicGraph::reverse_neighbors(int id) const {
    auto it = radj_.find(id);
    return it != radj_.end() ? it->second : kEmpty;
}

const NodeMeta* DynamicGraph::node_meta(int id) const {
    auto it = meta_.find(id);
    return it != meta_.end() ? &it->second : nullptr;
}

bool DynamicGraph::has_node(int id) const {
    return meta_.find(id) != meta_.end();
}

bool DynamicGraph::has_edge(int from, int to) const {
    auto it = adj_.find(from);
    if (it == adj_.end()) return false;
    for (auto& e : it->second)
        if (e.to == to) return true;
    return false;
}