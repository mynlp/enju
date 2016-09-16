/* $Id: initial.h,v 1.4 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */
//
///  <ID>$Id: initial.h,v 1.4 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION>MakLib Code Library</COLLECTION>
//
///  <name>initial.h</name>
//
///  <overview>
///  <jpn>初期化/終了処理</jpn>
///  <eng>Initialization and Termination</eng>
///  </overview>

#ifndef __initial_h
#define __initial_h

#include "lconfig.h"

namespace lilfes {
	
/// <notiondef>
///
/// <name> initial_overview </name>
/// <title> 初期化/終了処理ルーチン </title>
/// 
/// <desc>
/// 
/// <p> <ref>initial.h</ref> には、プログラムの起動時や終了時に処理を行わせるためのライブラリが入っています。</p>
/// <ul>
/// <li> モジュールをリンクするだけで、そのモジュールに必要な初期化/終了の処理を行わせることができます。
/// <li> 初期化/終了の順序を指定できます。通常の大域オブジェクトのコンストラクタ/デストラクタを使った処理では
///      順序の指定が事実上不可能でした。
/// <li> 初期化/終了の間に起こる例外を捕捉できます。
/// </ul>
/// <p>使い方は下のサンプルをご覧ください。</p>
/// 
/// <samp>
/// #include "initial.h"<br>
/// <br>
/// int main()<br>
/// {<br>
/// 　　<ref>Initializer</ref>::performAll();<br>
/// <br>
/// 　　... (プログラム本体)
/// <br>
/// 　　<ref>Terminator</ref>::performAll();<br>
/// }<br>
/// <br>
/// <br>
/// class InitializeHoge : private Initializer<br>
/// {<br>
/// 　　virtual void work();<br>
///	public:<br>
/// 　　InitializeHoge() : Initializer(100) { }<br>
/// };<br>
/// static InitializeHoge initializerobj;<br>
/// void InitializeHoge::work()<br>
/// {<br>
/// 　　... (初期化処理)<br>
/// }<br>
/// <br>
/// class TerminateHoge : private Terminator<br>
/// {<br>
/// 　　virtual void work();<br>
///	public:<br>
/// 　　TerminateHoge() : Terminator(100) { }<br>
/// };<br>
/// static TerminateHoge Terminatorobj;<br>
/// void TerminateHoge::work()<br>
/// {<br>
/// 　　... (終了処理)<br>
/// }<br>
/// <br>

/// </samp>

/// </desc>
/// <see> Initializer, Terminator</see>
/// </notiondef>

////////////////////////////////////////
//
//  Base class of Initializer and Terminator
//
/// <classdef>
/// <name> InitTermBase </name>

/// <overview> <jpn>初期化/終了処理ベースクラス</jpn><eng>Base class for initialization/termination</eng> </overview>
/// <desc>
/// <jpn>
/// <p>
/// <ref>Initializer</ref> クラスと <ref>Terminator</ref> クラスの共通の抽象基底クラスです。
/// 初期化と終了で必要なルーチンを共通化させるために作られています。ユーザーが直接操作することはありません。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// </p>
/// </eng>
/// </desc>
/// <see> Initializer, Terminator </see>

/// <body>
class InitTermBase
{
	InitTermBase *next;
protected:
	int priority;

private:
	virtual void Work() = 0;

public:
	InitTermBase(InitTermBase **table, int ip);
	virtual ~InitTermBase() {}

	static void PerformAll(InitTermBase *table);
};
/// </body></classdef>


////////////////////////////////////////
//
//  General Initializer/Terminator
//
/// <classdef>
/// <name> Initializer </name>

/// <overview> <jpn>初期化処理ベースクラス</jpn><eng>Base class for initialization</eng> </overview>
/// <desc>
/// <jpn>
/// <p>
/// 初期化処理のための抽象クラスです。このクラスを継承して、Work() に初期化で必要な処理を記述してください。
/// </p>
/// <p>
/// 単に関数を呼ぶだけならば <ref>InitFunction</ref> が便利です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// </p>
/// </eng>
/// </desc>
/// <see> Terminator, InitFunction </see>

/// <body>
class Initializer : private InitTermBase
{
private:
	static InitTermBase *initable;

	virtual void Work() = 0;
  /// 初期化処理の実行時にこの関数が呼ばれます。
public:
	Initializer(int ip=100);
  /// 初期化テーブルにこの初期化関数を登録します。
  /// 実際の初期化 (Work() の呼び出し) は PerformAll() が呼ばれるまで行われません。
  /// <var>priority</var> は優先値です。優先値が小さいものから先に初期化されます。
  /// 同じ優先順位のものが複数あった場合は順序は不定です。
	static void PerformAll();
  /// 初期化テーブルに登録されたすべての初期化関数を、優先値の小さいものから順に実行します。
};
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> Terminator </name>

/// <overview> <jpn>終了処理ベースクラス</jpn><eng>Base class for initialization</eng> </overview>
/// <desc>
/// <jpn>
/// <p>
/// 終了時処理のための抽象クラスです。このクラスを継承して、Work() に終了時に必要な処理を記述してください。
/// </p>
/// <p>
/// 単に関数を呼ぶだけならば <ref>TermFunction</ref> が便利です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// </p>
/// </eng>
/// </desc>
/// <see> Initializer, TermFunction </see>

/// <body>

class Terminator : private InitTermBase
{
private:
	static InitTermBase *trmtable;

	virtual void Work() = 0;
  /// 終了処理の実行時にこの関数が呼ばれます。
public:
	Terminator(int ip=100);
  /// 終了時処理テーブルにこの終了時関数を登録します。
  /// 実際の終了時処理 (Work() の呼び出し) は PerformAll() が呼ばれるまで行われません。
  /// <var>priority</var> は優先値です。優先値が大きいものから先に終了時処理されます。
  /// 同じ優先順位のものが複数あった場合は順序は不定です。
	static void PerformAll();
  /// 終了時処理テーブルに登録されたすべての終了時関数を、優先値の大きいものから順に実行します。
};
/// </body></classdef>


////////////////////////////////////////
//
//  Initializer/Terminator calls function
//
/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> InitFunction </name>

/// <overview> <jpn>初期化関数呼び出しクラス</jpn><eng>Initialization function caller</eng> </overview>
/// <desc>
/// <jpn>
/// <p>
/// Initializer から継承されたクラスです。初期化処理 Work() として指定された関数を呼び出します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// </p>
/// </eng>
/// </desc>
/// <see> Initializer, TermFunction </see>

/// <body>

class InitFunction : private Initializer 
{ 
	void (*func)();
	virtual void Work(); 
#ifdef DEBUG
	char *file;
#endif
public: 
	InitFunction(void (*f)(), int ip=100);
  /// 初期化テーブルにこの初期化関数を登録します。
  /// 実際の初期化処理 Work() では、指定された関数 <var>f</var> が呼び出されます。<var>f</var> は引数なしで
  /// void を返すような関数でなければいけません。<var>priority</var> が小さいものが先に実行されます。
}; 
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> TermFunction </name>

/// <overview> <jpn>終了時関数呼び出しクラス</jpn><eng>Initialization function caller</eng> </overview>
/// <desc>
/// <jpn>
/// <p>
/// Terminator から継承されたクラスです。終了処理 Work() として指定された関数を呼び出します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// </p>
/// </eng>
/// </desc>
/// <see> Terminator, InitFunction </see>

/// <body>

class TermFunction : private Terminator
{ 
	void (*func)();
	virtual void Work(); 
#ifdef DEBUG
	char *file;
#endif
public: 
	TermFunction(void (*f)(), int ip=100);
  /// 終了時処理テーブルにこの初期化関数を登録します。
  /// 実際の終了時処理 Work() では、指定された関数 <var>f</var> が呼び出されます。<var>f</var> は引数なしで
  /// void を返すような関数でなければいけません。<var>priority</var> が大きいものが先に実行されます。
}; 
/// </body></classdef>


#if 0
////////////////////////////////////////
//
//  Initializer/Terminator making a instance
//
/// <classdef>
/// <name> InitTermInstance </name>

/// <overview>
///<jpn></jpn>
///<eng></eng>
///</overview>
/// <desc>
/// <jpn>
/// <p>
/// 
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// </p>
/// </eng>
/// </desc>
/// <see> Terminator, InitFunction </see>

/// <body>

template <class T>
class InitTermInstance
{
	class InitInstance : private Initializer
	{
		T *&p;
		virtual void Work() { p = new T; }
	public: 
		InitInstance(T *&p, int ip) : Initializer(ip) { }
	}; 
	class TermInstance : private Terminator
	{
		T *&p;
		virtual void Work(); 
	public: 
		TermInstance(T *&p, int ip);
	}; 

	
};
/// </body></classdef>
#endif

} // namespace lilfes 

#endif
