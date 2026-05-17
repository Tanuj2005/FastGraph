#pragma once
#include <string>
#include <vector>

enum class ParseResult {
    Complete,
    Incomplete,
    Error
};

class RespParser {
public:
    ParseResult parse( std::string& buf, std::vector<std::string>& out ) ;

private:
    ParseResult parse_array( std:: string& buf, std::vector<std::string>& out ) ;
    ParseResult parse_inline( std::string& buf, std::vector<std::string>& out ) ;
    int find_crlf( const std::string& buf, size_t start = 0 ) ;
};