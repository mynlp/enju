/**
 * @module  = misc
 * @cvs     = $Id: misc.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Miscellaneous
 * @jdesc   = その他
 * その他の組込み述語です．
*/

#include "builtin.h"
#include <ostream>
#include <sstream>

namespace lilfes {

using std::endl;
using std::ends;
using std::ostringstream;


/**
  * @predicate	= lilfes_version($X) 
  * @desc		= Return lilfes version.
  * @param		= +$X/bot: the variable which get the lilfes version
  * @example	= 
  > ?- lilfes_version($X).
  X: "LiLFeS 1.2          by Takaki Makino and Tsujii lab., Tokyo Univ.
  Last Compiled on May 14 2004, 20:22:09 [PROF=1]"

  Return lilfes version.
  * @end_example

  * @jdesc		= LiLFeSのバージョンを返します．
  * @jparam		= +$NAME/bot :LiLFeSのバージョンを格納する変数
  * @jexample	=
  > ?- lilfes_version($X).
  X: "LiLFeS 1.2          by Takaki Makino and Tsujii lab., Tokyo Univ.
  Last Compiled on May 14 2004, 20:22:09 [PROF=1]"

  LiLFeSのバージョンを返します．
  * @end_jexample
*/

extern const char *Version;
extern const char *Copyright;
extern const char *LastCompiled;

namespace builtin {

bool lilfes_version( machine&, FSP arg1 )
{

        ostringstream oss;
	oss << Version << Copyright << endl;
	oss << "Last Compiled on " << LastCompiled
#if defined( DEBUG )
	<< " [DEBUG]"
#elif defined( DOASSERT )
	<< " [DOASSERT]"
#endif
#if defined( PROFILE )
	<< " [PROF=" << PROFILE << "]"
#endif
	<< ends;

        return arg1.Unify(oss.str().c_str());
}

LILFES_BUILTIN_PRED_1(lilfes_version, lilfes_version);

/**
  * @predicate	= lilfes_version($X, $Y)
  * @desc		= Return lilfes version.
  * @param		= +$X/bot: lilfes version
  * @param		= +$X/bot: last compiled time
  * @example	= 
  > ?- lilfes_version($X).
  X: "LiLFeS 1.2"
  Y: "May 14 2004, 20:22:09"

  Return lilfes version.
  * @end_example

  * @jdesc		= LiLFeSのバージョンを返します．
  * @jparam		= +$X/bot :LiLFeSのバージョンを格納する変数
  * @jparam		= +$Y/bot :コンパイルされた時間を格納する変数
  * @jexample	=
  > ?- lilfes_version($X).
  X: "LiLFeS 1.2"
  Y: "May 14 2004, 20:22:09"

  LiLFeSのバージョンを返します．
  * @end_jexample
*/

bool lilfes_version_2( machine&, FSP arg1, FSP arg2 )
{
  return arg1.Unify( Version ) && arg2.Unify( LastCompiled );
}

LILFES_BUILTIN_PRED_OVERLOAD_2(lilfes_version_2, lilfes_version_2, lilfes_version);

} // namespace builtin

} // namespace lilfes

