/**
  * @module = regex
  * @cvs    = $Id: lregex.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $
  * @desc   = Regular expression
  * @copyright = Copyright (c) 1999-2000, Takashi Ninomiya
  * @copyright = You may distribute this file under the terms of the Artistic License.
  * String match and substitution with regular expression
  * @jdesc  = 正規表現
  * @japanese =
  * 正規表現を使って文字列のマッチや置換を行なう述語です．
  * @end_japanese
*/

#include "lconfig.h"

#ifdef WITH_REGEX

#include "errors.h"
#include "builtin.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "lregex.h"

#include <algorithm>
#include <cstring>
#include <list>
#include <map>
#include <string>
#include <utility>

//////////////////////////////////////////////////////////////////////
// definition of class LiLFeSRegEx

namespace lilfes {

using std::list;
using std::map;
using std::pair;
using std::replace;
using std::strcmp;
using std::string;


const int LiLFeSRegExError::ERRBUF_SIZE = 100;
char LiLFeSRegExError::errbuf[ ERRBUF_SIZE ];

map< LiLFeSRegEx::Key, regex_t > LiLFeSRegEx::regex_hash;

int LiLFeSRegEx::compile_internal( void ) {
//   Key key( regex, cflags );
//   const map< LiLFeSRegEx::Key, regex_t >::iterator it = regex_hash.find( key );
//   if ( it == regex_hash.end() ) {
//     int status = regcomp( preg, regex.c_str(), cflags );
//     if ( status == 0 ) {
//       regex_hash.insert( pair< LiLFeSRegEx::Key, regex_t >( key, *preg ) );
//     }
//     return status;
//   } else {
//     preg = &(it->second);
//   }
  // return regcomp( preg, regex.c_str(), cflags );

  typedef map< Key, regex_t > GlobalTable;

  GlobalTable::const_iterator it = regex_hash.find( Key( regex, cflags ) );
  if (it == regex_hash.end()) {
    regex_t reg;
	int status = regcomp( &reg, regex.c_str(), cflags);
	if (status == 0) {
	  pair< GlobalTable::iterator, bool > ib = regex_hash.insert(pair< Key, regex_t >( Key( regex, cflags ), reg ) );
	  preg = &(ib.first->second);
	}
	else {
	  regfree(&reg); /// we need this?
	}
	return status;
  }
  else {
    preg = &(it->second);
    return 0;
  }
}

void LiLFeSRegEx::addFlags( int f ) {
  if ( f & ICASE ) {
    cflags |= REG_ICASE;
  }
  if ( f & NOSUB ) {
    cflags |= REG_NOSUB;
  }
  if ( f & NEWLINE ) {
    cflags |= REG_NEWLINE;
  }
  if ( f & NOTBOL ) {
    eflags |= REG_NOTBOL;
  }
  if ( f & NOTEOL ) {
    eflags |= REG_NOTEOL;
  }
}

void LiLFeSRegEx::addFlags( FSP arg ) {
  while ( arg.GetType() != nil ) {
    if ( arg.GetType() == cons ) {
      FSP p = arg.Follow( hd );
      arg = arg.Follow( tl );
      if ( p.IsString() ) {
	const char* option = p.ReadString();
	if ( strcmp( option, "ICASE" ) == 0 ) {
	  cflags |= REG_ICASE;
        } else if ( strcmp( option, "i" ) == 0 ) {
          cflags |= REG_ICASE;
	} else if ( strcmp( option, "NEWLINE" ) == 0 ) {
	  cflags |= REG_NEWLINE;
	} else if ( strcmp( option, "NOTBOL" ) == 0 ) {
	  eflags |= REG_NOTBOL;
	} else if ( strcmp( option, "NOTEOL" ) == 0 ) {
	  eflags |= REG_NOTEOL;
	} else if ( strcmp( option, "GLOBAL" ) == 0 ) {
          global = true;
        } else if ( strcmp( option, "g" ) == 0 ) {
          global = true;
        } else {
	  string msg( "Invalid option for regex: " );
	  msg.append( option );
	  throw LiLFeSRegExError( msg );
	}
      } else {
	throw LiLFeSRegExError( "regex: Option must be specified with string" );
      }
    } else {
      throw LiLFeSRegExError( "regex: Option must be specified with string" );
    }
  }
}

void LiLFeSRegEx::compile( void ) {
#if 0
  if ( preg != NULL ) {
    regfree( preg );
  } else { // preg == NULL
    preg = new regex_t;
  }
  if ( preg == NULL ) {
    throw LiLFeSRegExError( "regex: Out of memory" );
  }
#endif
  int ret = compile_internal();
  if ( ret != 0 ) {
#if 0
    delete preg;
    preg = NULL;
    throw LiLFeSRegExError( ret, preg );
#endif
    throw LiLFeSRegExError( ret, 0 );
  }
}

bool LiLFeSRegEx::match( const string& str ) {
  if ( preg == NULL ) {
    throw LiLFeSRegExError( "regex: regex is not compiled yet" );
  }
  target = str;
  substrs.resize( 0 );
  int status = regexec( preg, str.c_str(), 0, NULL, eflags );
  if ( status == REG_NOMATCH ) {
    return false;
  }
  ASSERT( status == 0 );
  return true;
}

bool LiLFeSRegEx::exec( const string& str ) {
  if ( preg == NULL ) {
    throw LiLFeSRegExError( "regex: regex is not compiled yet" );
  }
  target = str;
  if ( global ) {
    substrs.resize( 0 );
    int offset = 0;
    regmatch_t pmatch;
    int status = regexec( preg, target.c_str(), 1, &pmatch, eflags );
    if ( status == REG_NOMATCH ) return false;
    while ( status != REG_NOMATCH ) {
      ASSERT( pmatch.rm_so >= 0 );
      regoff_t so = pmatch.rm_so;
      int n = pmatch.rm_eo - so;
      substrs.push_back( target.substr( offset + so, n ) );
      offset += pmatch.rm_eo;
      status = regexec( preg, target.substr( offset ).c_str(), 1, &pmatch, eflags );
    }
  } else {
#if LILFES_WIN_
    size_t nmatch = preg->re_nsub + 1;
    regmatch_t* pmatch = new regmatch_t[nmatch];
#else
    size_t nmatch = preg->re_nsub + 1;
    regmatch_t pmatch[ nmatch ];
#endif
    int status = regexec( preg, target.c_str(), nmatch, pmatch, eflags );
    if ( status == REG_NOMATCH ) {
      substrs.resize( 0 );
      return false;
    }
    ASSERT( status == 0 );
    substrs.resize( nmatch );
    for ( size_t i = 0; i < nmatch; ++i ) {
      if ( pmatch[ i ].rm_so < 0 ) {
        substrs[ i ] = "";
      } else {
        regoff_t so = pmatch[ i ].rm_so;
        size_t n = pmatch[ i ].rm_eo - so;
        substrs[ i ] = target.substr( so, n );
      }
    }
#if LILFES_WIN_
    delete[] pmatch;
#endif
  }
  return true;
}

bool LiLFeSRegEx::subst( const string& str, const string& substs ) {
  if ( preg == NULL ) {
    throw LiLFeSRegExError( "regex: regex is not compiled yet" );
  }
  target = str;
  substrs.resize( 0 );
  size_t nmatch = 1;
  regmatch_t pmatch;
  if ( global ) {
    int offset = 0;
    int status = regexec( preg, target.c_str(), nmatch, &pmatch, eflags );
    if ( status == REG_NOMATCH ) return false;
    while ( status != REG_NOMATCH ) {
      ASSERT( pmatch.rm_so >= 0 );
      regoff_t so = pmatch.rm_so;
      int n = pmatch.rm_eo - so;
      target.replace( offset + so, n, substs );
      offset += ( so + substs.size() );
      status = regexec( preg, target.substr( offset ).c_str(), nmatch, &pmatch, eflags );
    }
  } else {
    int status = regexec( preg, target.c_str(), nmatch, &pmatch, eflags );
    if ( status == REG_NOMATCH ) {
      return false;
    }
    ASSERT( status == 0 );
    ASSERT( pmatch.rm_so >= 0 );
    regoff_t so = pmatch.rm_so;
    int n = pmatch.rm_eo - so;
    target.replace( so, n, substs );
  }
  return true;
}

//////////////////////////////////////////////////////////////////////
// definition of built-in predicates

namespace builtin {

// like m// in Perl
/**
  * @predicate	= regex_match(+REGEX, +TARGET) 
  * @desc	= Success if regular expression REGEX matches the TARGET.
  * @param	= +REGEX/ string : regular expression
  * @param	= +TARGET/ string : target string
  * @example	= 
  > ?- regex_match("i+", "lilfes").
  yes 
  * @end_example

  * @jdesc	= 正規表現にマッチするかどうか判定します． 
  * @jparam	= +REGEX/ string : 正規表現
  * @jparam	= +TARGET/ string : 対象の文字列
  * @jexample	=
  > ?- regex_match("i+", "lilfes").
  yes 
  * @end_jexample
*/
bool regex_match( machine&, FSP arg1, FSP arg2 ) {

  if ( ! arg1.IsString() ) {
    RUNERR("1st argument of 'regex_match/2' must be a regular expression pattern");
    return false;
  }
  if ( ! arg2.IsString() ) {
    RUNERR("2nd argument of 'regex_match/2' must be a target string");
    return false;
  }

  try {

    LiLFeSRegEx regex( arg1.ReadString() );
    regex.addFlags( LiLFeSRegEx::NOSUB );
    regex.compile();
    const string& target( arg2.ReadString() );

    if ( ! regex.match( target ) ) {
      return false;
    }

  } catch ( const LiLFeSRegExError& e ) {
    RUNERR( e.message() );
    return false;
  }
  return true;
}

LILFES_BUILTIN_PRED_2( regex_match, regex_match );

/**
  * @predicate	= regex_match(+REGEX, +TARGET, -RESULT) 
  * @desc	= Get partial strings matched the regular expression.
  * @param	= +REGEX/ string : regular expression
  * @param	= +TARGET/ string : target string
  * @param	= -RESULT/ list : The first element is the whole matched string .The following elements is parts of matched expression(enclosed with parenthesis).
  * @example	= 
  > ?- regex_match("(hoge)(poge)", "hogepoge",X).
  X: < "hogepoge", "hoge", "poge" > 
  * @end_example

  * @jdesc	= 正規表現にマッチする部分文字列を返します． 
  * @jparam	= +REGEX/ string : 正規表現
  * @jparam	= +TARGET/ string : 対象の文字列
  * @jparam	= -RESULT/ list : 第1要素にマッチした文字列全体，第2要素以降にマッチした部分表現(かっこで囲まれた部分)が入ります． 
  * @jexample	=
  > ?- regex_match("(hoge)(poge)", "hogepoge",X).
  X: < "hogepoge", "hoge", "poge" > 
  * @end_jexample
*/
bool regex_match_3( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {

  if ( ! arg1.IsString() ) {
    RUNERR("1st argument of 'regex_match/3' must be a regular expression pattern");
    return false;
  }
  if ( ! arg2.IsString() ) {
    RUNERR("2nd argument of 'regex_match/3' must be a target string");
    return false;
  }

  try {

    LiLFeSRegEx regex( arg1.ReadString() );
    regex.compile();
    const string& target( arg2.ReadString() );

    if ( ! regex.exec( target ) ) {
      return false;
    }

    FSP f( m );
    FSP froot( m, f.GetAddress() );
    for ( size_t i = 0; i < regex.numSubstr(); ++i ) {
      FSP g( m, regex.getSubstr( i ).c_str() );
      f.Follow( hd ).Unify( g );
      f = f.Follow( tl );
    }
    f.Coerce( nil );

    if ( ! arg3.Unify( froot ) ) {
      return false;
    }
    return true;

  } catch ( LiLFeSRegExError e ) {
    RUNERR( e.message() );
    return false;
  }
}

LILFES_BUILTIN_PRED_OVERLOAD_3(regex_match_3, regex_match_3, regex_match);

/**
  * @predicate	= regex_match(+REGEX, +OPTIONS, +TARGET, -RESULT) 
  * @desc	= Get partial strings matched the regular exoression.
  * @param	= +REGEX/ string : regular expression
  * @param	= +OPTIONS/ list : list of options
  * @param	= +TARGET/ string : target string
  * @param	= -RESULT/ list : The first element is the whole matched string .The following elements is parts of matched expression(enclosed with parenthesis).
  * @note	= The folowing options can be used.
  * @note	= <UL>
  * @note	= <LI>"ICASE" : Not case-sensitive
  * @note	= <LI>"i" : same as "ICASE" 
  * @note	= <LI>"GLOBAL" : global matching (see example)
  * @note	= <LI>"g" : same as "GLOBAL" 
  * @note	= <LI>"NEWLINE" : use character of linefeed as segmentation
  * @note	= <LI>"NOTBOL" : If appointed "NEWLINE","^" does not match linefeed.
  * @note	= <LI>"NOTEOL" : If appointed "NEWLINE","$" does not match linefeed.</LI></UL>
  * @example	= 
  > ?- regex_match("(a)+", ["i"], "aAbbaaa",X).
  X: < "aA", "a" >
  > ?- regex_match("(a)+", ["i", "g"], "aAbbaaa",X).
  X: < "aA", "aaa" > 
  * @end_example

  * @jdesc	= 正規表現にマッチする部分文字列を返します． 
  * @jparam	= +REGEX/ string : 正規表現
  * @jparam	= +OPTIONS/ list : オプションのリスト．
  * @jparam	= +TARGET/ string : 対象の文字列
  * @jparam	= -RESULT/ list : 第1要素にマッチした文字列全体，第2要素以降にマッチした部分表現(かっこで囲まれた部分)が入ります． 
  * @jnote	= 以下のオプションを指定できます． 
  * @jnote	= <UL>
  * @jnote	= <LI>"ICASE" : 大文字／小文字を区別しない． 
  * @jnote	= <LI>"i" : "ICASE" と同じ． 
  * @jnote	= <LI>"GLOBAL" : グローバルマッチング(使用例を参照)． 
  * @jnote	= <LI>"g" : "GLOBAL" と同じ． 
  * @jnote	= <LI>"NEWLINE" : 改行文字を文の区切りとする． 
  * @jnote	= <LI>"NOTBOL" : "NEWLINE" が指定されたとき，"^" が改行にマッチしない． 
  * @jnote	= <LI>"NOTEOL" : "NEWLINE" が指定されたとき，"$" が改行にマッチしない． </LI></UL>
  * @jexample	=
  > ?- regex_match("(a)+", ["i"], "aAbbaaa",X).
  X: < "aA", "a" >
  > ?- regex_match("(a)+", ["i", "g"], "aAbbaaa",X).
  X: < "aA", "aaa" > 
  * @end_jexample
*/
bool regex_match_4( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {

  if ( ! arg1.IsString() ) {
    RUNERR("1st argument of 'regex_match/4' must be a regular expression pattern");
    return false;
  }
  if ( ! arg3.IsString() ) {
    RUNERR("3rd argument of 'regex_match/4' must be a target string");
    return false;
  }

  try {

    LiLFeSRegEx regex( arg1.ReadString() );
    regex.addFlags( arg2 );
    regex.compile();
    const string& target( arg3.ReadString() );

    if ( ! regex.exec( target ) ) {
      return false;
    }

    FSP f( m );
    FSP froot( m, f.GetAddress() );
    for ( size_t i = 0; i < regex.numSubstr(); ++i ) {
      FSP g( m, regex.getSubstr( i ).c_str() );
      f.Follow( hd ).Unify( g );
      f = f.Follow( tl );
    }
    f.Coerce( nil );

    if ( ! arg4.Unify( froot ) ) {
      return false;
    }
    return true;

  } catch ( LiLFeSRegExError e ) {
    RUNERR( e.message() );
    return false;
  }
}

LILFES_BUILTIN_PRED_OVERLOAD_4(regex_match_4, regex_match_4, regex_match_3);

// like s/// in Perl
/**
  * @predicate	= regex_subst(+REGEX, +SUBST, +TARGET, -RESULT) 
  * @desc	= Displace  strings matched regular expression REGEX that is a part of TARGET with string SUBST.
  * @param	= +REGEX/ string : regular expression
  * @param	= +SUBST/ string : 置き換える文字列
  * @param	= +TARGET/ string : target string
  * @param	= -RESULT/ string : 置換された文字列 
  * @example	= 
  > ?- regex_subst("hoge", "poe", "hoge-pe-n", X).
  X: poe-pe-n 
  * @end_example

  * @jdesc	= 正規表現にマッチする部分文字列を他の文字列で置換します． 
  * @jparam	= +REGEX/ string : 正規表現
  * @jparam	= +SUBST/ string : 置き換える文字列
  * @jparam	= +TARGET/ string : 対象の文字列
  * @jparam	= -RESULT/ string : 置換された文字列 
  * @jexample	=
  > ?- regex_subst("hoge", "poe", "hoge-pe-n", X).
  X: poe-pe-n 
  * @end_jexample
*/
bool regex_subst( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {

  if ( ! arg1.IsString() ) {
    RUNERR("1st argument of 'regex_subst/4' must be a regular expression pattern");
    return false;
  }
  if ( ! arg2.IsString() ) {
    RUNERR("2nd argument of 'regex_subst/4' must be a substitution string");
    return false;
  }
  if ( ! arg3.IsString() ) {
    RUNERR("3rd argument of 'regex_subst/4' must be a target string");
    return false;
  }

  try {

    LiLFeSRegEx regex( arg1.ReadString() );
    regex.compile();
    const string& subst( arg2.ReadString() );
    const string& target( arg3.ReadString() );

    if ( ! regex.subst( target, subst ) ) {
      return false;
    }

    FSP f( m, regex.getTarget().c_str() );
    if ( ! arg4.Unify( f ) ) {
      return false;
    }
    return true;

  } catch ( LiLFeSRegExError e ) {
    RUNERR( e.message() );
    return false;
  }
}

LILFES_BUILTIN_PRED_4( regex_subst, regex_subst );

/**
  * @predicate	= regex_subst(+REGEX, +OPTIONS, +SUBST, +TARGET, -RESULT) 
  * @desc	= Displace  strings matched regular expression REGEX that is a part of TARGET with string SUBST.
  * @param	= +REGEX/ string : regular expression
  * @param	= +OPTIONS/ list : list of options
  * @param	= +SUBST/ string : 置き換える文字列
  * @param	= +TARGET/ string : target string
  * @param	= -RESULT/ string : 置換された文字列 
  * @note	= The folowing options can be used.
  * @note	= <UL>
  * @note	= <LI>"ICASE" : Not case-sensitive
  * @note	= <LI>"i" : same as "ICASE" 
  * @note	= <LI>"GLOBAL" : global matching (see example)
  * @note	= <LI>"g" : same as "GLOBAL" 
  * @note	= <LI>"NEWLINE" : use character of linefeed as segmentation
  * @note	= <LI>"NOTBOL" : If appointed "NEWLINE","^" does not match linefeed.
  * @note	= <LI>"NOTEOL" : If appointed "NEWLINE","$" does not match linefeed.</LI></UL>
  * @example	= 
  > ?- regex_subst("l+", ["i"], "m", "LiLFeS", X).
  X: miLFeS
  > ?- regex_subst("l+", ["i", "g"], "m", "LiLFeS", X).
  X: mimFeS 
  * @end_example

  * @jdesc	= 正規表現にマッチする部分文字列を他の文字列で置換します． 
  * @jparam	= +REGEX/ string : 正規表現
  * @jparam	= +OPTIONS/ list : オプションのリスト．
  * @jparam	= +SUBST/ string : 置き換える文字列
  * @jparam	= +TARGET/ string : 対象の文字列
  * @jparam	= -RESULT/ string : 置換された文字列 
  * @jnote	= 以下のオプションを指定できます． 
  * @jnote	= <UL>
  * @jnote	= <LI>"ICASE" : 大文字／小文字を区別しない． 
  * @jnote	= <LI>"i" : "ICASE" と同じ． 
  * @jnote	= <LI>"GLOBAL" : グローバルマッチング(使用例を参照)． 
  * @jnote	= <LI>"g" : "GLOBAL" と同じ． 
  * @jnote	= <LI>"NEWLINE" : 改行文字を文の区切りとする． 
  * @jnote	= <LI>"NOTBOL" : "NEWLINE" が指定されたとき，"^" が改行にマッチしない． 
  * @jnote	= <LI>"NOTEOL" : "NEWLINE" が指定されたとき，"$" が改行にマッチしない． </LI></UL>
  * @jexample	=
  > ?- regex_subst("l+", ["i"], "m", "LiLFeS", X).
  X: miLFeS
  > ?- regex_subst("l+", ["i", "g"], "m", "LiLFeS", X).
  X: mimFeS 
  * @end_jexample
*/
bool regex_subst_5( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5 ) {

  if ( ! arg1.IsString() ) {
    RUNERR("1st argument of 'regex_subst/5' must be a regular expression pattern");
    return false;
  }
  if ( ! arg3.IsString() ) {
    RUNERR("3rd argument of 'regex_subst/5' must be a substitution string");
    return false;
  }
  if ( ! arg4.IsString() ) {
    RUNERR("4th argument of 'regex_subst/5' must be a target string");
    return false;
  }

  try {

    LiLFeSRegEx regex( arg1.ReadString() );
    regex.addFlags( arg2 );
    regex.compile();
    const string& subst( arg3.ReadString() );
    const string& target( arg4.ReadString() );

    if ( ! regex.subst( target, subst ) ) {
      return false;
    }

    FSP f( m, regex.getTarget().c_str() );
    if ( ! arg5.Unify( f ) ) {
      return false;
    }
    return true;

  } catch ( LiLFeSRegExError e ) {
    RUNERR( e.message() );
    return false;
  }
}

LILFES_BUILTIN_PRED_OVERLOAD_5( regex_subst_5, regex_subst_5, regex_subst );

} // namespace builtin

} // namespace lilfes

#endif // WITH_REGEX
// end of regex.cpp
