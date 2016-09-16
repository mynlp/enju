/* $Id: in.h,v 1.8 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __in_h
#define __in_h

#include "lconfig.h"
#include "cell.h"
#include "structur.h"
#include "utility.h"
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#ifndef IN_BISON_FILE
#ifdef BISON_USE_PARSER_H_EXTENSION
#include "yacc.h"
#else
#include "yacc.hh"
#endif
#endif
#ifndef IN_LEX_LL
#include "FlexLexer.h"
#endif

namespace lilfes {

class lilfesFlexLexer : public yyFlexLexer
{
	class lilfesFlexLexer *next;

	std::istream *s;
	std::string infile;
	module *mod;		// このファイルを読み始めたときのモジュール
	std::string prefix;
	int proginput;
	YYSTYPE *yylvalp;
	int depth;
	int _line_no;

	int comment_start_line;
	int ref;

	bool ShouldDelete;
	bool skip;
	bool parsing;
	bool firstrun;
	bool progvars;
public:

 lilfesFlexLexer(std::istream *is, const std::string &ifile, int ipi=0) 
	 : yyFlexLexer(), next(NULL), s(is), infile(ifile), mod(module::CurrentModule()), prefix(""), proginput(ipi), yylvalp(NULL), _line_no(1), ShouldDelete(false), skip(false)
	{ 
		depth = -1;
		switch_streams(s, NULL);
		parsing = false;
		firstrun = true;
		progvars = false;
                  //		ref = 0;
		ref = 1;
	}
	lilfesFlexLexer(lilfesFlexLexer *inext, std::istream *is, bool iShouldDelete, const std::string &ifile, const std::string &iprefix="", int ipi=0) 
	 : yyFlexLexer(), next(inext), s(is), infile(ifile), mod(module::CurrentModule()), prefix(iprefix), proginput(ipi), yylvalp(NULL), _line_no(1), ShouldDelete(iShouldDelete), skip(false)
	{ 
		depth = -1;
		switch_streams(s, NULL);
		parsing = false;
		firstrun = true;
		progvars = false;
                //		ref = 0;
		ref = 1;
	}

	~lilfesFlexLexer() { if(ShouldDelete) delete s; }

	int Lineno() const { return _line_no; } 

	const std::string &InFile() const { return infile; }
	module *GetModule() { return mod; }
//	void SetModule(module *m) { mod = m; }
	lilfesFlexLexer *Next() { return next; }

	void Skip() { skip = true; }
	bool IsSkip() const { return skip; }
	void Interrupt() { if ( ! Isatty() ) { Skip(); if( next ) next->Interrupt(); } }

	int incRef() { return ++ref; }
	int decRef() { return --ref; }

	bool Isatty() const;
	void Prompt() const;
	void SetParsing(bool b) { parsing = b; }
	void Flush();

	bool FirstRun() { bool ret = firstrun; firstrun = false; return ret; }
	void SetFirstRun(bool b = true) { firstrun = b;}

	int ProgInput() { if( IsSkip() && next ) return next->ProgInput(); else return proginput; }
	bool ProgVars() { if( IsSkip() && next ) return next->ProgVars(); else return progvars; }
	void SetProgVars(bool b) { progvars = b; }
	int Depth() { if( depth >= 0 ) return depth; else if( next == NULL ) return 1; else return next->Depth()+1; }
	void SetDepth(int i) { depth = i; }

	virtual int yylex();
	void set_yylvalp(YYSTYPE *s) { yylvalp = s; }
	YYSTYPE &get_yylval() { return *yylvalp; }
};

class parsevar
{
	std::string name;
	core_p  cp;
	int count;
public:
	parsevar(const std::string &in, core_p ip = CORE_P_INVALID): name(in) { cp = ip; count = 0; }
	~parsevar() { }
	
	const std::string &GetKey() const { return name; }
	const std::string &GetName() const { return name; } 
	core_p addr() const { return cp; }
	void SetAddr(core_p p) { cp = p; }
	void IncCount() { count++; }
	int GetCount() const { return count; }
};

// exported from in.y
//void prompt();

extern FSP prog_types;
extern FSP prog_list;
extern FSP prog_list_top;
extern FSP prog_vars;
extern FSP prog_vars_top;

// exported from main.cpp

// int command(const char *cmd);
int eval(machine *m, const std::string &str, bool vars = false);
extern int nargc;
extern int ARGC;
extern char **ARGV;
void SetInteractive(int);
void free_dups(void);
extern int interactive;
extern void prompt(void);
extern void error_open_file(char *);
extern void success_open_file(char *);

extern std::istream *input_stream;
extern std::ostream *output_stream;
extern std::ostream *error_stream;

struct token_key {
	const char *str;
	int token_code;
	const char *GetKey() { return str; }
};

// const char *infile();
// int lineno();
// int proginput();

struct parser_control		// argument of yyparse/yylex
{
public:
	machine *mach;

public:
  explicit parser_control( machine* m ) : mach( m ) {}
  ~parser_control() {}
};

int readfile(machine *m);

} // namespace lilfes

// int yywrap(void);

#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2008/12/28 08:51:12  yusuke
 *
 * support for gcc 4.3
 *
 * Revision 1.6  2008/11/19 09:10:25  hkhana
 * used noyywrap option
 *
 * Revision 1.5  2005/02/11 01:17:46  n165
 * ブランチPIPE_STREAMを統合し、stringstreamの使用、標準出力、標準エラー出力のスイッチができるようになりました。
 *
 * Revision 1.4.4.1  2005/02/09 10:37:25  n165
 * output_streamとerror_streamの書き換えを実装。
 * 標準出力と標準エラー出力についてはポインタを書き換えたので、
 * cout/cerrでハードコードしてある部分はそのままにしてlillib/debugは書き換えた。
 *
 * Revision 1.4  2004/05/30 08:40:26  yusuke
 *
 * memory leak check のプログラムを実装。それにともない、memory leak を減
 * らした。
 *
 * Revision 1.3  2004/04/23 08:59:27  yusuke
 *
 * へっだふぁいるのばしょにいどう。
 *
 * Revision 1.4  2004/04/20 15:58:55  yusuke
 *
 * yacc.yy の save が初期化されていなかったので、メモりが破壊されていた。
 *
 * Revision 1.3  2004/04/08 06:23:29  tsuruoka
 * lexer_t: modified the initialization method of reference counting
 *
 */
