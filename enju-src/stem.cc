// simple implementation of a stemmer
// assuming PTB-style POS

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <map>
#include <functional>
#include <algorithm>
#include <ctype.h>

//#include <liblilfes/errors.h>
//#include <liblilfes/CppPred.h>

using namespace std;
//using namespace lilfes;

enum POS {
  NOUN, VERB, ADJ, ADV, POS_SIZE
};

map< string, string > exc_list[ POS_SIZE ];
set< string > base_list[ POS_SIZE ];

//////////////////////////////////////////////////////////////////////

// remove prefix before non-alphabets
int removePrefix( const string& word, string& word2 ) {
  string::const_reverse_iterator it = find_if( word.rbegin(), word.rend(),
                                               not1( pointer_to_unary_function<int, int>(isalpha) ) );
  if ( it != word.rend() ) {
    int len = it - word.rbegin();
    if ( len > 0 ) {
      word2 = word.substr( word.size() - len );
      return len;
    }
  }
  return 0;
}
// remove all non-alphabets
bool removeSymbols( const string& word, string& word2 ) {
  string::const_iterator prev_it = word.begin();
  string::const_iterator it = find_if( word.begin(), word.end(),
                                       not1( pointer_to_unary_function<int, int>(isalpha) ) );
  if ( it == word.end() ) return false;
  word2.reserve( word.size() );
  while ( it != word.end() ) {
    word2 += word.substr( prev_it - word.begin(), it - prev_it );
    prev_it = it + 1;
    it = find_if( prev_it, word.end(),
                  not1( pointer_to_unary_function<int, int>(isalpha) ) );
  }
  word2 += word.substr( prev_it - word.begin() );
  //cerr << "remove: " << word2 << endl;
  return true;
}

// search for the exceptional form
bool findExc( const string& word, POS pos, string& base ) {
  map< string, string >::const_iterator it = exc_list[ pos ].find( word );
  if ( it != exc_list[ pos ].end() ) {
    base = it->second;
    return true;
  }
  string word2;
  int len = removePrefix( word, word2 );
  if ( len ) {
    it = exc_list[ pos ].find( word2 );
    if ( it != exc_list[ pos ].end() ) {
      base = word.substr( 0, word.size() - len ) + it->second;
      return true;
    }
  }
  string word3;
  if ( removeSymbols( word, word3 ) ) {
    it = exc_list[ pos ].find( word3 );
    if ( it != exc_list[ pos ].end() ) {
      base = it->second;
      return true;
    }
  }
  return false;
}

// search the database for a base form
bool findBase( const string& base, POS pos ) {
  set< string >::const_iterator it = base_list[ pos ].find( base );
  if ( it != base_list[ pos ].end() ) return true;
  string base2;
  if ( removePrefix( base, base2 ) ) {
    it = base_list[ pos ].find( base2 );
    if ( it != base_list[ pos ].end() ) return true;
  }
  string base3;
  if ( removeSymbols( base, base3 ) ) {
    it = base_list[ pos ].find( base3 );
    if ( it != base_list[ pos ].end() ) return true;
  }
  return false;
}

// import WordNet files
bool importExcFile( const string& file_name, POS pos ) {
  ifstream exc_file( file_name.c_str() );
  if ( ! exc_file ) {
    //RUNWARN( "Warning: import_dictionary: cannot open " + file_name );
    return false;
  }

  map< string, string >& exc = exc_list[ pos ];
  string line;
  while ( getline( exc_file, line ) ) {
    string::size_type p = line.find( ' ' );
    string::size_type q = line.find_first_not_of( ' ', p );
    string::size_type r = line.find( ' ', q );
    string::size_type n = r == string::npos ? string::npos : r - q;
    exc[ line.substr( 0, p ) ] = line.substr( q, n );
    //cerr << line.substr( 0, p ) << '/' << line.substr( q ) << endl;
  }
  return true;
}
bool importIndexFile( const string& file_name, POS pos ) {
  ifstream index_file( file_name.c_str() );
  if ( ! index_file ) {
    //RUNWARN( "Warning: import_dictionary: cannot open " + file_name );
    return false;
  }

  set< string >& base = base_list[ pos ];
  string line;
  while ( getline( index_file, line ) ) {
    if ( line[ 0 ] != ' ' ) {
      string word = line.substr( 0, line.find( ' ' ) );
      if ( word.find( '_' ) == word.npos ) {
        base.insert( word );
      }
    }
  }
  return true;
}

#if 0
bool importDict() {
  exc_list[ VERB ][ "were" ] = "be";
  exc_list[ VERB ][ "was" ] = "be";
  exc_list[ VERB ][ "'s" ] = "be";
  exc_list[ VERB ][ "had" ] = "have";
  exc_list[ VERB ][ "has" ] = "have";
  exc_list[ VERB ][ "'d" ] = "have";

  exc_list[ ADJ ][ "better" ] = "good";
  exc_list[ ADJ ][ "best" ] = "good";
  exc_list[ ADJ ][ "worse" ] = "bad";
  exc_list[ ADJ ][ "worst" ] = "bad";
  exc_list[ ADJ ][ "more" ] = "many";
  exc_list[ ADJ ][ "most" ] = "many";
  exc_list[ ADJ ][ "less" ] = "little";
  exc_list[ ADJ ][ "least" ] = "little";

  exc_list[ ADV ][ "better" ] = "well";
  exc_list[ ADV ][ "best" ] = "well";
  exc_list[ ADV ][ "worse" ] = "bad";
  exc_list[ ADV ][ "worst" ] = "bad";
  exc_list[ ADV ][ "more" ] = "much";
  exc_list[ ADV ][ "most" ] = "much";
  exc_list[ ADV ][ "less" ] = "little";
  exc_list[ ADV ][ "least" ] = "little";

  return importExcFile( "Corpus/noun.exc", NOUN )
    && importExcFile( "Corpus/verb.exc", VERB )
    && importExcFile( "Corpus/adj.exc", ADJ )
    && importExcFile( "Corpus/adv.exc", ADV )
    && importIndexFile( "Corpus/index.noun", NOUN )
    && importIndexFile( "Corpus/index.verb", VERB )
    && importIndexFile( "Corpus/index.adj", ADJ ) 
    && importIndexFile( "Corpus/index.adv", ADV );
}
#endif

bool importDict( const std::string &dirName ) {
  for (unsigned i = 0; i < POS_SIZE; ++i) {
    exc_list[ i ].clear();
    base_list[ i ].clear();
  }

  exc_list[ VERB ][ "were" ] = "be";
  exc_list[ VERB ][ "was" ] = "be";
  exc_list[ VERB ][ "'s" ] = "be";
  exc_list[ VERB ][ "had" ] = "have";
  exc_list[ VERB ][ "has" ] = "have";
  exc_list[ VERB ][ "'d" ] = "have";

  exc_list[ ADJ ][ "better" ] = "good";
  exc_list[ ADJ ][ "best" ] = "good";
  exc_list[ ADJ ][ "worse" ] = "bad";
  exc_list[ ADJ ][ "worst" ] = "bad";
  exc_list[ ADJ ][ "more" ] = "many";
  exc_list[ ADJ ][ "most" ] = "many";
  exc_list[ ADJ ][ "less" ] = "little";
  exc_list[ ADJ ][ "least" ] = "little";

  exc_list[ ADV ][ "better" ] = "well";
  exc_list[ ADV ][ "best" ] = "well";
  exc_list[ ADV ][ "worse" ] = "bad";
  exc_list[ ADV ][ "worst" ] = "bad";
  exc_list[ ADV ][ "more" ] = "much";
  exc_list[ ADV ][ "most" ] = "much";
  exc_list[ ADV ][ "less" ] = "little";
  exc_list[ ADV ][ "least" ] = "little";

  return importExcFile( dirName + "/noun.exc", NOUN )
    && importExcFile( dirName + "/verb.exc", VERB )
    && importExcFile( dirName + "/adj.exc", ADJ )
    && importExcFile( dirName + "/adv.exc", ADV )
    && importIndexFile( dirName + "/index.noun", NOUN )
    && importIndexFile( dirName + "/index.verb", VERB )
    && importIndexFile( dirName + "/index.adj", ADJ ) 
    && importIndexFile( dirName + "/index.adv", ADV );
}

//////////////////////////////////////////////////////////////////////

// Check whether the suffix needs special treatment
bool checkSuffix( const string& str, string::size_type pos ) {
  if ( str.compare( pos - 2, 2, "sh" ) == 0 ) return true;
  if ( str.compare( pos - 2, 2, "ch" ) == 0 ) return true;
  if ( str[ pos - 1 ] == 's' ) return true;
  if ( str[ pos - 1 ] == 'x' ) return true;
  if ( str[ pos - 1 ] == 'z' ) return true;
  if ( str[ pos - 1 ] == 'o' ) return true;
  return false;
}

// Check whether the character is a vowel
const static string vowels( "aeiou" );
bool isVowel( char c ) {
  return vowels.find( c ) != vowels.npos;
}

//////////////////////////////////////////////////////////////////////

// modal verb
void stemmingModal( const string& word, string& base ) {
//   if (word == "'ll") base = "will";
//   else if ( word == "would" ) base = "will";
//   else if ( word == "'d" ) base = "will";
//   else if ( word == "ca" ) base = "can";
//   else if ( word == "could" ) base = "can";
//   else if ( word == "might" ) base = "may";
//   else base = word;
  if (word == "'ll") base = "will";
  else if ( word == "wo") base = "will";
  else if ( word == "'d" ) base = "would";
  else if ( word == "ca" ) base = "can";
  else base = word;
}

// present participle verb
void stemmingPrp( const string& word, string& base ) {
  if ( findExc( word, VERB, base ) ) return;
  if ( word.size() < 5 || word.compare( word.size() - 3, 3, "ing" ) != 0 ) {
    base = word;
    return;
  }
  if ( word.size() > 5 &&
       word[ word.size() - 4 ] == word[ word.size() - 5 ] &&
       isVowel( word[ word.size() - 6 ] ) ) {
    string base1 = word.substr( 0, word.size() - 3 );
    if ( findBase( base1, VERB ) ) {
      base = base1;
      return;
    }
    base = word.substr( 0, word.size() - 4 );
    return;
  }
  string base2 = word.substr( 0, word.size() - 2 );
  base2[ base2.size() - 1 ] = 'e';
  if ( findBase( base2, VERB ) ) {
    base = base2;
    return;
  }
  base = word.substr( 0, word.size() - 3 );
  return;
}

// 3rd singular verb
void stemming3sg( const string& word, string& base ) {
  if ( findExc( word, VERB, base ) ) return;
  if ( word.size() < 3 || word[ word.size() - 1 ] != 's' ) {
    base = word;
    return;
  }
  if ( word.size() >= 4 &&
       word.compare( word.size() - 3, 3, "ies" ) == 0 &&
       ! isVowel( word[ word.size() - 4 ] ) ) {
    base = word.substr( 0, word.size() - 2 );
    base[ base.size() - 1 ] = 'y';
    return;
  }
  if ( word.size() >= 4 &&
       word.compare( word.size() - 2, 2, "es" ) == 0 &&
       checkSuffix( word, word.size() - 2 ) ) {
    string base2 = word.substr( 0, word.size() - 1 );
    if ( findBase( base2, VERB ) ) {
      base = base2;
      return;
    }
    string base3 = word.substr( 0, word.size() - 2 );
    base = base3;
    return;
  }
  base = word.substr( 0, word.size() - 1 );
  return;
}

// past tense or past participle
void stemmingPast( const string& word, string& base ) {
  if ( findExc( word, VERB, base ) ) return;
  if ( word.size() < 3 || word.compare( word.size() - 2, 2, "ed" ) != 0 ) {
    base = word;
    return;
  }
  if ( word.size() >= 4 &&
       word.compare( word.size() - 3, 3, "ied" ) == 0 &&
       ! isVowel( word[ word.size() - 4 ] ) ) {
    base = word.substr( 0, word.size() - 2 );
    base[ base.size() - 1 ] = 'y';
    return;
  }
  if ( word.size() > 4 &&
       word[ word.size() - 3 ] == word[ word.size() - 4 ] &&
       isVowel( word[ word.size() - 5 ] ) ) {
    string base1 = word.substr( 0, word.size() - 2 );
    if ( findBase( base1, VERB ) ) {
      base = base1;
      return;
    }
    base = word.substr( 0, word.size() - 3 );
    return;
  }
  string base2 = word.substr( 0, word.size() - 1 );
  if ( findBase( base2, VERB ) ) {
    base = base2;
    return;
  }
  base = word.substr( 0, word.size() - 2 );
  return;
}

// no-3sg verb
void stemmingNo3sg( const string& word, string& base ) {
  if ( word == "'ve" ) base = "have";
  else if ( word == "are" ) base = "be";
  else if ( word == "'re" ) base = "be";
  else if ( word == "am" ) base = "be";
  else if ( word == "'m" ) base = "be";
  else base = word;
}

// plural noun
void stemmingPlural( const string& word, string& base ) {
  if ( findExc( word, NOUN, base ) ) return;
  if ( word.size() < 2 ) return;
  if ( word.size() >= 2 && word.compare( word.size() - 2, 2, "ss" ) == 0 ) {
    base = word;
    return;
  } else if ( word.size() >= 3 && word.compare( word.size() - 3, 3, "men" ) == 0 ) {
    base = word;
    base[ base.size() - 2 ] = 'a';
    return;
  }
  if ( word.size() >= 4 &&
       word.compare( word.size() - 3, 3, "ies" ) == 0 &&
       ! isVowel( word[ word.size() - 4 ] ) ) {
    base = word.substr( 0, word.size() - 2 );
    base[ base.size() - 1 ] = 'y';
    return;
  }
  if ( word.size() >= 4 &&
       word.compare( word.size() - 2, 2, "es" ) == 0 &&
       checkSuffix( word, word.size() - 2 ) ) {
    string base2 = word.substr( 0, word.size() - 1 );
    if ( findBase( base2, NOUN ) ) {
      base = base2;
      return;
    }
    string base3 = word.substr( 0, word.size() - 2 );
    base = base3;
    return;
  }
  if ( word[ word.size() - 1 ] == 's' ) {
    base = word.substr( 0, word.size() - 1 );
  } else {
    base = word;
  }
  return;
}

// adjective/adverb
void stemmingAdj( const string& word, string& base ) {
  if ( word == "n't" ) base = "not";
  else base = word;
}

// comparative adjective/adverb
void stemmingComp( const string& word, POS pos, string& base ) {
  if ( findExc( word, pos, base ) ) return;
  if ( word.size() < 3 || word.compare( word.size() - 2, 2, "er" ) != 0 ) {
    base = word;
    return;
  }
  if ( word.size() >= 4 &&
       word.compare( word.size() - 3, 3, "ier" ) == 0 &&
       ! isVowel( word[ word.size() - 4 ] ) ) {
    base = word.substr( 0, word.size() - 2 );
    base[ base.size() - 1 ] = 'y';
    return;
  }
  string base2 = word.substr( 0, word.size() - 2 );
  if ( findBase( base2, pos ) ) {
    base = base2;
    return;
  }
  base = word.substr( 0, word.size() - 1 );
  return;
}

// superative adjective/adverb
void stemmingSup( const string& word, POS pos, string& base ) {
  if ( findExc( word, pos, base ) ) return;
  if ( word.size() < 4 || word.compare( word.size() - 3, 3, "est" ) != 0 ) {
    base = word;
    return;
  }
  if ( word.size() >= 5 &&
       word.compare( word.size() - 4, 4, "iest" ) == 0 &&
       ! isVowel( word[ word.size() - 5 ] ) ) {
    base = word.substr( 0, word.size() - 3 );
    base[ base.size() - 1 ] = 'y';
    return;
  }
  string base2 = word.substr( 0, word.size() - 3 );
  if ( findBase( base2, pos ) ) {
    base = base2;
    return;
  }
  base = word.substr( 0, word.size() - 2 );
  return;
}

//////////////////////////////////////////////////////////////////////

#if 0
// main function
string stemming( const string& word, const string& pos ) {
  string base;
  if ( pos == "MD" ) {
    // modal verbs
    stemmingModal( word, base );
  } else if ( pos == "VBG" ) {
    // present participle
    stemmingPrp( word, base );
  } else if ( pos == "VBZ" ) {
    // 3rd singular
    stemming3sg( word, base );
  } else if ( pos == "VBD" || pos == "VBN" ) {
    // past tense or past participle 
    stemmingPast( word, base );
  } else if ( pos == "VBP" ) {
    // present tense no-3sg
    stemmingNo3sg( word, base );
  } else if ( pos == "NNS" || pos == "NNPS" ) {
    // plural
    stemmingPlural( word, base );
  } else if ( pos == "JJR" ) {
    // comparative
    stemmingComp( word, ADJ, base );
  } else if ( pos == "RBR" ) {
    // comparative
    stemmingComp( word, ADV, base );
  } else if ( pos == "JJS" ) {
    // superative
    stemmingSup( word, ADJ, base );
  } else if ( pos == "RBS" ) {
    // superative
    stemmingSup( word, ADV, base );
  }
  if ( ! base.empty() ) return base; else return word;
}
#endif

// main function
void stemming(
  const string& word,
  const string& pos,
  string &base,
  string &basePos
) {
  base.clear();
  basePos.clear();

  if ( pos == "MD" ) {
    // modal verbs
    stemmingModal( word, base );
    basePos = "MD";
  } else if ( pos == "VBG" ) {
    // present participle
    stemmingPrp( word, base );
    basePos = "VB";
  } else if ( pos == "VBZ" ) {
    // 3rd singular
    stemming3sg( word, base );
    basePos = "VB";
  } else if ( pos == "VBD" || pos == "VBN" ) {
    // past tense or past participle 
    stemmingPast( word, base );
    basePos = "VB";
  } else if ( pos == "VBP" ) {
    // present tense no-3sg
    stemmingNo3sg( word, base );
    basePos = "VB";
  } else if ( pos == "NNS" || pos == "NNPS" ) {
    // plural
    stemmingPlural( word, base );
    basePos = ( pos == "NNS" ) ? "NN" : "NNP";
  } else if ( pos == "JJ" || pos == "RB") {
    // adjective or adverb
    stemmingAdj( word, base );
    basePos = pos;
  } else if ( pos == "JJR" ) {
    // comparative
    stemmingComp( word, ADJ, base );
    basePos = "JJ";
  } else if ( pos == "JJS" ) {
    // superative
    stemmingSup( word, ADJ, base );
    basePos = "JJ";
  } else if ( pos == "RBR" ) {
    // comparative
    stemmingComp( word, ADV, base );
    basePos = "RB";
  } else if ( pos == "RBS" ) {
    // superative
    stemmingSup( word, ADV, base );
    basePos = "RB";
  }

  if ( base.empty() ) {
    base = word;
  }

  if ( basePos.empty() ) {
    basePos = pos;
  }
}

//////////////////////////////////////////////////////////////////////

#if 0
// test program

int main() {
  cerr << "Reading data files... ";
  if ( ! importDict() ) {
    cerr << "dictionary reading failed." << endl;
    exit( 1 );
  }
  cerr << "Done." << endl;
  string line;
  while ( getline( cin, line ) ) {
    istringstream in( line );
    string token;
    while ( in >> token ) {
      string::size_type p = token.find( '/' );
      if ( p != string::npos ) {
        string word = token.substr( 0, p );
        string pos = token.substr( p + 1 );
        string base = stemming( word, pos );
        //cout << "word=" << word << ", pos=" << pos << ", base=" << base << endl;
        cout << base << ' ';
      }
    }
    cout << endl;
  }
}
#endif

//////////////////////////////////////////////////////////////////////
// Lilfes predicates
//////////////////////////////////////////////////////////////////////

#if 0
// stemming(+$Surface, +$Pos, -$Base, -$BasePos)
class StemmingPred : public lilfes::Pred<4> {
public:

  StemmingPred() : lilfes::Pred<4>( "stemming" ) {}

  bool exec( lilfes::machine &, lilfes::FSP argv[])
  {
    if ( ! argv[0].IsString() ) {
      RUNWARN( "Warning: arg1 of stemming/4 must be a string" );
      return false;
    }

    if ( ! argv[1].IsString() ) {
      RUNWARN( "Warning: arg2 of stemming/4 must be a string" );
      return false;
    }

    std::string base;
    std::string basePos;

    stemming( argv[0].ReadString(), argv[1].ReadString(), base, basePos );

    return argv[2].Unify( base.c_str() ) && argv[3].Unify( basePos.c_str() );
  }
};

// import_dictionary(+$DirName)
class ImportDictionaryPred : public lilfes::Pred<1> {
public:

  ImportDictionaryPred() : lilfes::Pred<1>( "import_dictionary" ) {}

  bool exec( lilfes::machine &, lilfes::FSP argv[])
  {
    if ( ! argv[0].IsString() ) {
      RUNWARN( "Warning: import_dictionary requires a string" );
      return false;
    }

    return importDict( argv[0].ReadString() );
  }
};


//////////////////////////////////////////////////////////////////////
// Module and predicate decl
//////////////////////////////////////////////////////////////////////

static lilfes::ModuleDecl mod( "enju:stem" );
static lilfes::Registrator regStemmingPred( mod, new StemmingPred() );
static lilfes::Registrator regImportDictionaryPred( mod, new ImportDictionaryPred() );
# endif

