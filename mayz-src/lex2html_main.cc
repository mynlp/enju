/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file lex2html_main.cc
 *  @version Time-stamp: <2010-07-01 19:51:01 yusuke>
 *  Main program of lex2html
 *
 **********************************************************************/

#include "LexiconTable.h"
#include "FSArray.h"
#include "Word.h"
#include "Derivation.h"
#include <liblilfes/utility.h>
#include <liblilfes/xml.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <set>

using namespace std;
using namespace lilfes;
using namespace mayz;
using namespace up;

struct LexEnt {
  string word;
  string pos;
  string templ;
  vector< string > keys;
  LexEnt( const string& w, const string& p, const string& t, const vector< string >& k )
    : word( w ), pos( p ), templ( t ), keys( k ) {}
};
typedef vector< vector< LexEnt > > Lexbank;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

string grammar_module;
string lexicon_name;
string temptable_name;
string lexbank_name;
string output_dir( "." );
string output_prefix;
string input_stylesheet;
string input_xslt;
int verbose_level = 0;

// procedure* lexicon_lookup_key = NULL;
// procedure* unknown_word_lookup_key = NULL;
// procedure* lex_template_label = NULL;
procedure* sign_label = NULL;
// procedure* lexicon_key_label = NULL;
// procedure* word_label = NULL;

int lex_file_size = 100;
int temp_file_size = 20;
int lexeme_file_size = 100;
int sentence_file_size = 1000;

string charset_string;

const string NAMESPACE_FS = "fs";

//////////////////////////////////////////////////////////////////////

class lex2html_exception : public exception {
private:
  string message;
public:
  lex2html_exception( const string& m ) : message( m ) {}
  virtual ~lex2html_exception() throw () {}
  const char* what() const throw () { return message.c_str(); }
};

//////////////////////////////////////////////////////////////////////

bool load_lexbank( const string& filename, Lexbank& lexbank ) {
  ifstream f(filename.c_str());
  if (!f) return false;
  string s, dummy, word, pos, templ, key;
  vector< LexEnt > sentence;
  while ( getline( f, s ) ) {
    if ( s.empty() ) {
      lexbank.push_back( sentence );
      sentence.clear();
    } else {
      istringstream line(s);
      line >> word >> pos >> dummy >> dummy >> dummy >> dummy >> templ;
      vector< string > keys;
      while ( line >> key ) {
        keys.push_back( key );
      }
      sentence.push_back( LexEnt( word, pos, templ, keys ) );
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

// void inc_count_db( FSArray< FSP, int >& db, FSP key ) {
//   int count = 0;
//   db.find( key, count );
//   if ( ! db.insert( key, ++count ) )
//     throw lex2html_exception( "database access error (inc_count_db)" );
//   return;
// }

// void update_occurrence_db( FSArray< FSP, pair< int, FSP > >& db, FSP key, int id, FSP val ) {
//   pair< int, FSP > dummy;
//   if ( ! db.find( key, dummy ) ) {
//     if ( ! db.insert( key, make_pair( id, val ) ) )
//       throw lex2html_exception( "database access error (update_occurrence_db)" );
//   }
// }

// void update_expand_db( FSArray< FSP, vector< FSP > >& db, FSP key, FSP val ) {
//   vector< FSP > expand_list;
//   db.find( key, expand_list );
//   bool flag = false;
//   for ( vector< FSP >::iterator it = expand_list.begin();
//         it != expand_list.end();
//         ++it ) {
//     if ( builtin::equivalent( *(db.getMachine()), *it, val ) ) {
//       flag = true;
//       break;
//     }
//   }
//   if ( ! flag ) {
//     expand_list.push_back( val );
//     if ( ! db.insert( key, expand_list ) ) {
//       throw lex2html_exception( "database access error (update_expand_db)" );
//     }
//   }
// }

//////////////////////////////////////////////////////////////////////

// FSP get_lexicon_key( machine* mach, FSP word ) {
//   vector< FSP > args( 2 );
//   args[ 0 ] = word;
//   args[ 1 ] = FSP( mach );
//   if ( ! call_proc( *mach, lexicon_lookup_key, args ) )
//     throw lex2html_exception( "calling lexicon_lookup_key/2 failed" );
//   return args[ 1 ];
// }

// FSP get_unknown_word_key( machine* mach, FSP word ) {
//   vector< FSP > args( 2 );
//   args[ 0 ] = word;
//   args[ 1 ] = FSP( mach );
//   if ( ! call_proc( *mach, unknown_word_lookup_key, args ) )
//     throw lex2html_exception( "calling unknown_word_lookup_key/2 failed" );
//   return args[ 1 ];
// }

// string get_template_name( machine* mach, LexTemplate temp ) {
//   string name;
//   vector< FSP > args( 2 );
//   args[ 0 ] = temp;
//   args[ 1 ] = FSP( *mach );
//   if ( call_proc( *mach, lex_template_label, args ) &&
//        args[ 1 ].IsString() ) {
//     name = args[ 1 ].ReadString();
//   }
//   return name;
// }

// string get_lexeme_name( machine*, FSP lexeme ) {
//   string name;
//   if ( lexeme.IsString() ) {
//     name = lexeme.ReadString();
//   }
//   return name;
// }

string get_sign_symbol( machine* mach, FSP sign ) {
  string name;
  vector< FSP > args( 2 );
  args[ 0 ] = sign;
  args[ 1 ] = FSP( *mach );
  if ( call_proc( *mach, sign_label, args ) &&
       args[ 1 ].IsString() ) {
    name = args[ 1 ].ReadString();
  }
  return name;
}

// string get_lexkey_symbol( machine* mach, FSP word ) {
//   string name;
//   vector< FSP > args( 2 );
//   args[ 0 ] = word;
//   args[ 1 ] = FSP( *mach );
//   if ( call_proc( *mach, lexicon_key_label, args ) &&
//        args[ 1 ].IsString() ) {
//     name = args[ 1 ].ReadString();
//   }
//   return name;
// }

// string get_word_symbol( machine* mach, FSP word ) {
//   string name;
//   vector< FSP > args( 2 );
//   args[ 0 ] = word;
//   args[ 1 ] = FSP( *mach );
//   if ( call_proc( *mach, word_label, args ) &&
//        args[ 1 ].IsString() ) {
//     name = args[ 1 ].ReadString();
//   }
//   return name;
// }

// int get_word_id( machine* mach, FSArray< FSP, int >& word_id_map, Word word ) {
//   FSP key = get_lexicon_key( mach, word );
//   int word_id = 0;
//   if ( word_id_map.find( key, word_id ) ) return word_id;
//   key = get_unknown_word_key( mach, word );
//   if ( word_id_map.find( key, word_id ) ) return word_id;
//   return -1;
// }

string get_file_name( const string& name, int id ) {
  ostringstream str;
  str << output_prefix << "-" << name << "-" << id << ".xhtml";
  return str.str();
}

string get_file_name( const string& name ) {
  ostringstream str;
  str << output_prefix << "-" << name << ".xhtml";
  return str.str();
}

string get_file_name( const string& name, const string& ext ) {
  ostringstream str;
  str << output_prefix << "-" << name << "." << ext;
  return str.str();
}

string get_output_file_name( const string& name, int id ) {
  return output_dir + "/" + get_file_name( name, id );
}

string get_output_file_name( const string& name ) {
  return output_dir + "/" + get_file_name( name );
}

string get_output_file_name( const string& name, const string& ext ) {
  return output_dir + "/" + get_file_name( name, ext );
}

void output_index_link( ostream* os ) {
  *os << "<hr />\n<a href=\"" << output_prefix << "-index.xhtml\">Index</a>\n<hr />\n";
}

string encode_xml_string( const string& str ) {
  ostringstream ostr;
  FSXML::outputXMLString( str, ostr );
  return ostr.str();
}

//////////////////////////////////////////////////////////////////////

class HTMLFile : public ofstream {
private:
  string filename;
  string title;
  string stylesheet;
  string xslt;
  string charset_string;
  vector< pair< string, string > > xmlns;
protected:
  void outputHeader() {
    //*this << "<!doctype html public \"-//w3c//dtd html 4.0//en\">\n";
    *this << "<?xml version=\"1.0\"?>\n";
    *this << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n";
    if ( ! xslt.empty() ) {
      *this << "<?xml-stylesheet type=\"text/xsl\" href=\"" << xslt << "\"?>\n";
    }
    *this << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:" << NAMESPACE_FS << "=\"http://www-tsujii.is.s.u-tokyo.ac.jp/lilfes\"";
    for ( vector< pair< string, string > >::const_iterator it = xmlns.begin();
          it != xmlns.end();
          ++it ) {
      *this << " xmlns:" << it->first << "\"" << it->second << "\"";
    }
    *this << ">\n";
    *this << "<head>\n";
    *this << "<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml";
    if ( ! charset_string.empty() ) {
      *this << "; charset=" << charset_string;
    }
    *this << "\" />\n";
    if ( ! stylesheet.empty() ) {
      *this << "<link rel=\"stylesheet\" type=\"text/css\" href=\"" << stylesheet << "\" />\n";
//       ifstream css( stylesheet.c_str() );
//       if ( css ) {
//         *this << "<style type=\"text/css\">\n<!--\n";
//         while ( true ) {
//           string css_line;
//           getline( css, css_line );
//           if ( ! css ) break;
//           *this << css_line << '\n';
//         }
//         *this << "-->\n</style>\n";
//       }
    }
    *this << "<title>" << title << "</title>\n";
    *this << "</head>\n\n";
    *this << "<body>\n";
  }
  void outputFooter() {
    *this << "</body>\n";
    *this << "</html>\n";
  }
public:
  HTMLFile( const string& f, const string& t, const string& s = "", const string& x = "", const string& charset = "" )
    : ofstream( f.c_str() ), filename( f ), title( t ), stylesheet( s ), xslt( x ), charset_string( charset ) {
    if ( *this ) {
      outputHeader();
    }
  }
  virtual ~HTMLFile() {
    if ( *this ) {
      outputFooter();
    }
  }
};

class compare_count {
private:
  map< string, int >* count_db;
public:
  compare_count( map< string, int >* db ) : count_db( db ) {
  }
  ~compare_count() {}
  bool operator()( const string& f1, const string& f2 ) const {
    return (*count_db)[ f1 ] > (*count_db)[ f2 ];
  }
};

// class lexeme_compare_alphabetical {
// private:
//   machine* mach;
// public:
//   lexeme_compare_alphabetical( machine* m ) : mach( m ) {
//   }
//   ~lexeme_compare_alphabetical() {}
//   bool operator()( FSP l1, FSP l2 ) const {
//     return get_lexeme_name( mach, l1 ) < get_lexeme_name( mach, l2 );
//   }
// };

// class symbol_compare {
// public:
//   bool operator()( const pair< string, int >& s1, const pair< string, int >& s2 ) const {
//     return s1.second > s2.second;
//   }
// };

//////////////////////////////////////////////////////////////////////

void output_lexeme_data( ostream* ostr, const string& lexeme,
                         map< string, int >& lexeme_id_map,
                         map< string, int >& lexeme_count_db,
                         map< string, pair< int, string > >& lexeme_occurrence_db,
                         map< string, int >& key_id_map ) {
  int count = lexeme_count_db[ lexeme ];
  int lexeme_id = lexeme_id_map[ lexeme ];
  *ostr << "No. " << lexeme_id << " <a href=\""
        << get_file_name( "lexeme", lexeme_id / lexeme_file_size ) << "#"
        << lexeme_id << "\">" << " " << encode_xml_string( lexeme ) << "</a> Freq.=" << count << "\n";
  map< string, pair< int, string > >::const_iterator occurrence = lexeme_occurrence_db.find( lexeme );
  if ( occurrence != lexeme_occurrence_db.end() ) {
    const string& key = occurrence->second.second;
    if ( key_id_map.find( key ) != key_id_map.end() ) {
      int key_id = key_id_map[ key ];
      *ostr << " for <a href=\"" << get_file_name( "lex", key_id / lex_file_size ) << "#"
            << key_id << "\">\"" << encode_xml_string( key ) << "\"</a>\n";
    } else {
      *ostr << " for \"" << encode_xml_string( key ) << "\"";
    }
  }
  return;
}

//////////////////////////////////////////////////////////////////////

void help_message() {
  *error_stream << "Usage: lex2html [options] grammar_module lexicon_name temptable_name lexbank_name output_prefix" << endl;
  *error_stream << "  grammar_module: LiLFeS program to be loaded" << endl;
  *error_stream << "  lexicon_name: lexicon" << endl;
  *error_stream << "  temptable_name: template table" << endl;
  *error_stream << "  lexbank_name: lexbank" << endl;
  *error_stream << "  output_prefix: prefix of output files" << endl;
  *error_stream << "Options:" << endl;
  *error_stream << "  -c file:\tcss stylesheet" << endl;
  *error_stream << "  -x file:\txslt stylesheet" << endl;
  *error_stream << "  -d directory:\toutput directory" << endl;
  *error_stream << "  -lexsize int, -tmpsize int, -lxmsize int, -sntsize int: size of HTML files" << endl;
  *error_stream << "  -charset string: charset specification in HTML header" << endl;
  *error_stream << "  -v:\t\tprint verbose messages" << endl;
  *error_stream << "  -vv:\t\tprint many verbose messages" << endl;
  return;
}

bool analyze_arguments( int argc, char** argv ) {
  char** end = argv + argc;
  ++argv;
  for ( ; argv != end; ++argv ) {
    if ( (*argv)[ 0 ] == '-' ) {
      // option without argument
      if ( std::strcmp( (*argv), "-v" ) == 0 ) {
        verbose_level = 1;
      } else if ( std::strcmp( (*argv), "-vv" ) == 0 ) {
        verbose_level = 2;
      } else {
        // option with argument
        if ( argv + 1 == end ) {
          *error_stream << *argv << " option requires additional argument" << endl;
          return false;
        }
        if ( std::strcmp( (*argv), "-c" ) == 0 ) {
          input_stylesheet = *(++argv);
        } else if ( std::strcmp( (*argv), "-x" ) == 0 ) {
          input_xslt = *(++argv);
        } else if ( std::strcmp( (*argv), "-d" ) == 0 ) {
          output_dir = *(++argv);
        } else if ( std::strcmp( (*argv), "-charset" ) == 0 ) {
          charset_string = *(++argv);

        } else if ( std::strcmp( (*argv), "-lexsize" ) == 0 ) {
          lex_file_size = strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-tmpsize" ) == 0 ) {
          temp_file_size =  strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-lxmsize" ) == 0 ) {
          lexeme_file_size =  strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-sntsize" ) == 0 ) {
          sentence_file_size =  strtol( *(++argv), NULL, 0 );
        } else {
          *error_stream << "Unknown option: " << *argv << endl;
          return false;
        }
      }
    } else {
      // arguments
      if ( end - argv != 5 ) {
        *error_stream << "lex2html requires five arguments" << endl;
        return false;
      }
      grammar_module = argv[ 0 ];
      lexicon_name = argv[ 1 ];
      temptable_name = argv[ 2 ];
      lexbank_name = argv[ 3 ];
      output_prefix = argv[ 4 ];
      return true;
    }
  }
  *error_stream << "lex2html requires five arguments" << endl;
  return false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int main( int argc, char** argv ) {

  *error_stream << "lex2html (MAYZ " << MAYZ_VERSION << ")" << endl;
  *error_stream << MAYZ_COPYRIGHT << endl;

  Initializer::PerformAll();

  // show help message
  if ( argc > 1 && std::strcmp( argv[ 1 ], "-h" ) == 0 ) {
    help_message();
    return 0;
  }
  // command-line arguments
  if ( ! analyze_arguments( argc, argv ) ) {
    help_message();
    return 1;
  }

  // make stylesheets
  string stylesheet;
  if ( ! input_stylesheet.empty() ) {
    ifstream f( input_stylesheet.c_str() );
    if ( ! f ) {
      *error_stream << "Cannot open css stylesheet: " << input_stylesheet << endl;
      return 1;
    }
    stylesheet = get_file_name( "css", "css" );
    ofstream of( get_output_file_name( "css", "css" ).c_str() );
    copy( istreambuf_iterator< char >( f ),
          istreambuf_iterator< char >(),
          ostreambuf_iterator< char >( of ) );
  }
  string xslt;
  if ( ! input_xslt.empty() ) {
    ifstream f( input_xslt.c_str() );
    if ( ! f ) {
      *error_stream << "Cannot open css stylesheet: " << input_xslt << endl;
      return 1;
    }
    xslt = get_file_name( "xsl", "xsl" );
    ofstream of( get_output_file_name( "xsl", "xsl" ).c_str() );
    copy( istreambuf_iterator< char >( f ),
          istreambuf_iterator< char >(),
          ostreambuf_iterator< char >( of ) );
  }

  machine mach;

  try {

    // Load grammar module
    *error_stream << "Loading grammar module \"" << grammar_module << "\"... ";
    if ( ! load_module( mach, "grammar module", grammar_module ) ) {
      *error_stream << "Grammar module \"" << grammar_module << "\" not found" << endl;
      return 1;
    }
    *error_stream << "done." << endl;

    // Initialization
    *error_stream << "Initializing... ";
//     lexicon_lookup_key = prepare_proc( module::UserModule(), "lexicon_lookup_key", 2 );
//     if ( ! lexicon_lookup_key ) {
//       *error_stream << "lexicon_lookup_key/2 not found" << endl;
//       return 1;
//     }
//     unknown_word_lookup_key = prepare_proc( module::UserModule(), "unknown_word_lookup_key", 2 );
//     if ( ! unknown_word_lookup_key ) {
//       *error_stream << "unknown_word_lookup_key/2 not found" << endl;
//       return 1;
//     }
//     lex_template_label = prepare_proc( module::UserModule(), "lex_template_label", 2 );
//     if ( ! lex_template_label ) {
//       *error_stream << "lex_template_label/2 not found" << endl;
//       return 1;
//     }
    sign_label = prepare_proc( module::UserModule(), "sign_label", 2 );
    if ( ! sign_label ) {
      *error_stream << "sign_label/2 not found" << endl;
      return 1;
    }
//     lexicon_key_label = prepare_proc( module::UserModule(), "lexicon_key_label", 2 );
//     if ( ! lexicon_key_label ) {
//       *error_stream << "lexicon_key_label/2 not found" << endl;
//       return 1;
//     }
//     word_label = prepare_proc( module::UserModule(), "word_label", 2 );
//     if ( ! word_label ) {
//       *error_stream << "word_label/2 not found" << endl;
//       return 1;
//     }

    *error_stream << "done." << endl;

  } catch ( exception& e ) {
    *error_stream << "Initialization failed: " << e.what() << endl;
    return 1;
  }

  //////////////////////////////////////////////////
  //  Load database
  //////////////////////////////////////////////////

  LexiconTable lexicon;
  FSArray< string, FSP > temptable( &mach );
  Lexbank lexbank;

  map< string, int > key_count_db;               // (key, count)
  map< string, int > temp_count_db;              // (template name, count)
  map< string, int > lexeme_count_db;            // (lexeme name, count)
  map< string, pair< int, string > > temp_occurrence_db;    // (template name, <sentence id, word>)
  map< string, pair< int, string > > lexeme_occurrence_db;  // (lexeme name, <sentence id, word>)
  map< string, set< string > > lexeme_expand_db;         // (lexeme name, list of template names)
  map< string, set< string > > lexeme_expand_db2;        // (lexeme name, list of template names)
  size_t num_sentences = 0;

  vector< string > key_list;
  vector< string > temp_list;
  vector< string > lexeme_list;
  vector< string > lexeme_list_alphabetical;
  map< string, int > key_id_map;      // (key, key id)
  map< string, int > temp_id_map;      // (template name, template id)
  map< string, int > lexeme_id_map;    // (lexeme name, lexeme id)
  map< string, string > temp_symbol_db;  // (template name, NTS)
  map< string, int > symbol_count_db;            // (NTS, count)

  try {
    *error_stream << "Loading lexicon: " << lexicon_name << endl;
    if ( ! lexicon.load( lexicon_name ) ) {
      *error_stream << "failed." << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading template table: " << temptable_name << endl;
    if ( ! temptable.load( temptable_name ) ) {
      *error_stream << "failed." << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading lexbank: " << lexbank_name << endl;
    if ( ! load_lexbank( lexbank_name, lexbank ) ) {
      *error_stream << "failed." << endl;
      return 1;
    }
    *error_stream << "done." << endl;
  } catch ( exception& e ) {
    *error_stream << "Loading database failed: " << e.what() << endl;
    return 1;
  }

  //////////////////////////////////////////////////
  //  Collect statistics
  //////////////////////////////////////////////////

  *error_stream << "Collecting lexbank statistics..." << endl;
  size_t sentence_id = 0;
  try {
    for ( ; sentence_id < lexbank.size(); ++sentence_id ) {
      const vector< LexEnt >& sentence = lexbank[ sentence_id ];
      if ( sentence.empty() ) continue;  // empty sentence
      ++num_sentences;
      for ( vector< LexEnt >::const_iterator lex_it = sentence.begin();
            lex_it != sentence.end();
            ++lex_it ) {
        // increment key count
        ++key_count_db[ lex_it->keys.front() ];
        // update template info.
        ++temp_count_db[ lex_it->templ ];
        temp_occurrence_db.insert( make_pair( lex_it->templ, make_pair( sentence_id, lex_it->word ) ) );
        // update lexeme info.
        string lexeme = lex_it->templ.substr( 0, lex_it->templ.find( "-" ) );
        ++lexeme_count_db[ lexeme ];
        lexeme_occurrence_db.insert( make_pair( lexeme, make_pair( sentence_id, lex_it->word ) ) );
        lexeme_expand_db2[ lexeme ].insert( lex_it->templ );
      }
      if ( sentence_id + 1 % 100 == 0 ) {
        *error_stream << "  " << sentence_id << " sentences processed." << endl;
      }
    }
  } catch ( exception& e ) {
    *error_stream << "Error in sentence " << sentence_id << ": " << e.what() << endl;
    return 1;
  }
  *error_stream << "  " << sentence_id << " sentences processed." << endl;
  *error_stream << "  # non-empty sentences: " << num_sentences << endl;
  *error_stream << "done." << endl;

  //////////////////////////////////////////////////
  //  Sorting
  //////////////////////////////////////////////////

  try {
    *error_stream << "Sorting lexicon... ";
    for ( LexiconTable::iterator key_it = lexicon.begin();
          key_it != lexicon.end();
          ++key_it ) {
      key_list.push_back( key_it->first );
    }
    sort( key_list.begin(), key_list.end(), compare_count( &key_count_db ) );
    for ( size_t i = 0; i < key_list.size(); ++i ) {
      key_id_map[ key_list[ i ] ] = i;
    }
    *error_stream << "done." << endl;
    *error_stream << "Sorting templates... ";
    for ( FSArray< string, FSP >::iterator temp_it = temptable.begin();
          temp_it != temptable.end();
          ++temp_it ) {
      temp_list.push_back( temp_it->first );
      string symbol = get_sign_symbol( &mach, temp_it->second );
      temp_symbol_db[ temp_it->first ] = symbol;
      symbol_count_db[ symbol ] += temp_count_db[ temp_it->first ];
      string lexeme = temp_it->first.substr( 0, temp_it->first.find( "-" ) );
      lexeme_expand_db[ lexeme ].insert( temp_it->first ); 
    }
    sort( temp_list.begin(), temp_list.end(), compare_count( &temp_count_db ) );
    for ( size_t i = 0; i < temp_list.size(); ++i ) {
      temp_id_map[ temp_list[ i ] ] = i;
    }
    *error_stream << "done." << endl;
    *error_stream << "Sorting lexemes... ";
    for ( map< string, int >::iterator lexeme_it = lexeme_count_db.begin();
          lexeme_it != lexeme_count_db.end();
          ++lexeme_it ) {
      lexeme_list.push_back( lexeme_it->first );
      lexeme_list_alphabetical.push_back( lexeme_it->first );
    }
    sort( lexeme_list.begin(), lexeme_list.end(), compare_count( &lexeme_count_db ) );
    sort( lexeme_list_alphabetical.begin(), lexeme_list_alphabetical.end() );
    for ( size_t i = 0; i < lexeme_list.size(); ++i ) {
      lexeme_id_map[ lexeme_list[ i ] ] = i;
    }
    *error_stream << "done." << endl;
  } catch( exception& e ) {
    *error_stream << "Fatal error: " << e.what() << endl;
    return 1;
  }

  //////////////////////////////////////////////////
  //  Output
  //////////////////////////////////////////////////

  *error_stream << "Output HTML files... ";
  try {

    // Output lexicon files
    string output_name = get_output_file_name( "lex", 0 );
    ofstream* output_file = new HTMLFile( output_name.c_str(), "Lexicon", stylesheet, xslt, charset_string );
    if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
    int num_lex = 0;
    for ( vector< string >::const_iterator key_it = key_list.begin();
          key_it != key_list.end();
          ++key_it ) {
      ++num_lex;
      int count = key_count_db[ *key_it ];
      int key_id = key_id_map[ *key_it ];
      *output_file << "<div>\n";
      *output_file << "<a name=\"" << key_id << "\">No. " << num_lex << " "
		   << *key_it << " Freq.=" << count << "</a>\n";
      vector< string > key_list;
      key_list.push_back( *key_it );
      vector< string > temp_list;
      if ( lexicon.lookup( key_list, temp_list ) ) {
        *output_file << "<ul>\n";
        for ( vector< string >::iterator temp_it = temp_list.begin();
              temp_it != temp_list.end();
              ++temp_it ) {
          int temp_id = temp_id_map[ *temp_it ];
          int count = temp_count_db[ *temp_it ];
          *output_file << "<li><a href=\"" << get_file_name( "temp", temp_id / temp_file_size ) << "#"
                       << temp_id << "\">" << encode_xml_string( *temp_it ) << "</a> Freq.="
                       << count << "</li>\n";
        }
        *output_file << "</ul>\n";
      }
      *output_file << "</div>\n";
      output_index_link( output_file );
      if ( num_lex % lex_file_size == 0 ) {
        delete output_file;
        output_name = get_output_file_name( "lex", num_lex / lex_file_size );
        output_file = new HTMLFile( output_name.c_str(), "Lexicon", stylesheet, xslt, charset_string );
        if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
      }
    }
    delete output_file;

    // Output template files
    output_name = get_output_file_name( "temp", 0 );
    output_file = new HTMLFile( output_name.c_str(), "List of templates", stylesheet, xslt, charset_string );
    if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
    int num_temp = 0;
    for ( vector< string >::const_iterator temp_it = temp_list.begin();
          temp_it != temp_list.end();
          ++temp_it ) {
      ++num_temp;
      int count = temp_count_db[ *temp_it ];
      int temp_id = temp_id_map[ *temp_it ];
      *output_file << "<div><a name=\"" << temp_id << "\">No. " << num_temp << " " << encode_xml_string( *temp_it ) << " Freq.=" << count << "</a></div>\n";
      map< string, pair< int, string > >::const_iterator occurrence = temp_occurrence_db.find( *temp_it );
      if ( occurrence != temp_occurrence_db.end() ) {
        const string& key = occurrence->second.second;
        if ( key_id_map.find( key ) != key_id_map.end() ) {
          int key_id = key_id_map[ key ];
          *output_file << "<div>Occurred for <a href=\"" << get_file_name( "lex", key_id / lex_file_size )
                       << "#" << key_id << "\">\""
                       << encode_xml_string( key ) << "\"</a>";
        } else {
          *output_file << "<div>Occurred for \"" << encode_xml_string( key ) << "\"";
        }
        if ( ! lexbank[ occurrence->second.first ].empty() ) {
          *output_file << " in <a href=\"" << get_file_name( "sentence", occurrence->second.first / sentence_file_size )
                       << "#" << occurrence->second.first << "\">sentence " << occurrence->second.first << "</a></div>\n";
        } else {
          *output_file << " in sentence " << occurrence->second.first << "</div>\n";
        }
      }
      FSP template_fs( mach );
      if ( temptable.find( *temp_it, template_fs ) ) {
        FSXML fs_xml;
        fs_xml.input( template_fs, NAMESPACE_FS );
        *output_file << "<div>" << fs_xml.getString() << "</div>\n";
      }
      output_index_link( output_file );
      if ( num_temp % temp_file_size == 0 ) {
        delete output_file;
        output_name = get_output_file_name( "temp", num_temp / temp_file_size );
        output_file = new HTMLFile( output_name, "List of templates", stylesheet, xslt, charset_string );
        if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
      }
    }
    delete output_file;

    // Output lexeme files
    output_name = get_output_file_name( "lexeme", 0 );
    output_file = new HTMLFile( output_name.c_str(), "List of lexemes", stylesheet, xslt, charset_string );
    if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
    int num_lexeme = 0;
    for ( vector< string >::const_iterator lexeme_it = lexeme_list.begin();
          lexeme_it != lexeme_list.end();
          ++lexeme_it ) {
      ++num_lexeme;
      int count = lexeme_count_db[ *lexeme_it ];
      int lexeme_id = lexeme_id_map[ *lexeme_it ];
      *output_file << "<div><a name=\"" << lexeme_id << "\">No. " << num_lexeme << " " << encode_xml_string( *lexeme_it ) << " Freq.=" << count << "</a></div>\n";
      map< string, pair< int, string > >::const_iterator occurrence = lexeme_occurrence_db.find( *lexeme_it );
      if ( occurrence != lexeme_occurrence_db.end() ) {
        const string& key = occurrence->second.second;
        if ( key_id_map.find( key ) != key_id_map.end() ) {
          int key_id = key_id_map[ key ];
          *output_file << "<div>Occurred for <a href=\"" << get_file_name( "lex", key_id / lex_file_size )
                       << "#" << key_id << "\">\""
                       << encode_xml_string( key ) << "\"</a>";
        } else {
          *output_file << "<div>Occurred for \"" << encode_xml_string( key ) << "\"";
        }
        if ( ! lexbank[ occurrence->second.first ].empty() ) {
          *output_file << " in <a href=\"" << get_file_name( "sentence", occurrence->second.first / sentence_file_size )
                       << "#" << occurrence->second.first << "\">sentence "
                       << occurrence->second.first << "</a></div>\n";
        } else {
          *output_file << " in sentence " << occurrence->second.first << "</div>\n";
        }
      }
      *output_file << "<ul>\n";
      vector< string > expand_list( lexeme_expand_db[ *lexeme_it ].begin(), lexeme_expand_db[ *lexeme_it ].end() );
      stable_sort( expand_list.begin(), expand_list.end(), compare_count( &temp_count_db ) );
      for ( vector< string >::iterator expand_it = expand_list.begin();
            expand_it != expand_list.end();
            ++expand_it ) {
        int temp_count = temp_count_db[ *expand_it ];
        int temp_id = temp_id_map[ *expand_it ];
        *output_file << "  <li><a href=\"" << get_file_name( "temp", temp_id / temp_file_size )
                     << "#" << temp_id << "\">"
                     << encode_xml_string( *expand_it ) << "</a> Freq.=" << temp_count << "</li>\n";
        }
      vector< string > expand_list2( lexeme_expand_db[ *lexeme_it ].begin(), lexeme_expand_db[ *lexeme_it ].end() );
      stable_sort( expand_list2.begin(), expand_list2.end(), compare_count( &temp_count_db ) );
      for ( vector< string >::iterator expand_it = expand_list2.begin();
            expand_it != expand_list2.end();
            ++expand_it ) {
        if ( find( expand_list.begin(), expand_list.end(), *expand_it ) == expand_list.end() ) {
          int temp_count = temp_count_db[ *expand_it ];
          int temp_id = temp_id_map[ *expand_it ];
          *output_file << "  <li><a href=\"" << get_file_name( "temp", temp_id / temp_file_size )
                       << "#" << temp_id << "\">"
                       << encode_xml_string( *expand_it ) << "</a> Freq.=" << temp_count << " (not in the lexicon)</li>\n";
        }
      }
      *output_file << "</ul>\n";
      output_index_link( output_file );
      if ( num_lexeme % lexeme_file_size == 0 ) {
        delete output_file;
        output_name = get_output_file_name( "lexeme", num_lexeme / lexeme_file_size );
        output_file = new HTMLFile( output_name, "List of lexemes", stylesheet, xslt, charset_string );
        if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
      }
    }
    delete output_file;

    // Output sentence file
    output_name = get_output_file_name( "sentence", 0 );
    output_file = new HTMLFile( output_name, "List of sentences", stylesheet, xslt, charset_string );
    if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
    for ( size_t i = 0; i < lexbank.size(); ++i ) {
      const vector< LexEnt >& sentence = lexbank[ i ];
      if ( sentence.empty() ) continue;
      *output_file << "<div><a name=\"" << i << "\">No. " << i << ":";
      for ( vector< LexEnt >::const_iterator lex_it = sentence.begin();
            lex_it != sentence.end();
            ++lex_it ) {
        *output_file << " ";
        if ( key_id_map.find( lex_it->keys.front() ) != key_id_map.end() ) {
          int key_id = key_id_map[ lex_it->keys.front() ];
          *output_file << "<a href=\"" << get_file_name( "lex", key_id / lex_file_size )
                       << "#" << key_id << "\">"
                       << encode_xml_string( lex_it->word ) << "/"
                       << encode_xml_string( lex_it->pos ) << "</a>";
        } else {
          *output_file << encode_xml_string( lex_it->word )
                       << encode_xml_string( lex_it->pos );
        }
      }
      *output_file << "</a></div>\n";
      if ( i % 10 == 0 ) output_index_link( output_file );
      if ( i % sentence_file_size == 0 ) {
        delete output_file;
        output_name = get_output_file_name( "sentence", i / sentence_file_size );
        output_file = new HTMLFile( output_name, "List of sentences", stylesheet, xslt, charset_string );
        if ( ! *output_file ) throw lex2html_exception( ( "Cannot open output file: " + output_name ).c_str() );
      }
    }
    delete output_file;

    // Output symbol file
    for ( map< string, int >::const_iterator symbol_it = symbol_count_db.begin();
          symbol_it != symbol_count_db.end();
          ++symbol_it ) {
      string output_symbol_name = get_output_file_name( "symbol-" + symbol_it->first );
      HTMLFile output_symbol_file( output_symbol_name, symbol_it->first, stylesheet, xslt, charset_string );
      output_symbol_file << "<h3>" << symbol_it->first << " Freq.=" << symbol_it->second << "</h3>\n";
      // Output in frequency order
      output_symbol_file << "<ol>\n";
      {
        size_t num_lexemes = 0;
        for ( vector< string >::iterator lexeme_it = lexeme_list.begin();
              lexeme_it != lexeme_list.end();
              ++lexeme_it ) {
          map< string, string >::const_iterator symbol_it2 = temp_symbol_db.find( *lexeme_it );
          if ( symbol_it2 != temp_symbol_db.end()
               && symbol_it2->second == symbol_it->first ) {
            ++num_lexemes;
            output_symbol_file << "<li>";
            output_lexeme_data( &output_symbol_file, *lexeme_it,
                                lexeme_id_map, lexeme_count_db, lexeme_occurrence_db,
                                key_id_map );
            output_symbol_file << "</li>";
            if ( num_lexemes % 10 == 0 ) {
              output_symbol_file << "<hr /><a href=\"" << output_prefix << "-index.xhtml\">Index</a>\n<hr />\n";
            }
          }
        }
      }
      output_symbol_file << "</ol>\n";
      // Output in alphabetical order
      output_symbol_file << "<h3>Alphabetical order</h3>\n";
      output_symbol_file << "<ol>\n";
      {
        size_t num_lexemes = 0;
        for ( vector< string >::iterator lexeme_it = lexeme_list_alphabetical.begin();
              lexeme_it != lexeme_list_alphabetical.end();
              ++lexeme_it ) {
          map< string, string >::const_iterator symbol_it2 = temp_symbol_db.find( *lexeme_it );
          if ( symbol_it2 != temp_symbol_db.end()
               && symbol_it2->second == symbol_it->first ) {
            ++num_lexemes;
            output_symbol_file << "<li>";
            output_lexeme_data( &output_symbol_file, *lexeme_it,
                                lexeme_id_map, lexeme_count_db, lexeme_occurrence_db,
                                key_id_map );
            output_symbol_file << "</li>";
            if ( num_lexemes % 10 == 0 ) {
              output_symbol_file << "<hr /><a href=\"" << output_prefix << "-index.xhtml\">Index</a>\n<hr />\n";
            }
          }
        }
      }
      output_symbol_file << "</ol>\n";
      output_symbol_file << "<hr /><a href=\"" << output_prefix << "-index.xhtml\">Index</a>\n<hr />\n";
    }

    // Output index file
    string index_name = get_output_file_name( "index" );
    HTMLFile index_file( index_name, "Index: " + output_prefix, stylesheet, xslt, charset_string );
    if ( ! index_file ) throw ( ( "Cannot open output file: " + index_name ).c_str() );
    // sentences
    index_file << "<h3>Index of sentences</h3>\n";
    index_file << "<ul>\n";
    for ( size_t i = 0; i < lexbank.size(); i += sentence_file_size ) {
      string filename = get_file_name( "sentence", i / sentence_file_size );
      index_file << "  <li><a href=\"" << filename << "\">" << filename << "</a></li>\n";
    }
    index_file << "</ul>\n";
    // symbols
    index_file << "<h3>Index of symbols</h3>\n";
    index_file << "<ul>\n";
    for ( map< string, int >::iterator it = symbol_count_db.begin();
          it != symbol_count_db.end();
          ++it ) {
      index_file << "  <li><a href=\"" << get_file_name( "symbol-" + it->first ) << "\">" << it->first << "</a> Freq.=" << it->second << "</li>\n";
    }
    index_file << "</ul>\n";
    // lexemes
    index_file << "<h3>Index of lexemes</h3>\n";
    index_file << "<ul>\n";
    for ( size_t i = 0; i < lexeme_list.size(); i += lexeme_file_size ) {
      string filename = get_file_name( "lexeme", i / lexeme_file_size );
      index_file << "  <li><a href=\"" << filename << "\">" << filename << "</a></li>\n";
    }
    index_file << "</ul>\n";
    // templates
    index_file << "<h3>Index of templates</h3>\n";
    index_file << "<ul>\n";
    for ( size_t i = 0; i < temp_list.size(); i += temp_file_size ) {
      string filename = get_file_name( "temp", i / temp_file_size );
      index_file << "  <li><a href=\"" << filename << "\">" << filename << "</a></li>\n";
    }
    index_file << "</ul>\n";
    // lexicon
    index_file << "<h3>Index of lexicon</h3>\n";
    index_file << "<ul>\n";
    for ( size_t i = 0; i < key_list.size(); i += lex_file_size ) {
      string filename = get_file_name( "lex", i / lex_file_size );
      index_file << "  <li><a href=\"" << filename << "\">" << filename << "</a></li>\n";
    }
    index_file << "</ul>\n";
    *error_stream << "done." << endl;

  } catch ( exception& e ) {
    cerr << "Fatal error: " << e.what() << endl;
    return 1;
  }

  Terminator::PerformAll();
}

