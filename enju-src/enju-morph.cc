#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

// escaper
extern void escape_word( const string& input, const std::string &input_pos, string& escaped, bool to_lower );
extern void escape_pos( const string& input, string& escaped, bool to_lower );

// stemmer
extern void stemming( const string& word, const string& pos, string &base, string &basePos );
extern bool importDict( const std::string &dirName );

void processToken(void) {
  string token;
  while ( getline( cin, token ) ) {
    if ( token.empty() ) {
      cout << '\n';
    } else {
      string::size_type p = token.rfind( '/' );
      if ( p != string::npos ) {
        string input_word = token.substr( 0, p );
        string input_pos = token.substr( p + 1 );
        string surface, pos, base, base_pos;
        escape_word( input_word, input_pos, surface, true );
        escape_pos( input_pos, pos, false );
        stemming( surface, pos, base, base_pos );
        //cout << "word=" << word << ", pos=" << pos << ", base=" << base << endl;
	//On AIX, the tabs in the output text which is obtained through the pseudo terminal in procstream is replaced by some spaces.
	//The direct cause of this is not clear, but currently work arround this by replacing tabs with spaces.
        //cout << input_word << '\t' << input_pos << '\t' << surface << '\t' << pos << '\t' << base << '\t' << base_pos << '\n';
        cout << input_word << ' ' << input_pos << ' ' << surface << ' ' << pos << ' ' << base << ' ' << base_pos << '\n';
      }
    }
  }
}

struct InToken {
  unsigned begin;
  unsigned end;
  std::string input_word;
  std::string input_pos;
  double prob;
};

struct OutToken {
  unsigned begin;
  unsigned end;
  std::string input_word;
  std::string input_pos;
  std::string surface;
  std::string pos;
  std::string base;
  std::string base_pos;
  double prob;
  std::vector<std::string> lookup_keys;
};

bool readSentence(std::istream &ist, std::vector<InToken> &in)
{
    in.clear();
    InToken t;

    std::string line;
    while (std::getline(ist, line)) {
      if (line.empty()) {
        return true;
      }

      std::istringstream iss(line);
      if (! (iss >> t.begin >> t.end >> t.input_word >> t.input_pos >> t.prob)) {
        std::cerr << "enju-moph: WARN: input format error: " << line << std::endl;
        continue;
      }

      in.push_back(t);
    }

    if (! in.empty()) {
        std::cerr << "enju-morph: WARN: no empty line after the last token" << std::endl;
        return true;
    }
    else {
        return false;
    }
}

inline
std::ostream &operator<<(std::ostream &ost, const OutToken &t) {
    ost << t.begin << ' ' << t.end << ' ' << t.prob << '\n'
        << t.input_word << ' ' << t.input_pos << ' ' << t.surface << ' '
        << t.pos << ' ' << t.base << ' ' << t.base_pos << '\n';
    for (std::vector<std::string>::const_iterator k = t.lookup_keys.begin(); k != t.lookup_keys.end(); ++k) {
        if (k != t.lookup_keys.begin()) ost << ' ';
        ost << *k;
    }
    ost << '\n';

    return ost;
}

inline
bool isDeletePos(const std::string &input_pos)
{
    return input_pos == ".";
}

void analyze(const std::vector<InToken> &in, std::vector<OutToken> &out) {
  out.clear();
  OutToken u;
  for (std::vector<InToken>::const_iterator t = in.begin(); t != in.end(); ++t) {
    if (isDeletePos(t->input_pos)) {
      continue;
    }

    u.begin = t->begin;
    u.end   = t->end;
    u.prob  = t->prob;
    u.input_word = t->input_word;
    u.input_pos  = t->input_pos;
    escape_word( u.input_word, u.input_pos, u.surface, true );
    escape_pos( u.input_pos, u.pos, false );
    stemming( u.surface, u.pos, u.base, u.base_pos );

    u.lookup_keys.clear();
    u.lookup_keys.push_back(u.base + "/" + u.pos);
    u.lookup_keys.push_back("@UNK@/" + u.pos);
    if (u.pos != "NN") u.lookup_keys.push_back("@UNK@/NN");

    out.push_back(u);
  }
}

void processSentence(void) {
  std::vector<InToken> in;
  std::vector<OutToken> out;
  while (readSentence(std::cin, in)) {
    analyze(in, out);
    for (std::vector<OutToken>::const_iterator t = out.begin(); t != out.end(); ++t) {
      std::cout << *t;
    }
    std::cout << std::endl;
  }
}

int main( int argc, char** argv ) {
  bool process_sentence = false;
  if (argc >= 2 && argv[1] == std::string("-s")) {
    process_sentence = true;
    --argc;
    ++argv;
  }

  if ( argc != 2 ) {
    cerr << "Usage: enju-morph [-s] WORDNET_DATA_DIR" << endl;
    exit( 1 );
  }

  if ( ! importDict( argv[ 1 ] ) ) {
    cerr << "enju-morph: dictionary reading failed: " << argv[ 1 ] << endl;
    exit( 1 );
  }

  if (process_sentence) {
    processSentence();
  }
  else { // process one token at a time
    processToken();
  }

  return 0;
}


