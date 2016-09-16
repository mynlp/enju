/* $Id: lex.ll,v 1.20 2011-05-03 06:39:24 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

%option c++
%option noyywrap
%option yyclass="lilfesFlexLexer"
%option outfile="lex.yy.c"

%{

//#define DEBUG_INCLUDE
static const char rcsid[] = "$Id: lex.ll,v 1.20 2011-05-03 06:39:24 matuzaki Exp $";
  /* 初期設定 */
#define IN_LEX_LL
#include "builtin.h"
#include "in.h"
#include "lconfig.h"
#include "search_path.h"
#include "lilfes.h"
#include "rl_stream.h"
#include <iostream>
#include <sstream>
#include <string>
//#include <FlexLexer.h>

#ifdef LILFES_WIN_
#include <Windows.h>
#include <io.h>
#undef SearchPath
#else
#include <pwd.h>
#include <unistd.h>
#endif


#define YY_SKIP_YYWRAP  /* to avoid collision with namespace */

using namespace lilfes;

namespace lilfes {

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::istream;
using std::istringstream;
using std::string;

int token_search(char *);

char *my_strdup(char *str);

void error_open_file(char *);
void success_open_file(char *);
}

#define yylval (*yylvalp)

%}
%x COMMENT

%%

<COMMENT>"*/"  BEGIN INITIAL; 
<COMMENT>\n { /* printf("%s %d:\n", infile, lineno); fflush(stdout);*/ 	_line_no++; }
<COMMENT>.  /* eat up multi-line comments */
"%/*"        { BEGIN COMMENT; comment_start_line = Lineno(); }

"%"$                 /* eat up comments   */ 
"%"[^/\n].*$         /* eat up comments   */ 
"%/"[^*\n].*$         /* eat up comments   */ 
[\t\f\v\r ]+   /* eat up whitespace */
\n         { /*printf("%s %d:\n", infile, lineno); fflush(stdout);*/ 	_line_no++; Prompt(); }

\.\/	{ return ATTR_DEF; }

\. { parsing = false; return '.';}

[\(\)\,\[\]\|\{\}] { parsing = true; return *yytext;}

\-?[0-9]*\.[0-9]+[Ee][-+][0-9]* /* 実数E+整数 */ {
		parsing = true;
		yylval.dval = atof(yytext); 
		return TOK_DOUBLE; 
	}

\-?[0-9]*\.[0-9]+ /* 実数 */ {
		parsing = true;
		yylval.dval = atof(yytext); 
		return TOK_DOUBLE; 
	}


\-?(0x)?[0-9]+ /* 数字 */ {
		parsing = true;
		yylval.ival = (int)strtol(yytext, (char**)NULL, 0); 
		return TOK_INTEGER;
	}

\"([^\"\n]|\"\")*\" /* DoubleQuote された文字列 */ { 
		parsing = true;
		char *p,*q; 
		yylval.str = my_strdup(yytext+1); 
		yylval.str[strlen(yylval.str)-1]='\0'; 
		p = yylval.str; 
		while((q=strstr(p,"\"\"")) != NULL) 
		{ 
			memmove(q,q+1,strlen(q)); 
			p=q+1; 
		} 
		return(TOK_STRING); 
	}

\'([^\'\n]|\'\')+\'\( /* SingleQuote された ( で終わる文字列 */ { 
		parsing = true;
		char *p,*q; 
		yylval.str = my_strdup(yytext+1); 
		yylval.str[strlen(yylval.str)-2]='\0'; 
		p = yylval.str; 
		while((q=strstr(p,"''")) != NULL) 
		{ 
			memmove(q,q+1,strlen(q)); 
			p=q+1; 
		} 
		return(TOK_STARTP); 
	}

\'([^\'\n]|\'\')+\'\: /*SingleQuoted modulename*/ {
		parsing = true;
                char *p,*q;
                yylval.str = my_strdup(yytext+1);
                yylval.str[strlen(yylval.str)-2]='\0';
                p = yylval.str;
                while((q=strstr(p,"''")) != NULL)
                {
                        memmove(q,q+1,strlen(q));
                        p=q+1;
                }
                return(TOK_MODULENAME);
        }


\'([^\'\n]|\'\')+\'\\ /* SingleQuote された \ で終わる文字列 */ { 
		parsing = true;
		char *p,*q; 
		yylval.str = my_strdup(yytext+1); 
		yylval.str[strlen(yylval.str)-2]='\\'; 
		yylval.str[strlen(yylval.str)-1]='\0'; 
		p = yylval.str; 
		while((q=strstr(p,"''")) != NULL) 
		{ 
			memmove(q,q+1,strlen(q)); 
			p=q+1; 
		} 
		return(TOK_FEAT); 
	}

\'([^\'\n]|\'\')+\' /* SingleQuote された文字列 */ { 
		parsing = true;
		char *p,*q; 
		yylval.str = my_strdup(yytext+1); 
		yylval.str[strlen(yylval.str)-1]='\0'; 
		p = yylval.str; 
		while((q=strstr(p,"''")) != NULL) 
		{ 
			memmove(q,q+1,strlen(q)); p=q+1; 
		} 
		return(TOK_ATOM); 
	}

([$A-Za-z_]|[\x80-\xff])([$A-Za-z0-9_]|[\x80-\xff])*\\ /* \ で終わる */ {
		parsing = true;
		yylval.str = my_strdup(yytext); 
//		yylval.str[strlen(yylval.str)-1] = '\0'; 
		return(TOK_FEAT); 
	}

([a-z]|[\x80-\xff])([$A-Za-z0-9_]|[\x80-\xff])*\( /* ( で終わる */ {
		parsing = true;
		yylval.str = my_strdup(yytext); 
		yylval.str[strlen(yylval.str)-1] = '\0'; 
		return(TOK_STARTP);
	}

([A-Za-z]|[\x80-\xff])([$A-Za-z0-9_]|[\x80-\xff])*\: /*end with : */ { 
		parsing = true;
                yylval.str = my_strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return(TOK_MODULENAME);
        }

([a-z]|[\x80-\xff])([$A-Za-z0-9_]|[\x80-\xff])* /* 小文字で始まる */ {
		parsing = true;
		yylval.str = my_strdup(yytext); 
		if( !strcmp(yytext, "is") ) return TOK_IS; 
		if( !strcmp(yytext, "mod") ) return TOK_MOD; 
		return(TOK_ATOM);
	}

[_$A-Z]([$A-Za-z0-9_]|[\x80-\xff])* /* 大文字で始まる */ {
		parsing = true;
		yylval.str = my_strdup(yytext); 
		return(TOK_VAR);
	}

[\!\#\&\-\+\/\:\;\*\<\=\>\?\@\\\`\~\^]+ /* 記号列 */ { 
		parsing = true;
		yylval.str = my_strdup(yytext); 
		return token_search(yylval.str);
}

\' { 
		inerr(this) << "Quoted atom not terminated" << endmsg; 
		return TOK_INVALID; 
	}
\" { 
		inerr(this) << "String not terminated" << endmsg;
		return TOK_INVALID; 
	}

. { 
		inerr(this) << "Internal error: unknown input" << endmsg; 
		return TOK_INVALID; 
	}

%%
#if 0
 [\!\#\&\-\+\/\:\;\*\<\=\>\?\@\\\`\~]+\( /* ( で終わる記号列 */ {yylval.str = my_strdup(yytext); yylval.str[strlen(yylval.str)-1] = '\0'; return(TOK_STARTP);} 
#endif

/*
main() {
  yylex();
}
*/

//void SetInteractive(int i) { interactive = i; yy_set_interactive(i); }


// #define DEBUG_INCLUDE

// lilfesFlexLexer *lexer;

namespace lilfes {

int yylex(YYSTYPE *yylvalp, void *parm_) 
{ 
	parser_control *parm = (parser_control *)parm_;
	P1("Parse"); 
	if( ! parm->mach->getLexer() )
		return YY_NULL;

	while( parm->mach->getLexer()->IsSkip() )
	{
#ifdef DEBUG_INCLUDE
		cout << "POP_FILE " 
		  << (parm->mach->getLexer()->Next() ? (parm->mach->getLexer()->Next()->InFile()) : "(null)") 
		  << ":" 
		  << (parm->mach->getLexer()->Next() ? parm->mach->getLexer()->Next()->Lineno() : 0) 
		  << " <- " 
		  << parm->mach->getLexer()->InFile() 
		  << ":"
		  << parm->mach->getLexer()->Lineno()
		  << endl;
#endif
                //cerr << "1: " << parm->mach->getLexer()->InFile() << endl;
		if( parm->mach->getLexer()->GetModule() != module::CurrentModule() )
		{
                        //cerr << "1: finish!" << endl;
			module::CurrentModule()->Finish();
			module::SetCurrentModule(parm->mach->getLexer()->GetModule());
		}
		lilfesFlexLexer *next = parm->mach->getLexer()->Next();
		lilfesFlexLexer *lexer = (lilfesFlexLexer *)(parm->mach->getLexer());
		delete lexer;
		parm->mach->setLexer(next);
		if( next == NULL )
			return YY_NULL;
	}

	if( parm->mach->getLexer()->FirstRun() ) {
                //cerr << "poe";
		parm->mach->getLexer()->Prompt();
        }

	parm->mach->getLexer()->set_yylvalp(yylvalp); 
	int ret = parm->mach->getLexer()->IsSkip() ? YY_NULL : parm->mach->getLexer()->yylex(); 
	if( ret != YY_NULL )
		return ret;
	else
	{
//		return YY_NULL;
		if( parm->mach->getLexer()->Next() == NULL || parm->mach->getLexer()->ProgInput() != parm->mach->getLexer()->Next()->ProgInput() )
		{
#ifdef DEBUG_INCLUDE
		cout << "Finish=YY_NULL " 
		  << parm->mach->getLexer()->InFile() 
		  << ":"
		  << parm->mach->getLexer()->Lineno()
		  << endl;
#endif
                        parm->mach->getLexer()->Skip();
			return YY_NULL;
		}
		else
		{
#ifdef DEBUG_INCLUDE
		cout << "Finish=EOF " 
		  << parm->mach->getLexer()->InFile() 
		  << ":"
		  << parm->mach->getLexer()->Lineno()
		  << endl;
#endif
			parm->mach->getLexer()->Skip();
			return TOK_EOF;
		}
	}
}

int eval(machine *m, const string &str, bool progvars)
{
#ifdef DEBUG_INCLUDE
		cout << "Start Eval " 
		  << lexer->InFile() 
		  << ":"
		  << lexer->Lineno()
		  << endl;
#endif
	istringstream *iss = new istringstream( str );
	lilfesFlexLexer *lex = new lilfesFlexLexer( NULL, iss, true, "eval", "", 1 );
	prog_list = prog_list_top = FSP(m);
	prog_vars = prog_vars_top = FSP(m);
#ifdef DEBUG_INCLUDE
		cout << "Eval Lexer created " 
		  << lex->InFile() 
		  << ":"
		  << lex->Lineno()
		  << endl;
#endif
	lex->SetProgVars(progvars);
//	while( lex->ProgInput() )
//	{
		m->parse(lex);
//	}
	prog_list.Coerce(nil);
	prog_vars.Coerce(nil);
	return 1;
}

static InitFunction IF(search_path::init);
static TermFunction TF(search_path::term);

int readfile(machine *m)
{
	while( m->getLexer()->ProgInput() )
	{
		parse(m);
	}
	return 1;
}

namespace builtin {

bool get_search_path( machine& m, FSP arg1 )
{
	search_path::SetRelativePath("(current file relative)");
	search_path *s = search_path::SearchPath();
	while( s )
	{
		if(	arg1.Follow(hd).Unify(FSP(m,s->GetName() )) == false ||
			! (arg1 = arg1.Follow(tl)).IsValid() )
		{
			return false;
		}
		s = s->Next();
	}
	return arg1.Coerce(nil) ? true : false;
}

LILFES_BUILTIN_PRED_1(get_search_path, get_search_path);

} // namespace builtin

// const char *infile() { return lexer ? lexer->InFile() : "(closed)"; }
// int lineno() { return lexer ? lexer->Lineno() : -1; }
// int proginput() { return lexer ? lexer->ProgInput() : 0;} 


bool lilfesFlexLexer::Isatty() const {
  if( IsSkip() && next )
    return next->Isatty();
  else
    return (s == &cin ||
            s == &rl_istream::rl_stdin) &&
      isatty(0);
}

void lilfesFlexLexer::Prompt() const
{
  if ( s == &rl_istream::rl_stdin ) {
    rl_istream::rl_stdin.set_prompt( parsing? "| " : "> " );
  }
//	if( Isatty() ) 
//		cout << (parsing ? "| " : "> "); 
//	else
//		cout << (parsing ? "(|) " : "(>) "); 
//	else
//		cout << "(" << InFile() << ") ";
}

//#include <stdio.h>

void lilfesFlexLexer::Flush()
{
	parsing = false; 
	//YY_FLUSH_BUFFER();
	s->sync();
	while(s->rdbuf()->in_avail())
		if(s->get() == '\n')
			break;
	SetFirstRun();
}

lexer_t::lexer_t(lilfesFlexLexer *il) : l(il) 
{
	if(l) l->incRef();
}

lexer_t::lexer_t(const lexer_t &lx) : l(lx.l)
{
	if(l) l->incRef();
}

lexer_t::lexer_t(istream *is, const char *filename, int iproginput )
{
	l = new lilfesFlexLexer(is, filename, iproginput);
//	l->incRef();   // now it is done in the constructor of lilfesFlexLexer
}
lexer_t::~lexer_t()
{
	if( l && l->decRef() == 0 )
		delete l;
}

} // namespace lilfes


// #undef yywrap
// int yywrap(void)
// {
// 	return 1;
// }
// #if defined(HAVE_YYFLEXLEXER_YYWRAP) && !defined(AVAILABLE_YYFLEXLEXER_YYWRAP)
// int yyFlexLexer::yywrap(void)
// {
//	return 1;
// }
// #endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.19  2011/05/02 10:38:23  matuzaki
 *
 * windows
 *
 * Revision 1.18  2010/05/14 14:26:19  yusuke
 *
 * support for g++ 4.4
 *
 * Revision 1.17  2008/12/28 08:51:13  yusuke
 *
 * support for gcc 4.3
 *
 * Revision 1.16  2008/11/19 09:10:26  hkhana
 * used noyywrap option
 *
 * Revision 1.15  2008/11/17 13:46:28  hkhana
 * checking in configuration is perhaps a better way to compile in old env.
 *
 * Revision 1.13  2008/05/20 03:15:00  hkhana
 *
 * utf8 enabled, while sjis disabled
 *
 * Revision 1.12  2004/08/18 14:59:07  yusuke
 *
 * c_to_lilfes が vector< cell > に対応。
 * load_module で同じモジュールを複数回 load すると segmentation fault になるのを修正。
 *
 * Revision 1.11  2004/05/30 08:40:27  yusuke
 *
 * memory leak check のプログラムを実装。それにともない、memory leak を減
 * らした。
 *
 * Revision 1.10  2004/05/18 08:14:57  yusuke
 *
 * eval/3 をじっそう。
 *
 * Revision 1.9  2004/04/23 08:38:41  yusuke
 *
 * std:: をつけた。
 *
 * Revision 1.8  2004/04/20 03:07:51  yusuke
 *
 * readline をくみこんだ。
 *
 * Revision 1.7  2004/04/19 15:39:34  yusuke
 *
 * builtin のいしょくかんりょう。
 *
 * Revision 1.6  2004/04/08 06:23:29  tsuruoka
 * lexer_t: modified the initialization method of reference counting
 *
 */

