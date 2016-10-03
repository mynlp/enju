/**
 * @module  = fsmanip
 * @cvs     = $Id: builtinfs.cpp,v 1.18 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Feature structure manipulation
 * @jdesc   = 素性構造操作
 * Predicates for manipulating feature structures
 * @japanese =
 * 素性構造を操作するための述語です．
 * @end_japanese
*/

#include "builtin.h"
#include <algorithm>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace lilfes {

using std::copy;
using std::list;
using std::make_pair;
using std::pair;
using std::set;
using std::string;
using std::vector;


namespace builtin {

/**
  * @predicate	= copy(+$S1, -$S2) 
  * @desc	= Copy the feature structure <i>$S1</i> in heap to <i>$S2</i>.
  * @param	= +$S1/bot : Feature structure1
  * @param	= -$S2/bot : Feature structure2
  * Copy the feature structure <i>$S1</i> in heap to <i>$S2</i>.
  * It is different from assignment by unification such as S1=S2,because there causes no structure sharing.
  * @example	= 
  > ?- X = person, X = Y.
  X: [$1] person
  Y: [$1] ...
  > ?- X = person, copy(X, Y).
  X: person
  Y: person
  * @end_example

  * @jdesc		= 素性構造<i>S1</i>をヒープ上でコピーして<i>S2</i>に入れます。
  * @jparam		= +$S1/bot : 素性構造1
  * @jparam		= -$S2/bot : 素性構造2
  * @japanese =
  * 素性構造<i>S1</i>をヒープ上でコピーして<i>S2</i>に入れます。 
  * S1 = S2 と、単一化によって代入する場合と違って、構造共有が起こりません。 
  * @end_japanese
  * @jexample	=
  > ?- X = person, X = Y.
  X: [$1] person
  Y: [$1] ...
  > ?- X = person, copy(X, Y).
  X: person
  Y: person
  * @end_jexample
*/
bool copy( machine&, FSP arg1, FSP arg2 )
{
  return arg2.Unify( arg1.Copy() );
}

LILFES_BUILTIN_PRED_2( copy, copy );

/**
  * @predicate	= normalize(+$S1, -$S2) 
  * @desc	= Normalize the feature structure <i>$S1</i> in heap to <i>$S2</i>.
  * @param	= +$S1/bot : Feature structure1
  * @param	= -$S2/bot : Feature structure2
  * @note       = Second argument must be 'bot'.
  * Normalize the feature structure <i>$S1</i> in heap to <i>$S2</i>.
  * Copied feature structure can be manipulated with 'b_copy/2' and 'recopy'.
  * @jdesc	= 素性構造 <i>$S1</i> を正規化して <i>$S2</i> にコピーします．
  * @jparam	= +$S1/bot : Feature structure1
  * @jparam	= -$S2/bot : Feature structure2
  * @jnote      = 第２引数は bot でなければなりません．
  * @japanese   =
  * 素性構造 <i>$S1</i> を正規化して <i>$S2</i> にコピーします．
  * コピーした素性構造にたいして，'b_copy/2' や 'recopy' を使うことが出来ます．
  * @end_japanese
  */
bool normalize( machine& m, FSP arg1, FSP arg2 )
{
  if ( arg2.GetType() != bot ) {
    RUNWARN("2nd argument of normalize/2 must be 'bot'");
    return false;
  }
  FSP x = arg1.Copy();
  m.WriteHeap(arg2.Deref().GetAddress(), PTR2c(x.GetAddress()));
  return true;
}

LILFES_BUILTIN_PRED_2(normalize, normalize);

/**
  * @predicate	= canonical_copy(+$S1, -$S2) 
  * @desc	= Copy the feature structure <i>$S1</i> in heap to <i>$S2</i>.
  * @param	= +$S1/bot : Feature structure1
  * @param	= -$S2/bot : Feature structure2
  * Copy the feature structure <i>$S1</i> in heap to <i>$S2</i>.
  * Feature structures with default feature values are reduced to atoms.
  * @jdesc	= 素性構造 <i>$S1</i> を <i>$S2</i> にコピーします．
  * @jparam	= +$S1/bot : Feature structure1
  * @jparam	= -$S2/bot : Feature structure2
  * @japanese   =
  * 素性構造 <i>$S1</i> を <i>$S2</i> にコピーします．素性の値がすべて
  * デフォルト値の素性構造は正規化されて atom に変換します．
  * @end_japanese
*/
bool canonical_copy( machine&, FSP arg1, FSP arg2 )
{
  FSP x = arg1.CanonicalCopy();
  return arg2.Unify( x );
}

LILFES_BUILTIN_PRED_2( canonical_copy, canonical_copy );

/**
  * @predicate = unifiable(+$A, +$B)
  * @param     = +$A/bot
  * @param     = +$B/bot
  * @desc      = check unifiability of feature structures without doing unification
  * This predicate succeeds if $A and $B are unifiable.
  * $A and $B remain unchanged after this predicate succeeds.
  * @jparam     = +$A/bot
  * @jparam     = +$B/bot
  * @jdesc      = 実際に単一化をすることなく単一化可能かどうかチェックします．
  * @japanese   =
  * $A と $B が単一化可能なときにこの述語は成功します．
  * この述語を実行した後，$A と $B は書きかえられません．
  * @end_japanese
 */
bool unifiable( machine& m, FSP arg1, FSP arg2 )
{
  m.SetTrailPoint();
  bool ret = arg1.Unify( arg2 );
  m.TrailBack();
  return ret;
}

LILFES_BUILTIN_PRED_2( unifiable, unifiable );

//////////////////////////////////////////////////////////////////////

static void marknodes(FSP f, vector<core_p>& slist) {
  FSP fs = f.Deref();
  for ( vector< core_p >::const_iterator it = slist.begin();
        it != slist.end();
        ++it )
    if ( *it == fs.GetAddress() ) return;
  slist.push_back( fs.GetAddress() );

  cell c = fs.ReadCell();
  if (IsSTR(c)) {
    const type *t = c2STRorVAR(c);
    for (int i = 0; i < t->GetNFeatures(); i++)
      marknodes(fs.FollowNth(i),slist);
  }
}

static bool isshared_sub(FSP f, vector<core_p>& slist1, vector<core_p>& slist2) {
  FSP fs = f.Deref();
  for ( vector< core_p >::const_iterator it = slist1.begin();
        it != slist1.end();
        ++it )
    if ( *it == fs.GetAddress() ) // SHARE !!
      return true;
  for ( vector< core_p >::const_iterator it = slist2.begin();
        it != slist2.end();
        ++it )
    if ( *it == fs.GetAddress() )
      return false;
  slist2.push_back( fs.GetAddress() );

  cell c = fs.ReadCell();
  if (IsSTR(c)) {
    const type *t = c2STRorVAR(c);
    for (int i = 0; i < t->GetNFeatures(); i++)
      if (isshared_sub(fs.FollowNth(i),slist1,slist2))
	return true;
  }
  return false;
}

/**
  * @predicate	= isshared(+S1, +S2) 
  * @desc	= Success if there is any shared structure between S1 and S2 
  * @param	= +S1/ bot  : Feature structure1
  * @param	= +S2/ bot  : Feature structure2 
  * @example	= 
  > ?- X = NAME\"John", Y = FATHER\X, isshared(X,Y).
  　
          |~person       ~|
  X: [$1] | NAME:"John"   |
          | AGE:integer   |
          |_FATHER:person_|
     |~person         ~|
  Y: | NAME:string     |
     | AGE:integer     |
     |_FATHER:[$1] ..._|
  * @end_example

  * @jdesc	= ２つの素性構造の間に構造共有がある時に成功します。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jexample	= 
  > ?- X = NAME\"John", Y = FATHER\X, isshared(X,Y).
  　
          |~person       ~|
  X: [$1] | NAME:"John"   |
          | AGE:integer   |
          |_FATHER:person_|
     |~person         ~|
  Y: | NAME:string     |
     | AGE:integer     |
     |_FATHER:[$1] ..._|
  * @end_jexample
*/
bool isshared( machine&, FSP arg1, FSP arg2 ) {
  TRACE("execwork:isshared");
  vector<core_p> slist1;
  vector<core_p> slist2;
  marknodes(arg1,slist1);
  return isshared_sub(arg2,slist1,slist2);
}

LILFES_BUILTIN_PRED_2(isshared,isshared);

/**
  * @predicate	= isnotshared(+S1, +S2) 
  * @desc	= Success if there isn't any shared structure between S1 and S2 
  * @param	= +S1/ bot  : Feature structure1
  * @param	= +S2/ bot  : Feature structure2 
  * @example	= 
  > ?- X = NAME\"John", Y = FATHER\X, isnotshared(X,Y).
  no 
  * @end_example

  * @jdesc	= ２つの素性構造の間に構造共有がない時に成功します。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jexample	= 
  > ?- X = NAME\"John", Y = FATHER\X, isnotshared(X,Y).
  no 
  * @end_jexample
*/
bool isnotshared( machine&, FSP arg1, FSP arg2 ) {
  TRACE("execwork:isnotshared");
  vector<core_p> slist1;
  vector<core_p> slist2;
  marknodes(arg1,slist1);
  return !isshared_sub(arg2,slist1,slist2);
}

LILFES_BUILTIN_PRED_2(isnotshared,isnotshared);

/**
  * @predicate	= recopy(+S1, -S2) 
  * @desc	= Copy feature structure 1 to feature structure 2.
  * @param	= +S1/ bot  : Feature structure1
  * @param	= -S2/ bot  : Feature structure2 
  * Copy feature structure 1 to feature structure 2.
  * Feature structure1 must be one part in heap.  This can be used copy after copy.
  * @example	= 
  * @end_example

  * @jdesc	= 素性構造S1をS2へコピーします。
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= -S2/ bot : 素性構造２ 
  * @japanese =
  * 素性構造S1をS2へコピーします。 ただし素性構造1がヒープ上で一塊になっていなくてはいけません．コピーの後などに更にコピーする場合に使えます．
  * @end_japanese
  * @jexample	= 
  * @end_jexample
*/
core_p recopy(core_p addr1, machine& m) {
#ifdef PROF_COPY
  P1("Array::Recopy");
#else
  P2("Array::Recopy");
#endif

#ifdef FORCE_NORMAL_COPY
	return FSP(m, addr1).Copy().GetAddress();
#else
  const int LIMIT_CHECK_INTERVAL = 1024;
  core_p addr2 = m.GetHP();
  int offset = addr2 - addr1;
  const cell endcell = PTR2c(CORE_P_INVALID); // Barrier cell
  core_p paddr;
  int i = 0;
  while(1) {
    cell c = m.ReadHeap(addr1 + i);
    if( (i++ & (LIMIT_CHECK_INTERVAL-1)) == 0 )
      m.CheckHeapLimit(m.GetHP()+LIMIT_CHECK_INTERVAL+100);
    switch(Tag(c)) {
    case T_PTR3:
      if (c == endcell) {
	m.PutHeap(c);
	return addr2;
      }
    case T_PTR:
    case T_PTR1:
    case T_PTR2:
      paddr = c2PTR(c);
      m.PutHeap(PTR2c(paddr + offset));
      continue;
    default:
      m.PutHeap(c);
    };
  }
#endif
}

bool recopy(machine& m, FSP arg1, FSP arg2 ) {
    core_p addr2 = recopy(arg1.GetAddress(), m);
    return arg2.Unify(FSP(m, addr2));
}

LILFES_BUILTIN_PRED_2(recopy, recopy);

/**
  * @predicate	= b_equiv(+S1, +S2) 
  * @desc	= Success if feature structure1 are equivalent to feature structure 2.
  * @param	= +S1/ bot  : Feature structure1
  * @param	= +S2/ bot  : Feature structure2 
  * @example	= 
  * @end_example

  * @jdesc	= ２つの素性構造が同等であれば成功します。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jexample	= 
  * @end_jexample
*/
bool b_equiv( machine&, FSP arg1, FSP arg2 ) {
  return arg1.b_equiv( arg2 );
}

LILFES_BUILTIN_PRED_2(b_equiv,b_equiv);

/**
  * @predicate	= identical(+FS1,+FS2)
  * @desc	= Sucess if feature structure1 <i>FS1</i> and feature structure2<i>FS2</i> are identical. 
  * @param	= +FS1/: feature structure 1
  * @param	= +FS2/: feature structure 2
  * @example	= 
  > ?- X = person & AGE\10, X = Y, identical(X,Y). 
  　
          |~person       ~|
  X: [$1] | NAME:string   |
          | AGE:10        |
          |_FATHER:person_|
  　
  > ?- identical(person & AGE\10, person & AGE\10).
  no 
  * @end_example

  * @jdesc	= ２つの素性構造<i>FS1</i>と<i>FS2</i>が構造共有している場合に成功します。
  * @jparam	= +FS1/: 素性構造1
  * @jparam	= +FS2/: 素性構造2
  * @jexample	= 
  > ?- X = person & AGE\10, X = Y, identical(X,Y). 
  　
          |~person       ~|
  X: [$1] | NAME:string   |
          | AGE:10        |
          |_FATHER:person_|
  　
  > ?- identical(person & AGE\10, person & AGE\10).
  no 
  * @end_jexample
*/
bool identical( machine&, FSP arg1, FSP arg2 ) {
  TRACE("execwork:identical");
  if (arg1.Deref().GetAddr() != arg2.Deref().GetAddr()){   return false;}
  else{  return true;}
}

LILFES_BUILTIN_PRED_2(identical,identical);

/**
  * @predicate = not_identical(+$FS1, +$FS2)
  * @param     = +$FS1/bot
  * @param     = +$FS2/bot
  * @desc      = This is true when $FS1 and $FS2 are not identical.
  * @jparam     = +$FS1/bot
  * @jparam     = +$FS2/bot
  * @jdesc      = 素性構造 $FS1 と $FS2 が identical でないときに成功します．
*/
bool not_identical( machine&, FSP arg1, FSP arg2 ) {
  if (arg1.Deref().GetAddr() != arg2.Deref().GetAddr()){ return true; }
  else { return false; }
}

LILFES_BUILTIN_PRED_2(not_identical,not_identical);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= is_integer(+T)
  * @desc	= Success if the input is an integer. 
  * @param	= +T/ bot : Type 
  * @example	= 
  > ?- is_integer(5).
  yes 
  * @end_example

  * @jdesc	= 入力が整数であれば成功します。 
  * @jparam	= +T/ bot: 型 
  * @jexample	= 
  > ?- is_integer(5).
  yes 
  * @end_jexample
*/
bool is_integer( machine&, FSP arg1 )
{
          return arg1.IsInteger();
}

LILFES_BUILTIN_PRED_1(is_integer, is_integer);

/**
  * @predicate	= is_string(+T)
  * @desc	= Success if the input is an string. 
  * @param	= +T/ bot : Type 
  * @example	= 
  > ?- is_string("abc").
  yes 
  * @end_example

  * @jdesc	= 入力が文字列であれば成功します。 
  * @jparam	= +T/ bot: 型 
  * @jexample	= 
  > ?- is_string("abc").
  yes 
  * @end_jexample
*/
bool is_string( machine&, FSP arg1 )
{
          return arg1.IsString();
}

LILFES_BUILTIN_PRED_1(is_string, is_string);

/**
  * @predicate	= is_float(+T)
  * @desc	= Success if the input is an float. 
  * @param	= +T/ bot : Type 
  * @example	= 
  > ?- is_float(-5.3).
  yes 
  * @end_example

  * @jdesc	= 入力が実数であれば成功します。 
  * @jparam	= +T/ bot: 型 
  * @jexample	= 
  > ?- is_float(-5.3).
  yes 
  * @end_jexample
*/
bool is_float( machine&, FSP arg1 )
{
  return arg1.IsFloat();
}

LILFES_BUILTIN_PRED_1(is_float, is_float);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= have_child(+S)
  * @desc	= Success if <i>S</i> has a child (= <i>S</i> is node).
  * @param	= +F/ bot : feature structure
  * @example	= 
  * @end_example

  * @jdesc	= <i>S</i>が子を持てば（＝nodeであれば）成功します。 
  * @jparam	= +F/ bot : 素性構造
  * @jexample	= 
  * @end_jexample
*/
bool have_child( machine&, FSP arg1 )
{
  RUNWARN("Obsolete predicate have_child/1; use compound/1 instead");
  return arg1.IsNode();
}

LILFES_BUILTIN_PRED_1(have_child, have_child);

/**
  * @predicate	= have_no_child(+S)
  * @desc	= Success if <i>S</i> has no child (= <i>S</i> is leaf).
  * @param	= +F/ bot : feature structure
  * @example	= 
  * @end_example

  * @jdesc	= <i>S</i>が子を持っていなければ（＝leafであれば）成功します。 
  * @jparam	= +F/ bot : 素性構造
  * @jexample	= 
  * @end_jexample
*/
bool have_no_child( machine&, FSP arg1 )
{
  RUNWARN("Obsolete predicate have_no_child/1; use simple/1 instead");
  return arg1.IsLeaf();
}

LILFES_BUILTIN_PRED_1(have_no_child, have_no_child);

/**
  * @predicate	= compound(+S)
  * @desc	= Success if <i>S</i> has a child (= <i>S</i> is node).
  * @param	= +F/ bot : feature structure
  * @example	= 
  * @end_example

  * @jdesc	= <i>S</i>が子を持てば（＝nodeであれば）成功します。 
  * @jparam	= +F/ bot : 素性構造
  * @jexample	= 
  * @end_jexample
*/
bool compound( machine&, FSP arg1 )
{
  return arg1.IsNode();
}

LILFES_BUILTIN_PRED_1(compound, compound);

/**
  * @predicate	= simple(+S)
  * @desc	= Success if <i>S</i> has no child (= <i>S</i> is leaf).
  * @param	= +F/ bot : feature structure
  * @example	= 
  * @end_example

  * @jdesc	= <i>S</i>が子を持っていなければ（＝leafであれば）成功します。 
  * @jparam	= +F/ bot : 素性構造
  * @jexample	= 
  * @end_jexample
*/
bool simple( machine&, FSP arg1 )
{
  return arg1.IsLeaf();
}

LILFES_BUILTIN_PRED_1(simple, simple);

//////////////////////////////////////////////////////////////////////

/**
  * @predicate	= follow(+S1, +L, ?S2) 
  * @desc	= Follow the path <i>L</i> of the feature structure <i>S1</i> and return the result in <i>S2</i> .
  * @param	= +S1/ bot : Feature Structure
  * @param	= +L:/ feature or list : Feature and list of paths of features
  * @param	= ?S2/ bot  : Returned Feature Result. 
  * @note	= +L should contain atleast the parents in the hierarchy or children with features. In both the cases, \ is required after Feature name. 
  * @example	= 
  > ?- follow(a & F1\n ,F1\ ,X).
  X: n
  > ?- follow([a, b, c], [tl\, hd\], X).
  X: b 
  　
  This example is equal to "?- a & F1\n = F1\X." and "?- [a,b,c] = tl\hd\X.",but actually this can be used feature got by type_featurelist on L.
  * @end_example

  * @jdesc	= 素性構造<i>S1</i>のパス<i>L</i>をたどった結果を取得して<i>S2</i>として返します。
  * @jparam	= +S1/ bot : 素性構造
  * @jparam	= +L/ feature or list : 素性のパスを表すリスト (パスの長さが1の場合、直接素性を記述してもかまいません) 
  * @jparam	= ?S2/ bot : 結果の素性構造 
  * @jnote	= +L は、最も上の階層の素性を指定する他に、下の階層の素性（素性の型の持つ素性など）のパスをリストとして指定することができます。いずれの場合も、素性名には \が必要です。 
  * @jexample	= 
  > ?- follow(a & F1\n ,F1\ ,X).
  X: n
  > ?- follow([a, b, c], [tl\, hd\], X).
  X: b 
  　
  上の例は、?- a & F1\n = F1\X. や ?- [a,b,c] = tl\hd\X. と等価ですが、実際は L の部分を type_featurelist で取得した素性にしたりして使えます。 
  * @end_jexample
*/
bool follow( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
	const type *tf;

	while( arg2.GetType()->IsSubType(cons) )
	{
		tf = arg2.Follow(hd).GetType();
		const feature *f = feature::Search(tf);
		if( ! ( arg1 = arg1.Follow(f) ).IsValid() )
		{
			return false;
		}
		arg2 = arg2.Follow(tl);
	}
	tf = arg2.GetType();
	if( ! tf->IsSubType(nil) )
	{
		const feature *f = feature::Search(tf);
		if( ! ( arg1 = arg1.Follow(f) ).IsValid() )
		{
			return false;
		}
	}
          return arg3.Unify(arg1);
}

LILFES_BUILTIN_PRED_3(follow, follow);

/**
  * @predicate	= follow_if_exists(+S1, +L, ?S2) 
  * @desc	= Follow the path <i>L</i> of the feature structure <i>S1</i> and return the result in <i>S2</i> .However, if <i>F1</i> does not exist in path then return failure. 
  * @param	= +S1/ bot : Feature Structure
  * @param	= +L:/ feature or list : Feature and list of paths of features
  * @param	= ?S2/ bot  : Returned Feature Result. 
  * @note	= +L should contain atleast the parents in the hierarchy or children with features. In both the cases, \ is required after Feature name. 
  * @example	= 
  > ?- follow_if_exists([a|_], [tl\, hd\], X).
  no 
  * @end_example

  * @jdesc	= 素性構造<i>S1</i>のパス<i>L</i>をたどった結果を取得して<i>S2</i>として返します。ただし<i>F1</i>にパスLがない場合は失敗します。 
  * @jparam	= +S1/ bot : 素性構造
  * @jparam	= +L/ feature or list : 素性のパスを表すリスト (パスの長さが1の場合、直接素性を記述してもかまいません) 
  * @jparam	= ?S2/ bot : 結果の素性構造 
  * @jnote	= +L は、最も上の階層の素性を指定する他に、下の階層の素性（素性の型の持つ素性など）のパスをリストとして指定することができます。いずれの場合も、素性名には \が必要です。 
  * @jexample	= 
  > ?- follow_if_exists([a|_], [tl\, hd\], X).
  no 
  * @end_jexample
*/
bool follow_if_exists( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
	const type *tf;

	while( arg2.GetType()->IsSubType(cons) )
	{
		tf = arg2.Follow(hd).GetType();
		const feature *f = feature::Search(tf);
		if( arg1.IsLeaf() || arg1.GetType()->GetIndex(f) == -1 || ! ( arg1 = arg1.Follow(f) ).IsValid() )
		{
			return false;
		}
		arg2 = arg2.Follow(tl);
	}
	tf = arg2.GetType();
	if( ! tf->IsSubType(nil) )
	{
		const feature *f = feature::Search(tf);
		if( arg1.IsLeaf() || arg1.GetType()->GetIndex(f) == -1 || ! ( arg1 = arg1.Follow(f) ).IsValid() )
		{
			return false;
		}
	}
  return arg3.Unify(arg1);
}

LILFES_BUILTIN_PRED_3(follow_if_exists, follow_if_exists);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= restriction($S, +F) 
  * @desc	= Clear specified feature( modify it to the most generistic type).
  * @param	= $S/ bot ：Feature structure
  * @param	= +L/ feature or list : Feature and list of paths of features
  * @example	= 
  > ?- X = NAME\"John" & AGE\10 & FATHER\(NAME\"Tom" & AGE\40), restriction(X,FATHER\). 
     |~person       ~|
  X: | NAME:"John"   |
     | AGE:10        |
     |_FATHER:person_|
  * @end_example

  * @jdesc	= 素性構造の指定された素性を消去（最汎の型に変更）します。 
  * @jparam	= $S/ bot ：素性構造
  * @jparam	= +L/ feature or list : 素性のパスを表すリスト (パスの長さが1の場合、直接素性を記述してもかまいません)

  * @jexample	=
  > ?- X = NAME\"John" & AGE\10 & FATHER\(NAME\"Tom" & AGE\40), restriction(X,FATHER\). 
     |~person       ~|
  X: | NAME:"John"   |
     | AGE:10        |
     |_FATHER:person_|
  * @end_jexample
*/
bool restriction( machine& m, FSP arg1, FSP arg2 )
{
	const type *tf;

	while( arg2.GetType()->IsSubType(cons) && ! arg2.Follow(tl).GetType()->IsSubType(nil) )
	{
		if( arg1.IsLeaf() )
			return true;
		tf = arg2.Follow(hd).GetType();
		if( ! tf->IsSubType(t_feature) || tf == t_feature )
		{
			RUNWARN("Warning: Predicate restriction/2 requires feature in the 2nd argument");
			return false;
		}
		else
		{
			const feature *f = feature::Search(tf);
			arg1 = arg1.Follow(f);
                        if ( ! arg1.IsValid() ) return true;
		}
		arg2 = arg2.Follow(tl);
	}
	if( arg2.GetType()->IsSubType(cons) )
		arg2 = arg2.Follow(hd);

	if( arg1.IsLeaf() )
		return true;

	tf = arg2.GetType();
	if( ! tf->IsSubType(t_feature) || tf == t_feature )
	{
		RUNWARN("Warning: Predicate restriction/2 requires feature in the 2nd argument");
		return false;
	}
	else
	{
		const feature *f = feature::Search(tf);
                if ( ! arg1.CheckFeature( f ) ) return true;

		core_p newaddr = m.GetHP();
		core_p origaddr = arg1.Deref().GetAddr();
		const type *t = arg1.GetType();
		int nf = t->GetNFeatures();

		m.CheckHeapLimit(newaddr + nf + 1);
		m.PutHeap(m.ReadHeap(origaddr));
		for( int i=0; i<nf; i++ )
		{
			if( t->Feature(i) == f )
				m.PutHeap(VAR2c(t->GetAppType(i)));
			else
				m.PutHeap(PTR2c(origaddr+i+1));
		}
		m.WriteHeap( origaddr, PTR2c(newaddr) );
          return true;
	}
}

LILFES_BUILTIN_PRED_2(restriction, restriction);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= overwrite($S, +F, $T) 
  * @desc	= Overwrite specified feature (modify it to the new value).
  * @param	= $S/ bot ：Feature structure
  * @param	= +L/ feature or list : Feature and list of paths of features
  * @param      = $T/ bot : Feature structure
  * @example	= 
  > ?- X = NAME\"John" & AGE\10 & FATHER\(NAME\"Tom" & AGE\40), overwrite(X,[FATHER\,NAME\],"Bob"). 
     |~person               ~|
     | NAME:"John"           |
  X: | AGE:10                |
     |        |~person    ~| |
     | FATHER:| NAME:"Bob" | |
     |_       |_AGE:40    _|_|
  * @end_example

  * @jdesc	= 素性構造の指定された素性を上書きします。 
  * @jparam	= $S/ bot ：素性構造
  * @jparam	= +L/ feature or list : 素性のパスを表すリスト (パスの長さが1の場合、直接素性を記述してもかまいません)
  * @jparam     = $T/ bot : 素性構造

  * @jexample	=
  > ?- X = NAME\"John" & AGE\10 & FATHER\(NAME\"Tom" & AGE\40), restriction(X,FATHER\). 
     |~person               ~|
     | NAME:"John"           |
  X: | AGE:10                |
     |        |~person    ~| |
     | FATHER:| NAME:"Bob" | |
     |_       |_AGE:40    _|_|
  * @end_jexample
*/
bool overwrite( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	const type *tf;

	while( arg2.GetType()->IsSubType(cons) && ! arg2.Follow(tl).GetType()->IsSubType(nil) )
	{
		if( arg1.IsLeaf() )
			return false;
		tf = arg2.Follow(hd).GetType();
		if( ! tf->IsSubType(t_feature) || tf == t_feature )
		{
			RUNWARN("Warning: Predicate overwrite/3 requires feature in the 2nd argument");
			return false;
		}
		else
		{
			const feature *f = feature::Search(tf);
			arg1 = arg1.Follow(f);
                        if ( ! arg1.IsValid() ) return false;
		}
		arg2 = arg2.Follow(tl);
	}
	if( arg2.GetType()->IsSubType(cons) )
		arg2 = arg2.Follow(hd);

	if( arg1.IsLeaf() )
		return false;

	tf = arg2.GetType();
	if( ! tf->IsSubType(t_feature) || tf == t_feature )
	{
		RUNWARN("Warning: Predicate overwrite/3 requires feature in the 2nd argument");
		return false;
	}

	const feature *f = feature::Search(tf);
        if ( ! arg1.CheckFeature( f ) ) return false;

	core_p newaddr = m.GetHP();
	core_p origaddr = arg1.Deref().GetAddr();
	const type *t = arg1.GetType();
	int nf = t->GetNFeatures();

	m.CheckHeapLimit(newaddr + nf + 1);
	m.PutHeap(m.ReadHeap(origaddr));
	for( int i=0; i<nf; i++ )
	  {
	    if( t->Feature(i) == f )
	      m.PutHeap(VAR2c(t->GetAppType(i)));
	    else
	      m.PutHeap(PTR2c(origaddr+i+1));
	  }
	m.WriteHeap( origaddr, PTR2c(newaddr) );

        arg1 = arg1.Follow( f );
        if ( ! arg1.IsValid() ) {
          return false;
        }
        return arg1.Unify( arg3 );
}

LILFES_BUILTIN_PRED_3(overwrite, overwrite);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= generalize($S, +F, $T)
  * @desc	= Generalize specified feature (modify it to a general type)
  * @param	= $S/ bot ：Feature structure
  * @param	= +L/ feature or list : Feature and list of paths of features
  * @param      = $T/ bot : Feature structure

  * @jdesc	= 素性構造の指定された素性を一般化します。
  * @jparam	= $S/ bot ：素性構造
  * @jparam	= +L/ feature or list : 素性のパスを表すリスト (パスの長さが1の場合、直接素性を記述してもかまいません)
  * @jparam     = $T/ bot : 素性構造
*/
bool generalize( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	const type *tf;

	while( arg2.GetType()->IsSubType(cons) && ! arg2.Follow(tl).GetType()->IsSubType(nil) )
	{
		if( arg1.IsLeaf() )
			return false;
		tf = arg2.Follow(hd).GetType();
		if( ! tf->IsSubType(t_feature) || tf == t_feature )
		{
			RUNWARN("Warning: Predicate generalize/3 requires feature in the 2nd argument");
			return false;
		}
		else
		{
			const feature *f = feature::Search(tf);
			arg1 = arg1.Follow(f);
                        if ( ! arg1.IsValid() ) return false;
		}
		arg2 = arg2.Follow(tl);
	}
	if( arg2.GetType()->IsSubType(cons) )
		arg2 = arg2.Follow(hd);

	if( arg1.IsLeaf() )
		return false;

	tf = arg2.GetType();
	if( ! tf->IsSubType(t_feature) || tf == t_feature )
	{
		RUNWARN("Warning: Predicate generalize/3 requires feature in the 2nd argument");
		return false;
	}

	const feature *f = feature::Search(tf);
        if ( ! arg1.CheckFeature( f ) ) return false;

	core_p newaddr = m.GetHP();
	core_p origaddr = arg1.Deref().GetAddr();
	const type *t = arg1.GetType();
	int nf = t->GetNFeatures();

	m.CheckHeapLimit(newaddr + nf + 1);
	m.PutHeap(m.ReadHeap(origaddr));
	for( int i=0; i<nf; i++ )
	  {
	    if( t->Feature(i) == f )
	      m.PutHeap(VAR2c(t->GetAppType(i)));
	    else
	      m.PutHeap(PTR2c(origaddr+i+1));
	  }
        FSP old_fs = arg1.Follow( f );
	m.WriteHeap( origaddr, PTR2c(newaddr) );
        FSP new_fs = arg1.Follow( f );
	if ( ! new_fs.Unify( arg3 ) ) return false;
        const type* new_type = arg3.GetType();
        for ( int i = 0; i < new_type->GetNFeatures(); i++ ) {
          const feature* f = new_type->Feature( i );
          FSP old_val = old_fs.Follow( f );
          if ( ! old_val.IsValid() ) return false;
          if ( ! new_fs.Follow( f ).Unify( old_val ) ) return false;
        }
        return true;
}

LILFES_BUILTIN_PRED_3(generalize, generalize);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= subnodelist(+S, -L) 
  * @desc	= returns the list of all children (including grand children) nodes as a list. 
  * @param	= +S/ bot : Feature structure
  * @param	= -L/ list : Result list  
  * @example	= 
  > ?- subnodelist(person & Name\"Mary",X)
       |~person            ~|                                          
       | NAME:[$1] "Jack"   |                                          
  X: < | AGE:[$2] integer   |, [$1] ..., [$2] ..., [$3] ..., [$4] ... >
       | FATHER:[$3] person |                                          
       |_MOTHER:[$4] person_|   
  * @end_example

  * @jdesc	= 素性構造の子ノードの一覧をリストとして得ます。直接の子ノードだけでなく、さらに下のノードも一列にリストにします。 
  * @jparam	= +S/ bot : 素性構造
  * @jparam	= -L/ list : 結果のリスト 
  * @jexample	=
  > ?- subnodelist(person & Name\"Mary",X)
       |~person            ~|                                          
       | NAME:[$1] "Jack"   |                                          
  X: < | AGE:[$2] integer   |, [$1] ..., [$2] ..., [$3] ..., [$4] ... >
       | FATHER:[$3] person |                                          
       |_MOTHER:[$4] person_|   
  * @end_jexample
*/
bool subnodelist( machine& m, FSP arg1, FSP arg2 )
{
	ShareList sl;
	
	sl.AddMark(arg1);

	FSP f(m), fr = f;
	for( uint i=0; i<sl.MarkArray().size(); i++ )
	{
		f.Follow(hd).Unify(FSP( m, (core_p)((sl.MarkArray())[i].addr) ));
		f = f.Follow(tl);
	}
	f.Coerce(nil);
	
  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(subnodelist, subnodelist);

/**
  * @predicate	= subnodelist_count(+S, -L) 
  * @desc	= returns the count of nodes in subnodelist. 
  * @param	= +S/ bot : Feature structure
  * @param	= -L/ list : Result list  
  * @example	= 
  * @end_example

  * @jdesc	= subnodelist の出力の要素に、辿った回数を付加したリストを返します。 
  * @jparam	= +S/ bot : 素性構造
  * @jparam	= -L/ list : 結果のリスト 
  * @jexample	=
  * @end_jexample
*/
bool subnodelist_count( machine& m, FSP arg1, FSP arg2 )
{
	ShareList sl;
	
	sl.AddMark(arg1);

	FSP f(m), fr = f;
	for( uint i=0; i<sl.MarkArray().size(); i++ )
	{
		FSP g = f.Follow(hd);
		g.Follow(hd).Unify(FSP( m, (sl.MarkArray())[i].addr ));
		g = g.Follow(tl);
		g.Follow(hd).Unify(FSP( m, (mint)((sl.MarkArray())[i].ref) ));
		g.Follow(tl).Coerce(nil);
		f = f.Follow(tl);
	}
	f.Coerce(nil);
	
  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(subnodelist_count, subnodelist_count);

//////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= sharednodelist(+S, -L) 
  * @desc	= Returns a list of shared nodes. 
  * @param	= +S/ bot : Feature structure
  * @param	= -L/ list : Result list  
  * @example	= 
  > ?- A = ( p(X,Y) :- q(X,Z), r(Z,Y) ), sharednodelist(A, B).
     |~:-/2                                          ~|
     |       |~p/2          ~|                        |
     | chead:| arg1:[$1] bot |                        |
  A: |       |_arg2:[$2] bot_|                        |
     |         |~q/2          ~|  |~r/2          ~|   |
     | cbody:< | arg1:[$1] ... |, | arg1:[$3] ... | > |
     |_        |_arg2:[$3] bot_|  |_arg2:[$2] ..._|  _|
  B: < [$1] ..., [$2] ..., [$3] ... >
  　
  This can be used for getting list of variables as this example shows.But variables of singleton is not added list because structure sharing does not occur.
  * @end_example

  * @jdesc	= 素性構造のうち、構造共有しているノードをリストとして得ます。 
  * @jparam	= +S/ bot : 素性構造
  * @jparam	= -L/ list : 結果のリスト 
  * @jexample	=
  > ?- A = ( p(X,Y) :- q(X,Z), r(Z,Y) ), sharednodelist(A, B).
     |~:-/2                                          ~|
     |       |~p/2          ~|                        |
     | chead:| arg1:[$1] bot |                        |
  A: |       |_arg2:[$2] bot_|                        |
     |         |~q/2          ~|  |~r/2          ~|   |
     | cbody:< | arg1:[$1] ... |, | arg1:[$3] ... | > |
     |_        |_arg2:[$3] bot_|  |_arg2:[$2] ..._|  _|
  B: < [$1] ..., [$2] ..., [$3] ... >
  　
  上の例のように、変数リストの取得に使うことができます。但し、Singletonの変数は、構造共有が置きないため、リストに加えられることがないことに注意して下さい。 
  * @end_jexample
*/
bool sharednodelist( machine& m, FSP arg1, FSP arg2 )
{
//	cout << "SharedNodeList start" << endl;

	ShareList sl;
	
	sl.AddMark(arg1);

	FSP f(m), fr = f;
	for( uint i=0; i<sl.MarkArray().size(); i++ )
	if( (sl.MarkArray())[i].ref >= 2 )
		{
			f.Follow(hd).Unify(FSP( m, (core_p)((sl.MarkArray())[i].addr) ));
			f = f.Follow(tl);
		}
	f.Coerce(nil);
	
  return arg2.Unify(fr);

//	cout << "SharedNodeList finish" << endl;
}

LILFES_BUILTIN_PRED_2(sharednodelist,sharednodelist);

/**
  * @predicate	= contains(+S1, +S2) 
  * @desc	= Sucess if the feature structure1 contains the feature structure2. 
  * @param	= +S1/ bot  : Feature structure１
  * @param	= +S2/ bot  : Feature structure２
  * @example	= 
  > ?- N = "John", X = NAME\N & FATHER\NAME\N , contains(X,N).
  (if successful, return values of X and N) 
  > ?- X = NAME\"John" & FATHER\NAME\"John" , contains(X,N).
  no 
  　
  As can be seen from the example, although the values are same, since they are not shared, it fails. 
  Can be used to find out if S1 contains S2. 
  * @end_example

  * @jdesc	= 素性構造S1の中でS2を構造共有していれば成功します。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jexample	=
  > ?- N = "John", X = NAME\N & FATHER\NAME\N , contains(X,N).
  （成功して、XとNの値が返る） 
  > ?- X = NAME\"John" & FATHER\NAME\"John" , contains(X,N).
  no 
  　
  例のように、値が同じでも構造共有していなければ失敗します。
  S1の中に変数S2があるかどうかを調べるのに使うことができます。 
  * @end_jexample
*/
bool contains( machine&, FSP arg1, FSP arg2 )
{
	ShareList sl;
	
	sl.AddMark(arg1);
	
	arg2 = arg2.Deref();
	core_p cp2 = arg2.GetAddr();
	bool ret = false;

	for( uint i=0; i<sl.MarkArray().size(); i++ )
	{
		core_p cp = (core_p)((sl.MarkArray())[i].addr);
		if( cp == cp2 )
		{
			i = sl.MarkArray().size();	// succeed
			ret = true;
		}
	}

  return ret;
}

LILFES_BUILTIN_PRED_2(contains, contains);

/**
  * @predicate	= is_cyclic(+S) 
  * @desc	= Sucess if the feature structure is cyclic.
  * @param	= +S/ bot  : Feature structure
  * @jdesc	= 素性構造Sがサイクルをもっていれば成功します。
  * @jparam	= +S/ bot : 素性構造
*/

typedef _HASHMAP< core_p, bool, _HASHMAP_NS::hash<core_p> > core_p_hash;

bool __is_cyclic(machine& m, core_p addr, vector<core_p>& stack, core_p_hash& mark) {
    addr = m.Deref(addr);
    if(mark.find(addr) != mark.end()) {
        vector<core_p>::iterator it = stack.begin(), last = stack.end();
        for(; it != last ; it++)
            if((*it) == addr) return true;
        return false;
    }
    mark.insert(core_p_hash::value_type(core_p_hash::key_type(addr),
                                        core_p_hash::mapped_type(true)));
    cell c = m.ReadHeap(addr);
    if(!IsSTR(c)) return false;
    
    stack.push_back(addr);
    int nf = c2STR(c)->GetNFeatures();
    for(int i = 0 ; i < nf ; i++)
        if( __is_cyclic(m, addr + i + 1, stack, mark) ) return true;
    stack.pop_back();
    return false;
}

bool is_cyclic( machine& m, FSP arg)
{
    vector<core_p> stack;
    core_p_hash mark;
    return __is_cyclic(m, arg.GetAddress(), stack, mark);
}

LILFES_BUILTIN_PRED_1(is_cyclic, is_cyclic);

//////////////////////////////////////////////////////////////////////

#define CT(t1,t2) ( ((t1)<<TAG_WIDTH) + (t2) )

static bool equiv_sub_1(machine& m,
                        vector< pair< core_p, core_p > >& pairinfo,
                        core_p addr, const type *p, int nfeat) {
  addr = m.Deref(addr);
  cell c = m.ReadHeap(addr);
  const type* apptp = p->GetAppType(nfeat);
  for ( vector< pair< core_p, core_p > >::const_iterator it = pairinfo.begin();
        it != pairinfo.end();
        ++it ) {
    if ( it->first == addr ) {
      return false;
    }
  }
  pairinfo.push_back( make_pair( addr, CORE_P_INVALID ) );
  switch(Tag(c)) {
    case T_STR:
    {
      if (apptp != c2STR(c))
	return false;
      int featnum = apptp->GetNFeatures();
      for (int i = 0; i < featnum; i++)
	if (!equiv_sub_1(m, pairinfo, addr + i + 1, apptp, i))
	  return false;
    }
    break;
    case T_VAR:
    {
      if (apptp != c2VAR(c))
	return false;
    }
    break;
    case T_INT:
    case T_FLO:
    case T_CHR:
    case T_STG:
    {
      return false;
    }
    break;
    default:
    {
      ABORT("UNKNOWN TAG!!");
    }
    break;
  }
  return true;
}

static bool equiv_sub_2(machine& m,
                        vector< pair< core_p, core_p > >& pairinfo,
                        core_p addr, const type *p, int nfeat) {
  addr = m.Deref(addr);
  cell c = m.ReadHeap(addr);
  const type* apptp = p->GetAppType(nfeat);
  for ( vector< pair< core_p, core_p > >::const_iterator it = pairinfo.begin();
        it != pairinfo.end();
        ++it ) {
    if (it->second == addr) {
      return false;
    }
  }
  pairinfo.push_back( make_pair( CORE_P_INVALID, addr ) );
  switch(Tag(c)) {
    case T_STR:
    {
      if (apptp != c2STR(c))
	return false;
      int featnum = apptp->GetNFeatures();
      for (int i = 0; i < featnum; i++)
	if (!equiv_sub_2(m, pairinfo, addr + i + 1, apptp, i))
	  return false;
    }
    break;
    case T_VAR:
    {
      if (apptp != c2VAR(c))
	return false;
    }
    break;
    case T_INT:
    case T_FLO:
    case T_CHR:
    case T_STG:
    {
      return false;
    }
    break;
    default:
    {
      ABORT("UNKNOWN TAG!!");
    }
    break;
  }
  return true;
}

static bool equiv_main(machine& m,
                       vector< pair< core_p, core_p > >& pairinfo,
                       core_p addr1, core_p addr2) {
  addr1 = m.Deref(addr1);
  addr2 = m.Deref(addr2);
//
// Structure Sharings Check
//

  for ( vector< pair< core_p, core_p > >::const_iterator it = pairinfo.begin();
        it != pairinfo.end();
        ++it ) {
    if ( it->first == addr1 ) {
      if ( it->second == addr2 ) { // The same node
	return true;
      } else {
        return false;
      }
    } else {
      if ( it->second == addr2 ) {
	return false;
      }
    }
  }
  pairinfo.push_back( make_pair( addr1, addr2 ) );
//
// Cell Check
//
  cell c1 = m.ReadHeap(addr1);
  cell c2 = m.ReadHeap(addr2);
  switch(CT(Tag(c1), Tag(c2)))
    {
    case CT(T_VAR, T_VAR):
    case CT(T_INT, T_INT):
    case CT(T_FLO, T_FLO):
    case CT(T_CHR, T_CHR):
    case CT(T_STG, T_STG):
      {
	if (c1 != c2)
	  return false;
      } 
      break;
    case CT(T_VAR, T_STR):
      {
	if (c2VARS(c1) != c2STRS(c2))
	  return false;
	const type* tp = c2STR(c2);
	int featnum = tp->GetNFeatures();
	for (int i = 0; i < featnum; i++)
	  if (!equiv_sub_2(m, pairinfo, addr2+i+1, tp, i))
	    return false;
      }
      break;
    case CT(T_STR, T_VAR):
      {
	if (c2STRS(c1) != c2VARS(c2))
	  return false;
	const type* tp = c2STR(c1);
	int featnum = tp->GetNFeatures();
	for (int i = 0; i < featnum; i++)
	  if (!equiv_sub_1(m, pairinfo, addr1+i+1, tp, i))
	    return false;
      }
      break;
    case CT(T_STR, T_STR):
      {
	if (c1 != c2)
	  return false;
	int featnum = c2STR(c1)->GetNFeatures();
	for (int i = 0; i < featnum; i++)
	  if (!equiv_main(m, pairinfo, addr1+i+1, addr2+i+1)) // Traverse
	    return false;
      }
      break;
    default:
      {
	return false;
      }
  };

  return true;
}

/**
  * @predicate	= equivalent(+S1, +S2) 
  * @desc	= Sucess if feature structure1 and feature structure2 are equivalent. 
  * @param	= +S1/ bot : Feature structure１
  * @param	= +S2/ bot : Feature structure２ 
  * @example	= 
  > ?- equivalent(person & AGE\10, person & NAME\"Mary").
  no
  * @end_example

  * @jdesc	= ２つの素性構造が完全に等しい場合に成功します。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jexample	= 
  > ?- equivalent(person & AGE\10, person & NAME\"Mary").
  no
  * @end_jexample
*/
bool equivalent( machine& m, FSP arg1, FSP arg2 ) {
  vector< pair< core_p, core_p > > pairinfo;
  return equiv_main(m, pairinfo, arg1.GetAddress(), arg2.GetAddress());
}

LILFES_BUILTIN_PRED_2(equivalent,equivalent);

/**
  * @predicate = not_equivalent(+$FS1, +$FS2)
  * @param     = +$FS1/bot
  * @param     = +$FS2/bot
  * This is true when $FS1 and $FS2 are not equivalent.
*/
bool not_equivalent( machine& m, FSP arg1, FSP arg2 ) {
  vector< pair< core_p, core_p > > pairinfo;
  return ! equiv_main(m, pairinfo, arg1.GetAddress(), arg2.GetAddress());
}

LILFES_BUILTIN_PRED_2(not_equivalent, not_equivalent);

//////////////////////////////////////////////////////////////////////

static bool subsume_main(machine& m,
                         vector< pair< core_p, core_p > >& pairinfo,
                         core_p addr1, core_p addr2) {
  addr1 = m.Deref(addr1);
  addr2 = m.Deref(addr2);
  for ( vector< pair< core_p, core_p > >::const_iterator it = pairinfo.begin();
        it != pairinfo.end();
        ++it ) {
    if ( it->first == addr1 ) {
      if ( it->second == addr2 ) {
        return true;
      } else {
        return false;
      }
    }
  }
  pairinfo.push_back( make_pair( addr1, addr2 ) );

//
// Cell Check
//
  cell c1 = m.ReadHeap(addr1);
  cell c2 = m.ReadHeap(addr2);
  switch(CT(Tag(c1), Tag(c2)))
    {
      case CT(T_INT, T_INT):
      case CT(T_FLO, T_FLO):
      case CT(T_CHR, T_CHR):
      case CT(T_STG, T_STG):
      {
	if (c1 != c2)
	  return false;
      }
      break;
      case CT(T_VAR, T_VAR):
      {
	if (c1 != c2) {
	  const type* tp1 = c2VAR(c1);
	  const type* tp2 = c2VAR(c2);
	  if (!(tp1->IsSuperType(tp2)))
	    return false;  
	}
      }
      break;
      case CT(T_VAR, T_INT):
      {
	if (c1 != VAR2c(bot) && c1 != VAR2c(t_int))
	  return false;
      }
      break;
      case CT(T_VAR, T_FLO):
      {
	if (c1 != VAR2c(bot) && c1 != VAR2c(t_flo))
	  return false;
      }
      break;
      case CT(T_VAR, T_CHR):
      {
	if (c1 != VAR2c(bot) && c1 != VAR2c(t_chr))
	  return false;
      }
      break;
      case CT(T_VAR, T_STG):
      {
	if (c1 != VAR2c(bot) && c1 != VAR2c(t_stg))
	  return false;
      }
      break;
      case CT(T_VAR, T_STR):
      {
	const type* tp1 = c2VAR(c1);
	const type* tp2 = c2STR(c2);
	if (!(tp1->IsSuperType(tp2)))
	  return false;
      }
      break;
      case CT(T_STR, T_VAR):
      {
	const type* tp1 = c2STR(c1);
	const type* tp2 = c2VAR(c2);
	if (!(tp1->IsSuperType(tp2)))
	  return false;	
	int featnum = tp1->GetNFeatures();
	for (int i = 0; i < featnum; i++)
	  if (!equiv_sub_1(m, pairinfo, addr1+i+1, tp1, i))
	    return false;
      }
      break;
      case CT(T_STR, T_STR):
      {
	const type* tp1 = c2STR(c1);
	const type* tp2 = c2STR(c2);
	if ((c1 != c2) && !(tp1->IsSuperType(tp2)))
	  return false;
	int featnum = tp1->GetNFeatures();
	for (int i = 0; i < featnum; i++) {
	  if (!subsume_main(m, pairinfo, addr1+i+1, addr2+tp2->GetIndex(tp1->Feature(i))+1))
	    return false;
	}
      }
      break;
      default:
      {
	return false;
      }
    };
  return true;
}

/**
  * @predicate	= subsume(+S1, +S2) 
  * @desc	= Sucess if feature structure1 is a subsume of feature structure2.
  * @param	= +S1/ bot : Feature structure１
  * @param	= +S2/ bot : Feature structure２ 
  * @note	= If type_subsume is different, it checks the internals of the feature. 
  * @example	= 
  > ?- subsume(person & AGE\10, person)
  no
  > ?- subsume(person, person & AGE\10)
  yes
  * @end_example

  * @jdesc	= 素性構造S1が素性構造S2を包摂するなら成功。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jnote	= type_subsumeとの違いは、素性の中まで調べることです。 
  * @jexample	= 
  > ?- subsume(person & AGE\10, person)
  no
  > ?- subsume(person, person & AGE\10)
  yes
  * @end_jexample
*/
bool subsume( machine& m, FSP arg1, FSP arg2 ) {
  vector< pair< core_p, core_p > > pairinfo;
  return subsume_main(m, pairinfo, arg1.GetAddress(), arg2.GetAddress());
}

LILFES_BUILTIN_PRED_2(subsume,subsume);

//////////////////////////////////////////////////////////////////////

/**
  * @predicate	= type_equal(+FS1, +FS2) 
  * @desc	= Success if the type of both feature structure are the same. 
  * @param	= +FS1/ bot  : Feature structure１
  * @param	= +FS2/ bot  : Feature structure２
  * @example	= 
  > ?- type_equal(foo & F1\a, foo & F1\b).
  yes 
  * @end_example

  * @jdesc	= ２つの素性構造のうち、型の部分が等しいなら成功します。 
  * @jparam	= +S1/ bot : 素性構造１
  * @jparam	= +S2/ bot : 素性構造２ 
  * @jexample	=
  > ?- type_equal(foo & F1\a, foo & F1\b).
  yes 
  * @end_jexample
*/
bool type_equal( machine&, FSP arg1, FSP arg2 )
{
  if( (arg1.GetTop().ReadCell() != arg2.GetTop().ReadCell()) )
    return false;
  else
    return true;
}

LILFES_BUILTIN_PRED_2(type_equal, type_equal);

/**
  * @predicate	= type_subsume(+TYPE1, +TYPE2) 
  * @desc	= Success if the TYPE2 is included into the TYPE1. 
  * @param	= +TYPE1/ bot  : Type(Feature structure)１
  * @param	= +TYPE2/ bot  : Type(Feature structure)２
  * @example	= 
  > ?- type_subsume(list, nil).
  yes
  > ?- type_subsume(nil, list).
  no 
  * @end_example

  * @jdesc	= TYPE1がTYPE2を包摂する（TYPE1はTYPE2を汎化したもの）なら成功。 
  * @jparam	= +TYPE1/ bot : 型（素性構造）１
  * @jparam	= +TYPE2/ bot : 型（素性構造）２
  * @jexample	=
  > ?- type_subsume(list, nil).
  yes
  > ?- type_subsume(nil, list).
  no 
  * @end_jexample
*/
bool type_subsume( machine&, FSP arg1, FSP arg2 )
{
	FSP f = arg1.GetTop();
	FSP g = arg2.GetTop();
	cell c = g.ReadCell();
	if( f.Unify(g) == false )
          return false;
	else
	{
          if( f.ReadCell() != c )
            return false;
          else
            return true;
	}
}

LILFES_BUILTIN_PRED_2(type_subsume, type_subsume);

/**
  * @predicate	= type_toptype(+FS, -TYPE) 
  * @desc	= Generates just the types of a feature structure. 
  * @param	= +FS/ bot  : Feature structure
  * @param	= -TYPE/ bot  : Feature structure type 
  * @example	= 
  > ?- type_toptype(Racing Horse & Lineage\'Hail to Reason',X).
  X: Racing Horse
  > ?- type_toptype(p(a,b,c),X).
  X: p/3 
  * @end_example

  * @jdesc	= 素性構造のうち、型だけを取り出します。 
  * @jparam	= +FS/ bot : 素性構造
  * @jparam	= -TYPE/ bot : 素性構造の型 
  * @jexample	=
  > ?- type_toptype(競走馬 & 系統\'Hail to Reason',X).
  X: 競走馬
  > ?- type_toptype(p(a,b,c),X).
  X: p/3 
  * @end_jexample
*/
bool type_toptype( machine&, FSP arg1, FSP arg2 )
{
  return arg2.Unify(arg1.GetTop());
}

LILFES_BUILTIN_PRED_2(type_toptype, type_toptype);

/**
  * @predicate = type_copy(+$FS, -$TYPE)
  * @param     = +$FS/bot
  * @param     = -$TYPE/bot
  * @desc      = copying of a type
  * This lets the value of $TYPE be the type of $FS.
  * @jparam     = +$FS/bot
  * @jparam     = -$TYPE/bot
  * @jdesc      = 型のコピー
  * $FS の型を $TYPE にコピーします．
*/
bool type_copy( machine&, FSP arg1, FSP arg2 )
{
  return arg2.Unify(arg1.GetTop());
}

LILFES_BUILTIN_PRED_2(type_copy, type_copy);

/**
  * @predicate	= type_extendable(+TYPE) 
  * @desc	= Success if TYPE is expendable(i.e.subtype of TYPE can be defined in other modules). 
  * @param	= +FS/ bot  : Type(Feature structure)
  * @example	= 
  * @end_example

  * @jdesc	= 他のモジュールでその型のsubtypeが定義できれば成功します．
  * @jparam	= +FS/ bot : 型の素性構造
  * @jexample	=
  * @end_jexample
*/
bool type_extendable( machine&, FSP arg1 )
{
  return arg1.GetType()->IsExtendable();
}

LILFES_BUILTIN_PRED_1(type_extendable, type_extendable);

/**
  * @predicate	= type_unify(+TYPE1, +TYPE2, -RESULT) 
  * @desc	= Return the simplification TYPE1 and TYPE2 in the RESULT . 
  * @param	= +TYPE1/ bot  : Type(Feature structure)１
  * @param	= +TYPE2/ bot  : Type(Feature structure)２
  * @param	= -RESULT/ bot   : Just the type of the simplification result. 
  * @example	= 
  > ?- type_unify(Rectangle, Rhombus, X).
  X: Square 
  * @end_example

  * @jdesc	= TYPE1とTYPE2を単一化したものを RESULTに返します。 
  * @jparam	= +TYPE1/ bot : 型（素性構造）１
  * @jparam	= +TYPE2/ bot : 型（素性構造）２
  * @jparam	= -RESULT/ bot : 型のみの単一化の結果 
  * @jexample	=
  > ?- type_unify(長方形, 菱形, X).
  X: 正方形 
  * @end_jexample
*/
bool type_unify( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
  if( arg3.Unify(arg1.GetTop()) == false 
      || arg3.Unify(arg2.GetTop()) == false )
    return false;
  else
    return true;
}

LILFES_BUILTIN_PRED_3(type_unify, type_unify);

template <class T> inline T tmin(T a, T b) { return a < b ? a : b; }
template <class T> inline T tmax(T a, T b) { return a > b ? a : b; }

/**
  * @predicate	= type_common(+FS1, +FS2, +FS3) 
  * @desc	= Success if FS1 and FS2 and FS3 ars the same type.
  * @param	= +FS1/ bot  : Feature structure１
  * @param	= +FS2/ bot  : Feature structure２
  * @param	= +FS2/ bot  : Feature structure３
  * @example	= 
  * @end_example

  * @jdesc	= FS1とFS2とFS3が同じ型であれば成功します． 
  * @jparam	= +FS1/ bot : 素性構造１
  * @jparam	= +FS2/ bot : 素性構造２
  * @jparam	= +FS2/ bot : 素性構造３
  * @jexample	=
  * @end_jexample
*/
bool type_common( machine&, FSP arg1, FSP arg2, FSP arg3 )
{
	if( arg1.IsInteger() && arg2.IsInteger() )
	{
		if( arg1.ReadInteger() == arg2.ReadInteger() )
		{
			if( arg3.Unify(arg1) == false )
                          return false;
		}
                else{
			if( arg3.Coerce(t_int) == false )
                          return false;
                }
		return true;
	}
	else if( arg1.IsString() && arg2.IsString() )
	{
		if( arg1.ReadString() == arg2.ReadString() )
		{
			if( arg3.Unify(arg1) == false )
                          return false;
		}
		else
			if( arg3.Coerce(t_stg) == false )
                          return false;
		return true;
	}
	else if( arg1.IsFloat() && arg2.IsFloat() )
	{
		if( arg1.ReadFloat() == arg2.ReadFloat() )
		{
			if( arg3.Unify(arg1) == false )
                          return false;
		}
		else
			if( arg3.Coerce(t_flo) == false )
                          return false;
		return true;
	}

	const type *t1 = arg1.GetType();
	const type *t2 = arg2.GetType();
	int t1s = t1->GetSerialNo();
	int t2s = t2->GetSerialNo();

	int i = tmin(t1s, t2s);
	for(;;)
	{
		ASSERT( i >= 0 );
		if( coerceData(i,t1s).result_s == t1s && coerceData(i,t2s).result_s == t2s )
			break;
		i--;
	}

  if( arg3.Coerce(type::Serial(i)) == false )
    return false;
  else
    return true;
}

LILFES_BUILTIN_PRED_3(type_common, type_common);

/**
  * @predicate	= type_nfeature(+TYPE, -NUM) 
  * @desc	= Return in how many types exists of the TYPE parameter. 
  * @param	= +TYPE/ bot : Type(Feature structure)
  * @param	= -NUM/ integer : Number of features 
  * @example	= 
  > ?- type_nfeature(cons, N).
  N : 2 
  * @end_example

  * @jdesc	= TYPE がいくつ素性を持つかを返します。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -NUM/ integer : 素性の数 
  * @jexample	=
  > ?- type_nfeature(cons, N).
  N : 2 
  * @end_jexample
*/
bool type_nfeature( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	int nf = t->GetNFeatures();
	
  return arg2.Unify(FSP(m, (mint)nf));
}

LILFES_BUILTIN_PRED_2(type_nfeature, type_nfeature);

/**
  * @predicate	= type_featurelist(+TYPE, -LIST) 
  * @desc	= Return a list of features from TYPE 
  * @param	= +TYPE/ bot : Type(Feature structure)
  * @param	= -LIST/ list  : Feature list 
  * @example	= 
  > a <- [bot] + [F\].
  > b <- [a] + [G\].
  > c <- [a] + [H\].
  > d <- [b,c]+ [J\].
  > ?- type_featurelist(d, L).
  L: < F\, G\, H\, J\ > 
  * @end_example

  * @jdesc	= TYPE の持つ素性をリストとして返します。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -LIST/ list : 素性のリスト 
  * @jexample	=
  > a <- [bot] + [F\].
  > b <- [a] + [G\].
  > c <- [a] + [H\].
  > d <- [b,c]+ [J\].
  > ?- type_featurelist(d, L).
  L: < F\, G\, H\, J\ > 
  * @end_jexample
*/
bool type_featurelist( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	FSP f(m), fr = f;
	int nf = t->GetNFeatures();
	for( int i=0; i<nf; i++ )
	{
		f.Follow(hd).Coerce(t->Feature(i)->GetRepType());
		f = f.Follow(tl);
	}
	f.Coerce(nil);
	
  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(type_featurelist, type_featurelist);

/**
  * @predicate	= type_havefeature(+TYPE, +FEATURE) 
  * @desc	= Success is the feature structure in TYPE have the parameter feature. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= +FEATURE/ feature : Feature 
  * @note	= Please add the character \ to the end of the FEATURE. 
  * @example	= 
  > ?- type_havefeature(Fresh, Stud\).
  yes 
  * @end_example

  * @jdesc	= TYPE が 素性として FEATURE を持つなら成功します。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= +FEATURE/ feature : 素性 
  * @jnote	= FEATURE には必ず \ を付けて下さい。 
  * @jexample	=
  > ?- type_havefeature(フラッシュ, スート\).
  yes 
  * @end_jexample
*/
bool type_havefeature( machine&, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();
	const type *tf = arg2.GetType();

	if( t->GetIndex(feature::Search(tf)) == -1 )
          return false;
        else
          return true;
}

LILFES_BUILTIN_PRED_2(type_havefeature, type_havefeature);

/**
  * @predicate	= type_subtypes(+TYPE, -LIST) 
  * @desc	= Get a list of the subtypes of TYPE. The list includes parents and all ancestors. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -LIST/ list  : List pf subtypes 
  * @example	= 
  > ?- type_subtypes(Triangle, L).
  L : < Triangle, Isometric, Right Angle Triangle, Isometric Right Angle Triangle, Equilateral Triangle > 
  > ?- type_subtypes(bot, L).
  L : (all the subtypes are displayed) 
  * @end_example

  * @jdesc	= TYPE のsubtypeの一覧をリストとして得ます。子だけでなく、全ての子孫が含まれます。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -LIST/ list : subtypeのリスト 
  * @jexample	=
  > ?- type_subtypes(三角形, L).
  L : < 三角形, 二等辺三角形, 直角三角形, 直角二等辺三角形, 正三角形 > 
  > ?- type_subtypes(bot, L).
  L : （起動直後に実行すると、全ての組み込み型を表示） 
  * @end_jexample
*/
bool type_subtypes( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	FSP f(m), fr = f;
	int nt = type::GetSerialCount();
	for( int i=0; i<nt; i++ )
	{
		if( type::Serial(i)->IsSubType(t) )
		{
		  
			f.Follow(hd).Coerce(type::Serial(i));
			f = f.Follow(tl);
		}
	}
	f.Coerce(nil);

  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(type_subtypes, type_subtypes);

/**
  * @predicate	= type_nsubtypes(+TYPE, -NUM) 
  * @desc	= Return how many the subtypes TYPE has. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -NUM/ integer  : number of subtype
  * @example	= 
  * @end_example

  * @jdesc	= TYPE にいくつsubtypeがあるかを返します．
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -NUM/ integer : subtypeの数
  * @jexample	=
  * @end_jexample
*/
bool type_nsubtypes( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	int ret = 0;
	int nt = type::GetSerialCount();
	for( int i=0; i<nt; i++ )
		if( type::Serial(i)->IsSubType(t) )
		        ret++;
	
  return arg2.Unify(FSP(m, (mint) ret));
}

LILFES_BUILTIN_PRED_2(type_nsubtypes, type_nsubtypes);

/**
  * @predicate	= type_supertypes(+TYPE, -LIST) 
  * @desc	= Get a list of the supertypes of TYPE. The list includes parents and all ancestors. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -LIST/ list  : List pf supertypes 
  * @example	= 
  > ?- type_supertypes(Square, L).
  L : < Square, Rectangle, Rhombus, Parallelogram, Quadrilateral, bot > 
  * @end_example

  * @jdesc	= TYPE のsupertypeの一覧をリストとして得ます。親だけでなく、全ての祖先が含まれます。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -LIST/ list : supertypeのリスト 
  * @jexample	=
  > ?- type_supertypes(正方形, L).
  L : < 正方形, 長方形, 凧形, 平行四辺形, 四角形, bot > 
  * @end_jexample
*/
bool type_supertypes( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	FSP f(m), fr = f;
	int nt = type::GetSerialCount();
	for( int i=nt-1; i>=0; i-- )
	{
		if( type::Serial(i)->IsSuperType(t) )
		{
			f.Follow(hd).Coerce(type::Serial(i));
			f = f.Follow(tl);
		}
	}
	f.Coerce(nil);
	
	return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(type_supertypes, type_supertypes);

/**
  * @predicate	= type_nsupertypes(+TYPE, -NUM) 
  * @desc	= Return how many the supertypes TYPE has. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -NUM/ integer  : number of supertype
  * @example	= 
  * @end_example

  * @jdesc	= TYPE にいくつsupertypeがあるかを返します．
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -NUM/ integer : supertypeの数
  * @jexample	=
  * @end_jexample
*/
bool type_nsupertypes( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	int ret = 0;
	int nt = type::GetSerialCount();
	for( int i=nt-1; i>=0; i-- )
		if( type::Serial(i)->IsSuperType(t) )
		        ret++;
	
  return arg2.Unify(FSP(m, (mint) ret));
}

LILFES_BUILTIN_PRED_2(type_nsupertypes, type_nsupertypes);

/**
  * @predicate	= type_unifiables(+TYPE, -LIST) 
  * @desc	= Return a list of all unifiable type of TYPE. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -LIST/ list  : list of unifiable types
  * @example	= 
  * @end_example

  * @jdesc	= TYPE にunify可能な型のリストを取得します．
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -LIST/ list  : unufy可能な型のリスト
  * @jexample	=
  * @end_jexample
*/
bool type_unifiables( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	FSP f(m), fr = f;
	int nt = type::GetSerialCount();
	for( int i=0; i<nt; i++ )
	{
		if( type::Serial(i)->TypeUnify(t))
		{
		  
			f.Follow(hd).Coerce(type::Serial(i));
			f = f.Follow(tl);
		}
	}
	f.Coerce(nil);
	
  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(type_unifiables, type_unifiables);

/**
  * @predicate	= type_nunifiables(+TYPE, -NUM) 
  * @desc	= Return how many the unifiable types TYPE has. 
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -NUM/ integer  : number of unifiable types
  * @example	= 
  * @end_example

  * @jdesc	= TYPE にいくつunify可能な型があるかを返します．
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -NUM/ integer : unify可能な型の数
  * @jexample	=
  * @end_jexample
*/
bool type_nunifiables( machine& m, FSP arg1, FSP arg2 )
{
	bool cs0 = m.GetConstraintEnabled();
	m.EnableConstraint(false);

	const type *t = arg1.GetType();

	int ret = 0;
	int nt = type::GetSerialCount();
	for( int i=0; i<nt; i++ )
		if( type::Serial(i)->TypeUnify(t))
		        ret++;
	m.EnableConstraint(cs0);
  return arg2.Unify(FSP(m, (mint) ret));
}

LILFES_BUILTIN_PRED_2(type_nunifiables,type_nunifiables);

/**
  * @predicate	= type_directsubtypes(+TYPE, -LIST)
  * @desc	= Get a list of the direct subtypes of TYPE.  
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -LIST/ list  : Direct subtype list 
  * @example	= 
  > ?- type_directsubtypes(pred/3, L).
  L: < pred/4, findall/3, commonpart/3, add_to_array/3, read_array/3, +/3, -/3,  //3, ///3, mod/3, follow/3, intersectionlist/3, type_unify/3 > 
  * @end_example

  * @jdesc	= TYPE のsubtypeの一覧をリストとして得ます。親だけでなく、全ての祖先が含まれます。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -LIST/ list : 直接のsubtypeのリスト 
  * @jexample	=
  > ?- type_directsubtypes(pred/3, L).
  L: < pred/4, findall/3, commonpart/3, add_to_array/3, read_array/3, +/3, -/3,  //3, ///3, mod/3, follow/3, intersectionlist/3, type_unify/3 > 
  * @end_jexample
*/
bool type_directsubtypes( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	FSP f(m), fr = f;
	int nt = t->GetNChildren();
	for( int i=0; i<nt; i++ )
	{
		f.Follow(hd).Coerce(t->Child(i));
		f = f.Follow(tl);
	}
	f.Coerce(nil);
	
  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(type_directsubtypes, type_directsubtypes);

/**
  * @predicate	= type_directsupertypes(+TYPE, -LIST) 
  * @desc	= Get a list of the direct supertypes of TYPE.  
  * @param	= +TYPE/bot : Type(Feature structure)
  * @param	= -LIST/ list  : Direct supertype list 
  * @example	= 
  > ?- type_directsupertypes(Isometric Right Angle Triangle, L).
  L : < Isometric, Right Angle Triangle > 
  * @end_example

  * @jdesc	= TYPE のsupertypeの一覧をリストとして得ます。親だけでなく、全ての祖先が含まれます。 
  * @jparam	= +TYPE/ bot : 型（素性構造）
  * @jparam	= -LIST/ list : 直接のsupertypeのリスト 
  * @jexample	=
  > ?- type_directsupertypes(直角二等辺三角形, L).
  L : < 二等辺三角形, 直角三角形 > 
  * @end_jexample
*/
bool type_directsupertypes( machine& m, FSP arg1, FSP arg2 )
{
	const type *t = arg1.GetType();

	FSP f(m), fr = f;
	int nt = t->GetNParents();
	for( int i=0; i<nt; i++ )
	{
		f.Follow(hd).Coerce(t->Parent(i));
		f = f.Follow(tl);
	}
	f.Coerce(nil);
	
  return arg2.Unify(fr);
}

LILFES_BUILTIN_PRED_2(type_directsupertypes, type_directsupertypes);

//////////////////////////////////////////////////////////////////////

/**
  * @predicate	= fs_compare(+FS1,+FS2,-$Result) 
  * @desc	= Complete order of FS1 and FS2
  * @param	= +FS1/bot : Feature structure1
  * @param	= +FS2/bot : Feature structure2
  * @param      = -$Result : Result of comparison (-1, 0, 1)
  * @jdesc	= 素性構造間の全順序を定義します．
  * @jparam	= +FS1/bot : 素性構造1
  * @jparam	= +FS2/bot : 素性構造2
  * @jparam      = -$Result : 素性構造1と2の順序 (-1, 0, 1)
*/
bool fs_compare( machine&, FSP arg1, FSP arg2, FSP arg3 ) {
  vector< cell > fs1, fs2;
  arg1.Serialize( fs1 );
  arg2.Serialize( fs2 );
  int ret = 0;

  if ( fs1.size() < fs2.size() ) {
    ret = -1;
  } else if ( fs1.size() > fs2.size() ) {
    ret = 1;
  } else {
    for ( size_t i = 0; i < fs1.size(); ++i ) {
      if ( (cell_int)fs1[ i ] < (cell_int)fs2[ i ] ) {
	ret = -1; break;
      } else if ( (cell_int)fs1[ i ] > (cell_int)fs2[ i ] ) {
	ret = 1; break;
      }
    }
  }
  return ( arg3.Unify( (mint)ret ) );
}

LILFES_BUILTIN_PRED_3(fs_compare, fs_compare);

} // namespace builtin

} // namespace lilfes

