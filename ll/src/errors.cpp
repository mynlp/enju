/* $Id: errors.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1996-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//
//  LightSpeed HPSG Parser Project
//
//  errors.cpp - Implementation of error exception handling.
//
//  **LICENSE**
//
//  Change Log:
//
//  97/04/25  0.10 
//  96/11/20  Created. / mak

static const char rcsid[] = "$Id: errors.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $";

#include "errors.h"
#include "in.h"
#include "option.h"
#include <iostream>
#include <sstream>
#include <string>

namespace lilfes {

using std::ostringstream;
using std::string;


static errorstream runerrstream("Run-time Error: ", "");
static errorstream runwarnstream("Run-time Warning: ", "");
static errorstream internalerrstream("Internal error: ", "");
static errorstream internalwarnstream("Internal system warning: ", "");
static errorstream dummystream("", "", false);
  
static tracestream tracerstream("debug> ");

    
#undef tracer
#undef inerr
#undef inwarn
#undef runerr
#undef runwarn
#undef internalerr
#undef internalwarn

tracestream &tracer() { return tracerstream; }
errorstream inerr(lexer_t l)  { Errors++; return errorstream("Error: ", l ? ( string(" at ") + l->InFile() + ":" + int2str(l->Lineno())) : ""); }
errorstream inwarn(lexer_t l)  { Warning++; return lilfes_flag::Search("warning")->GetIntVal() ? (errorstream("Warning: ", l ? (string(" at ") + l->InFile() + ":" + int2str(l->Lineno())) : "")) : dummystream; }
errorstream &runerr()  { Errors++; return runerrstream; }
errorstream &runwarn()  { Warning++; return lilfes_flag::Search("warning")->GetIntVal() ? runwarnstream : dummystream; }
errorstream &internalerr()  { Errors++; return internalerrstream; }
errorstream &internalwarn()  { Warning++; return internalwarnstream; }

errorstream  errorstream::sourceinfo(const char *file, int line)
{
	ostringstream oss;
	oss << endline << " (on source code " << file << ":" << line << ", passcount " << PASSCOUNT << ")";
	endline = oss.str().c_str();
	return *this;
}

#ifdef DEBUG
bool DEB = true;
#endif

long int PASSCOUNT = 0;
long int TRAILBACK = 0;
long int Errors = 0;
long int Warning = 0;

} // namespace lilfes












