#include "protocol/resp.hpp"
#include <cstdlib>

int RespParser::find_crlf( const std::string& buf, size_t start ) {
    for ( size_t i = start ; i + 1 < buf.size() ; i++ ) {
        if ( buf[i] == '\r' && buf[i+1] == '\n' ) return (int) i ;
    }

    return -1 ;
}

ParseResult RespParser::parse( std::string& buf, std::vector<std::string>& out ) {
    if ( buf.empty() ) return ParseResult::Incomplete ;
    if ( buf[0] == '*' ) return parse_array( buf, out ) ;
    return parse_inline( buf, out ) ;
}

ParseResult RespParser::parse_array( std::string& buf, std::vector<std::string>& out ) {
    int crlf = find_crlf( buf ) ;
    if ( crlf < 0 ) return ParseResult::Incomplete ;

    int count = std::atoi( buf.c_str() + 1 ) ;
    if ( count <= 0 ) return ParseResult::Error ;

    size_t pos = crlf + 2 ;
    std::vector<std::string> args ;

    for ( int i = 0 ; i  < count ; i++ ) {
        if ( pos >= buf.size() ) return ParseResult::Incomplete ;
        if ( buf[pos] != '$' ) return ParseResult::Error ;

        int len_crlf = find_crlf( buf, pos ) ;
        if ( len_crlf < 0 ) return ParseResult::Incomplete ;

        int len = std::atoi( buf.c_str() + pos + 1 ) ;
        pos = len_crlf + 2 ;

        if ( pos + (size_t)len + 2 > buf.size() ) return ParseResult::Incomplete ;

        args.push_back( buf.substr( pos, len ) ) ;
        pos += len + 2 ;
    }

    buf.erase( 0, pos ) ;
    out = std::move( args ) ;
    return ParseResult::Complete ;
}

ParseResult RespParser::parse_inline( std::string& buf, std::vector<std::string>& out ) {
    int crlf = find_crlf( buf ) ;
    if ( crlf < 0 ) return ParseResult::Incomplete ;

    std::string line = buf.substr( 0, crlf ) ;
    buf.erase( 0, crlf + 2 ) ;

    std::vector<std::string> args ;
    size_t start = 0 ;
    while ( start < line.size() ) {
        size_t end = line.find( ' ', start ) ;
        if ( end == std::string::npos ) end = line.size() ;
        if ( end > start ) args.push_back( line.substr( start, end - start ) ) ;
        start = end + 1 ;
    }


    if ( args.empty() ) return ParseResult::Error ;
    out = std::move( args ) ;
    return ParseResult::Complete ;
}

