/* $Id: ftype.h,v 1.3 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1996-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */
///  <ID>$Id: ftype.h,v 1.3 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>ftype.h</name>
//
///  <overview>
///  <jpn>素性構造　ルーチン</jpn>
///  <eng>feature structure routine </eng>
///  </overview>


#ifndef __ftype_h
#define __ftype_h

namespace lilfes {

class type;
class feature;

} // namespace lilfes

#include <string>
#include <utility>
#include <vector>

#include "lconfig.h"

#include "profile.h"
#include "module.h"

namespace lilfes {

////////////////////////////////////////
// Constants

// TYPE_GROW_STEP
//    When the array type::children has no rest for a new child, the array 
//    is expanded with the width TYPE_GROW_STEP. Must be 2^n.
const int TYPE_GROW_STEP = 4;

// FEAT_GROW_STEP
//    When the array type::feats has no rest for a new feature, the array 
//    is expanded with the width FEAT_GROW_STEP. Must be 2^n.
const int FEAT_GROW_STEP = 4;

// FEATTABLE_GROW_BITS
//    When the array type::feats has no rest for a new feature, the array 
//    is expanded with the width 2^FEAT_GROW_BITS.
const int FEATTABLE_GROW_BITS = 8;

// TYPETABLE_GROW_BITS
//    When the array type::feats has no rest for a new feature, the array 
//    is expanded with the width 2^FEAT_GROW_BITS.
const int TYPETABLE_GROW_BITS = 8;

#include "errors.h"

#define FEATURE_POSTFIX "\\"

//////////////////////////////////////////////////////////////////////////////
//
// Global variable definition
/// <vardef>
/// <name> bot </name>
/// <overview>bottom 型</overview>
/// <desc>
/// <p>
/// LiLFeSに組み込みの型で、全ての型のsupertypeとなる型です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern type * bot;		// Global variable for the bottom
/// </body></vardef>

//////////////////////////////////////////////////////////////////////////////
//
//  class feature
//
/// <classdef>
/// <name> feature </name>
/// <overview>素性 クラス</overview>
/// <desc>
/// <p>
/// 素性をあらわすクラスです。
/// </p>
/// </desc>
/// <see>type</see>
/// <body>
class feature
{
	const type *reptype;
	int priority;		// Priority of the feature (the order of features)
	tserial basetype;	// Serial No. of the feature's base type
	fserial serialno;	// Serial No. of this feature

	std::string name;
	std::string propername;
	
	static fserial builtin_limit;
	static _HASHMAP<tserial, feature *> hash;			// Hash table for name -> feature
	static std::vector<const feature *> serialtable;
											// Table for serial -> feature

public:
	feature(const std::string &repname, module *mod, int ip=0, bool exported = true);
  /// <var>repname</var>：名前<br>
  /// <var>mod</var>：属するモジュール名<br>
  /// <var>ip</var>：initial priority<br>
  /// <var>exported</var>：reptypeがexportedかどうか<br>
		feature(const type *reptype, int ip=0);
  /// <var>reptype</var>：表現タイプ<br>
  /// <var>ip</var>：initial priority<br>
		~feature();
		
	tserial GetKey() const;
  /// キーとしてnameを返します。
	const std::string GetSimpleName() const { return name; }
  /// 名前を取得します。この名前は正式なものではありません。正式な名前を取得するときはGetProperName()を使用してください。
	const std::string &GetProperName() const { return propername; }
  /// 正式な名前を取得します。
	const std::string &GetPrintName() const;
  /// 表示用の名前を取得します。
	const std::string &GetName() const  { return GetProperName(); }
  /// 正式な名前を取得します。
	int GetPriority() const { return priority; }
  /// 自分が属する型の中での優先順位を取得します。
	fserial GetSerialNo() const { return serialno; }
  /// シリアル番号を取得します。
	tserial GetBaseTypeS() const { return basetype; }
  /// ベースタイプ（その素性が最初に定義された型）のシリアル番号を取得します。
	const type *GetBaseType() const;
  /// ベースタイプ（その素性が最初に定義された型）を取得します。
	const type *GetRepType() const;
  /// 表現タイプ（その素性を表現する型）を取得します。
	void SetBaseType(const type *t);
  /// type <var>t</var>をベースタイプ（その素性が最初に定義された型）に設定します。
	bool IsBuiltIn() const { return serialno < builtin_limit; }
  /// 組み込み済みのものかどうかを調べます。組み込みのものならtrue、ユーザーが定義したものならfalseを返します。
	void Delete();
  /// featureオブジェクトを削除します。
	friend int compfeature(const feature *a, const feature *b) 
	{ int x = a->priority - b->priority; return x ? x : a->name.compare(b->name); }
	friend int compfeature(const void *a, const void *b) { return compfeature(*(const feature **)a, *(const feature **)b); }
  /// ２つのfeatureオブジェクトのベースタイプ中での順序を比較します。基本的にはpriorityの順で、priorityが同じなら辞書順になります
	static const feature* Search(const type *t);
	static const feature* Search(tserial t)
	{
		_HASHMAP<tserial, feature *>::const_iterator it = hash.find(t);
		return it == hash.end() ? NULL : it->second;
	}
  /// reptype<var>t</var>に対応するfeatureオブジェクトを探します。
	static const feature* Serial(fserial ser) { ASSERT(ser < serialtable.GetSize()); return serialtable[ser]; }
  /// シリアル番号から対応するfeatureオブジェクトを取得します。
//	static const feature* New   (char *n, const type *app = bot, int ip=0) 
//			{ const feature *r = hash.Find(n); return r ? r : new feature(n, app, ip); }

	static fserial GetSerialCount() { return (fserial)serialtable.size(); }
  /// シリアル番号がいくつまで割り当てられているか、つまりfeatureオブジェクトがいくつ存在するかを取得します。
	static void SetBuiltInLimit() { builtin_limit = GetSerialCount(); }
  /// その時点で存在する全てのfeatureオブジェクトをBuiltInであるとします。
	static void terminate();
  /// 終了時の処理（後始末）を行います。
};
/// </body></classdef>

//////////////////////////////////////////////////////////////////////////////
//
//  class type
//

class FSP;	// forward
class machine;		// forward

//////////////////////////////////////////////////////////////////////////////
//
//  class type
/// <classdef>
/// <name> type </name>
/// <overview>型 クラス</overview>
/// <desc>
/// <p>
/// 型をあらわすクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class type {
	std::string name;
	std::string propername;
	module *mod; /// 型が属するモジュールの名前です。
	bool exported;
	bool dynamic;
	bool multifile;
	bool deleted;
public:
	int nf;				// the number of features
  /// その型が有するfeatureの数を表します。
	const feature ** feats;
  /// その型が有する素性（の配列）です。
	const type ** apptypes;
  /// その型の中のある素性がとりうる型（の配列）。例えば素性feats[4]がとりうる型(appropriate type)はapptypes[4]で参照できる。
	int nc;				// the number of child-types
  /// 子のtypeの数を表します。
	const type ** children;
  /// その型の子のtype（の配列）です。
	int np;				// the number of parent-types
  /// 親のtypeの数を表します。
	const type ** parents;
  /// 親のtype（の配列）です。
	tserial serialno;
  /// その型のシリアル番号です。
#ifdef XCONSTR
	bool constrained;		// constrained?
  /// constraintが存在するかどうか。
	bool feat_constrained;	// feat_constrained?
  /// その型が有する素性のapptypeにconstraintが存在するかどうか。
	bool fixed;				//
  /// typeオブジェクトが定義済みかどうかをあらわします。
	bool _dummy; /// for padding
#endif
	int appcounter;
  /// その型がapptypeとしていくつの型の中で用いられたか。
	short *featindex;		// Table of feature -> index
  /// fserialとindexの対応を保持する配列。featindex[素性のシリアル番号] = その素性のindex
	FSP *attr;			// using pointer since forwarding class def
#ifdef XCONSTR
	code *constraint;			// pointer that holds constraint code
  /// その型が作られるときに実行される命令。
#endif

#ifdef FEATURETABLE
	static int feattablesize;
  /// featindexの配列の要素数です。
#endif

	static std::vector<const type *> serialtable;
  /// シリアル番号とfeatureオブジェクトの対応関係を保持するテーブルです。
  // Table for serial -> feature
	static tserial builtin_limit;
  /// シリアル番号のいくつまでが組み込み型であるかを表します。
public:
	type(const std::string &in, module *mod, bool iexported = true);
  /// <var>in</var>：名前<br>
  /// <var>mod</var>：属するモジュール名<br>
  /// <var>iexported</var>：expotedかどうか（下記のIsExported()を参照）<br>
	~type();
	
	const std::string &GetKey() const { return name; }
  /// キーとしてnameを返します。
	const std::string &GetSimpleName() const { return name; }
  /// nameを返します。
	const std::string GetSimpleProperName() const;
  /// ???
	const std::string &GetProperName() const { return propername; }
  /// propernameを取得します。
	const std::string &GetPrintName() const;
  /// 表示用の名前を取得します。
	const std::string &GetName() const  { return GetProperName(); }
  /// 名前を取得します。propernameを取得します。
	bool GetDynamic() const { return dynamic; }
  /// その型がdynamicかどうか（ユーザーが定義を動的に変更できるかどうか）を調べます。
	bool GetMultifile() const { return multifile; }
  /// その型がmultifileかどうか（複数のソースファイルで定義される可能性があるかどうか）を調べます。
	void SetDynamic() { dynamic = true; }
  /// その型をdynamic（ユーザーが定義を動的に変更できるように）にします。
	void SetMultifile() { multifile = true; }
  /// その型をmultifile（複数のソースファイルで定義される可能性をもたせる）にします。
	module *GetModule() const { return mod; }
  /// その型が属するモジュールを取得します。
	tserial GetSerialNo() const { P5("type::GetSerialNo"); return serialno; }
  /// シリアル番号を取得します。
#ifdef FEATURETABLE
	int GetIndex(fserial fSN) const { ASSERT(fSN < feattablesize); return feattable[fSN]; }
#else
	int GetIndex(fserial fSN) const { return GetIndex(feature::Serial(fSN)); }
  /// 引数に指定した素性が、その型のなかで何番目に位置するかを調べます。最初の番号は0から始まります。
#endif
	const feature *Feature(int n) const { return feats[n]; }
  /// <var>n</var>番目のfeatureを取得します。<var>n</var>は0からGetNFeatures()-1までです。
	tserial GetAppTypeS(int n) const { return apptypes[n]->GetSerialNo(); }
  /// <var>n</var>番目の素性のapptypeのシリアル番号を取得します。<var>n</var>は0からGetNFeatures()-1までです。
	tserial GetAppTypeS(const feature *f) const { int idx = GetIndex(f->GetSerialNo()); return idx == -1 ? TS_INVALID : apptypes[idx]->GetSerialNo(); }
  /// feature<var>f</var>に対応するapptypeのシリアル番号を取得します。
	const type *GetAppType(int n) const { return apptypes[n]; }
  /// n番目のapptypeを取得します。
	const type *GetAppType(const feature *f) const { int idx = GetIndex(f->GetSerialNo()); return idx == -1 ? NULL : apptypes[idx]; }
  /// feature<var>f</var>に対応するapptypeを取得します。
	int GetNFeatures() const { return nf; }
  /// featureの数を取得します。
	const type *Child(int n) const { return children[n]; }
  /// <var>n</var>番目の子typeを取得します。<var>n</var>は0からGetNChildren()-1までです。
	int GetNChildren() const { return nc; }
  /// 子の型の数を取得します。
	const type *Parent(int n) const { return parents[n]; }
  /// <var>n</var>番目の親typeを取得します。
	int GetNParents() const { return np; }
  /// 親の型の数を取得します。
	int GetIndex(const feature *f) const;
		// Get the index of feature f
		// return -1 if not found

	bool SetAsChildOf(type *parent);
  /// type <var>parent</var>の子に設定します。（注：Fix()される前にしか使えません。）
	int  AddFeature(feature *f, const type *apptype = bot);
  /// その型に素性を追加します。（注：Fix()される前にしか使えません。）
	void SetAttribute(const FSP &);
  /// ???（注：Fix()される前にしか使えません。）
	FSP GetAttribute(machine &m) const;
  /// ???
	const type *TypeUnify(const type *t) const;
  /// その型と型<var>t</var>との単一化を行います。成功すれば結果の型を返し、失敗すればNULLを返します。
		// Get the unified type between this and t
		// return NULL if no candidate is found
	bool IsSubType(const type *t) const { return TypeUnify(t) == this; }
  /// type<var>t</var>のsubtypeかどうかを調べます。
	bool IsSuperType(const type *t) const { return t->TypeUnify(this) == t; }
  /// type<var>t</var>のsupertypeかどうかを調べます。
	bool IsExtendable() const;
  /// 他のモジュールでその型のsubtypeが定義できるかどうかを調べます。

#ifdef XCONSTR
	code *GetConstraint() const { return constraint; }
  /// ???
	bool IsConstrained() const { return constrained; }
  /// constraintが存在するかどうかを調べます。
	bool IsFeatureConstrained() const { return feat_constrained; }
  /// 素性にconstraintが存在するかどうかを調べます。
#endif
		
	const type *AcceptableType(const feature *f) const;
  /// その型とその型のすべてのsubtypeの中でfeature<var>f</var>を持つことができる型を取得します。ある型<var>a</var>とそのsubtype<var>b</var>両方がその素性を持つことができるときは、一番上の型である<var>a</var>を返します。
		// Get the nearest descendant type that involves feature f
		// return NULL if no candidate is found

	bool IsBuiltIn() const { return serialno < builtin_limit; }
  /// 組み込み済みのものかどうかを調べます。組み込みのものならtrue、ユーザーが定義したものならfalseを返します。
//	bool IsSpecial();
	bool IsFinal() const;
  /// その型にsubtypeが存在しないかどうかを調べます。
	bool IsExported() const { return exported; }
  /// 他のモジュールで参照する際に、モジュール名を含めた正式名を使わずにアクセスできるかどうかを調べます。
	bool IsDeleted() const { return deleted; }
  /// そのtypeオブジェクトが削除済みかどうかを調べます。
// Static member functions
	static const type* Serial(tserial ser) { ASSERT(ser < serialtable.GetSize()); return serialtable[ser]; }
  /// シリアル番号から対応するtypeオブジェクトを取得します。
	static tserial GetSerialCount() { return (tserial)serialtable.size(); }
  /// シリアル番号がいくつまで割り当てられているか、つまりtypeオブジェクトがいくつ存在するかを取得します。
	static void SetBuiltInLimit() { builtin_limit = GetSerialCount(); }
  /// その時点で存在する全てのtypeオブジェクトをBuiltInであるとします。
#ifdef FEATURETABLE
	static void ExpandFeatureTable(int newnf);
  /// feature tableを拡張します。
#endif

	static void terminate();
  /// 終了時の処理（後始末）を行います。
	bool Fix(bool displayError = true);		// return true if success
  /// その型の定義を確定します。
	bool Delete();	// return true if success
  /// typeオブジェクトを削除します。
	friend void CompileTypes();
  /// ???(compile.cpp内にある)
	void IncAppCounter() const { ((type *)this)->appcounter++; }
  /// appcounterを１増やします。
	void DecAppCounter() const { ((type *)this)->appcounter--; }
  /// appcounterを１減らします。
};
/// </body></classdef>

// About compile ... these are in compile.cpp
/// <classdef>
/// <name> featcode </name>
/// <overview>素性と型の単一化のためのクラス</overview>
/// <desc>
/// <p>
/// 素性と型の単一化の際に実行される処理。<ref>featureCode</ref>でしか使いません。
/// </p>
/// </desc>
/// <see></see>
/// <body>
struct featcode
{
	tserial result;
  /// 単一化の結果の型のシリアル番号です。
	ushort index;
  /// 単一化のターゲットの素性のindexです。<ref>type</ref>を参照。
	code *coercion;
  /// 型変換を行うための処理をあらわします。
};
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> CompileType </name>
/// <overview> code生成</overview>
/// <desc>
/// <jpn>
/// <p>
/// 2つの型の単一化を実行するためのcodeを生成し<ref>coerceHash</ref>に登録します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 単一化する2つの型へのポインタ（またはシリアル番号）を渡します。</args>
/// <retval> なし。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
void CompileType(tserial unifiee, tserial unifier);
void CompileType(const type *unifiee, const type *unifier);
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> featureCode </name>
/// <overview>featcodeの2次元配列</overview>
/// <desc>
/// <p>
/// featcodeの2次元配列です。ある型のある素性を参照するときに必要な情報が入っています。<br>
/// 1次元目はtypeのシリアル番号、2次元目はfeatureのシリアル番号です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern featcode **featureCode;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> coerceNone[] </name>
/// <overview> 何もしないcode </overview>
/// <desc>
/// <p>
/// 何もしないcodeです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern code coerceNone[];
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> onlyFail[] </name>
/// <overview></overview>
/// <desc>
/// <p>
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern code onlyFail[];
/// </body></vardef>


#ifdef XCONSTR
/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> finalize_constraint </name>
/// <overview></overview>
/// <desc>
/// <p>
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern code *finalize_constraint;
/// </body></vardef>

#define FINALIZE_CONSTRAINT finalize_constraint
#endif

inline tserial feature::GetKey() const { return reptype->GetSerialNo(); }
inline const type *feature::GetBaseType() const { return type::Serial(basetype); }
inline const type *feature::GetRepType() const { return reptype; }
inline void feature::SetBaseType(const type *t) { basetype = t->GetSerialNo(); }
inline const feature *feature::Search(const type *t) { if( t == NULL ) return NULL; else return Search(t->GetSerialNo()); }

#ifdef TYPE_TABLE_HASH
//////////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> coercode </name>
/// <overview>型同士の単一化のためのクラス</overview>
/// <desc>
/// <p>
/// 型同士の単一化の際に実行される処理。<ref>coerceCode</ref>でしか使いません。 
/// </p>
/// </desc>
/// <see></see>
/// <body>
struct coercode
{
	int	 key;
  /// 2次元テーブルを使うと効率が悪いのでハッシュを使う。そのためのキー。<br>tserial<var>a</var>とtserial<var>b</var>があったら対応するキーはab。a=0012,b=0034ならkey=00120034（実際は<ref>tserial</ref>は16bitです。）
	tserial result_s;
  /// 単一化の結果の型のシリアル番号です。単一化が失敗するときは<ref>TS_INVALID</ref>が入ります。
	code *coercion;
  /// 型変換を行うための処理をあらわします。
#ifdef XCONSTR
	code *coercion_var;
  /// ???
	code *constraint;
  /// ???
#endif
	code *unification;
  /// 単一化を行うための処理をあらわします。

	int GetKey() { return key; }
  /// ハッシュ用のキーとしてkeyを返します。
};
/// </body></classdef>
/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> coerceHash </name>
/// <overview> coercodeのハッシュ </overview>
/// <desc>
/// <p>
/// キーが整数、値がcoercodeであるハッシュです。coerceCodeをハッシュにしたものです。<br>
/// 2つの型を単一化するのに必要な情報が入っています。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern _HASHMAP<int, coercode *> coerceHash;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> coercode_fail </name>
/// <overview>単一化が失敗するときのcoercode</overview>
/// <desc>
/// <p>
/// 単一化が失敗するときの処理のcoercodeオブジェクトです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern const coercode coercode_fail;
/// </body></vardef>

inline int t2key(tserial a, tserial b) { return (((a)<<16)+(b)); }
/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> coerceRef </name>
/// <overview> coercodeオブジェクトを参照するための関数。</overview>
/// <desc>
/// <jpn>
/// <p>
/// tserial <var>a</var>, tserial <var>b</var>に対応するcoercodeオブジェクトを<ref>coerceHash</ref>から取得します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 2つのtserialオブジェクトを渡します。</args>
/// <retval> 対応するcoercodeオブジェクトがあればそれを返します。 対応するものがなければNULLを返します。</retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline const coercode * coerceRef(tserial a, tserial b) 
{
	_HASHMAP<int, coercode *>::const_iterator it = coerceHash.find(t2key(a, b));
	return it == coerceHash.end() ? NULL : it->second;
}
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> coerceData </name>
/// <overview> coercodeオブジェクトを参照するための関数。（対応するものがない場合coercode_failを返す）</overview>
/// <desc>
/// <jpn>
/// <p>
/// tserial <var>a</var>, tserial <var>b</var>に対応するcoercodeオブジェクトを取得します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 2つのtserialオブジェクトを渡します。</args>
/// <retval> 対応するcoercodeオブジェクトがあればそれを返します。 対応するものがなければ<ref>coercode_fail</ref>を返します。</retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline const coercode & coerceData(tserial a, tserial b) 
{
	_HASHMAP<int, coercode *>::const_iterator it = coerceHash.find(t2key(a, b));
	return it == coerceHash.end() ? coercode_fail : *(it->second);
}
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> coerceDataAdd </name>
/// <overview> coercodeオブジェクトを作成するための関数。</overview>
/// <desc>
/// <jpn>
/// <p>
/// tserial <var>a</var>, tserial <var>b</var>に対応するcoercodeオブジェクトを作成します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 2つのtserialオブジェクトを渡します。</args>
/// <retval> 既に対応するものが存在すればそれを返します。<br> 無ければ対応するcoercodeオブジェクトを新しく作ってそれを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline coercode & coerceDataAdd(tserial a, tserial b) 
{
	_HASHMAP<int, coercode *>::const_iterator it = coerceHash.find(t2key(a, b));
	coercode *f;
	if(it != coerceHash.end())
	{
		f = it->second;
	}
	else
	{ 
		f = new coercode; 
		f->key=t2key(a,b); 
		f->coercion = f->coercion_var = f->constraint = f->unification = NULL;
		coerceHash.insert(std::make_pair(f->key, f));
	}
	return *f; 
}
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> coerceDataDelete </name>
/// <overview>coercodeオブジェクトを削除するための関数。 </overview>
/// <desc>
/// <jpn>
/// <p>
/// tserial <var>a</var>, tserial <var>b</var>に対応するcoercodeオブジェクトを削除します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 2つのtserialオブジェクトを渡します。</args>
/// <retval> 成功すればtrueを、失敗すればfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool coerceDataDelete(tserial a, tserial b) 
{ 
	coerceHash.erase(t2key(a,b)); 
	return true;
}
/// </body></funcdef>

#else
struct coercode
{
	tserial result_s;
	code *coercion;
#ifdef XCONSTR
	code *coercion_var;
	code *constraint;
#endif
	code *unification;
};

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> coerceCode </name>
/// <overview>coercodeの2次元配列</overview>
/// <desc>
/// <p>
/// coercodeの2次元配列です。
/// </p>
/// </desc>
/// <see>unification_overview</see>
/// <body>
extern coercode **coerceCode;
/// </body></vardef>

inline const coercode & coerceData(tserial a, tserial b) 
	{ return coerceCode[(a)][(b)]; }
inline coercode & coerceDataAdd(tserial a, tserial b) 
	{ return coerceCode[(a)][(b)]; }
inline bool coerceDataDelete(tserial a, tserial b)  {
	coerceCode[(a)][(b)].result_s = TS_INVALID; 
	coerceCode[(a)][(b)].coercion = coerceCode[(a)][(b)].coercion_var = coerceCode[(a)][(b)].constraint = coerceCode[(a)][(b)].unification = onlyFail;
}
#endif


std::string Quote(const std::string &name, bool AllowCapital = false);

} // namespace lilfes

#endif // __ftype_h
