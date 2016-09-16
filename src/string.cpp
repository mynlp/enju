/**
 * @module  = string
 * @cvs     = $Id: string.cpp,v 1.14 2011-05-02 10:38:24 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = String manipulation
 * Predicates for manipulating strings
 * @jdesc   = 文字列操作
 * @japanese =
 * 文字列操作を行なう述語です．
 * @end_japanese
*/

#include "builtin.h"
#include "utility.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <ios>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>


namespace lilfes {

using std::cout;
using std::ios;
using std::list;
using std::ostringstream;
using std::replace;
using std::string;
using std::vector;

namespace builtin {

/**
  * @predicate	= strtolist(+STRING, -LIST) 
  * @desc	= Convert a string into a list of ASCII codes. 
  * @param	= +STRING/ string : Character string
  * @param	= -LIST/ list  : List of ASCII codes 
  * @example	= 
  > ?- strtolist("abc", X).
  X: < 97, 98, 99 > 
  * @end_example

  * @jdesc	= 文字列を、アスキーコードのリストに変換します。 
  * @jparam	= +STRING/ string : 文字列
  * @jparam	= -LIST/ list : アスキーコードのリスト 
  * @jexample	= 
  > ?- strtolist("abc", X).
  X: < 97, 98, 99 > 
  * @end_jexample
*/
bool strtolist( machine& m, FSP arg1, FSP arg2 )
{
	cell c = arg1.ReadCell();
	if( !IsSTG(c) )
	{
 		RUNWARN("Warning: strtolist/2 requires a string"); // Modified by mitsuisi (06/10/98)
		return false;
	}
	const char *s = c2STG(c);
	while( *s )
	{
		if( arg2.Follow(hd).Unify(FSP(m, (mint)(unsigned char)*s)) == false )
		{
			return false;
		}
		arg2 = arg2.Follow(tl);
		s++;
	}
  return arg2.Coerce(nil);
}

LILFES_BUILTIN_PRED_2(strtolist, strtolist);

//////////////////////////////////////////////////////////////////////////////
// strtodlist/3 : decompose string into dlist (by mitsuisi (1999.05.20))

/**
  * @predicate	= strtodlist(+STRING, -LIST, -DLIST) 
  * @desc	= Convert a string into a list of ASCII codes and add other list to that list.
  * @param	= +STRING/ string : Character string
  * @param	= -LIST/ list  : List of ASCII codes 
  * @param	= -DLIST/ list  : Tail of the list
  * @example	= 
  * @end_example

  * @jdesc	= 文字列を、アスキーコードのリストに変換し、そのリストに別のリストを付け加えます． 
  * @jparam	= +STRING/ string : 文字列
  * @jparam	= -LIST/ list : アスキーコードのリスト 
  * @jparam	= -DLIST/ list : リストの末尾
  * @jexample	= 
  * @end_jexample
*/
bool strtodlist( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c = arg1.ReadCell();
	if( !IsSTG(c) )
	{
 		RUNWARN("Warning: strtodlist/3 requires a string"); // Modified by mitsuisi (06/10/98)
		return false;
	}
	const char *s = c2STG(c);
	while( *s )
	{
		if( arg2.Follow(hd).Unify(FSP(m, (mint)(unsigned char)*s)) == false )
		{
			return false;
		}
		arg2 = arg2.Follow(tl);
		s++;
	}
  return arg2.Unify(arg3);
}

LILFES_BUILTIN_PRED_3(strtodlist, strtodlist);

//////////////////////////////////////////////////////////////////////////////
// listtostr/2 : compose list into string

/**
  * @predicate	= listtostr(+LIST, -STRING) 
  * @desc	= Convert a list of ASCII codes into a string 
  * @param	= +LIST/ list  : List of ASCII codes 
  * @param	= -STRING/ string : Character string
  * @note	= If the list include a 0, the return is undefined. 
  * @example	= 
  > ?- listtostr([65,66,67], X).
  X: "ABC" 
  * @end_example

  * @jdesc	= アスキーコードのリストを、文字列に変換します。 
  * @jparam	= +LIST/ list : アスキーコードのリスト 
  * @jparam	= -STRING/ string : 文字列
  * @jnote	= 入力のリスト中に 0 が含まれている場合の動作は保証しません。 
  * @jexample	= 
  > ?- listtostr([65,66,67], X).
  X: "ABC" 
  * @end_jexample
*/

bool listtostr( machine& m, FSP arg1, FSP arg2 )
{
	char buf[8192];
	char *bufp = buf;
	uint bufidx = 0;
	uint bufsize = sizeof(buf);

	while( arg1.ReadCell() != VAR2c(nil) )
	{
		if( arg1.ReadCell() != STR2c(cons) )
		{
			RUNWARN("Warning: listtostr requires a list");
			return false;
		}
		FSP f = arg1.Follow(hd);
		if( ! IsINT(f.ReadCell()) )
		{
			RUNWARN("Warning: listtostr requires a list of integer");
			return false;
		}
		bufp[bufidx++] = c2INT(f.ReadCell());
		if( bufidx == bufsize )
		{
			int newbufsize = bufsize*3/2;
			char *newbufp = new char[newbufsize];
			memcpy(newbufp, bufp, bufsize);
			if( bufp != buf )
				delete bufp;
			bufp = newbufp;
			bufsize = newbufsize;
		}
		arg1 = arg1.Follow(tl);
	}
	bufp[bufidx] = '\0';

	FSP result(m, bufp);
	if( arg2.Unify(result) == false )
	{
          if( bufp != buf )
            delete[] bufp;

          return false;
	}
	if( bufp != buf )
		delete[] bufp;
  return true;
}

LILFES_BUILTIN_PRED_2(listtostr, listtostr);

/**
  * @predicate	= strcat(+STR1,+STR2,-RET) 
  * @desc	= Concatenate strings.
  * @param	= +STR1/ string :  string1
  * @param	= +STR2/ string :  string2
  * @param	= -RET/ string :  returned string
  * @example	= 
  * @end_example

  * @jdesc	= 文字列を接合します．
  * @jparam	= +STR1/ string : 文字列1
  * @jparam	= +STR2/ string : 文字列2
  * @jparam	= +STR1/ string : 結果の文字列
  * @jexample	= 
  * @end_jexample
*/
bool strcat( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	cell c1= arg1.ReadCell(); cell c2 = arg2.ReadCell();
	if( !IsSTG(c1) || !IsSTG(c2) )
	{
          RUNWARN("Warning: strcat/3 requires a string");
          return false;
	}

        string str( arg1.ReadString() );
        str += arg2.ReadString();

	return arg3.Unify( FSP( m, str.c_str() ) );
}

LILFES_BUILTIN_PRED_3(strcat, strcat);

/**
  * @predicate	= strlen(+STR,-LEN) 
  * @desc	= Length of string
  * @param	= +STR/ string :  string
  * @param	= -LEN/ string :  length of string
  * @example	= 
  * @end_example

  * @jdesc	= 文字列の長さを返します．
  * @jparam	= +STR/ string : 文字列
  * @jparam	= +LEN/ string : 長さ
  * @jexample	= 
  * @end_jexample
*/
bool strlen( machine& m, FSP arg1, FSP arg2 )
{
  if ( ! arg1.IsString() ) {
    RUNWARN("Warning: strlen/2 requires a string");
    return false;
  }
  const char* str = arg1.ReadString();
  size_t len = std::strlen( str );
  return arg2.Unify( FSP( m, (mint)len ) );
}

LILFES_BUILTIN_PRED_2(strlen, strlen);

/**
  * @predicate	= strtotype(+STRING, -TYPE) 
  * @desc	= Convert a string into a type. 
  * @param	= +LIST/ string : String
  * @param	= -STRING/ bot : Type name 
  * @note	= In the case of the type name isn't defined, the function fails. 
  * @example	= 
  > ?- strtotype("assert/1", X).
  X: assert/1 
  * @end_example

  * @jdesc	= 文字列を型に変換します。 
  * @jparam	= +LIST/ string : 文字列
  * @jparam	= -STRING/ bot : 型名 
  * @jnote	= 型名が型定義されていないものの場合、失敗します。 
  * @jexample	= 
  > ?- strtotype("assert/1", X).
  X: assert/1 
  * @end_jexample
*/
bool strtotype( machine&, FSP arg1, FSP arg2 )
{
//cout << "ENTER STRTOTYPE" << endl;
//	cout << arg1.DisplayLiLFeS();
	if( !arg1.IsString() )
	{
		RUNWARN("strtotype/2 requires a string");
//		cout << arg1.DisplayLiLFeS();
		return false;
	}
	
	const type *t = module::CurrentModule()->Search(arg1.ReadString());
	
	if( t == NULL )
          return false;
	else
	{
          return arg2.Coerce(t);
	}
//cout << "LEAVE STRTOTYPE" << endl;
}

LILFES_BUILTIN_PRED_2(strtotype, strtotype);

/**
  * @predicate	= strtotype(+MODULE,+STRING, -TYPE) 
  * @desc	= Convert a string into a type in a module. 
  * @param	= +MODULE/string :module name
  * @param	= +LIST/ string : String
  * @param	= -STRING/ bot : Type name 
  * @note	= In the case of the type name isn't defined, the function fails. 
  * @example	= 
  > ?- strtotype("assert/1", X).
  X: assert/1 
  * @end_example

  * @jdesc	= 文字列をモジュールの型に変換します。 
  * @jparam	= +MODULE/ string : モジュール名
  * @jparam	= +LIST/ string : 文字列
  * @jparam	= -STRING/ bot : 型名 
  * @jnote	= 型名が型定義されていないものの場合、失敗します。 
  * @jexample	= 
  > ?- strtotype("assert/1", X).
  X: assert/1 
  * @end_jexample
*/
bool strtotype_3( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
//cout << "ENTER STRTOTYPE" << endl;
//	cout << arg1.DisplayLiLFeS();
	if( !arg1.IsString() )
	{
		RUNWARN("strtotype/3 requires a module name in the first argument");
//		cout << arg1.DisplayLiLFeS();
		return false;
	}
	
	if( !arg2.IsString() )
	{
		RUNWARN("strtotype/3 requires a type name in the first argument");
//		cout << arg1.DisplayLiLFeS();
		return false;
	}
	module* mod = module::SearchModule(arg1.ReadString());
	if (mod == 0) {
		RUNWARN("Warning: module "<<arg1.ReadString() << " have not loaded yet.");
		return false;
	}
	
	const type *t = mod->Search(arg2.ReadString());
	
	if( t == NULL )
          return false;
	else
	{
          return arg3.Coerce(t);
	}
//cout << "LEAVE STRTOTYPE" << endl;
}

LILFES_BUILTIN_PRED_OVERLOAD_3(strtotype_3, strtotype_3, strtotype);

/**
  * @predicate	= typetostr(+TYPE, -STRING) 
  * @desc	= Convert a type into an string. This gets simple name of type.
  * @param	= +TYPE/ bot : Type name 
  * @param	= -STRING/ string : String
  * @note	= The case of numeric type is not considered. typetostr(1,X) returns integer. 
  * @example	= 
  > ?- typetostr("assert/1", X).
  X: assert/1 
  * @end_example

  * @jdesc	= 型を文字列に変換します．省略名を取得します．
  * @jparam	= +TYPE/ bot : 型名 
  * @jparam	= -STRING/ string : 文字列
  * @jnote	= 文字列や数値は、それ自身が型とはみなされません。typetostr(1,X) は integer を返します。 
  * @jexample	= 
  > ?- typetostr("assert/1", X).
  X: assert/1 
  * @end_jexample
*/
bool typetostr( machine& m, FSP arg1, FSP arg2 )
{
  return arg2.Unify(FSP(m, arg1.GetType()->GetSimpleName()));
}

LILFES_BUILTIN_PRED_2(typetostr, typetostr);

/**
  * @predicate	= feattostr(+TYPE, -STRING) 
  * @desc	= convert a feature to a string
  * @param	= +TYPE/ feature : Feature name 
  * @param	= -STRING/ string : String
  * @example	= 
  * @end_example

  * @jdesc	= 素性を文字列に変換します．
  * @jparam	= +TYPE/ feature : 素性名 
  * @jparam	= -STRING/ string : 文字列
  * @jexample	= 
  * @end_jexample
*/
bool feattostr( machine& m, FSP arg1, FSP arg2 )
{
	const feature *f = feature::Search(arg1.GetType());
	if( f == NULL || arg2.Unify(FSP(m, f->GetSimpleName())) == false )
          return false;
        else
          return true;
}

LILFES_BUILTIN_PRED_2(feattostr, feattostr);

/**
  * @predicate	= inttostr(+INT, -STRING) 
  * @desc	= Convert an integer into an string 
  * @param	= +INT/ int  : Number
  * @param	= -STRING/ string : String 
  * @example	= 
  > ?- inttostr(123, X).
  X: "123" 
  * @end_example

  * @jdesc	= 文字列を、整数に変換します。 
  * @jparam	= +INT/ int : 整数
  * @jparam	= -STRING/ string : 文字列 
  * @jexample	= 
  > ?- inttostr(123, X).
  X: "123" 
  * @end_jexample
*/
bool inttostr( machine& m, FSP arg1, FSP arg2 ) {
  if (!arg1.IsInteger()) {
    return false;
  }
  int val = arg1.ReadInteger();
  bool Isminus;
  if (val < 0) {
    Isminus = true;
    val = -val;
  } else {
    Isminus = false;
  }
  char buf[102];
  buf[101] = '\0';
  int idx = 100;
  do {
    buf[idx--] = (val % 10) + '0';
    val /= 10;
  } while (val != 0);
  if (Isminus)
    buf[idx--] = '-';
  return arg2.Unify(FSP(m, &buf[idx+1]));
}

LILFES_BUILTIN_PRED_2(inttostr, inttostr);

/**
  * @predicate	= strtoint(+STRING, -INT) 
  * @desc	= Convert a string into an integer 
  * @param	= +STRING/ string : String 
  * @param	= -INT/ int  : Number
  * @example	= 
  > ?- inttostr(123, X).
  X: "123" 
  * @end_example

  * @jdesc	= 整数を、文字列に変換します。 
  * @jparam	= +STRING/ string : 文字列 
  * @jparam	= -INT/ int : 整数
  * @jexample	= 
  > ?- inttostr("123", X).
  X: 123
  * @end_jexample
*/
bool strtoint( machine& m, FSP arg1, FSP arg2 ) {
  cell c = arg1.ReadCell();
  if( !IsSTG(c) ) {
    runwarn() << "strtoint/2 requires a string" << endmsg;
    return false;
  }
  const char *s = c2STG(c);
  int val = 0;
  bool Isminus;
  if (*s == '-') {
    Isminus = true;
    s++;
  }
  else 
    Isminus = false;

  while( *s ) {
    val *= 10;
    if (!isdigit(*s)) {
      return false;
    }
    val += ((*s) - '0');
    s++;
  }
  if (Isminus) val = -val;
  return arg2.Unify(FSP(m, (mint)val));
}

LILFES_BUILTIN_PRED_2(strtoint, strtoint);

/**
  * @predicate	= floattostr(+FLOAT, -STRING) 
  * @desc	= Convert a float into a string.
  * @param	= +FLOAT/ float  : Float
  * @param	= -STRING/ string  : String 
  * @example	= 
  > ?- floattostr(1.23, X).
  X: "1.23000" 
  * @end_example
  * @jdesc	= float を string に変換します．
  * @jparam	= +FLOAT/ float  : Float
  * @jparam	= -STRING/ string  : String 
*/
bool floattostr( machine&, FSP arg1, FSP arg2 ) {
    if (!arg1.IsFloat()) {
      return false;
    }
    float val = arg1.ReadFloat();
    ostringstream oss;
    oss.setf(ios::showpoint);
    oss << val;
  return arg2.Unify(oss.str().c_str());
}

LILFES_BUILTIN_PRED_2(floattostr, floattostr);

/**
  * @predicate	= strtofloat(+STRING, -FLOAT) 
  * @desc	= Convert a float into a string.
  * @param	= +STRING/ string  : String 
  * @param	= -FLOAT/ float  : Float
  * @example	= 
  > ?- strtofloat("1.23", X).
  X: 1.23000 
  * @end_example
  * @jdesc	= string を float に変換します．
  * @jparam	= +STRING/ string  : String 
  * @jparam	= -FLOAT/ float  : Float
*/
bool strtofloat( machine& m, FSP arg1, FSP arg2 ) {
    cell c = arg1.ReadCell();
    if( !IsSTG(c) ) {
        RUNWARN( "strtofloat/2 requires a string" );
	return false;
    }
    const char *s = c2STG(c);
    char* endptr;
    mfloat val = strtod(s, &endptr);
    //cerr << "val=" << val << ", errno=" << errno << endl;
    //cerr << "endptr=\"" << endptr << "\""<< endl;
    if ( endptr[0] != '\0' ) {   // Encountered a non-digit character
	return false;
    }
    if ( val == 0.0 && errno == EINVAL ) {   // Invalid format
	return false;
    }
  return arg2.Unify(FSP(m, (mfloat) val));
}

LILFES_BUILTIN_PRED_2(strtofloat, strtofloat);

/**
 * @predicate = chop(+$X, -$Y)
 * @param     = +$X/string
 * @param     = -$Y/string
 * @desc      = same as Perl
 * Same as 'chop' in Perl
*/
bool chop( machine&, FSP arg1, FSP arg2 ) {
  if ( ! arg1.IsString() ) {
    RUNWARN( "arg1 of chop/2 must be a string" );
    return false;
  }
  string s = arg1.ReadString();
  return arg2.Unify( s.substr( 0, s.size() - 1 ).c_str() );
}

LILFES_BUILTIN_PRED_2(chop, chop);

/**
 * @predicate = join(+$LIST, +$STR, -$STRING)
 * @param     = +$LIST/list of strings
 * @param     = +$STR/string
 * @param     = -$STRING/string
 * @desc      = same as Perl
 * Same as 'join' in Perl.
 * @example =
 * join(["He", "runs"], "__", "He__runs")
 * @end_example
*/
bool join( machine&, FSP arg1, FSP arg2, FSP arg3 ) {
  if ( ! arg2.IsString() ) {
    RUNWARN( "arg2 of join/3 must be a string" );
    return false;
  }
  if ( arg1.GetType() == nil ) {
    return arg3.Unify( "" );
  }
  if ( arg1.GetType() == cons ) {
    string c = arg2.ReadString();
    FSP head = arg1.Follow( hd );
    if ( ! head.IsString() ) {
      RUNWARN( "arg1 of join/3 must be a list of strings" );
      return false;
    }
    string s( head.ReadString() );
    arg1 = arg1.Follow( tl );
    while ( arg1.GetType() == cons ) {
      head = arg1.Follow( hd );
      if ( ! head.IsString() ) {
        RUNWARN( "arg1 of join/3 must be a list of strings" );
        return false;
      }
      s += c + head.ReadString();
      arg1 = arg1.Follow( tl );
    }
    return arg3.Unify( s.c_str() );
  }
  RUNWARN( "arg1 of join/3 must be a list of strings" );
  return false;
}

LILFES_BUILTIN_PRED_3(join, join);

/**
  * @predicate = split(+$STR, +$DIV, -$LIST)
  * @param     = +$STR/string : string to be splitted
  * @param     = +$DIV/integer or string: character code or string
  * @param     = -$LIST/list of strings : splitted strings
  * @desc      = split a string into substrings
  * Split $STR into a list of strings $LIST divided by $DIV
*/

bool split( machine&m, FSP arg1, FSP arg2, FSP arg3 ) {
    if(! arg1.IsString() ) {
        RUNWARN( "arg1 of split/3 must be a string" );
        return false;
    }
    char splitint[2];
    const char* splitstr;
    if( arg2.IsString() ) {
        splitstr = arg2.ReadString();
    } else if( arg2.IsInteger() ) {
        unsigned char tmp = arg2.ReadInteger();
        splitint[0] = *((char*) &tmp);
        splitint[1] = '\0';
        splitstr = splitint;
    } else {
        RUNWARN( "arg2 of split/3 must be a string or integer" );
        return false;
    }
    size_t splitstrsize = std::strlen(splitstr);
    if(splitstrsize == 0) {
        RUNWARN( "splitter is not specified in arg2 of split/3" );
        return false;
    }

    FSP r(&m);
    FSP root = r;
    
    string str = arg1.ReadString();
    string::size_type curr = 0, prev = 0, len = str.size();
    while(curr < len) {
        bool match = true;
        for(int i = 0 ; splitstr[i] ; ++i) {
            if(splitstr[i] != str[curr+i]) {
                match = false;
                break;
            }
        }
        if(match) {
            r.Coerce(cons);
            r.Follow(hd).Unify(FSP(&m, str.substr(prev, curr - prev).c_str()));
            r = r.Follow(tl);
            curr += splitstrsize;
            prev = curr;
        } else {
            ++curr;
        }
    }
    r.Coerce(cons);
    r.Follow(hd).Unify(FSP(&m, str.substr(prev, curr - prev).c_str()));
    r = r.Follow(tl);
    r.Coerce(nil);
    return arg3.Unify(root);
}

/**
  * @predicate = split(+$STR, +$DIV, +$ESC, -$LIST)
  * @param     = +$STR/string : string to be splitted
  * @param     = +$DIV/integer or string: character code or string
  * @param     = +$ESC/integer or string: character code or string
  * @param     = -$LIST/list of strings : splitted strings
  * @desc      = split a string into substrings
  * Split $STR into a list of strings $LIST divided by $DIV
  * where $ESC is an escape character for $DIV
*/

bool split( machine&m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
    if(! arg1.IsString() ) {
        RUNWARN( "arg1 of split/4 must be a string" );
        return false;
    }
    char splitint[2];
    const char* splitstr;
    if( arg2.IsString() ) {
        splitstr = arg2.ReadString();
    } else if( arg2.IsInteger() ) {
        unsigned char tmp = arg2.ReadInteger();
        splitint[0] = *((char*) &tmp);
        splitint[1] = '\0';
        splitstr = splitint;
    } else {
        RUNWARN( "arg2 of split/4 must be a string or integer" );
        return false;
    }
    size_t splitstrsize = std::strlen(splitstr);
    if(splitstrsize == 0) {
        RUNWARN( "splitter is not specified in arg2 of split/4" );
        return false;
    }
    char escapeint[2];
    const char* escapestr;
    if( arg3.IsString() ) {
        escapestr = arg3.ReadString();
    } else if( arg2.IsInteger() ) {
        unsigned char tmp = arg2.ReadInteger();
        escapeint[0] = *((char*) &tmp);
        escapeint[1] = '\0';
        escapestr = escapeint;
    } else {
        RUNWARN( "arg3 of split/4 must be a string or integer" );
        return false;
    }
    size_t escapestrsize = std::strlen(escapestr);
    if(escapestrsize == 0) {
        RUNWARN( "escape is not specified in arg3 of split/4" );
        return false;
    }


    FSP r(&m);
    FSP root = r;
    
    string str = arg1.ReadString();
    string::size_type curr = 0, prev = 0, len = str.size();
    bool escape = false;
    while(curr < len) {
        if( escape ) {
            escape = false;
            ++curr;
            continue;
        }
            // try to match escape string
        bool match = true;
        for(int i = 0 ; escapestr[i] ; ++i) {
            if( escapestr[i] != str[curr + i] ) {
                match = false;
                break;
            }
        }
        if(match) {
            escape = true;
            curr += escapestrsize;
            continue;
        }

            // try to match split string
        match = true;
        for(int i = 0 ; splitstr[i] ; ++i) {
            if( splitstr[i] != str[curr + i] ) {
                match = false;
                break;
            }
        }
        if(match) {
            r.Coerce(cons);
            r.Follow(hd).Unify(FSP(&m, str.substr(prev, curr - prev).c_str()));
            r = r.Follow(tl);
            curr += splitstrsize;
            prev = curr;
            continue;
        }
        ++curr;
    }
    r.Coerce(cons);
    r.Follow(hd).Unify(FSP(&m, str.substr(prev, curr - prev).c_str()));
    r = r.Follow(tl);
    r.Coerce(nil);
    return arg4.Unify(root);
}

LILFES_BUILTIN_PRED_3(split, split);
LILFES_BUILTIN_PRED_OVERLOAD_4(split, split_4, split);

/**
 * @predicate = concatenate_string(+$X, +$Y, -$Z)
 * @param     = +$X/string
 * @param     = +$Y/string
 * @param     = -$Z/string
 * @desc      = concatenation of strings
 * This lets $Z be the concatination of $X and $Y.
*/
/**
 * @predicate = concatenate_string(+$L, -$R)
 * @param     = +$L/ list of string
 * @param     = -$R/ string
 * @desc      = concatenation of strings
 * This lets $R be the concatination of all the elements in $L.
*/
bool concatenate_string( machine& m, FSP arg1, FSP arg2 ) {
  vector< string > str_list;
  if ( ! lilfes_to_c< vector< string > >().convert( m, arg1, str_list ) ) {
    RUNWARN( "arg1 of concatenate_string/2 must be a list of strings" );
    return false;
  }
  string s;
  for ( vector< string >::const_iterator it = str_list.begin();
        it != str_list.end();
        ++it ) {
    s += *it;
  }
  return arg2.Unify( s.c_str() );
}

LILFES_BUILTIN_PRED_2( concatenate_string, concatenate_string );
LILFES_BUILTIN_PRED_OVERLOAD_3( strcat, concatenate_string_3, concatenate_string );

/**
  * @predicate = replace_substring(+$STR, +$EXP, +$SUBST, -$STROUT)
  * @param     = +$STR/string : target string
  * @param     = +$EXP/string: pattern string
  * @param     = +$SUBST/string: substitution string
  * @param     = -$STROUT/strings : replaced string
  * @desc      = replace substring $EXP in $STR with $SUBST
*/

bool replace_substring( machine&m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
    static vector<char> buf(1000);
    if(! arg1.IsString() ) {
        RUNWARN( "arg1 of replace_substring/4 must be a string" );
        return false;
    }
    if(! arg2.IsString() ) {
        RUNWARN( "arg2 of replace_substring/4 must be a string" );
        return false;
    }
    if(! arg3.IsString() ) {
        RUNWARN( "arg3 of replace_substring/4 must be a string" );
        return false;
    }
    string str = arg1.ReadString(); size_t str_len = str.size();
    string exp = arg2.ReadString(); size_t exp_len = exp.size();
    string subst = arg3.ReadString(); size_t subst_len = subst.size();

    string::size_type strp = 0, bufp = 0;
    bool match = false;
    while(strp < str_len) {
        if(str.compare(strp, exp_len, exp) == 0) {
            match = true;
            for(size_t i = 0 ; i < subst_len ; ++i) {
                if(buf.size() < bufp) buf.resize(2*buf.size());
                buf[bufp++] = subst[i];
            }
            strp += exp_len;
        } else {
            if(buf.size() < bufp) buf.resize(2*buf.size());
            buf[bufp++] = str[strp++];
        }
    }
    if(!match) return false;
    buf[bufp] = '\0';
    return arg4.Unify(FSP(&m, &buf[0]));
}

LILFES_BUILTIN_PRED_4(replace_substring, replace_substring);
} // namespace builtin

} // namespace lilfes

