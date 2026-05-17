#pragma once
#include <string>
#include <vector>

class RespEncoder {
public:
    static std::string simple_string( const std::string& s ) ;
    static std::string error( const std::string& msg ) ;
    static std::string integer( long long n ) ;
    static std::string bulk_string( const std::string& s ) ;
    static std::string null_bulk() ;
    static std::string array( const std::vector<std::string>& items ) ;
    static std::string empty_array() ;
};