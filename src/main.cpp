#include "net/server.hpp"
#include "config/config.hpp"
#include <cstdio>

int main( int argc, char** argv ) {

    // Default config file location
    std::string cfg_path = "config.conf" ;


    // Parsing command line arguments
    for ( int i = 1 ; i < argc ; i++ ) {
        std::string a = argv[i] ;
        // Checks if a custom config file path is provided by user and uses it
        if ( a == "--config" && i + 1 < argc ) {
            cfg_path = argv[++i] ;
        }
    }

    // Initializing config object
    Config cfg = Config::from_file(cfg_path) ;
    cfg.apply_args( argc, argv ) ;

    printf("FastGraph starting with config:\n") ;
    cfg.print() ;

    Server server(cfg) ;
    server.start() ;
    return 0 ;

}