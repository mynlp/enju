/* $Id: basic.h,v 1.5 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: basic.h,v 1.5 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>basic.h</name>
//
///  <overview>
///  <jpn>型定義ルーチン</jpn>
///  <eng>type define routine </eng>
///  </overview>

/// <notiondef>
/// <name> basic_overview </name>
/// <title> 型定義ルーチン </title>
/// 
/// <desc>
/// 
/// <p> <ref>basic.h</ref>ではよく使用する型や定数が定義されています。<br>
/// </p>
/// </desc>
/// <see> </see>
/// </notiondef>

#ifndef __basic_h
#define __basic_h

#include "lconfig.h"

#include <string>

// Function attribute.

#ifdef HAVE_ATTRIBUTE
#define CONSTF const __attrib__(const)
#define CALCF __attrib__(const)    /* Is this right? */
#else
#define CONSTF const
#define CALCF
#endif



// Classes and Types.

namespace lilfes {

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> Version </name>

/// <overview>バージョン文字列</overview>
/// <desc>
/// <p>
/// バージョン文字列です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern const char *Version;
/// </body></vardef>

/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> Copyright </name>

/// <overview>著作権表示</overview>
/// <desc>
/// <p>
/// 著作権文字列です。よければどっかで表示してください。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern const char *Copyright;
/// </body></vardef>



/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> 最終コンパイル日時 </name>

/// <overview>最終コンパイル日時</overview>
/// <desc>
/// <p>
/// 最後にコンパイルされた日付です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern const char *LastCompiled;
/// </body></vardef>


/////////////////////////////////////////////////////////////////////////
/// <vardef>
/// <name> コンパイルオプション </name>

/// <overview>コンパイルオプション</overview>
/// <desc>
/// <p>
/// コンパイルされたときに指定されていたオプションです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
extern const char *CompileOptions;
/// </body></vardef>


/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> ushort </name>

/// <overview>unsigned short クラス</overview>
/// <desc>
/// <p>
/// 符号無し単精度整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned short ushort;
/// </body></classdef>


/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> ulong </name>

/// <overview>unsigned long クラス</overview>
/// <desc>
/// <p>
/// 符号無し倍精度整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned long  ulong;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> uchar </name>

/// <overview>unsigned char クラス</overview>
/// <desc>
/// <p>
/// 符号無し文字クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned char  uchar;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> uint </name>

/// <overview>unsigned int クラス</overview>
/// <desc>
/// <p>
/// 符号無し整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned int   uint;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> sshort </name>

/// <overview>signed short クラス</overview>
/// <desc>
/// <p>
/// 符号付き単精度整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef   signed short sshort;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> slong </name>

/// <overview>signed long クラス</overview>
/// <desc>
/// <p>
/// 符号付き倍精度整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef   signed long  slong;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> schar </name>

/// <overview>signed char クラス</overview>
/// <desc>
/// <p>
/// 符号付き文字クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef   signed char  schar;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////

/// <classdef>
/// <name> sint </name>

/// <overview>signed int クラス</overview>
/// <desc>
/// <p>
/// 符号付き倍精度整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef   signed int   sint;
/// </body></classdef>



// for easy-using of string type

#ifdef HAVE_CSTRING_H
#  include <cstring.h>
#  define repeat_string(a,b) std::string((a),(b))
#else // HAVE_CSTRING_H
#  include <string>
#  ifdef __SUNPRO_CC
#    define string _RW_STD::string
#  endif
#  define repeat_string(a,b) std::string((b),(a))
#endif // HAVE_CSTRING_H

/// <funcdef>
/// <name> int2str </name>
/// <overview> 整数値を文字列に変換</overview>
/// <desc>
/// <p>
/// int型の値をstring型に変換します。
/// </p>
/// </desc>
/// <args> int 値を渡します。</args>
/// <retval> stringオブジェクトを返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
std::string int2str(int i);
/// </body></funcdef>



//////////////////////////////////////////////////////////////////////////////
//
//  class code .... actually "unsigned char"
/// <classdef>
/// <name> code </name>
/// <overview> 中間コード クラス </overview>
/// <desc>
/// <p>
/// LiLFeSスクリプトからLiLFeSインタプリタが生成する中間コードです。中身は符号無し文字クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned char code;
/// </body></classdef>

/// <notiondef>
/// <name> serial_overview </name>
/// <title> シリアル </title>
/// <desc>
/// <p> LiLFeS のなかでは、型(type)、素性(feature)、手続き(procedure)などにそれぞれにユニークな ID を割り当てており、そのIDのことをシリアル番号と呼んでいます。<br>
/// </p>
/// </desc>
/// </notiondef>

//////////////////////////////////////////////////////////////////////////////
//
//  class serial .... actually "unsigned short"
/// <classdef>
/// <name> serial </name>

/// <overview> シリアルクラス</overview>
/// <desc>
/// <p>
/// 中身は符号無し単精度整数クラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef unsigned short serial;
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> S_INVALID </name>
/// <overview>無効なシリアルを表す定数</overview>
/// <desc>
/// <jpn>
/// <p>
/// 対応するオブジェクトがないシリアルを表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// Invalid serial number
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const serial S_INVALID = (serial)-1;	/* Invalid serial number */
/// </body></constdef>

//////////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> tserial </name>
/// <overview> 個々のtypeに一意に割り当てられるID のクラス </overview>
/// <desc>
/// <p>
/// 中身は符号無し16bit整数クラスです。型（type）にユニークに割り当てられるシリアル番号です。
/// </p>
/// </desc>
/// <see>serial_overview</see>
/// <body>
typedef uint16 tserial;	// type serial
/// </body></classdef>

//////////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> fserial </name>
/// <overview> 個々のfeatureに一意に割り当てられるID のクラス </overview>
/// <desc>
/// <p>
/// 中身は符号無し16bit整数クラスです。素性（feature）にユニークに割り当てられるシリアル番号です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef uint16 fserial;	// feature serial
/// </body></classdef>

//////////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> pserial </name>
/// <overview> 個々のprocedureに一意に割り当てられるID のクラス</overview>
/// <desc>
/// <p>
/// 中身は符号無し16bit整数クラスです。手続き（procedure）にユニークに割り当てられるシリアル番号です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef uint16 pserial;	// procedure serial
/// </body></classdef>

//////////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> sserial </name>
/// <overview> 個々のstringに一意に割り当てられるID のクラス </overview>
/// <desc>
/// <p>
/// 中身は符号無し32bit整数クラスです。文字列（string）にユニークに割り当てられるシリアル番号です。
/// </p>
/// </desc>
/// <see></see>
/// <body>
typedef uint32 sserial;	// string serial
/// </body></classdef>

/////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> TS_INVALID </name>
/// <overview>無効なtserialを表す定数 </overview>
/// <desc>
/// <jpn>
/// <p>
/// 対応するtypeオブジェクトがないtserialを表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// Invalid tserial number
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const tserial TS_INVALID = (tserial)(-1);	/* Invalid serial number */
/// </body></constdef>

/////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> FS_INVALID </name>
/// <overview>無効なfserialを表す定数 </overview>
/// <desc>
/// <jpn>
/// <p>
/// 対応するfeatureオブジェクトがないfserialを表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// Invalid fserial number
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const fserial FS_INVALID = (fserial)(-1);	/* Invalid serial number */
/// </body></constdef>

/////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> PS_INVALID </name>
/// <overview>無効なpserialを表す定数 </overview>
/// <desc>
/// <jpn>
/// <p>
/// 対応するprocedureオブジェクトがないpserialを表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// Invalid pserial number
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const pserial PS_INVALID = (pserial)(-1);	/* Invalid serial number */
/// </body></constdef>

/////////////////////////////////////////////////////////////////////////
/// <constdef>
/// <name> SS_INVALID </name>
/// <overview>無効なsserialを表す定数 </overview>
/// <desc>
/// <jpn>
/// <p>
/// 対応するstringオブジェクトがないsserialを表す定数です。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// Invalid sserial number
/// </p>
/// </eng>
/// </desc>
/// <see></see>
/// <body>
const sserial SS_INVALID = (sserial)(-1);	/* Invalid serial number */
/// </body></constdef>


//////////////////////////////////////////////////////////////////////////////
//
//  endian manipulation definition

#ifdef BIG_ENDIAN_
/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> Unaligned16 </name>

/// <overview> unaligned な状態でメモリに格納されているデータを適切に読み込むための関数</overview>
/// <desc>
/// <jpn>
/// <p>
/// メモリー上のあるアドレスから16bit分の値をひとつの16bit整数値として取得します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 取得する文字列へのポインタを渡します。</args>
/// <retval> 16bit整数値を返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline int16 Unaligned16(const  char *p) { return (int16)(((uint16)p[0]<<8)+(uchar)p[1]); }
inline int16 Unaligned16(const uchar *p) { return (int16)(((uint16)p[0]<<8)+(uchar)p[1]); }
inline int16 Unaligned16(const schar *p) { return (int16)(((uint16)p[0]<<8)+(uchar)p[1]); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> Unaligned32 </name>

/// <overview> unaligned な状態でメモリに格納されているデータを適切に読み込むための関数</overview>
/// <desc>
/// <jpn>
/// <p>
/// メモリー上のあるアドレスから32bit分の値をひとつの32bit整数値として取得します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 取得する文字列へのポインタを渡します。</args>
/// <retval> 32bit整数値を返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline int32 Unaligned32(const  char *p) { return (int32)((((((uint32)p[0] << 8) + (uchar)p[1]) << 8) + (uchar)p[2]) << 8) + (uchar)p[3]; }
inline int32 Unaligned32(const uchar *p) { return (int32)((((((uint32)p[0] << 8) + (uchar)p[1]) << 8) + (uchar)p[2]) << 8) + (uchar)p[3]; }
inline int32 Unaligned32(const schar *p) { return (int32)((((((uint32)p[0] << 8) + (uchar)p[1]) << 8) + (uchar)p[2]) << 8) + (uchar)p[3]; }
/// </body></funcdef>
#ifdef HAS_INT64

/// <funcdef>
/// <name> Unaligned64 </name>

/// <overview> unaligned な状態でメモリに格納されているデータを適切に読み込むための関数</overview>
/// <desc>
/// <jpn>
/// <p>
/// メモリー上のあるアドレスから64bit分の値をひとつの64bit整数値として取得します。
/// </p>
/// </jpn>
/// <eng>
/// <p>
/// not yet
/// </p>
/// </eng>
/// </desc>
/// <args> 取得する文字列へのポインタを渡します。</args>
/// <retval> 64bit整数値を返します。 </retval>
/// <remark></remark>
/// <see></see>
/// <body>
inline int64 Unaligned64(const  char *p) { return (int64)((((((((((((((uint64)p[0] << 8) + (uchar)p[1]) << 8) + (uchar)p[2]) << 8) + (uchar)p[3]) << 8) + (uchar)p[4]) << 8) + (uchar)p[5]) << 8) + (uchar)p[6]) << 8) + (uchar)p[7]; }
inline int64 Unaligned64(const uchar *p) { return (int64)((((((((((((((uint64)p[0] << 8) + (uchar)p[1]) << 8) + (uchar)p[2]) << 8) + (uchar)p[3]) << 8) + (uchar)p[4]) << 8) + (uchar)p[5]) << 8) + (uchar)p[6]) << 8) + (uchar)p[7]; }
inline int64 Unaligned64(const schar *p) { return (int64)((((((((((((((uint64)p[0] << 8) + (uchar)p[1]) << 8) + (uchar)p[2]) << 8) + (uchar)p[3]) << 8) + (uchar)p[4]) << 8) + (uchar)p[5]) << 8) + (uchar)p[6]) << 8) + (uchar)p[7]; }
/// </body></funcdef>
#endif // HAS_INT64
#else // BIG_ENDIAN_
inline int16 Unaligned16(const  char *p) { return (int16)(((uint16)p[1]<<8)+(uchar)p[0]); }
inline int16 Unaligned16(const uchar *p) { return (int16)(((uint16)p[1]<<8)+(uchar)p[0]); }
inline int16 Unaligned16(const schar *p) { return (int16)(((uint16)p[1]<<8)+(uchar)p[0]); }
inline int32 Unaligned32(const  char *p) { return (int32)((((((uint32)p[3] << 8) + (uchar)p[2]) << 8) + (uchar)p[1]) << 8) + (uchar)p[0]; }
inline int32 Unaligned32(const uchar *p) { return (int32)((((((uint32)p[3] << 8) + (uchar)p[2]) << 8) + (uchar)p[1]) << 8) + (uchar)p[0]; }
inline int32 Unaligned32(const schar *p) { return (int32)((((((uint32)p[3] << 8) + (uchar)p[2]) << 8) + (uchar)p[1]) << 8) + (uchar)p[0]; }
#ifdef HAS_INT64
inline int64 Unaligned64(const  char *p) { return (int64)((((((((((((((uint64)p[7] << 8) + (uchar)p[6]) << 8) + (uchar)p[5]) << 8) + (uchar)p[4]) << 8) + (uchar)p[3]) << 8) + (uchar)p[2]) << 8) + (uchar)p[1]) << 8) + (uchar)p[0]; }
inline int64 Unaligned64(const uchar *p) { return (int64)((((((((((((((uint64)p[7] << 8) + (uchar)p[6]) << 8) + (uchar)p[5]) << 8) + (uchar)p[4]) << 8) + (uchar)p[3]) << 8) + (uchar)p[2]) << 8) + (uchar)p[1]) << 8) + (uchar)p[0]; }
inline int64 Unaligned64(const schar *p) { return (int64)((((((((((((((uint64)p[7] << 8) + (uchar)p[6]) << 8) + (uchar)p[5]) << 8) + (uchar)p[4]) << 8) + (uchar)p[3]) << 8) + (uchar)p[2]) << 8) + (uchar)p[1]) << 8) + (uchar)p[0]; }
#endif // HAS_INT64
#endif // BIG_ENDIAN

} // namespace lilfes

#endif // __basic_h
