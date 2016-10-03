/* $Id: bitmap.h,v 1.5 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//
///  <ID>$Id: bitmap.h,v 1.5 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>bitmap.h</name>
//
///  <overview>
///  <jpn>ビットマップ操作</jpn>
///  <eng>Bitmap routines </eng>
///  </overview>

#ifndef __bitmap_h
#define __bitmap_h

#include "lconfig.h"
#include "errors.h"
#include <iostream>
#include <ostream>

namespace lilfes {

//////////////////////////////////////////////////////////////////////////////
//
// class Bitmap
//
/// <classdef>
/// <name> Bitmap </name>

/// <overview></overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>

/// <body>
class Bitmap
{
	unsigned size;
	int32 *ar;
	
public:
	Bitmap(unsigned is);
	Bitmap(const Bitmap &p);
	~Bitmap() { delete[] ar; }
	Bitmap & operator =(const Bitmap &);

	void Set(unsigned b)   { ASSERT( size > b ); ar[b/32] |=   (int32)1<<(b%32); }
	void Reset(unsigned b) { ASSERT( size > b ); ar[b/32] &= ~((int32)1<<(b%32)); }
	bool Test(unsigned b) const { ASSERT( size > b ); return ar[b/32] & ((int32)1<<(b%32)) ? true : false; }
	void Merge(const Bitmap &b);

	bool IsAllZero() const;

	bool Ident(const Bitmap &b) const;
  /// 引数に指定されたBitmap オブジェクト <var>b</var> の中身が自分と同じか調べます
  /// <br>同じなら<var>true</var>をそうでなければ<var>false</var>を返します

	void Output(std::ostream &o) const;
};
/// </body></classdef>

} // namespace lilfes

#endif // __bitmap_h
