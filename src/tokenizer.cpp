#include "tokenizer.h"

#include <istream>

namespace lilfes {

using std::istream;

tokenizer::tokenizer( istream& is, size_type s, size_type m )
  : in_stream( is ), max_buffer_size( s ) {
	if ( max_buffer_size <= 0 ) max_buffer_size = DEFAULT_BUFFER_SIZE;
	buffer = new char_type[ max_buffer_size ];
	buffer_ptr = buffer_last = buffer;
	current_token = new char_type[ m + 1 ];
	current_token_ptr = current_token;
	current_token_max = current_token + m;
}

tokenizer::~tokenizer() {
    delete [] current_token;
    delete [] buffer;
}

bool tokenizer::readNextToken( const char_type* delim ) {
    //P1( "tokenizer::readNextToken" );
    clearCurrentToken();
    int next_char = nextChar();
    while ( next_char != EOF && isDelimiter( next_char, delim ) )
		next_char = nextChar();
    while ( next_char != EOF && ! isDelimiter( next_char, delim ) ) {
		putNewChar( next_char );
		next_char = nextChar();
    }
    fixCurrentToken();
    return ! currentTokenEmpty();
}

}

