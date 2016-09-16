/* $Id: lregex.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $ 
 *
 *    Copyright (c) 2000, Yusuke Miyao
 *
 *    You may distribute this file under the terms of the Artistic License.
 */

#ifndef lregex_h_
#define lregex_h_

#ifdef WITH_REGEX

#if LILFES_WIN_
#include <boost/cregex.hpp>

using namespace boost;
#else
#include <regex.h>
#endif
#include <map>
#include <string>
#include <vector>

namespace lilfes {

class machine;
class FSP;

//////////////////////////////////////////////////////////////////////

class LiLFeSRegExError {
 private:
  std::string error_message;

  static const int ERRBUF_SIZE;
  static char errbuf[];

 public:
  LiLFeSRegExError( const std::string& m ) {
    error_message = m;
  }
  LiLFeSRegExError( const char* s ) {
    error_message = s;
  }
  LiLFeSRegExError( int c, const regex_t* preg ) {
    regerror( c, preg, errbuf, ERRBUF_SIZE );
    error_message = errbuf;
  }
  ~LiLFeSRegExError() {}

  const std::string& message( void ) const {
    return error_message;
  }
};

//////////////////////////////////////////////////////////////////////

class LiLFeSRegEx {
 private:
  // regex_t* preg;
  const regex_t* preg;
  std::vector< std::string > substrs;
  std::string regex;
  std::string target;
  int cflags, eflags;
  bool global;

  static const int DEFAULT_CFLAGS = REG_EXTENDED;
  static const int DEFAULT_EFLAGS = 0;

  struct Key {
    // const std::string& regex;
    const std::string regex;
    int cflags;
    Key( const std::string& r, int c );
    bool operator<( const Key& k ) const {
      // return ( regex < k.regex ) && ( cflags < k.cflags );
	  return (regex < k.regex) || ((regex == k.regex) && (cflags < k.cflags));
    }
  };

  static std::map< Key, regex_t > regex_hash;

  int compile_internal( void );

 public:
  enum {
    ICASE = 1,
    NOSUB = 2,
    NEWLINE = 4,
    NOTBOL = 8,
    NOTEOL = 16
  };

  LiLFeSRegEx( void );
  LiLFeSRegEx( const std::string& r );
  LiLFeSRegEx( const std::string& r, int f );
  virtual ~LiLFeSRegEx();

  void setRegEx( const std::string& r );
  void addFlags( int c );
  void addFlags( FSP arg );

  void compile( void );

  bool match( const std::string& str );
  bool exec( const std::string& str );
  bool subst( const std::string& str, const std::string& substs );

  size_t numSubstr( void ) const;
  const std::string& getSubstr( int i ) const;
  std::string getSubstr( int i );
  const std::string& getTarget( void ) const;
  std::string getTarget( void );
};



inline LiLFeSRegEx::Key::Key( const std::string& r, int c ) : regex( r ) {
  cflags = c;
}
inline LiLFeSRegEx::LiLFeSRegEx( void ) {
  preg = NULL;
  cflags = DEFAULT_CFLAGS;
  eflags = DEFAULT_EFLAGS;
  global = false;
}
inline LiLFeSRegEx::LiLFeSRegEx( const std::string& r ) {
  preg = NULL;
  regex = r;
  cflags = DEFAULT_CFLAGS;
  eflags = DEFAULT_EFLAGS;
  global = false;
}
inline LiLFeSRegEx::LiLFeSRegEx( const std::string& r, int f ) {
  preg = NULL;
  regex = r;
  cflags = DEFAULT_CFLAGS;
  eflags = DEFAULT_EFLAGS;
  global = false;
  addFlags( f );
}
inline LiLFeSRegEx::~LiLFeSRegEx() {
  // if ( preg != NULL ) {
  //  regfree( preg );
  //  delete preg;
  // }
}

inline void LiLFeSRegEx::setRegEx( const std::string& r ) {
  regex = r;
}

inline size_t LiLFeSRegEx::numSubstr( void ) const {
  return substrs.size();
}
inline const std::string& LiLFeSRegEx::getSubstr( int i ) const {
  return substrs[ i ];
}
inline std::string LiLFeSRegEx::getSubstr( int i ) {
  return substrs[ i ];
}
inline const std::string& LiLFeSRegEx::getTarget( void ) const {
  return target;
}
inline std::string LiLFeSRegEx::getTarget( void ) {
  return target;
}

namespace builtin {

  bool regex_match( machine&, FSP, FSP );
  bool regex_match_3( machine&, FSP, FSP, FSP );
  bool regex_match_4( machine&, FSP, FSP, FSP, FSP );
  bool regex_subst( machine&, FSP, FSP, FSP, FSP );
  bool regex_subst_5( machine&, FSP, FSP, FSP, FSP, FSP );

} // namespace builtin

} // namespace lilfes

#endif // WITH_REGEX

#endif // lregex_h_
// end of lregex.h
