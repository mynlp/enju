/**
 * @module  = display
 * @cvs     = $Id: display.cpp,v 1.7 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Display
 * @jdesc   = 表示
 * Builtin predicates for displaying features structures etc.
 * @japanese =
 * 素性構造などを表示するための組み込み述語です．
 * @end_japanese
*/

#include "builtin.h"
#include "in.h"

#include <list>
#include <sstream>
#include <string>

namespace lilfes {

using std::list;
using std::ostringstream;
using std::string;


namespace builtin {

/**
  * @predicate	= print(+$X)
  * @desc	= Display feature structure or predicate <i>$X</i> in the form that can read LiLFeS.Predicates and lists is also showed as feature structure.
  * @param	= +$X/bot  : predicate or feature structure etc..
  * @example	= 
  > :- print(append([A|X], Y, [A|Z]) :- append(X,Y,Z)).
  　
  (:-/2
   ,chead:(append/3
           ,arg1:[ $1
                 | $2, 'list']
           ,arg2:$3
           ,arg3:[ $1
                 | $4, 'list'])
   ,cbody:[ (append/3
             ,arg1:$2
             ,arg2:$3
             ,arg3:$4)]        ) 
  * @end_example

  * @jdesc		= 素性構造や述語<i>$X</i>を、LiLFeSに読み込める形で表示します．述語やリストも素性構造として表されます． 
  * @jparam		= +$X/bot : 素性構造・述語など
  * @jexample	=
  > :- print(append([A|X], Y, [A|Z]) :- append(X,Y,Z)).
  　
  (:-/2
   ,chead:(append/3
           ,arg1:[ $1
                 | $2, 'list']
           ,arg2:$3
           ,arg3:[ $1
                 | $4, 'list'])
   ,cbody:[ (append/3
             ,arg1:$2
             ,arg2:$3
             ,arg3:$4)]        ) 
  * @end_jexample
*/
bool print( machine&, FSP arg1 )
{
  (*output_stream) << arg1.DisplayLiLFeS();
  return true;
}

LILFES_BUILTIN_PRED_1(print, print);

/**
  * @predicate	= printtostr(+$X, -$Y)
  * @desc	= Display feature structure or predicate into string.
  * @param	= +$X/bot  : predicate or feature structure etc.
  * @param      = -$Y/string : resulting string
  * @see        = #print/1

  * @jdesc	= 素性構造や述語を、string 形式に表示します．
  * @jparam	= +$X/bot : 素性構造・述語など
  * @jparam     = -$Y/string : 結果の文字列
*/
bool printtostr( machine&, FSP arg1, FSP arg2 ) {
  ostringstream os;
  os << arg1.DisplayLiLFeS();
  return arg2.Unify(os.str().c_str());
}

LILFES_BUILTIN_PRED_2(printtostr, printtostr);

/**
  * @predicate	= printAVM(+$S) 
  * @desc	= Display feature structure <i>$S</i> as AVM-tree form.
  * @param	= +$S/bot : Feature structure
  * @example	= 
  > :- printAVM(person & NAME\"John" & AGE\10 & FATHER\(NAME\"Tom" & AGE\40 & FATHER\(NAME\"Bob" & AGE\70))).
  　
  |~person                             ~|
  | NAME:"John"                         |
  | AGE:10                              |
  |        |~person                  ~| |
  |        | NAME:"Tom"               | |
  |        | AGE:40                   | |
  | FATHER:|        |~person       ~| | |
  |        | FATHER:| NAME:"Bob"    | | |
  |        |        | AGE:70        | | |
  |_       |_       |_FATHER:person_|_|_|
  * @end_example

  * @jdesc		= 素性構造<i>$S</i>をＡＶＭ木で表示します．
  * @jparam		= +$S/bot : 素性構造
  * @jexample	=
  > :- printAVM(person & NAME\"John" & AGE\10 & FATHER\(NAME\"Tom" & AGE\40 & FATHER\(NAME\"Bob" & AGE\70))).
  　
  |~person                             ~|
  | NAME:"John"                         |
  | AGE:10                              |
  |        |~person                  ~| |
  |        | NAME:"Tom"               | |
  |        | AGE:40                   | |
  | FATHER:|        |~person       ~| | |
  |        | FATHER:| NAME:"Bob"    | | |
  |        |        | AGE:70        | | |
  |_       |_       |_FATHER:person_|_|_|
  * @end_jexample
*/
bool printAVM( machine&, FSP arg1 )
{
  (*output_stream) << arg1.DisplayAVM();
  return true;
}

LILFES_BUILTIN_PRED_1(printAVM, printAVM);

/**
  * @predicate	= printAVMtostr(+FS,-STR) 
  * @desc	= Return feature structure to string by LiLFeS program.
  * @param	= +FS/bot: Feature Structure
  * @param	= -STR/string: string
  * @see        = #printAVM/1
  * @example	= 
  * @end_example
  * @jdesc	= 素性構造<i>$S</i>をＡＶＭ木で文字列にします．
  * @jparam	= +FS/bot: 素性構造
  * @jparam     = -STR/string: 文字列
  * @jexample	=
*/
bool printAVMtostr( machine&, FSP arg1, FSP arg2 ) {
  ostringstream os;
  os << arg1.DisplayAVM();
  return arg2.Unify(os.str().c_str());
}

LILFES_BUILTIN_PRED_2(printAVMtostr, printAVMtostr);

} // namespace builtin

} // namespace lilfes

