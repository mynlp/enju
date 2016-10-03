/* $Id: errors.h,v 1.10 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */
//
///  <ID>$Id: errors.h,v 1.10 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>errors.h</name>
//
///  <overview>
///  <jpn>エラー処理に関するルーチン</jpn>
///  <eng>errors </eng>
///  </overview>

//
//  LightSpeed HPSG Parser Project
//
//  errors.h - Definition of error exception handling.
//
//  **LICENSE**
//
//  Change Log:
//
//  96/11/20  Created. / mak
/// <notiondef>
/// <name> errors_overview </name>
/// <title> エラー処理ルーチン </title>
/// 
/// <desc>
/// 
/// <p> <ref>errors.h</ref> には、エラー発生時にエラー情報を表示させるためのライブラリが入っています。<br>
/// errorstream はエラー発生時にエラーの内容を出力します。<br>
/// tracestream はデバッグ用の出力で、通常はなにも出力しないようになってますが、DEBUG が定義されて、変数 DEB に true が入っていると、出力します。
/// </p>
/// </desc>
/// <see> errorstream, tracestream </see>
/// </notiondef>

#ifndef __errors_h
#define __errors_h

#include "lconfig.h"

#include "basic.h" // for string type
#include "lexer.h" 
#include <cstdlib>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

namespace lilfes {

#ifdef DEBUG
extern bool DEB;
#else
const bool DEB = false;
#endif

enum endmsg_t { endmsg };

extern std::istream* input_stream;
extern std::ostream* output_stream;
extern std::ostream* error_stream;

/// <classdef>
/// <name> tracestream </name>

/// <overview>クラス</overview>
/// <desc>
/// <p>
/// tracestream はデバッグ用の出力で、通常はなにも出力しないようになってますが、DEBUG が定義されて、変数 DEB に true が入っていると、出力します。
/// </p>
/// </desc>
/// <see></see>

/// <body>

class tracestream
{
	std::string topline;
	bool show_topline;

public:
	tracestream(std::string itopline = "") : topline(itopline), show_topline(true) { }

	template <class T>
	tracestream& operator<<(T &t)
	{
		if( DEB ) {
			if( show_topline )
			{
				*output_stream << topline;
				show_topline = false;
			}
			*output_stream << t;
		}
		return *this;
	}
	template <class T>
	tracestream& operator<<(const T &t)
	{
		if( DEB )
		{
			if( show_topline )
			{
				*output_stream << topline;
				show_topline = false;
			}
			*output_stream << t;
		}
		return *this;
	}
	void operator<< (const endmsg_t &)
	{
		if( DEB )
		{
			*output_stream << std::endl;
			show_topline = true;
		}
	}
};
/// </body></classdef>

tracestream &tracer();

#define TRACE(x) (lilfes::tracer() << x << lilfes::endmsg)

/// <classdef>
/// <name> errorstream </name>

/// <overview>クラス</overview>
/// <desc>
/// <p><br>
/// 使用例<br>
/// errorstream errout("Error: ", " at somewhere"); <br>
/// errout &lt;&lt; "Undefined something '" << name << "'" << endmsg; <br>
/// と書くと、画面に <br>
/// Error: Undefined something 'hogehoge' at somewhere\n <br>
/// が表示されます。<br>
/// </p>
/// </desc>
/// <see></see>

/// <body>

class errorstream
{

  // エラーメッセージをcerrではなくstringstreamに貯めておくための変数です。
  //static std::ostream *error_ptr;

  std::string topline;
  std::string endline;
  bool show_topline;
  bool display;

public:
	errorstream(std::string itopline = "", std::string iendline = "", bool idisplay = true) 
		: topline(itopline), endline(iendline), show_topline(true), display(idisplay) { }

	errorstream sourceinfo( const char *file, int line );
  /// <var>file</var>で与えられるファイル名と<var>line</var>で与えられる行数を出力します。
	template <class T>
	errorstream operator<<(const T &t) 
	{ 
		if( display )
		{
			if( show_topline )
			{
				(*error_stream) << topline;
				show_topline = false;
			}
			(*error_stream) << t;
		}
		return *this;
	}
	void operator<<(const endmsg_t &) 
	{
		if( display )
		{
			(*error_stream) << endline << std::endl;
			show_topline = true;
		}
	}
// 	static void SwitchErr(std::ostream *ptr) {
// 	  error_ptr = ptr;
// 	}
};
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> runerr </name>

/// <overview>実行時エラーの出力</overview>
/// <desc>
/// <jpn>
/// <p>
/// 実行時エラーを出力します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see>errorstream</see>

/// <body>
errorstream &runerr();
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> runwarn </name>

/// <overview>実行時警告の出力</overview>
/// <desc>
/// <jpn>
/// <p>
/// 実行時警告を出力します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see>errorstream</see>

/// <body>
errorstream &runwarn();
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> inerr </name>

/// <overview>読み込みエラーの出力</overview>
/// <desc>
/// <jpn>
/// <p>
/// 読み込みエラーを出力します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see>errorstream</see>
errorstream inerr(lexer_t l);
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> inwarn </name>

/// <overview>読み込み警告の出力</overview>
/// <desc>
/// <jpn>
/// <p>
/// 読み込み警告を出力します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see>errorstream</see>
errorstream inwarn(lexer_t l);
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> internalerr </name>

/// <overview>内部エラーの出力</overview>
/// <desc>
/// <jpn>
/// <p>
/// 内部エラーを出力します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see>errorstream</see>
errorstream &internalerr();
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> internalwarn </name>

/// <overview>内部警告の出力</overview>
/// <desc>
/// <jpn>
/// <p>
/// 内部警告を出力します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <see>errorstream</see>
errorstream &internalwarn();
/// </body></funcdef>


#ifdef DOASSERT
#define runerr() runerr().sourceinfo(__FILE__, __LINE__)
#define runwarn() runwarn().sourceinfo(__FILE__, __LINE__)
#define inerr(l) inerr(l).sourceinfo(__FILE__, __LINE__)
#define inwarn(l) inwarn(l).sourceinfo(__FILE__, __LINE__)
#endif
#define internalerr() internalerr().sourceinfo(__FILE__, __LINE__)
#define internalwarn() internalwarn().sourceinfo(__FILE__, __LINE__)

// transitional macros (obsolete)
#define RUNERR(e)  (runerr() << e << endmsg)
#define RUNWARN(e) (runwarn() << e << endmsg)
#define INERR(m,e)   (inerr((m)->getLexer()) << e << endmsg)
#define INWARN(m,e)  (inwarn((m)->getLexer()) << e << endmsg)
#define ABORT(x)   do { internalerr() << x << endmsg; abort();} while(false)
#define DMSG(x)   (internalwarn() << x << endmsg)

#if defined(DOASSERT)
#define ASSERT(x) if( !(x) ) { internalerr() << "Assertion failed: (" #x ")" << endmsg; abort(); } else
#else
#define ASSERT(x)
#endif


extern long int PASSCOUNT;
extern long int TRAILBACK;
extern long int Errors, Warning;

} // namespace lilfes

#endif /* __errors_h */
