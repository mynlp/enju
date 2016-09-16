/* $Id: lilfes.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: lilfes.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>lilfes.h</name>
//
///  <overview>
///  <jpn>外部から利用するためのヘッダファイル</jpn>
///  <eng>Header-file for external programs</eng>
///  </overview>

#ifndef __lilfes_h
#define __lilfes_h

#include "ftype.h"
#include "structur.h"
#include "lexer.h"

namespace lilfes {

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> parse </name>

/// <overview>LiLFeS パーザーの実行</overview>
/// <desc>
/// <jpn>
/// <p>
/// LiLFeS パーザーを実行します。既定では、プロンプトを表示し、入力まちになります。
/// </p>
/// </jpn>
/// </desc>

/// <body>
inline void parse(machine *m, lexer_t l) { return m->parse(l); }
inline void parse(machine *m) { return m->parse(); }
/// </body></funcdef>


/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> eval </name>

/// <overview>文字列の評価</overview>
/// <desc>
/// <jpn>
/// <p>
/// 文字列を評価して、list をあらわす FSP にして返します。。
/// </p>
/// </jpn>
/// </desc>

/// <body>
inline FSP eval(machine *m, const char *s) { return m->eval(s); }
/// </body></funcdef>

/////////////////////////////////////////////////////////////////////////

/// <funcdef>
/// <name> call </name>

/// <overview>述語の実行</overview>
/// <desc>
/// <jpn>
/// <p>
/// FSP を実行します。
/// </p>
/// </jpn>
/// </desc>

/// <body>
inline bool call(machine *m, FSP f) { return m->call(f); }
/// </body></funcdef>


}

#endif
