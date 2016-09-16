/* $Id: proc.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: proc.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>proc.h</name>
//
///  <overview>
///  <jpn>述語を扱うためのルーチン</jpn>
///  <eng>??? </eng>
///  </overview>
/// <see>proc.cpp</see>

/// <notiondef>
///
/// <name> indexing_overview </name>
/// <title> indexing </title>
/// 
/// <desc>
/// indexingとは高速化手法の１種です。述語の第1引数によって無駄なコードの実行を省いて、処理を高速にします。
/// <p> </p>
/// </desc>
/// <see>  </see>
/// </notiondef>

#ifndef __proc_h
#define __proc_h

#include "lconfig.h"
#include "bitmap.h"
#include "code.h"

#include <string>
#include <vector>

namespace lilfes {

////////////////////////////////////////
/// <classdef>
/// <name> proc_t </name>
/// <overview> 各述語が各節で呼ばれたときに実行すべき処理の情報を保持するクラス</overview>
/// <desc>
/// <p>
/// ある述語がある呼ばれ方をしたときに何をすべきかの情報が入っています。
/// ある述語の1つの節に1つのproc_tオブジェクトが対応します。
/// </p>
/// </desc>
/// <see></see>
/// <body>
struct proc_t
{
	code *cl;
  /// コンパイルした中間コード
	const type *tt;
  /// 第1引数の型です
	mint ti;
  /// 第1引数がmint型の時の整数値です。
	sserial ts;
  /// 第1引数が文字列のときのその文字列のシリアル番号です。
	bool instant;
  /// 第1引数が型であるかインスタンスであるか。<br>
  /// 例えばある述語predがあったとして、pred(int)と呼ばれたときはfalseになり、pred(10)と呼ばれたときはtrueになります。
};
/// </body></classdef>

class procedure;
class procthunk;

////////////////////////////////////////
/// <classdef>
/// <name> procpair </name>
/// <overview> procedureと対応するprocthunkを一組にしたクラス</overview>
/// <desc>
/// <p>
/// procedureと対応するprocthunkを一組にして扱うためのクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
struct procpair
{
	procedure *pc;
	procthunk *pt;
  /// procedure<var>pc</var>に対応するprocthukオブジェクトです。
};
/// </body></classdef>

////////////////////////////////////////
/// <classdef>
/// <name> procthunk </name>
/// <overview> thunkクラス</overview>
/// <desc>
/// <p>
/// procedure から述語の入り口部分だけを独立させたものです。
/// indexingを使ったり組み込み述語を作ったりするためのクラスです。
/// このクラスは抽象クラスで、このクラスのインスタンスを作ることはできません。
/// <ref>indexing_overview</ref>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class procthunk
{
protected:
	uint arity;
  /// 引数の数
public:
	procthunk(uint ar) { arity = ar;}
  /// <var>ar</var>：引数の数<br>
	virtual ~procthunk() {}
	virtual void execwork(machine &m) = 0;
  /// 実際に処理を行います。純粋仮想関数で、実際の処理はサブクラスで再定義します。
	virtual bool NeedNewTypeRevise() { return false; }
  /// 新しい型が定義されたときに情報収集が必要かどうかを表します。
	void exec(machine &m) { execwork(m); }
  /// execwork()を呼びます。
	static const char *className;
  /// そのオブジェクトのクラス名です。
	virtual const char *ClassName() { return className; }
  /// そのオブジェクトのクラス名を取得します。
};
/// </body></classdef>

////////////////////////////////////////
/// <classdef>
/// <name> procedure </name>
/// <overview> 述語クラス</overview>
/// <desc>
/// <p>
/// 述語のクラスです。<br>
/// １つの述語は、１つのprocthunkオブジェクトと複数のproc_tオブジェクトを持ちます。<br>
/// いくつのproc_tを持つかは、LiLFeSプログラム中でのその述語の呼ばれ方のうち、第１引数の異なるものがいくつあるかで決まります。<br>
/// このproc_tの配列のことをCodelistとも呼びます。(例)メソッドAddCodelist()
/// </p>
/// </desc>
/// <see></see>
/// <body>
class procedure
{
	const type *reptype;
	std::vector<proc_t> cls;
	uint arity;  /// 引数の数
	uint serialno;
	bool trace;
  /// LiLFeSの中のデバッグ機能の中でその述語をtraceするかどうかを表します。
	bool special;
	bool multifile;
	bool dynamic;

	static std::vector<procpair> serialtable;
	static _HASHMAP<tserial, procedure *> hash; /// key = procedureのreptypeのtserial  value = procedure

protected:
	void SetThunk(procthunk *);
  /// そのprocedureに対応するprocthunkオブジェクトを設定します。
	code *MakeCaller(uint ncls, Bitmap *clx, bool trace, code *failCode, const type * tracetype);
  /// <var>ncls</var>：clauseの数(Bitmap<var>clx</var>の長さ)<br>
  /// <var>clx</var>：<br>
  /// <var>trace</var>：LiLFeSのデバッグ機能を使うかどうか（デバッグしながらその述語を呼び出すコードにするかどうか）<br>
  /// <var>failCode</var>：失敗するときどこに行くかの情報<br>
  /// <var>tracetype</var>：traceの種類<BR><BR>
  /// どの節を実行するかの情報が入ったコードを生成します。<BR>
  /// 例えば述語Pがあり、Pを含んだ説が５つ(節１〜節５)あったとします。
  /// <TABLE BORDER=2 CELLPADDING=3>
  /// <TR> <TD>節１</TD><TD>P(String):- ○○○.</TD> </TR>
  /// <TR> <TD>節２</TD><TD>P("あ"):- ○×○.</TD> </TR>
  /// <TR> <TD>節３</TD><TD>P("い"):- ○××.</TD> </TR>
  /// <TR> <TD>節４</TD><TD>P("う"):- △○○.</TD> </TR>
  /// <TR> <TD>節５</TD><TD>P("え"):- △△○.</TD> </TR>
  /// </TABLE>
  /// 節１と節３と節４を順に呼ぶようなコードを生成する場合、中身が"10110"というBitmapオブジェクトを渡すと、
  /// <TABLE BORDER=2 CELLPADDING=3>
  /// <TR><TD>
  /// 節１を呼ぶ<BR>
  /// 節３を呼ぶ<BR>
  /// 節４を呼ぶ<BR>
  /// </TD></TR>
  /// </TABLE>
  /// というコードを生成します。
public:
	procedure(const type *rept, uint arity);
  /// <var>rept</var>：表現タイプ<br>
  /// <var>arity</var>：引数の数<br>
	virtual ~procedure();
		
	tserial GetKey() const { return reptype->GetSerialNo(); } // for hashtable...
  /// ハッシュ用のメソッドです。キーとしてその述語の表現タイプのシリアル番号を取得します。
	const std::string &GetName() const { return reptype->GetName(); }
  /// その述語の表現タイプの名前を取得します。
	uint GetArity() const { return arity; }
  /// 引数の数を取得します。
	uint GetSerialNo() const { return serialno; }
  /// シリアル番号を取得します。
	procthunk* GetThunk() { return SerialThunk(serialno); }
  /// そのprocedureに対応するprocthunkオブジェクトを取得します。
	void SetMultifile(bool flg = true) { multifile = flg; }
  /// そのprocedureをmultifile（複数のソースファイルで定義される可能性をもたせる）にします。 
	void SetDynamic(bool flg = true) { dynamic = flg; }
  /// そのprocedureをdynamic（ユーザーが定義を動的に変更できるように）にします。 

	bool Call(machine &m);
  /// C言語の中から述語を呼ぶときに使います。<BR>
  /// 使用法の比較（述語fatherの場合）<BR>
  /// LiLFeSプログラムの中で呼ぶとき　：　father(・・・)<BR>
  /// C言語の中で呼ぶとき　：　father.Call(・・・)<BR>
	bool Call(machine &m, FSP arg1);
  /// 
	bool Call(machine &m, FSP arg1, FSP arg2);
  /// 
	bool Call(machine &m, FSP arg1, FSP arg2, FSP arg3);
  /// 
	bool Call(machine &m, FSP arg1, FSP arg2, FSP arg3, FSP arg4);
  /// 
	bool Call(machine &m, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5);
  /// 
	bool Call(machine &m, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5, FSP arg6);
  /// 
	bool Call(machine &m, FSP args[ /*arity*/ ]);
  /// 

	void SetSpecialThunk(procthunk *p);
  /// その述語をspecialにし、procthunk<var>p</var>をその述語のprocthunkにします。
	void ClearSpecialThunk();
  /// その述語のspecialをfalseにし、procthunkを情報収集をするもの(proc_selectthunk)にします。
	virtual void AddCodelist(code *c, const type *toptype = NULL);
  /// code<var>c</var>を節リストに追加します。配列の後ろに追加されます。
	virtual void AddCodelistINT(code *c, const type *toptype, mint i);
  /// 第１引数が整数型のインスタンスの場合の、code<var>c</var>を節リストに追加します。mint<var>i</var>にはその整数値が入ります。配列の後ろに追加されます。
	virtual void AddCodelistSTG(code *c, const type *toptype, sserial ss);
  /// 第１引数が文字列型のインスタンスの場合の、code<var>c</var>を節リストに追加します。sserial<var>ss</var>にはその文字列のシリアル番号が入ります。配列の後ろに追加されます。
	virtual void AddCodelistTop(code *c, const type *toptype = NULL);
  /// code<var>c</var>を節リストに追加します。配列の先頭に追加されます。
	virtual void AddCodelistTopINT(code *c, const type *toptype, mint i);
    /// 第１引数が整数型のインスタンスの場合の、code<var>c</var>を節リストに追加します。mint<var>i</var>にはその整数値が入ります。配列の先頭に追加されます。
	virtual void AddCodelistTopSTG(code *c, const type *toptype, sserial ss);
  /// 第１引数が文字列型型のインスタンスの場合の、code<var>c</var>を節リストに追加します。sserial<var>ss</var>にはその文字列のシリアル番号が入ります。配列の先頭に追加されます。
	virtual void SetTrace(bool tr);
  /// その述語のデバッグをするかどうかを設定します。
	void Compile();
  /// <ref>indexing_overview</ref>に関する情報収集をし、適切なprocthunkをセットします。
	static procedure* New(const type *n, uint arity);
  /// 表現タイプが<var>n</var>引数の数が<var>arity</var>の述語を作ります。すでに同じものが存在すればその述語を返し、そうでなければコンストラクタを実行します。
	static procedure* New(const type *n, uint arity, procthunk *thunk);
  /// 表現タイプが<var>n</var>、引数の数が<var>arity</var>、procthunkが<var>thunk</var>の述語を作ります。
	static procedure* Search(const type *t) 
	{
		_HASHMAP<tserial, procedure *>::const_iterator it = hash.find(t->GetSerialNo());
		return (it == hash.end() ? NULL : it->second);
	}
  /// 型<var>t</var>を表現タイプとするprocedureオブジェクトを取得します。
        static procedure* Serial(uint serial) { return serialtable[serial].pc; }
  /// シリアル番号から対応するprocedureオブジェクトを取得します。
	static procthunk* SerialThunk(uint serial) { return serialtable[serial].pt; }
 /// シリアル番号から対応するprocthunkオブジェクトを取得します。
	static procedure* SearchByThunk(procthunk *pt);
  /// procthunk<var>pt</var>に対応するprocedureオブジェクトを取得します。
	static uint GetSerialCount() { return serialtable.size(); }
  /// いくつのシリアル番号が振られているかを調べます。
	static void NewTypeNotify();
  /// 新しい型が追加されるとこのメソッドが実行され、NeedNewTypeRevise()がtrueを返すprocthunkをもつ述語について、そのprocthunkを情報収集用のproc_selectthunkにします。
	static void terminate();
  /// 終了処理です。メモリを解放します。
};
/// </body></classdef>

////////////////////////////////////////
/// <classdef>
/// <name> proc_builtin </name>
/// <overview> 組み込み述語クラス</overview>
/// <desc>
/// <p>
/// 組み込みの述語に特化した procedure です。ユーザー定義の節とちがい再定義できないようにしています。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_builtin : public procedure
{
public:
	proc_builtin(const type *name, uint arity, procthunk *thunk);
  /// <var>name</var>：表現タイプ<br>
  /// <var>arity</var>：引数の数<br>
  /// <var>thunk</var>：対応するprocthunk<br>
	virtual void AddCodelist(code *c, const type *toptype = NULL);
	virtual void AddCodelistINT(code *c, const type *toptype, mint i);
	virtual void AddCodelistSTG(code *c, const type *toptype, sserial ss);
	virtual void AddCodelistTop(code *c, const type *toptype = NULL);
	virtual void AddCodelistTopINT(code *c, const type *toptype, mint i);
	virtual void AddCodelistTopSTG(code *c, const type *toptype, sserial ss);
	virtual void SetTrace(bool tr);
};
/// </body></classdef>

inline procedure* procedure::New(const type *t, uint arity) 
{
	_HASHMAP<tserial, procedure *>::const_iterator it = hash.find(t->GetSerialNo());
	return (it == hash.end() ? new procedure(t, arity) : it->second);
}
inline procedure* procedure::New(const type *t, uint arity, procthunk *thunk)
{
	_HASHMAP<tserial, procedure *>::const_iterator it = hash.find(t->GetSerialNo());
	if (it == hash.end())
	{
		return new proc_builtin(t, arity, thunk);
	}
	else
	{
		it->second->SetThunk(thunk);
		return it->second;
	}
}

/* void CompileProcedures(); */

} // namespace lilfes

#endif // __proc_h
