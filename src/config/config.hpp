#pragma once
#include <string>
#include <cstddef>

struct Config {
    int port = 6379 ;
    size_t num_threads =  4 ;
    int snapshot_interval = 300 ;   // seconds
    std::string rdb_path = "fastgraph.rdb" ;
    size_t max_connections = 10000 ;

    // Parse from file — returns default config if file not found
    static Config from_file( const std::string& path ) ;

    // Parse from argc/argv — overrides file config
    void apply_args( int argc, char** argv ) ;

    void print() const ;
};