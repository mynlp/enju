/**
 * @module  = system
 * @cvs     = $Id: system.cpp,v 1.13 2011-05-02 10:38:24 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = System operations
 * Predicates for operating a system
 * @jdesc   = システム制御
 * @japanese =
 * システム制御を行なう述語です．
 * @end_japanese
*/

#include "builtin.h"
#include <string>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif // HAVE_SYS_TIME_H
#ifdef HAVE_TIME_H
#include <time.h>
#endif // HAVE_TIME_H

namespace lilfes {

using std::string;


namespace builtin {

#ifdef LILFES_WIN_
#endif

/**
  * @predicate	= getenv(+STR,-ENV) 
  * @desc	= Return content of enviroment variable STR.
  * @param	= +STR/string : name of enviroment variable
  * @param	= -ENV/string : content of enviroment variable
  * @deprecated = use 'environ/2'
  * @example	= 
  * @end_example
  * @jdesc	= 環境変数の値を取得します．
  * @jparam	= +STR/string : 変数名
  * @jparam	= -ENV/string : 変数の値
  * @jdeprecated = 'environ/2' を使用してください．
  * @jexample	= 
  * @end_jexample
*/
  bool getenv( machine&, FSP arg1, FSP arg2 ) {
    cell c = arg1.ReadCell();
    if( !IsSTG(c) ) {
      RUNWARN( "getenv/2 requires a string" );
      return false;
    }

    const char *s = c2STG(c);
    char *e = ::getenv(s);

    if (e == NULL) {
      return false;
    }
    return arg2.Unify( e );
  }

  LILFES_BUILTIN_PRED_2(getenv, getenv);

/**
  * @predicate	= environ(+STR,-ENV) 
  * @desc	= Return content of enviroment variable STR.
  * @param	= +STR/string : name of enviroment variable
  * @param	= -ENV/string : content of enviroment variable
  * @example	= 
  * @end_example
  * @jdesc	= 環境変数の値を取得します．
  * @jparam	= +STR/string : 変数名
  * @jparam	= -ENV/string : 変数の値
  * @jexample	= 
  * @end_jexample
*/

#if LILFES_WIN_
#undef environ
#endif
  LILFES_BUILTIN_PRED_2(getenv, environ);
#if LILFES_WIN_
#define environ _environ
#endif


///////////////////////////////////////////////////////////////////////////////

/**
  * @predicate = getdate(-$DATE)
  * @param     = -$DATE/string : current date
  * @desc      = get the current date
  * @deprecated = use 'now/1'
  * You can get the current date with this predicate.
  * @jparam     = -$DATE/string : 現在の日時
  * @jdesc      = 現在の日時を取得します．
  * @jdeprecated = 'now/1' を使用してください．
  * @japanese  =
  * 現在の日時をあらわす文字列を取得することが出来ます．
  * @end_japanese
*/

  bool getdate( machine&, FSP arg1 ) {
#if defined( HAVE_GETTIMEOFDAY ) && defined( HAVE_CTIME )
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    string str( ctime( &current_time.tv_sec ) );
    return arg1.Unify( str.substr( 0, str.size() - 1 ).c_str() );
#elif defined(HAVE_TIME_H) && defined(LILFES_WIN_) // LILFES_WIN_
  __time32_t ltime;

  _time32( &ltime );

  string str(_ctime32( &ltime ));

  return arg1.Unify( str.substr( 0, str.size() - 1 ).c_str() );
#else // HAVE_GETTIMEOFDAY && HAVE_CTIME
    RUNWARN( "getdata: gettimeofday is not implemented in this system" );
    return arg1.Unify( "" );
#endif // HAVE_GETTIMEOFDAY && HAVE_CTIME
  }

  LILFES_BUILTIN_PRED_1(getdate, getdate);

/**
  * @predicate = now(-$DATE)
  * @param     = -$DATE/string : current date
  * @desc      = get the current date
  * You can get the current date with this predicate.
  * @jparam     = -$DATE/string : 現在の日時
  * @jdesc      = 現在の日時を取得します．
  * @japanese  =
  * 現在の日時をあらわす文字列を取得することが出来ます．
  * @end_japanese
*/

  LILFES_BUILTIN_PRED_1(getdate, now);


/**
  * @predicate = gettime(-$SEC, -$USEC)
  * @param     = -$SEC/integer: current second
  * @param     = -$USEC/integer: current micro second
  * @desc      = get the current time
  * @jparam     = -$SEC/integer: 現在の秒
  * @jparam     = -$USEC/integer: 現在のマイクロ秒
  * @jdesc      = 現在の時間を取得します．
*/
#define fail 0
  bool gettime( machine&m, FSP arg1, FSP arg2) {
#if defined( HAVE_GETTIMEOFDAY ) && defined( HAVE_CTIME )
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return arg1.Unify(FSP(&m, ((mint) current_time.tv_sec))) && arg2.Unify(FSP(&m, ((mint) current_time.tv_usec)));
#else // HAVE_GETTIMEOFDAY && HAVE_CTIME
    RUNWARN( "getdata: gettimeofday is not implemented in this system" );
    return fail;
#endif // HAVE_GETTIMEOFDAY && HAVE_CTIME
  }

  bool gettime( machine&m, FSP arg1, FSP arg2, FSP arg3) {
#if defined( HAVE_GETTIMEOFDAY ) && defined( HAVE_CTIME )
    struct timeval current_time;
    gettimeofday( &current_time, NULL );
    return arg1.Unify(FSP(&m, ((mint) current_time.tv_sec / 86400)))
        && arg2.Unify(FSP(&m, ((mint) current_time.tv_sec % 86400)))
        && arg3.Unify(FSP(&m, ((mint) current_time.tv_usec)));
#else // HAVE_GETTIMEOFDAY && HAVE_CTIME
    RUNWARN( "getdata: gettimeofday is not implemented in this system" );
    return fail;
#endif // HAVE_GETTIMEOFDAY && HAVE_CTIME
  }

  LILFES_BUILTIN_PRED_2(gettime, gettime);

  LILFES_BUILTIN_PRED_OVERLOAD_3(gettime, gettime_3, gettime);

///////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= system(+COMMAND) 
  * @desc	= Execute external command
  * @param	= +COMMAND/string : command
  * @example	= 
  * @end_example
  * @jdesc	= 外部コマンドを実行します．
  * @jparam	= +COMMAND/string : コマンド名
  * @jexample	= 
  * @end_jexample
*/

  bool system( machine&, FSP arg1 )
  {
    cell c1 = arg1.ReadCell();
    if( !IsSTG(c1) ) {
      RUNWARN( "Warning: system/1 requires a string" );
      return false;
    }

    const char *s1 = c2STG(c1);
#ifdef PROFILE    
    profiler::StopSignals ();
#endif
    int ret = ::system(s1);
#ifdef PROFILE
    profiler::StartSignals();
#endif
    return ret == 0;
  }

  LILFES_BUILTIN_PRED_1(system, system);

}




} // namespace lilfes










