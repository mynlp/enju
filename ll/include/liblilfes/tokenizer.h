#ifndef lilfes_tokenizer_h_
#define lilfes_tokenizer_h_

#include "lconfig.h"
#include "profile.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <istream>
#include <string>

namespace lilfes {

  class tokenizer {

  public:
    typedef char char_type;
    typedef size_t size_type;
    static const size_type DEFAULT_BUFFER_SIZE = 8192;
    static const size_type DEFAULT_TOKEN_MAX = 1;

  private:
    std::istream& in_stream;
    size_type max_buffer_size;
    char_type* buffer;
    char_type* buffer_ptr;
    char_type* buffer_last;
    char_type* current_token;
    char_type* current_token_ptr;
    char_type* current_token_max;

  protected:
    bool currentTokenEmpty() const {
      return current_token_ptr == current_token;
    }
    void clearCurrentToken() {
      current_token_ptr = current_token;
    }
    void putNewChar( char_type c ) {
      if ( current_token_ptr == current_token_max ) {
        size_type old_size = current_token_max - current_token;
        size_type new_size = old_size << 1;
        char_type* new_token_buffer = new char_type[ new_size + 1 ];
        strncpy( new_token_buffer, current_token, old_size );
        delete [] current_token;
        current_token = new_token_buffer;
        current_token_ptr = current_token + old_size;
        current_token_max = current_token + new_size;
      }
      (*current_token_ptr++) = c;
    }
    void fixCurrentToken() {
      (*current_token_ptr) = '\0';
    }

  public:
    explicit tokenizer( std::istream& is, size_type s = DEFAULT_BUFFER_SIZE, size_type m = DEFAULT_TOKEN_MAX );
    virtual ~tokenizer();

    operator void*() const {
      if ( in_stream || buffer_ptr != buffer_last || ! currentTokenEmpty() ) return const_cast< tokenizer* >( this );
      return NULL;
    }
    bool operator!() const {
      if ( ! in_stream && buffer_ptr == buffer_last && currentTokenEmpty() ) return true;
      return false;
    }

    const char_type* getToken() const {
      return current_token;
    }

    int nextChar() {
      while ( buffer_ptr == buffer_last ) {
        if ( ! in_stream ) return EOF;
        in_stream.read( buffer, max_buffer_size );
        buffer_ptr = buffer;
        buffer_last = buffer + in_stream.gcount();
      }
      return *(buffer_ptr++);
    }

    bool isDelimiter( char_type c, const char_type* delim ) {
      while ( *delim != '\0' ) {
        if ( c == *delim ) return true;
        ++delim;
      }
      return false;
    }

    static bool convert( const char* str, int& i ) {
      char* endptr;
      i = strtol( str, &endptr, 0 );
      return *endptr == '\0';
    }
    static bool convert( const char* str, long int& i ) {
      char* endptr;
      i = strtol( str, &endptr, 0 );
      return *endptr == '\0';
    }
    static bool convert( const char* str, unsigned int& i ) {
      char* endptr;
      i = strtoul( str, &endptr, 0 );
      return *endptr == '\0';
    }
    static bool convert( const char* str, unsigned long int& i ) {
      char* endptr;
      i = strtoul( str, &endptr, 0 );
      return *endptr == '\0';
    }
    static bool convert( const char* str, float& d ) {
      char* endptr;
      d = strtod( str, &endptr );
      return *endptr == '\0';
    }
    static bool convert( const char* str, double& d ) {
      char* endptr;
      d = strtod( str, &endptr );
      return *endptr == '\0';
    }

    template < class T >
    bool nextToken( T& token, const char_type* delim = " \n\r\t" ) {
      if ( readNextToken( delim ) && convert( current_token, token ) ) return true;
      return false;
    }

    bool readNextToken( const char_type* delim = " \n\r\t" );

  };

  template <>
  inline bool tokenizer::nextToken( std::string& token, const char_type* delim ) {
    //P1( "tokeizer::nextToken<string>" );
    if ( readNextToken( delim ) ) {
      token = current_token;
      return true;
    }
    return false;
  }

}

#endif // lilfes_tokenizer_h_
