// escape special characters in input sentences/POSs

#include <vector>
#include <string>
#include <iostream>
#include <ctype.h>

using namespace std;

inline void push( vector< char >& s1, const char* s2, unsigned int len ) {
  s1.insert( s1.end(), s2, s2 + len );
}

void escape( const string& input, string& word, bool to_lower = false ) {
  if ( input == "HYPH" ) {  // hack to deal with PennBio POS
    word = "-COLON-";
    return;
  }
  vector< char > s;
  for ( string::const_iterator it = input.begin(); it != input.end(); ++it ) {
    if ( isdigit( *it ) ) {
      int i = 0;
      bool is_digit = true;
      while ( it + i != input.end() &&
              ( isdigit( *( it + i ) ) || *( it + i ) == ',' ) ) {
        if ( ! isdigit( *( it + i ) ) ) is_digit = false;
        ++i;
      }
      if ( input.size() == 4 && i == 4 && is_digit ) {
        push( s, "-YEAR-", 6 );
      } else {
        push( s, "-NUMBER-", 8 );
      }
      it += i - 1;
      continue;
    }
    if ( it + 1 != input.end() ) {
      if ( *it == '\\' && *(it+1) == '/' ) {
        push( s, "-SLASH-", 7 );
        ++it;
        continue;
      } else if ( *it == '\\' && *(it+1) == '\\' ) {
        push( s, "-BS-", 4 );
        ++it;
        continue;
      } else if ( *it == '`' && *(it+1) == '`' ) {
        push( s, "-DQS-", 5 );
        ++it;
        continue;
      } else if ( *it == '\'' && *(it+1) == '\'' ) {
        push( s, "-DQE-", 5 );
        ++it;
        continue;
      }
    }
    switch ( *it ) {
    case '/': push( s, "-SLASH-", 7 ); break;
    case '(': push( s, "-LRB-", 5 ); break;
    case ')': push( s, "-RRB-", 5 ); break;
    case '[': push( s, "-LCB-", 5 ); break;
    case ']': push( s, "-RCB-", 5 ); break;
    case '{': push( s, "-LCB-", 5 ); break;
    case '}': push( s, "-RCB-", 5 ); break;
    case ':': push( s, "-COLON-", 7 ); break;
    case ';': push( s, "-SEMICOLON-", 11 ); break;
    case '#': push( s, "-SHARP-", 7 ); break;
    case ',': push( s, "-COMMA-", 7 ); break;
    case '.': push( s, "-PERIOD-", 8 ); break;
    case '$': push( s, "-DOLLAR-", 8 ); break;
    default: if ( to_lower ) s.push_back( tolower( *it ) ); else s.push_back( *it );
    }
  }
  word = string( s.begin(), s.end() );
}

void escape_word( const string& input, const std::string &input_pos, string& word, bool to_lower = false ) {
  if ( input == "\"" ) { // open/close determination of a double-quote char
    if ( input_pos == "``" || input_pos == "-DQS-" ) {
      word = "-DQS-";
    }
    else if ( input_pos == "''" || input_pos == "-DQE-" ) {
      word = "-DQE-";
    }
    else { // default: open
      word = "-DQS-";
    }
    return;
  }
  else if ( input == "'" ) { // open/close determination of "'"
    if ( input_pos == "``" || input_pos == "-DQS-" ) {
      word = "`";
    }
    else if ( input_pos == "''" || input_pos == "-DQE-" ) {
      word = "'";
    }
    else { // default: close (as is)
      word = "'";
    }
    return;
  }
  else {
    escape(input, word, to_lower);
  }
}

void escape_pos( const string& input, string& word, bool to_lower = false ) {
  escape(input, word, to_lower);
}

// for test
// int main( void ) {
//   string input;
//   while ( cin >> input ) {
//     string str;
//     escape( input, str );
//     cout << str << endl;
//   }
// }

