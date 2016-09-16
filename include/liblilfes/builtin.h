/* $Id: builtin.h,v 1.16 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: builtin.h,v 1.16 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name> builtin.h </name>
///  <overview>
///  <jpn>組み込み述語</jpn>
///  </overview>
///

/// <notiondef>
/// <name> builtin_overview </name>
/// <title> builtinルーチン </title>
/// 
/// <desc>
/// 組み込み述語や、C++言語中で組み込み述語を作るときに使用するマクロが定義されています。
/// </p>
/// </desc>
/// </notiondef>

#ifndef __builtin_h
#define __builtin_h 

#include "lconfig.h"
#include "ftype.h"
#include "initial.h"
#include "proc.h"
#include "machine.h"
#include "structur.h"

namespace lilfes {

const int MAXARGS = 32;

#define dummyturbo(X) X

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_list </name><overview>list型,cons型,nil型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// lilfesでリスト構造を表現するのに使われる型です。<BR>
/// 型階層は以下のようになっています。
/// <pre>
/// (cons hd\ tl\)   nil
///  |                |
///  ------------------
///           |
///         t_list
///           |
///          bot
/// </pre>
/// cons型は、hd\型とtl\型の素性を持ちます。
/// 1つのconsがリストの1つの要素に対応し、hd\型の素性にはその要素の値が、tl\型の素性には次の要素(非終端ならcons、終端ならnil)を示す情報が入ります。

/// 例えばリスト[1,2,3]は次のように表現されます。
/// <pre>
/// cons     -> cons     -> cons     -> nil
///  hd\ 1   |   hd\ 2   |   hd\ 3   |
///  tl\ ----    tl\ ----    tl\ ----
/// </pre>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_list, * cons, * nil;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> hd </name><overview>head型,tail型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// head型,tail型を表す変数です。<ref>t_list</ref>を参照してください。
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * hd, * tl;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cut </name><overview>"!"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// カット型を表す変数です。LiLFeSプログラム中の型"!"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cut;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_equal </name><overview>"="型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "="型を表す変数です。LiLFeSプログラム中の型"="に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_equal;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_means </name><overview>":-"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ":-"型を表す変数です。LiLFeSプログラム中の型":-"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_means;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_query </name><overview>"?-"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "?-"型を表す変数です。LiLFeSプログラム中の型"?-"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_query;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_not </name><overview>"\+"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "\+"型を表す変数です。LiLFeSプログラム中の型"\+"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_not;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_notuni </name><overview>"\="型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "\="型を表す変数です。LiLFeSプログラム中の型"\="に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_notuni;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_defclause </name><overview>"defclause"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "defclause"型を表す変数です。LiLFeSプログラム中の型"defclause"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_defclause;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> pred </name><overview> 述語のsupertype </overview>
/// <desc><jpn>
/// <p>
/// すべての述語のsupertypeとなる型です。
/// pred型のsubtypeとして<ref>t_pred[MAXARGS]</ref>が存在します。
/// 実際には引数の数ごとにt_pred[i] (iは引数の数)のsubtypeとして各述語を定義します。
/// </p>
/// </jpn></desc>
/// <body>
extern       type * pred;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_pred[MAXARGS] </name><overview> 引数の数ごとに述語型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// <ref>pred</ref>型のsubtypeです。<BR>
/// 引数を１つだけとる述語はt_pred[1],引数を２つだけとる述語はt_pred[2]のsubtypeになります。
/// </p>
/// </jpn></desc>
/// <body>
extern       type * t_pred[MAXARGS];
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> f_arg[MAXARGS] </name><overview>素性"arg1","arg2"..."argMAXARGS"を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 述語の引数を表す素性"arg1","arg2"..."argMAXARGS"、を表す変数）です。LiLFeSプログラム中の素性"arg1","arg2"..."argMAXARGS"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * f_arg[MAXARGS];
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> f_chead </name><overview>素性"chead"を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 節のheadを表す素性"chead"を表す変数です。LiLFeSプログラム中の素性"chead"に対応します。<BR>
/// 例えば<BR>
/// a(X, Y) :- b(X), !, X = Y.<BR>
/// というプログラムがあったとき、述語":-"の素性"chead"は、"a(X,Y)"になります。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * f_chead;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> f_cbody </name><overview>素性"cbody"を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 節のbodyを表す素性"cbody"を表す変数です。LiLFeSプログラム中の素性"cbody"に対応します。<BR>
/// 例えば<BR>
/// a(X, Y) :- b(X), !, X = Y.<BR>
/// というプログラムがあったとき、述語":-"の素性"cbody"は、"b(X), !, X = Y."になります。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * f_cbody;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> f_lilfesfileinfo </name><overview>素性"lilfesfileinfo"を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 素性"lilfesfileinfo"を表す変数です。
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * f_lilfesfileinfo;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> f_lilfesfilename </name><overview>素性"lilfesfilename"を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 素性"lilfesfilename"を表す変数です。
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * f_lilfesfilename;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> f_lilfesfileline </name><overview>素性"lilfesfileline"を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 素性"lilfesfileline"を表す変数です。
/// </p>
/// </jpn></desc>
/// <body>
extern const feature * f_lilfesfileline;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_int </name><overview> "integer"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// lilfes言語で"integer"型を表す変数です。全ての整数はこの型のサブタイプになります。
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_int;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_flo </name><overview> "float"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// lilfes言語で"float"型を表す変数です。全ての浮動小数点の数値はこの型のサブタイプになります。
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_flo;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_chr </name><overview> "char"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// lilfes言語で"char"型を表す変数です。全ての文字はこの型のサブタイプになります。<BR>
/// この型は現在使われていません。
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_chr;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_stg </name><overview> "string"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// lilfes言語で"string"型を表す変数です。全ての文字列はこの型のサブタイプになります。
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_stg;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_feature </name><overview> 素性型を表す型 </overview>
/// <desc><jpn>
/// <p>
/// 素性を表す全ての型は`feature'型のサブタイプになっています。t_featureは`feature'型を格納した変数です。
/// </p>
/// </jpn></desc>
/// <body>
extern       type * t_feature;
/// </body></vardef>


// types in fariths.cpp

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_comma </name><overview> ","型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ","型を表す変数です。LiLFeSプログラム中の型","に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_comma;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_semicolon </name><overview> ";"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ";"型を表す変数です。LiLFeSプログラム中の型";"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_semicolon;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_ifthen </name><overview> "->"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "->" 型を表す変数です。LiLFeSプログラム中の型"->"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_ifthen;
/// </body></vardef>



/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_plus </name><overview> "+" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "+" 型を表す変数です。LiLFeSプログラム中の型"+"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_plus;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_minus </name><overview> "-" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
///  "-" 型を表す変数です。LiLFeSプログラム中の型"-"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_minus;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_aster </name><overview> "*" を表す型 </overview>
/// <desc><jpn>
/// <p>
/// "*" 型を表す変数です。LiLFeSプログラム中の型"*"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_aster;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_slash </name><overview> "/" を表す型 </overview>
/// <desc><jpn>
/// <p>
/// "/" 型を表す変数です。LiLFeSプログラム中の型"/"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_slash;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_intdiv </name><overview> "//" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "//" 型を表す変数です。LiLFeSプログラム中の型"//"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_intdiv;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_modulo </name><overview> 剰余演算記号"mod" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "mod" 型を表す変数です。LiLFeSプログラム中の型"mod"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_modulo;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cmpl </name><overview> 比較演算子 "<" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 比較演算子 "<" 型を表す変数です。LiLFeSプログラム中の型"<"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cmpl;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cmple </name><overview> 比較演算子 "<=" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 比較演算子 "<="型を表す変数です。LiLFeSプログラム中の型"<="に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cmple;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cmpg </name><overview> 比較演算子 ">" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 比較演算子 ">" 型を表す変数です。LiLFeSプログラム中の型">"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cmpg;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cmpge </name><overview> 比較演算子 ">=" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 比較演算子 ">=" 型を表す変数です。LiLFeSプログラム中の型">="に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cmpge;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cmpeq </name><overview> 比較演算子 "=:=" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 比較演算子 "=:= "型を表す変数です。LiLFeSプログラム中の型"=:="に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cmpeq;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_cmpne </name><overview> 比較演算子 "=\=" 型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// 比較演算子 "=\=" 型を表す変数です。LiLFeSプログラム中の型"=\="に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_cmpne;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_circm </name><overview> "^" 型を表す変数 </overview>
/// <desc><jpn>
/// <p> 
/// "^" 型を表す変数です。LiLFeSプログラム中の型"^"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_circm;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_is </name><overview> "is"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// "is"型を表す変数です。LiLFeSプログラム中の型"is"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_is;
/// </body></vardef>



/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_bitand </name><overview> ビット演算のAND演算子"/\\"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ビット演算のAND演算子"/\\"型を表す変数です。LiLFeSプログラム中の型"/\\"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_bitand;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_bitor </name><overview> ビット演算のOR演算子"\/"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ビット演算のOR演算子"\/"型を表す変数です。LiLFeSプログラム中の型"\/"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_bitor;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_bitxor </name><overview> ビット演算のXOR演算子"#"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ビット演算のXOR演算子"#"型を表す変数です。LiLFeSプログラム中の型"#"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_bitxor;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_bitnot </name><overview> ビット演算のNOT演算子"\"型を表す変数 </overview>
/// <desc><jpn>
/// <p>
/// ビット演算のNOT演算子"\"型を表す変数です。LiLFeSプログラム中の型"\"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_bitnot;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> t_macro </name><overview> ":-"を表す型 </overview>
/// <desc><jpn>
/// <p>
/// ":-"を表す型です。LiLFeSプログラム中の型":-"に対応します。<BR>
/// </p>
/// </jpn></desc>
/// <body>
extern const type * t_macro;
/// </body></vardef>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// halt/1 が呼ばれた時に投げられる例外です．
class halt_exception {
private:
  machine* mach;
  int return_code;

public:
  explicit halt_exception( machine* m, int c = 0 ) : mach( m ), return_code( c ) {}
  virtual ~halt_exception() {}

  machine* GetMachine() const { return mach; }
  int GetReturnCode() const { return return_code; }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// 各ファイルの組み込み述語を参照している変数です
// lilfesプログラムから組み込み述語を使うときは，この変数への参照を入れてください
extern void* BUILTIN_PRED_SYMBOLS[];

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace builtin {

  // defined in builtin.cpp
  bool dynamic( machine&, FSP );
  bool is_dynamic( machine&, FSP );
  bool multifile( machine&, FSP );
  bool is_multifilx( machine&, FSP );
  bool disp_result( machine&, FSP );
  bool findall( machine&, FSP, FSP, FSP );
  bool findall_4( machine&, FSP, FSP, FSP, FSP );
  bool lilfes_assert( machine&, FSP );
  bool lilfes_asserta( machine&, FSP );
  bool lilfes_version( machine&, FSP );
  bool lilfes_eval( machine&, FSP, FSP );         // lilfes builtin "eval"
  bool lilfes_eval_3( machine&, FSP, FSP, FSP );  // eval_with_varname
  bool lilfes_call( machine&, FSP );              // lilfes builtin "call"

  // defined in builtinfs.cpp
  bool copy( machine&, FSP, FSP );
  bool normalize( machine&, FSP, FSP );
  bool canonical_copy( machine&, FSP, FSP );
  bool unifiable( machine&, FSP, FSP );
  bool isshared( machine&, FSP, FSP );
  bool isnotshared( machine&, FSP, FSP );
  bool b_equiv( machine&, FSP, FSP );
  core_p recopy(core_p, machine&);
  bool recopy( machine&, FSP, FSP );
  bool identical( machine&, FSP, FSP );
  bool not_identical( machine&, FSP, FSP );
  bool compound( machine&, FSP );
  bool simple( machine&, FSP );
  bool follow( machine&, FSP, FSP, FSP );
  bool follow_if_exists( machine&, FSP, FSP, FSP );
  bool restriction( machine&, FSP, FSP );
  bool overwrite( machine&, FSP, FSP, FSP );
  bool subnodelist( machine&, FSP, FSP );
  bool subnodelist_count( machine&, FSP, FSP );
  bool sharednodelist( machine&, FSP, FSP );
  bool contains( machine&, FSP, FSP );
  bool equivalent( machine&, FSP, FSP );
  bool not_equivalent( machine&, FSP, FSP );
  bool subsume( machine&, FSP, FSP );
  bool type_equal( machine&, FSP, FSP );
  bool type_subsume( machine&, FSP, FSP );
  bool type_toptype( machine&, FSP, FSP );
  bool type_copy( machine&, FSP, FSP );
  bool type_unify( machine&, FSP, FSP );
  bool type_common( machine&, FSP, FSP, FSP );
  bool type_nfeature( machine&, FSP, FSP );
  bool type_featurelist( machine&, FSP, FSP );
  bool type_havefeature( machine&, FSP, FSP );
  bool type_subtypes( machine&, FSP, FSP );
  bool type_nsubtypes( machine&, FSP, FSP );
  bool type_supertypes( machine&, FSP, FSP );
  bool type_nsupertypes( machine&, FSP, FSP );
  bool type_unifiables( machine&, FSP, FSP );
  bool type_nunifiables( machine&, FSP, FSP );
  bool type_directsubtypes( machine&, FSP, FSP );
  bool type_directsupertypes( machine&, FSP, FSP );
  bool fs_compare( machine&, FSP, FSP, FSP );

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define LILFES_BUILTIN_PRED_0(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_0(func, name, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_1(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_1(func, name, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_2(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_2(func, name, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_3(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_3(func, name, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_4(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_4(func, name, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_5(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_5(func, name, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_6(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_6(func, name, lilfes::module::BuiltinModule())

#define LILFES_BUILTIN_DIRECTIVE_0(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_0(func, name, lilfes::module::DirectiveModule())
#define LILFES_BUILTIN_DIRECTIVE_1(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_1(func, name, lilfes::module::DirectiveModule())
#define LILFES_BUILTIN_DIRECTIVE_2(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_2(func, name, lilfes::module::DirectiveModule())
#define LILFES_BUILTIN_DIRECTIVE_3(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_3(func, name, lilfes::module::DirectiveModule())
#define LILFES_BUILTIN_DIRECTIVE_4(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_4(func, name, lilfes::module::DirectiveModule())
#define LILFES_BUILTIN_DIRECTIVE_5(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_5(func, name, lilfes::module::DirectiveModule())
#define LILFES_BUILTIN_DIRECTIVE_6(func, name) LILFES_BUILTIN_PRED_WITH_MODULE_6(func, name, lilfes::module::DirectiveModule())

#define LILFES_BUILTIN_PRED_OVERLOAD_0(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_0(func, name, parent, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_OVERLOAD_1(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_1(func, name, parent, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_OVERLOAD_2(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_2(func, name, parent, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_OVERLOAD_3(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_3(func, name, parent, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_OVERLOAD_4(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_4(func, name, parent, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_OVERLOAD_5(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_5(func, name, parent, lilfes::module::BuiltinModule())
#define LILFES_BUILTIN_PRED_OVERLOAD_6(func, name, parent) LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_6(func, name, parent, lilfes::module::BuiltinModule())

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_0 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_0(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(0) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine &);				\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[0]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 0, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS0(mach)								\
            if( execwork_internal(mach) ){  return ; }					\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach)                         \
        { \
          return func(mach); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_1 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_1(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(1) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine & , FSP);			\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[1]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 1, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS1(mach,arg1)							\
            if( execwork_internal(mach,arg1) ){  return ; }				\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach , FSP arg1) \
        { \
          return func(mach, arg1); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_2 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_2(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(2) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine & , FSP ,FSP);			\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[2]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 2, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS2(mach,arg1,arg2)							\
            if( execwork_internal(mach,arg1,arg2) ){  return ; }			\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach , FSP arg1 , FSP arg2) \
        { \
          return func(mach, arg1, arg2); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_3 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_3(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(3) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine & , FSP , FSP , FSP);		\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[3]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 3, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS3(mach,arg1,arg2,arg3)							\
            if( execwork_internal(mach,arg1,arg2,arg3) ){  return ; }			\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach , FSP arg1 , FSP arg2 , FSP arg3) \
        { \
          return func(mach, arg1, arg2, arg3); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_4 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_4(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(4) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine & , FSP , FSP , FSP , FSP);	\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[4]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 4, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS4(mach,arg1,arg2,arg3,arg4)							\
            if( execwork_internal(mach,arg1,arg2,arg3,arg4) ){  return ; }			\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach , FSP arg1 , FSP arg2 , FSP arg3 , FSP arg4) \
        { \
          return func(mach, arg1, arg2, arg3, arg4); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_5 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_5(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(5) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine & , FSP , FSP , FSP , FSP , FSP);	\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[5]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 5, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5)							\
            if( execwork_internal(mach,arg1,arg2,arg3,arg4,arg5) ){  return ; }			\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach , FSP arg1 , FSP arg2 , FSP arg3 , FSP arg4 , FSP arg5) \
        { \
          return func(mach, arg1, arg2, arg3, arg4, arg5); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_MODULE_6 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_MODULE_6(func, name, module)					\
	class _bi_##name : public procthunk						\
	{										\
	public:										\
		_bi_##name() : procthunk(6) { }						\
		virtual void execwork(machine &);					\
		static bool execwork_internal(machine & , FSP , FSP , FSP , FSP , FSP , FSP);	\
		static void init();							\
		static const char *className;						\
		virtual const char *ClassName() { return className; }			\
	};										\
											\
	const char *_bi_##name::className = "builtin(" #name ")";				\
											\
	void _bi_##name::init()								\
	{										\
		type *t = new type(#name, (module));						\
		t->SetAsChildOf(t_pred[6]);				t->Fix();	\
		dummyturbo(new proc_builtin(t, 6, new _bi_##name()));			\
	}										\
											\
	static InitFunction INIT_##name(_bi_##name::init,100);				\
											\
	void _bi_##name::execwork(machine &mach)						\
          {										\
            NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6)							\
            if( execwork_internal(mach,arg1,arg2,arg3,arg4,arg5,arg6) ){  return ; }			\
            else{ mach.Fail();return ; }						\
	}										\
	bool _bi_##name::execwork_internal(machine &mach , FSP arg1 , FSP arg2 , FSP arg3 , FSP arg4 , FSP arg5 , FSP arg6) \
        { \
          return func(mach, arg1, arg2, arg3, arg4, arg5, arg6); \
        }
/// </body></macrodef>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_0 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_0(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(0) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &);						\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[0]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 0, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+0);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS0(mach)									\
          if( execwork_internal(mach) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach) { \
          return func( mach ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_1 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_1(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(1) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &, FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[1]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 1, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+1);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS1(mach,arg1)									\
          if( execwork_internal(mach, arg1) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach, FSP arg1) { \
          return func( mach, arg1 ); \
        }
/// </body></macrodef>
          
/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_2 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_2(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(2) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &, FSP, FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[2]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 2, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+2);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS2(mach,arg1,arg2)									\
          if( execwork_internal(mach, arg1, arg2) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach, FSP arg1, FSP arg2) { \
          return func( mach, arg1, arg2 ); \
        }
/// </body></macrodef>
          
/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_3 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_3(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(3) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &, FSP, FSP, FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[3]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 3, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+3);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS3(mach,arg1,arg2,arg3)									\
          if( execwork_internal(mach, arg1, arg2, arg3) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach, FSP arg1, FSP arg2, FSP arg3) { \
          return func( mach, arg1, arg2, arg3 ); \
        }
/// </body></macrodef>
          
/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_4 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_4(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(4) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &, FSP, FSP, FSP, FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[4]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 4, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+4);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS4(mach,arg1,arg2,arg3,arg4)									\
          if( execwork_internal(mach, arg1, arg2, arg3, arg4) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach, FSP arg1, FSP arg2, FSP arg3, FSP arg4) { \
          return func( mach, arg1, arg2, arg3, arg4 ); \
        }
/// </body></macrodef>
          
/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_5 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_5(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(5) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &, FSP, FSP, FSP, FSP, FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[5]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 5, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+5);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5)									\
          if( execwork_internal(mach, arg1, arg2, arg3, arg4, arg5) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5) { \
          return func( mach, arg1, arg2, arg3, arg4, arg5 ); \
        }
/// </body></macrodef>
          
/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_6 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_OVERLOAD_WITH_MODULE_6(func, name, parent, module)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(6) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &, FSP, FSP, FSP, FSP, FSP, FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" #name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(#name, module);				\
		t->SetAsChildOf(t_pred[6]);							\
		t->SetAsChildOf(module->Search(#parent));			\
		t->Fix();									\
		dummyturbo(new proc_builtin(t, 6, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100+6);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6)									\
          if( execwork_internal(mach, arg1, arg2, arg3, arg4, arg5, arg6) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5, FSP arg6) { \
          return func( mach, arg1, arg2, arg3, arg4, arg5, arg6 ); \
        }
/// </body></macrodef>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_0</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_0(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(0) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &);					\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "0", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[0]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 0, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS0(mach)									\
          if( execwork_internal(mach) ){ return ; }						\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach) { \
          return func( mach ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_1</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_1(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(1) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP);					\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "1", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[1]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 1, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS1(mach,arg1)									\
          if( execwork_internal(mach,arg1) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1) { \
          return func( mach, arg1 ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_2</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_2(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(2) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP);				\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "2", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[2]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 2, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS2(mach,arg1,arg2)								\
          if( execwork_internal(mach,arg1,arg2) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2) { \
          return func( mach, arg1, arg2 ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_3</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_3(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(3) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP,FSP);				\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "3", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[3]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 3, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS3(mach,arg1,arg2,arg3)								\
          if( execwork_internal(mach,arg1,arg2,arg3) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2,FSP arg3) { \
          return func( mach, arg1, arg2, arg3 ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_4</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_4(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(4) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP,FSP);				\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "4", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[4]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 4, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS4(mach,arg1,arg2,arg3,arg4)								\
          if( execwork_internal(mach,arg1,arg2,arg3,arg4) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2,FSP arg3,FSP arg4) { \
          return func( mach, arg1, arg2, arg3, arg4 ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_5</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_5(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(5) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP,FSP,FSP,FSP);			\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "5", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[5]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 5, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5)								\
          if( execwork_internal(mach,arg1,arg2,arg3,arg4,arg5) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2,FSP arg3,FSP arg4,FSP arg5) { \
          return func( mach, arg1, arg2, arg3, arg4, arg5 ); \
        }
/// </body></macrodef>

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_PRED_WITH_NAME_6</name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_PRED_WITH_NAME_6(func, name, lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(6) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP,FSP,FSP,FSP,FSP);		\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";				\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "6", lilfes::module::CoreModule()); s->SetAsChildOf(t); 		\
				s->SetAsChildOf(t_pred[6]);				s->Fix();	\
		dummyturbo(new proc_builtin(s, 6, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6)								\
          if( execwork_internal(mach,arg1,arg2,arg3,arg4,arg5,arg6) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2,FSP arg3,FSP arg4,FSP arg5,FSP arg6) { \
          return func( mach, arg1, arg2, arg3, arg4, arg5, arg6 ); \
        }
/// </body></macrodef>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_ARITH_2 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_ARITH_2(func,name,lilfes_name)				\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(2) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP);				\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";					\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "1", lilfes::module::CoreModule()); s->SetAsChildOf(t);		\
				s->SetAsChildOf(t_pred[1]);			s->Fix();	\
		type *r = new type(lilfes_name "/" "2", lilfes::module::CoreModule()); r->SetAsChildOf(s); 	\
				r->SetAsChildOf(t_pred[2]);				r->Fix();	\
		dummyturbo(new proc_builtin(r, 2, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS2(mach,arg1,arg2)								\
          if( execwork_internal(mach,arg1,arg2) ){ return ; }					\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2) { \
          return func( mach, arg1, arg2 ); \
        }
/// </body></macrodef>
          
/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_ARITH_3 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_ARITH_3(func,name,lilfes_name)						\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(3) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP,FSP);				\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";					\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *s = new type(lilfes_name "/" "2", lilfes::module::CoreModule()); s->SetAsChildOf(t);		\
				s->SetAsChildOf(t_pred[2]);			s->Fix();	\
		type *r = new type(lilfes_name "/" "3", lilfes::module::CoreModule()); r->SetAsChildOf(s); 	\
				r->SetAsChildOf(t_pred[3]);				r->Fix();	\
		dummyturbo(new proc_builtin(r, 3, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS3(mach,arg1,arg2,arg3)							\
          if( execwork_internal(mach,arg1,arg2,arg3) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2,FSP arg3) { \
          return func( mach, arg1, arg2, arg3 ); \
        }
/// </body></macrodef>
////////↑BUILTIN_ARITH_2,3///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
/// <macrodef>
/// <name> LILFES_BUILTIN_ARITH_UNARY_3 </name>
/// <desc>
/// </desc>
/// <body>
#define LILFES_BUILTIN_ARITH_UNARY_3(func, name, lilfes_name)				 	\
	class _bi_##name : public procthunk							\
	{											\
	public:											\
		_bi_##name() : procthunk(3) { }							\
		virtual void execwork(machine &);						\
		static bool execwork_internal(machine &,FSP,FSP,FSP);				\
		static void init();								\
		static const char *className;							\
		virtual const char *ClassName() { return className; }				\
	};											\
												\
	const char *_bi_##name::className = "builtin(" lilfes_name ")";					\
												\
	void _bi_##name::init()									\
	{											\
		type *t = new type(lilfes_name, lilfes::module::CoreModule()); t->SetAsChildOf(pred);	t->Fix();	\
		type *q = new type(lilfes_name "/" "1", lilfes::module::CoreModule()); q->SetAsChildOf(t); 		\
				q->SetAsChildOf(t_pred[1]);			q->Fix();	\
		type *s = new type(lilfes_name "/" "2", lilfes::module::CoreModule()); s->SetAsChildOf(q); 	\
				s->SetAsChildOf(t_pred[2]);			s->Fix();	\
		type *r = new type(lilfes_name "/" "3", lilfes::module::CoreModule()); r->SetAsChildOf(s); 	\
				r->SetAsChildOf(t_pred[3]);				r->Fix();	\
		dummyturbo(new proc_builtin(r, 3, new _bi_##name()));				\
	}											\
												\
	static InitFunction INIT_##name(_bi_##name::init,100);					\
												\
	void _bi_##name::execwork(machine &mach)							\
        {											\
          NGETARGS3(mach,arg1,arg2,arg3)							\
          if( execwork_internal(mach,arg1,arg2,arg3) ){ return ; }				\
          else{  mach.Fail();return ; }								\
	}											\
	bool _bi_##name::execwork_internal(machine &mach,FSP arg1,FSP arg2,FSP arg3) { \
          return func( mach, arg1, arg2, arg3 ); \
        }
/// </body></macrodef>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define NGETARGS0(mach)  P2(className);

#ifdef DOASSERT

#define NGETARGS1(mach,arg1)	NGETARGS0(mach);								\
		mach.SetRP(1); FSP arg1(mach, c2PTR(mach.ReadLocal(0)));

#define NGETARGS2(mach,arg1,arg2)	NGETARGS1(mach,arg1);							\
		mach.SetRP(2); FSP arg2(mach, c2PTR(mach.ReadLocal(1)));

#define NGETARGS3(mach,arg1,arg2,arg3)	NGETARGS2(mach,arg1,arg2);						\
		mach.SetRP(3); FSP arg3(mach, c2PTR(mach.ReadLocal(2)));

#define NGETARGS4(mach,arg1,arg2,arg3,arg4)	NGETARGS3(mach,arg1,arg2,arg3);					\
		mach.SetRP(4); FSP arg4(mach, c2PTR(mach.ReadLocal(3)));

#define NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5)	NGETARGS4(mach,arg1,arg2,arg3,arg4);			\
		mach.SetRP(5); FSP arg5(mach, c2PTR(mach.ReadLocal(4)));

#define NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6)	NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5);		\
		mach.SetRP(6); FSP arg6(mach, c2PTR(mach.ReadLocal(5)));

#define NGETARGS7(mach,arg1,arg2,arg3,arg4,arg5,arg6,arg7)	NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6);	\
		mach.SetRP(7); FSP arg7(mach, c2PTR(mach.ReadLocal(6)));

#else
#define NGETARGS1(mach , arg1)	NGETARGS0(mach);								\
		 FSP arg1(mach, c2PTR(mach.ReadLocal(0)));

#define NGETARGS2(mach,arg1,arg2)	NGETARGS1(mach,arg1);							\
		 FSP arg2(mach, c2PTR(mach.ReadLocal(1)));

#define NGETARGS3(mach,arg1,arg2,arg3)	NGETARGS2(mach,arg1,arg2);						\
		 FSP arg3(mach, c2PTR(mach.ReadLocal(2)));

#define NGETARGS4(mach,arg1,arg2,arg3,arg4)	NGETARGS3(mach,arg1,arg2,arg3);					\
		 FSP arg4(mach, c2PTR(mach.ReadLocal(3)));

#define NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5)	NGETARGS4(mach,arg1,arg2,arg3,arg4);			\
		 FSP arg5(mach, c2PTR(mach.ReadLocal(4)));

#define NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6)	NGETARGS5(mach,arg1,arg2,arg3,arg4,arg5);		\
		 FSP arg6(mach, c2PTR(mach.ReadLocal(5)));

#define NGETARGS7(mach,arg1,arg2,arg3,arg4,arg5,arg6,arg7)	NGETARGS6(mach,arg1,arg2,arg3,arg4,arg5,arg6);	\
		 FSP arg7(mach, c2PTR(mach.ReadLocal(6)));
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: make lilfes_str_flag

} // namespace lilfes

#endif // __builtin_h
