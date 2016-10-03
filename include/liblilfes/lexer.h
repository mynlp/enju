/* $Id: lexer.h,v 1.3 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: lexer.h,v 1.3 2011-05-02 08:48:58 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>lexer.h</name>
//
///  <overview>
///  <jpn>lexer_t のヘッダファイル</jpn>
///  <eng>Header-file for lexer_t</eng>
///  </overview>

#ifndef __lexer_h
#define __lexer_h

#include <istream>

namespace lilfes {

class lilfesFlexLexer;

class lexer_t {
	lilfesFlexLexer *l;

public:
	lexer_t() : l(NULL) { }
	lexer_t(const lexer_t &);
	lexer_t(lilfesFlexLexer *il);
	lexer_t(std::istream *is, const char *filename, int iproginput = 0 );
	~lexer_t();

//	operator bool() const { return l != NULL; } 
	bool operator !() const { return l == NULL; } 
	operator lilfesFlexLexer *() { return l; }
	operator const lilfesFlexLexer *() const { return l; }
	lilfesFlexLexer& operator*() { return *l; }
	const lilfesFlexLexer& operator*() const { return *l; }
	lilfesFlexLexer* operator->() { return l; }
	//This definition causes cyclic dependency among header files on AIX
	//const lilfesFlexLexer& operator->() const { return *l; }
};

} // namespace lilfes

#endif // __lexer_h
