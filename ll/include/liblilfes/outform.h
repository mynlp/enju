/* $Id: outform.h,v 1.3 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */
///  <ID>$Id: outform.h,v 1.3 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>outform.h</name>
//
///  <overview>
///  <jpn>素性構造を見やすくレイアウトして表示するための ルーチン </jpn>
///  <eng>outform routine </eng>
///  </overview>
#ifndef __outform_h
#define __outform_h

#include "lconfig.h"
#include "basic.h"

#include <ostream>
#include <string>

namespace lilfes {

////////////////////////////////////////
//
//  class outform
//
//  Used for formatting the feature structure.
/// <classdef>
/// <name> outform </name>
/// <overview> 素性構造をレイアウトして表示します。</overview>
/// <desc>
/// <p>
/// outformクラスの中身は文字列の配列です。レイアウトをするために空き空間にあたる部分は半角スペースで埋めています。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class outform
{
	int x, y;
	std::string *data;
	
public:
		outform();
		outform(const std::string &s);
  /// <var>s</var>：表示する中身の文字列<br>
		outform(const outform &o);
  /// outform<var>o</var>と同じ中身のoutformオブジェクトを作ります。
		~outform();

	outform& operator=(const outform &);
  /// 中身をコピーします。
	void SetString(const std::string &s);
  /// 中身に文字列をセットします。
	void AddAtBottom(const outform &);
  /// 下に文字列を追加します。左右方向の位置は左寄りになります。
	void AddAtRight(const outform &);
  /// 右に文字列を追加します。上下方向の位置は中心にきます。
	void AddAtRightTop(const outform &);
  /// 右上に文字列を追加します。上が揃います。
	void AddAtRightBottom(const outform &);
  /// 右下に文字列を追加します。下が揃います。
	void SurroundByBracket(const char * lt, const char * lm, const char * lb, const char * lx, const char * rt, const char * rm, const char * rb, const char * rx);
  /// 文字列  lt,lm,lb,lx,rt,rm,rb,rx  で囲みます。<br>
  /// ○中身が0行の時は、<br>
  /// ○中身が1行の時は、lxとrxで囲みます。<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;（例）中身がhogehogeで、lx = [ , rx = ] のとき、中身は[hogehoge]に変わります。<br>
  /// ○中身が2行以上の時は、lt,lm,lb,rt,rm,rbで囲みます。<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;（例）中身が<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hogehoge1<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hogehoge2<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hogehoge3<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hogehoge4<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;で、lt = [ , lm = | , lb = &lt;, rt = ] ,rm = | , rb = &gt;, のとき、中身は<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[hogehoge1]<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|hogehoge2|<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|hogehoge3|<br>
  /// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&lt;hogehoge4&gt;<br>
  /// に変わります。<br>
  /// <br>
	bool IsEmpty() const { return y==0; }
  /// 中身がからかどうかを調べます。
	friend std::ostream& operator<<(std::ostream&, outform);
  /// 内容を表示します。
};
/// </body></classdef>

} // namespace lilfes

#endif // __outform_h
