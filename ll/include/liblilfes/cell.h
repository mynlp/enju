/* $Id: cell.h,v 1.7 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: cell.h,v 1.7 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>cell.h</name>
//
///  <overview>
///  <jpn>cell 操作ルーチン</jpn>
///  <eng> cell operation routine </eng>
///  </overview>

/// <notiondef>
/// <name> cell_overview </name>
/// <title> cell 操作ルーチン </title>
/// 
/// <desc>
/// 
/// <p> <ref>cell.h</ref>は素性構造をメモリに格納するときにタグとデータを対にしたり、メモリから取り出すときにタグとデータを切り離したりするのに使います。<br>
/// </p>
/// </desc>
/// <see> </see>
/// </notiondef>

//
//  LightSpeed HPSG Parser Project
//
//  cell.h - Definition of Base classes.
//
//  **LICENSE**
//
//  Change Log:
//
//  96/11/22  Created. / mak
//  97/05/26  Readability improved. / mak

#ifndef __cell_h
#define __cell_h

#include <ostream>
#include <string>
#include <vector>

#include "lconfig.h"
#include "basic.h"
#include "ftype.h"
#include "profile.h"

namespace lilfes {

// String Symbol Class

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> strsymbol </name>

/// <overview> String Symbol　クラス</overview>
/// <desc>
/// <p>
/// 現 LiLFeS の実装では、文字列シンボルのそれぞれにユニークなシリアル番号を割り当てています。シリアル番号と文字列の実体の関係を記憶するクラスが strsymbol クラスです。
/// </p>
/// </desc>
/// <remark></remark>
/// <see></see>
/// <body>
class strsymbol
{
	std::string name;
	int serialno;
	mutable bool used;
	mutable bool permanent;

	static std::vector<const strsymbol *> serialtable;
	static _HASHMAP<std::string, strsymbol *> hash;

public:
	strsymbol(machine *mach, std::string name);
	~strsymbol();
		
	const std::string& GetKey() const { return name; }
  /// キーとしてnameを返します。
	const char *GetString() const { return name.c_str(); }
  /// 文字列の実体を取得します。
	int GetSerialNo() const { return serialno; }
  /// この文字列のシリアル番号を取得します。
	void SetPermanent() const { permanent = true; }
  /// Permanentフラグをセットします。この文字列はgcで解放されなくなります。
	void SetUsed() const { used = true; }
  /// Usedフラグをセットします。この文字列が現在使用中であることを示します。
	static const strsymbol* Serial(int serial) { return serialtable[serial]; }
  /// シリアル番号から対応する strsymbol クラスのポインタを引きます。
	static strsymbol* New(machine *mach, const std::string &n);
  /// 指定された文字列に対して、すでに対応する strsymbol クラスがあればそのポインタを、なければ新しい strsymbol クラスを確保して返します。
	static int GetSerialCount() { return serialtable.size(); }
  /// 現在、シリアル番号がいくつまで割り振られているかを調べます。
	static void gc(machine *mach);
  /// 文字列のガベジコレクション（使用されていないメモリを解放）を実行します。
};
/// </body></classdef>


inline strsymbol* strsymbol::New(machine *mach, const std::string &n) 
{
	_HASHMAP<std::string, strsymbol *>::const_iterator it = hash.find(n);
	return it != hash.end() ? it->second : new strsymbol(mach, n);
}


// Definition of cell

// cell is a data type that has a 32-bit width.
// We don't use a class because the performance of function calling loses, so
// We use enum type for cell.
// ____1 and ____2 guarantee that the type has a 32-bit (or 64-bit) width.

#ifdef CELL_64BIT
/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> cell </name>

/// <overview> タグと値が組みになったクラス</overview>
/// <desc>
/// <p>
/// ...です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
enum cell { ____1 = -1000000000000L, ____2 = 1000000000000L };
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> cell_int </name>

/// <overview> cell intクラス</overview>
/// <desc>
/// <p>
/// cellと同じサイズのint型です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef uint64 cell_int;
/// </body></classdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TAG_SHIFT </name>
/// <overview>データ部分のビット数を表す定数</overview>
/// <desc>
/// <jpn>
/// <p>
/// データ部分のビット数を表す定数です。全体をTAG_SHIFTビット分だけ右にシフトすればタグの部分が残ります。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const int TAG_SHIFT = 60;
/// </body></constdef>

#define SIZEOF_CELL 8

#else
enum cell { ____1 = -1000000000    , ____2 = 1000000000     };
typedef uint32 cell_int;
const int TAG_SHIFT = 28;
#define SIZEOF_CELL 4
#endif

// machine int / machine char / machine float definition

#ifdef CELL_64BIT
/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> mchar </name>

/// <overview> machine char クラス </overview>
/// <desc>
/// <p>
/// <ref>cell</ref>に格納できるサイズのchar型です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef uint16 mchar;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> mint </name>

/// <overview> machine int クラス </overview>
/// <desc>
/// <p>
/// <ref>cell</ref>に格納できるサイズのint型です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef int64  mint;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> mfloat </name>

/// <overview> machine float クラス </overview>
/// <desc>
/// <p>
/// <ref>cell</ref>に格納できるサイズのfloat型です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef double mfloat;
/// </body></classdef>
#else
typedef uint16 mchar;
typedef int32  mint;
typedef float  mfloat;
#endif

// cell <-> cell_int conversion functions
/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> cell2int </name>
/// <overview>cell型をcell_int型に変換します。</overview>
/// <desc>
/// <p>
/// 。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> cell_intオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline cell_int cell2int(cell c) { return (cell_int)c; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> int2cell </name>
/// <overview>cell_int型をcell型に変換します。</overview>
/// <desc>
/// <p>
/// 。
/// </p>
/// </desc>
/// <args> cell_intオブジェクトを渡します。</args>
/// <retval> cell_オブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline cell int2cell(cell_int i) { return (cell)i; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> int_float </name>
/// <overview> int <-> float 変換のための union </overview>
/// <desc>
/// <p>
/// type-punning で int <-> float を変換するためのクラス．gcc では動作が
/// 保証されているが，Standard では undefined なので，他のコンパイラでは
/// 動作しない可能性がある．
/// </p>
/// </desc>
/// <see></see>
/// <body>
union int_float {
  cell_int c;
  mfloat f;
};
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> float2int </name>
/// <overview> mfloat 型を cell_int 型に変換します．</overview>
/// <desc>
/// <p>
/// </p>
/// </desc>
/// <args> mfloatオブジェクトを渡します。</args>
/// <retval> cell_intオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline cell_int float2int( mfloat f ) { cell_int c = 0; int_float cf; cf.f = f; c = cf.c; return c; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> int2float </name>
/// <overview> cell_int 型を mfloat 型に変換します．</overview>
/// <desc>
/// <p>
/// </p>
/// </desc>
/// <args> cell_intオブジェクトを渡します。</args>
/// <retval> mfloatオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline mfloat int2float( cell_int c ) { mfloat f = 0.0; int_float cf; cf.c = c; f = cf.f; return f; }
/// </body></funcdef>


// definition of core_p
/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> core_p </name>

/// <overview> 抽象機械上のメモリへのポインタクラス</overview>
/// <desc>
/// <p>
/// core (LiLFeS 抽象機械上のメモリ) へのポインタに相当するクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef cell_int core_p;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> int2core_p </name>
/// <overview>cell_int型をcore_p型に変換します。</overview>
/// <desc>
/// <p>
/// <ref>core_p2int</ref>で変換したものを元に戻します。
/// </p>
/// </desc>
/// <args> cell_intオブジェクトを渡します。</args>
/// <retval> core_pオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline core_p   int2core_p(cell_int i) { return i; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> core_p2int </name>
/// <overview>core_p型をcell_int型に変換します。</overview>
/// <desc>
/// <p>
/// core_p型をintのように扱うための関数です。
/// </p>
/// </desc>
/// <args> core_pオブジェクトを渡します。</args>
/// <retval> cell_intオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>int2core_p</see>
/// <body>
inline cell_int core_p2int(core_p   i) { return i; }
/// </body></funcdef>


//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> CORE_P_INVALID </name>
/// <overview>無効なcore_pを表す定数</overview>
/// <desc>
/// <jpn>
/// <p>
/// 無効なcore_pを表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const core_p CORE_P_INVALID = ((cell_int)1 << (TAG_SHIFT+2)) - 1;
/// </body></constdef>


// abbreviated form of converting function
#define cp2int core_p2int
#define int2cp int2core_p


/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> cp_plus </name>
/// <overview>２つのcore_pオブジェクトの和を求めます。</overview>
/// <desc>
/// <p>
/// core_pオブジェクト<var>a</var>,<var>b</var>を足し合わせたものを返します。
/// </p>
/// </desc>
/// <args> core_pオブジェクトを渡します。</args>
/// <retval> core_pオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline core_p cp_plus(core_p a, core_p b) 
{ 
	return int2core_p(core_p2int(a)+core_p2int(b));
}
/// </body></funcdef>


/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> cp_minus </name>
/// <overview>２つのcore_pオブジェクトの差を求めます。</overview>
/// <desc>
/// <p>
/// core_pオブジェクト<var>a</var>から<var>b</var>を引いたものを返します。
/// </p>
/// </desc>
/// <args> core_pオブジェクトを渡します。</args>
/// <retval> core_pオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline core_p cp_minus(core_p a, core_p b) 
{ 
	return int2core_p(core_p2int(a)-core_p2int(b));
}
/// </body></funcdef>

//////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> tag_t </name>
/// <overview>タグ</overview>
/// <desc>
/// <jpn>
/// <p>
/// タグを表す列挙型の定数です。<br>
/// 値の対応<br>
/// T_PTR =0, T_PTR1=1, T_PTR2=2, T_PTR3=3,<br>
/// T_STR =4, T_FSTR=5, T_VAR =6, T_FVAR=7,<br>
/// T_INT =8, T_FLO =9, T_CHR=10, T_STG =11<br>
/// <br>PTR:ポインタ <br>VAR:変数 <br>F:freeze <br>INT:整数 <br>FLO:実数 <br>STG:文字列<br>
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
enum tag_t { 
  T_PTR =0,/// ポインタ
  T_PTR1=1,
  T_PTR2=2,
  T_PTR3=3,
  T_STR =4,/// structure
  T_FSTR=5,/// freeze structure
  T_VAR =6,/// 変数
  T_FVAR=7,/// freeze 変数
  T_INT =8,/// 整数
  T_FLO =9,/// 実数
  T_CHR=10,/// 文字
  T_STG =11/// 文字列
};
/// </body></classdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TAG_WIDTH </name>
/// <overview>タグのビット数</overview>
/// <desc>
/// <jpn>
/// <p>
/// タグのビット数（何ビット使っているか）を表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const int TAG_WIDTH = 4;
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TAG_MASK </name>
/// <overview>タグの...</overview>
/// <desc>
/// <jpn>
/// <p>
/// タグを表すビットが１になっている定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const cell_int TAG_MASK = ( ~ static_cast< cell_int >( 0 ) ) << TAG_SHIFT;
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> NTAG_MASK </name>
/// <overview>TAG_MASKを反転したもの。</overview>
/// <desc>
/// <jpn>
/// <p>
/// TAG_MASKを反転したものです。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const cell_int NTAG_MASK = ~ TAG_MASK ;
/// </body></constdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> Tag </name>
/// <overview>cellのタグを取得します。</overview>
/// <desc>
/// <p>
/// <ref>cell</ref>のタグを取得します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> tag_t オブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>tag_t</see>
/// <body>
inline tag_t Tag(cell c) { return (tag_t)(cell2int(c)>>TAG_SHIFT); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsPTR </name>
/// <overview>セルのタグがPTRかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがPTRかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがPTRならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsPTR(cell c)       { return ((cell2int(c) & ((cell_int)0xc << TAG_SHIFT)) == ((cell_int)0x0 << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsSTR </name>
/// <overview>セルのタグがSTRかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがSTRかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがSTRならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsSTR(cell c)       { return ((cell2int(c) & ((cell_int)0xe << TAG_SHIFT)) == ((cell_int)0x4 << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsVAR </name>
/// <overview>セルのタグがVARかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがVARかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがVARならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsVAR(cell c)       { return ((cell2int(c) & ((cell_int)0xe << TAG_SHIFT)) == ((cell_int)0x6 << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsSTRorVAR </name>
/// <overview>セルのタグがSTRもしくはVARかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがSTRもしくはVARかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがSTRもしくはVARならtrueを、そうでなければfalseを返します。  </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsSTRorVAR(cell c)  { return ((cell2int(c) & ((cell_int)0xc << TAG_SHIFT)) == ((cell_int)0x4 << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsFROZEN </name>
/// <overview>。</overview>
/// <desc>
/// <p>
/// を返します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> を返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsFROZEN(cell c)    { return ((cell2int(c) & ((cell_int)0xd << TAG_SHIFT)) == ((cell_int)0x4 << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsDAT </name>
/// <overview>セルのタグがDATかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがDATかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがDATならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsDAT(cell c)       { return ((cell2int(c) & ((cell_int)0xc << TAG_SHIFT)) == ((cell_int)0x8 << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsINT </name>
/// <overview>セルのタグがINTかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがINTかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがINTならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsINT(cell c)       { return ((cell2int(c) & ((cell_int)0xf << TAG_SHIFT)) == ((cell_int)T_INT << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsFLO </name>
/// <overview>セルのタグがFLOかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがFLOかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがFLOならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsFLO(cell c)       { return ((cell2int(c) & ((cell_int)0xf << TAG_SHIFT)) == ((cell_int)T_FLO << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsCHR </name>
/// <overview>セルのタグがCHRかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがCHRかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがCHRならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsCHR(cell c)       { return ((cell2int(c) & ((cell_int)0xf << TAG_SHIFT)) == ((cell_int)T_CHR << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> IsSTG </name>
/// <overview>セルのタグがSTGかどうかを調べます。</overview>
/// <desc>
/// <p>
/// セルのタグがSTGかどうかを調べます。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> タグがSTGならtrueを、そうでなければfalseを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline bool IsSTG(cell c)       { return ((cell2int(c) & ((cell_int)0xf << TAG_SHIFT)) == ((cell_int)T_STG << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> char_int </name>

/// <overview> ???クラス</overview>
/// <desc>
/// <p>
/// ???クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned short char_int;
/// </body></classdef>


/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> PTR2c </name>
/// <overview> 抽象機械上のメモリへのポインタからPTRタグ付きのセルを生成します。 </overview>
/// <desc>
/// <p>
/// 抽象機械上のメモリへのポインタからPTRタグ付きのセルを生成します。 
/// </p>
/// </desc>
/// <args> core_pオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2PTR</see>
/// <body>
inline cell PTR2c(core_p p)          { P5("PTR2c");   return int2cell(core_p2int(p)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> STR2c </name>
/// <overview>型または型のシリアル番号からSTRタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 型(type)または型のシリアル番号(tserial)からSTRタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> typeオブジェクトまたはtserialオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2STR</see>
/// <body>
inline cell STR2c(const type *p)     { P5("STR2c*");  return int2cell(p->GetSerialNo() | ((cell_int)T_STR  << TAG_SHIFT)); }
inline cell STR2c(tserial serial)    { P5("STR2c");   return int2cell(     serial      | ((cell_int)T_STR  << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> VAR2c </name>
/// <overview>型または型のシリアル番号からVARタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 型(type)または型のシリアル番号(tserial)からVARタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> typeオブジェクトまたはtserialオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2VAR</see>
/// <body>
inline cell VAR2c(const type *p)     { P5("VAR2c*");  return int2cell(p->GetSerialNo() | ((cell_int)T_VAR  << TAG_SHIFT)); }
inline cell VAR2c(tserial serial)    { P5("VAR2c");   return int2cell(     serial      | ((cell_int)T_VAR  << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> FSTR2c </name>
/// <overview>型または型のシリアル番号からFSTRタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 型(type)または型のシリアル番号(tserial)からFSTRタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> typeオブジェクトまたはtserialオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2FSTR</see>
/// <body>
inline cell FSTR2c(const type *p)    { P5("FSTR2c*"); return int2cell(p->GetSerialNo() | ((cell_int)T_FSTR << TAG_SHIFT)); }
inline cell FSTR2c(tserial serial)   { P5("FSTR2c");  return int2cell(     serial      | ((cell_int)T_FSTR << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> FVAR2c </name>
/// <overview>型または型のシリアル番号からFVARタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 型(type)または型のシリアル番号(tserial)からFVARタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> typeオブジェクトまたはtserialオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2FVAR</see>
/// <body>
inline cell FVAR2c(const type *p)    { P5("FVAR2c*"); return int2cell(p->GetSerialNo() | ((cell_int)T_FVAR << TAG_SHIFT)); }
inline cell FVAR2c(tserial serial)   { P5("FVAR2c");  return int2cell(     serial      | ((cell_int)T_FVAR << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> INT2c </name>
/// <overview>整数からINTタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 整数(mint)からINTタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> mintオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2INT</see>
/// <body>
inline cell INT2c(mint val)          { P5("INT2c");   return int2cell( ((cell_int)val & NTAG_MASK) | ((cell_int)T_INT  << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> FLO2c </name>
/// <overview>実数からFLOタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 実数(mfloat)からFLOタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> mfloatオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2FLO</see>
/// <body>
inline cell FLO2c(mfloat val)        { P5("FLO2c"); return int2cell( ((float2int(val) + ((cell_int)1<<(TAG_WIDTH-1)))>> TAG_WIDTH) | ((cell_int)T_FLO  << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> CHR2c </name>
/// <overview>文字からCHRタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 文字(mchar)からCHRタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> mcharオブジェクトを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2CHR</see>
/// <body>
inline cell CHR2c(mchar val)         { P5("CHR2c");   return int2cell( (cell_int)val | ((cell_int)T_CHR  << TAG_SHIFT)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> STG2c </name>
/// <overview>文字列または文字列シリアルからSTGタグ付きのセルを生成します。</overview>
/// <desc>
/// <p>
/// 文字列または文字列シリアルからSTGタグ付きのセルを生成します。
/// </p>
/// </desc>
/// <args> 文字列または文字列シンボルまたは文字列シリアルを渡します。</args>
/// <retval> cellオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>c2STG</see>
/// <body>
inline cell STG2c(machine *m, const std::string &s)     { P5("STG2c");   return int2cell( (cell_int)(strsymbol::New(m,s)->GetSerialNo()) | ((cell_int)T_STG << TAG_SHIFT) ); }
inline cell STG2c(strsymbol *s)      { P5("STG2c");   return int2cell( (cell_int)(               s ->GetSerialNo()) | ((cell_int)T_STG << TAG_SHIFT) ); }
inline cell STG2c(sserial s)         { P5("STG2c");   return int2cell( (cell_int)(               s                ) | ((cell_int)T_STG << TAG_SHIFT) ); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2PTR </name>
/// <overview>PTRタグのついたセルから抽象機械上のメモリへのポインタを取り出します。</overview>
/// <desc>
/// <p>
/// PTRタグのついたセルから抽象機械上のメモリへのポインタ(core_p)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> core_pオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>PTR2c</see>
/// <body>
inline core_p c2PTR(cell c)          { P5("c2PTR");  ASSERT(IsPTR(c)); return int2core_p(cell2int(c)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2STRS </name>
/// <overview>STRSタグのついたセルから型のシリアル番号を取り出します。</overview>
/// <desc>
/// <p>
/// STRSタグのついたセルから型のシリアル番号(tserial)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> tserialオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>STRS2c</see>
/// <body>
inline tserial c2STRS(cell c)        { P5("c2STR");  ASSERT(IsSTR(c)); return              (tserial)(cell2int(c) & NTAG_MASK) ; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2STR </name>
/// <overview>STRタグのついたセルから型へのポインタを取り出します。</overview>
/// <desc>
/// <p>
/// STRタグのついたセルから型(type)へのポインタを取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> typeオブジェクトへのポインタを返します。 </retval>
/// <remark></remark>
/// <see>STR2c</see>
/// <body>
inline const type *c2STR(cell c)     { P5("c2STR*"); ASSERT(IsSTR(c)); return type::Serial((tserial)(cell2int(c) & NTAG_MASK)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2VARS </name>
/// <overview>VARタグのついたセルから型のシリアル番号を取り出します。</overview>
/// <desc>
/// <p>
/// VARタグのついたセルから型のシリアル番号(tserial)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> tserialオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>VARSc2</see>
/// <body>
inline tserial c2VARS(cell c)        { P5("c2VAR");  ASSERT(IsVAR(c)); return              (tserial)(cell2int(c) & NTAG_MASK) ; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2VAR </name>
/// <overview>VARタグのついたセルから型へのポインタを取り出します。</overview>
/// <desc>
/// <p>
/// VARタグのついたセルから型(type)へのポインタを取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> typeオブジェクトへのポインタを返します。 </retval>
/// <remark></remark>
/// <see>VAR2c</see>
/// <body>
inline const type *c2VAR(cell c)     { P5("c2VAR*"); ASSERT(IsVAR(c)); return type::Serial((tserial)(cell2int(c) & NTAG_MASK)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2STRSorVARS </name>
/// <overview>STRもしくはVARタグのついたセルから型のシリアル番号を取り出します。</overview>
/// <desc>
/// <p>
/// STRもしくはVARタグのついたセルから型のシリアル番号(tserial)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> tserialオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline tserial c2STRSorVARS(cell c)  { P5("c2STRorVAR");  ASSERT(IsSTR(c) || IsVAR(c)); return              (tserial)(cell2int(c) & NTAG_MASK) ; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2STRorVAR </name>
/// <overview>STRもしくはVARタグのついたセルから型へのポインタを取り出します。</overview>
/// <desc>
/// <p>
/// STRもしくはVARタグのついたセルから型(type)へのポインタを取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> typeオブジェクトへのポインタを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline const type *c2STRorVAR(cell c){ P5("c2STRorVAR*"); ASSERT(IsSTR(c) || IsVAR(c)); return type::Serial((tserial)(cell2int(c) & NTAG_MASK)); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2INT </name>
/// <overview>INTタグのついたセルから整数を取り出します。</overview>
/// <desc>
/// <p>
/// INTタグのついたセルから整数(mint)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> mintオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>INT2c</see>
/// <body>
inline mint   c2INT(cell c)          { P5("c2INT");  ASSERT(IsINT(c)); return (mint)(cell2int(c) << 4) >> 4; }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2FLO </name>
/// <overview>FLOタグのついたセルから実数を取り出します。</overview>
/// <desc>
/// <p>
/// FLOタグのついたセルから実数(mfloat)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> mfloatオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>FLO2c</see>
/// <body>
inline mfloat c2FLO(cell c)          { P5("c2FLO");  ASSERT(IsFLO(c)); return int2float( (cell2int(c) << TAG_WIDTH) ); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2CHR </name>
/// <overview>FLOタグのついたセルから文字を取り出します。</overview>
/// <desc>
/// <p>
/// FLOタグのついたセルから文字(mchar)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> mcharオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>CHR2c</see>
/// <body>
inline mchar  c2CHR(cell c)          { P5("c2CJR");  ASSERT(IsCHR(c)); return (char_int)cell2int(c); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2STG </name>
/// <overview>STGタグのついたセルから文字列を取り出します。</overview>
/// <desc>
/// <p>
/// STGタグのついたセルから文字列を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> 文字列を返します。 </retval>
/// <remark></remark>
/// <see>STG2c</see>
/// <body>
inline const char * c2STG(cell c)    { P5("c2STG"); ASSERT(IsSTG(c)); return strsymbol::Serial(cell2int(c) & NTAG_MASK)->GetString(); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////
/// <funcdef>
/// <name> c2STGS </name>
/// <overview>STGタグのついたセルから文字列のシリアル番号を取り出します。</overview>
/// <desc>
/// <p>
/// STGタグのついたセルから文字列のシリアル番号(sserial)を取り出します。
/// </p>
/// </desc>
/// <args> cellオブジェクトを渡します。</args>
/// <retval> sserialオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see>STGS2c</see>
/// <body>
inline sserial c2STGS(cell c)        { P5("c2STG"); ASSERT(IsSTG(c)); return (sserial)(cell2int(c) & NTAG_MASK); }
/// </body></funcdef>

std::ostream& operator<<(std::ostream& os, cell c);

} // namespace lilfes

#endif // __cell_h
