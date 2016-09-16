%{
/* $Id: yacc.yy,v 1.18 2011-05-02 10:38:24 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: yacc.yy,v 1.18 2011-05-02 10:38:24 matuzaki Exp $";
	
#define IN_BISON_FILE

#include "lconfig.h"
#include "structur.h"
#include "machine.h"
#include "builtin.h"
#include "proc.h"
#include "parse.h"
#include "lilfes.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#else
#include <malloc.h>
#endif
#include <setjmp.h>

namespace lilfes {

using std::strcat;
using std::strcpy;
using std::strlen;
using std::string;

extern int codeoutflag;
static core_p save;

void yyerror(void *parm, const char* s);
//void yyerr(machine *m, const char* s);
//#define YYERROR_VERBOSE
//#define yyerror(X) yyerr(PARM->mach,(X))

#define progalter 0

#define ERRBUF 100
#define YYMAXDEPTH  250000
#define YYINITDEPTH 70000
// Avoid bison bug for C++ output
#define YYSTACK_USE_ALLOCA 0
#define __attribute__(X) 

//#define DEBUG
// #define YYDEBUG 1
//#define YYPARSE_PARAM parm
//#define YYLEX_PARAM parm
#define PARM ((parser_control *)parm)
bool compiled = false;
bool errored = false;

bool warn_old_feature_notation = false;

#if defined(PROFILE)
static prof *profx;
#endif	/* defined(PROFILE) */

FSP prog_types;
FSP prog_list;
FSP prog_list_top;
FSP prog_vars;
FSP prog_vars_top;

static string progalter_fprefix = "__";
static string progalter_tprefix = "__";

void CheckSingleton(machine *m, const string &where);

int yyparse();

//std::istream *input_stream = NULL;
//std::ostream *output_stream = NULL;
//std::ostream *error_stream = NULL;
std::istream *input_stream = &std::cin;
std::ostream *output_stream = &std::cout;
std::ostream *error_stream = &std::cerr;
bool constraint_status = true;

void change_modulename(char *);
char *my_malloc(int);
char *my_strdup(char *);

void hook()
{

}


bool bi_feature(char *s)
{
	if( strcmp(s, "hd") == 0 )	return true;
	if( strcmp(s, "tl") == 0 )	return true;
	if( strcmp(s, "chead") == 0 )	return true;
	if( strcmp(s, "cbody") == 0 )	return true;
	if( strcmp(s, "arg1") == 0 )	return true;
	if( strcmp(s, "arg2") == 0 )	return true;
	if( strcmp(s, "arg3") == 0 )	return true;
	if( strcmp(s, "arg4") == 0 )	return true;
	if( strcmp(s, "arg5") == 0 )	return true;
	if( strcmp(s, "arg6") == 0 )	return true;
	if( strcmp(s, "arg7") == 0 )	return true;
	if( strcmp(s, "arg8") == 0 )	return true;
	if( strcmp(s, "arg9") == 0 )	return true;
	if( strcmp(s, "arg10") == 0 )	return true;
	if( strcmp(s, "arg11") == 0 )	return true;
	if( strcmp(s, "arg12") == 0 )	return true;
	if( strcmp(s, "arg13") == 0 )	return true;
	if( strcmp(s, "arg14") == 0 )	return true;
	if( strcmp(s, "arg15") == 0 )	return true;
	return false;
}

bool bi_type(char *s)
{
	if( strcmp(s, "bot") == 0 )	return true;
	if( strcmp(s, "integer") == 0 )	return true;
	if( strcmp(s, "string") == 0 )	return true;
	if( strcmp(s, "float") == 0 )	return true;
	if( strcmp(s, "feature") == 0 )	return true;
	if( strcmp(s, "list") == 0 )	return true;
	if( strcmp(s, "cons") == 0 )	return true;
	if( strcmp(s, "nil") == 0 )	return true;
	if( strcmp(s, "true") == 0 )	return true;
	if( strcmp(s, "fail") == 0 )	return true;
	if( strcmp(s, "is") == 0 )	return true;
	if( strcmp(s, "type_attr") == 0 )	return true;
	if( strcmp(s, "final") == 0 )	return true;
	if( strcmp(s, "extendable") == 0 )	return true;
	if( strcmp(s, "pred") == 0 )	return true;
	if( strcmp(s, "pred/1") == 0 )	return true;
	if( strcmp(s, "pred/2") == 0 )	return true;
	if( strcmp(s, "pred/3") == 0 )	return true;
	if( strcmp(s, "pred/4") == 0 )	return true;
	if( strcmp(s, "pred/5") == 0 )	return true;
	if( strcmp(s, "pred/6") == 0 )	return true;
	if( strcmp(s, "pred/7") == 0 )	return true;
	if( strcmp(s, "pred/8") == 0 )	return true;
	if( strcmp(s, "pred/9") == 0 )	return true;
	if( strcmp(s, "pred/10") == 0 )	return true;
	if( strcmp(s, "pred/11") == 0 )	return true;
	if( strcmp(s, "pred/12") == 0 )	return true;
	if( strcmp(s, "pred/13") == 0 )	return true;
	if( strcmp(s, "pred/14") == 0 )	return true;
	if( strcmp(s, "pred/15") == 0 )	return true;
	if( strcmp(s, "<") == 0 )	return true;
	if( strcmp(s, "<=") == 0 )	return true;
	if( strcmp(s, ">") == 0 )	return true;
	if( strcmp(s, ">=") == 0 )	return true;
	if( strcmp(s, "=") == 0 )	return true;
	if( strcmp(s, "=\\=") == 0 )	return true;
	if( strcmp(s, "=:=") == 0 )	return true;
	if( strcmp(s, ":-") == 0 )	return true;
	if( strcmp(s, ":-/1") == 0 )	return true;
	if( strcmp(s, ":-/2") == 0 )	return true;
	if( strcmp(s, "?-") == 0 )	return true;
	if( strcmp(s, ",") == 0 )	return true;
	if( strcmp(s, ";") == 0 )	return true;
	if( strcmp(s, "+") == 0 )	return true;
	if( strcmp(s, "-") == 0 )	return true;
	if( strcmp(s, "*") == 0 )	return true;
	if( strcmp(s, "/") == 0 )	return true;
	if( strcmp(s, "//") == 0 )	return true;
	if( strcmp(s, "mod") == 0 )	return true;
	if( strcmp(s, "^") == 0 )	return true;
	if( strcmp(s, "\\/") == 0 )	return true;
	if( strcmp(s, "/\\") == 0 )	return true;
	if( strcmp(s, "#") == 0 )	return true;
	if( strcmp(s, "\\+") == 0 )	return true;
	if( strcmp(s, "\\=") == 0 )	return true;
	return false;
}

} // namespace lilfes

%}

%pure-parser
%param {void *parm}
%define parse.error verbose


%union {
  int      ival;
  double   dval;
  char*    str;
  lilfes::type*    typeval;
  const lilfes::type *ctypeval;
  lilfes::feature* featval;
  const lilfes::feature* cfeatval;
  lilfes::module *modval;
  struct {
	  lilfes::feature *feat;
	  const lilfes::type *app;
  } appfeatval;
  lilfes::core_p   pval;
} 

%type <pval> desc
%type <pval> i_desc
%type <pval> c_desc
%type <pval> x_desc
%type <pval> ix_desc
%type <pval> cx_desc
%type <pval> lists
%type <pval> nelistp
%type <pval> arglist

%type  <typeval> startp
%type  <typeval> atom
%type  <typeval> apptype
%type  <typeval> newtype
%type  <typeval> subsumedef
%type  <typeval> introdef
%type  <typeval> flagsdef
%type  <typeval> flagsdefx

%type <typeval> feature_with_module
%type <typeval> atom_with_module

%type <modval> optional_module_qualifier

%type  <appfeatval> newfeat

//%type  <str> featp
%type  <str> atom_or_atomop
%type  <str> modulename

%type  <typeval> atomtok
%token <str> TOK_ATOM
%token <str> TOK_FEAT
%token <str> TOK_VAR
%token <str> TOK_STARTP
%token <str> TOK_MODULENAME
%token <ival> TOK_INTEGER

%token <str> TOK_STRING
%token <dval> TOK_DOUBLE

%token <str> TOK_INVALID
%token <str> TOK_IS
%token <str> TOK_MOD

%token <str> TOK_EOF

%token SUBSUMED_BY
%token ATTR_DEF


%left ATOMP
%left LISTEND
%right MEANS MACRO_DEF
%right QUERY
%right UMEANS
%right ';'
%right IF_THEN
%right ','
%right NOT
%left CMP_L CMP_LE CMP_G CMP_GE CMP_EQ CMP_NE
%left '=' NOT_UNIFIABLE
%right TOK_IS
%left BIT_OR '#'
%left BIT_AND
%right '\\'
%left '+' '-'
%left '*' '/'
%left INTDIV TOK_MOD
%left USIGN
%left '^' 
%left '&' 
%left FEATURE

%start top

%{

#include "in.h"

using std::make_pair;
using std::string;

namespace lilfes {

_HASHMAP<string, parsevar *> vars;

int yylex(YYSTYPE *yylvalp, void *parm);

%}

%%

top:	  /* empty */	{ }
	| top TOK_EOF   { }
	| top typedef 	{ errored = false; free_dups(); vars.clear();  }
	| top command 	{ errored = false; free_dups(); vars.clear();  }
	| top error { if( PARM->mach->getLexer()->Isatty() ) { PARM->mach->getLexer()->Flush(); YYABORT; } } /* 
	*/ period { yyerrok; PARM->mach->getLexer()->SetParsing(false); errored = false; }
;

/************************************************************************/

period:
		'.'	{ }
	|   TOK_EOF { INERR(PARM->mach,"Incomplete statement (No period at EOF)");  };

typedef:
	  subsumedef period      { if( $1 != NULL ) {  $1->Fix(); procedure::NewTypeNotify(); if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) { prog_types.Follow(hd).Coerce($1); prog_types = prog_types.Follow(tl); } } }
	| subsumedef ']' period  { if( $1 != NULL ) {  $1->Fix(); procedure::NewTypeNotify(); if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) { prog_types.Follow(hd).Coerce($1); prog_types = prog_types.Follow(tl); } } }
	| introdef   period      { if( $1 != NULL ) {  $1->Fix(); procedure::NewTypeNotify(); if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) { prog_types.Follow(hd).Coerce($1); prog_types = prog_types.Follow(tl); } } }
	| introdef   ']' period  { if( $1 != NULL ) {  $1->Fix(); procedure::NewTypeNotify(); if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) { prog_types.Follow(hd).Coerce($1); prog_types = prog_types.Follow(tl); } } }
	| flagsdef   period      { if( $1 != NULL ) {          vars.clear(); procedure::NewTypeNotify(); if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) { prog_types.Follow(hd).Coerce($1); prog_types = prog_types.Follow(tl); } } }
;

flagsdef:
	flagsdefx i_desc { if( $1 != NULL && $2 != CORE_P_INVALID ) { $1->SetAttribute(FSP(PARM->mach, $2)); $$ = $1; } else $$ = NULL; };

flagsdefx:
	  subsumedef ']' ATTR_DEF { if( $1 != NULL ) $1->Fix(); $$ = $1; } 
	| introdef ']' ATTR_DEF   { if( $1 != NULL ) $1->Fix(); $$ = $1; } 
	| subsumedef ']' '/'      { INWARN(PARM->mach, "Attribute definition '/' is obsolete; use './' instead"); if( $1 != NULL ) $1->Fix(); $$ = $1; } 
	| introdef ']' '/'        { INWARN(PARM->mach, "Attribute definition '/' is obsolete; use './' instead"); if( $1 != NULL ) $1->Fix(); $$ = $1; } 
;

introdef:
	  subsumedef '+' newfeat   { if( $1 != NULL ) $1->AddFeature($3.feat, $3.app); $$ = $1; }
	| subsumedef ']' '+' newfeat   { if( $1 != NULL ) $1->AddFeature($4.feat, $4.app); $$ = $1; }
	| subsumedef '+' '[' newfeat   { if( $1 != NULL ) $1->AddFeature($4.feat, $4.app); $$ = $1; }
	| subsumedef ']' '+' '[' newfeat   { if( $1 != NULL ) $1->AddFeature($5.feat, $5.app); $$ = $1; }
	| introdef ',' newfeat     { if( $1 != NULL ) $1->AddFeature($3.feat, $3.app); $$ = $1; }
;

subsumedef:
	  newtype SUBSUMED_BY atom { if( $1 != NULL && $3 != NULL ) $1->SetAsChildOf($3); $$ = $1; }
	| newtype SUBSUMED_BY '[' atom { if( $1 != NULL && $4 != NULL ) $1->SetAsChildOf($4); $$ = $1; }
	| subsumedef ',' atom      { if( $1 != NULL && $3 != NULL ) $1->SetAsChildOf($3); $$ = $1; }
;

apptype:
	  /* empty */			{ $$ = bot; }
	| atom					{ $$ = $1; }
;

optional_equal:
	  /* empty */
	| '=';

optional_sharp:
	  /* empty */
	| '#';

atom_or_atomop:
		TOK_ATOM | TOK_IS %prec ATOMP | TOK_MOD %prec ATOMP ;

atom_with_module:     /* add modulename if atom does not have it*/
          atom_or_atomop
{
	if( progalter && !bi_type($1))
	{
		$$ = (type *)module::CurrentModule()->Search(progalter_tprefix + $1);
		if( $$ == NULL )
		{
			$$ = (type *)module::CurrentModule()->Search($1);
			if( $$ != NULL && ! $$->IsBuiltIn() )
				$$ = NULL;
		}
	      }
	else 
	  {
	    $$ = (type *)module::CurrentModule()->Search(($1));
	  }
	if( $$ == NULL ) 
	{
//		INWARN(PARM->mach, "Type '" << $1 << "' is undefined; treated as a subtype of 'bot'");
		$$ = new type(progalter && !bi_type($1) ? (progalter_tprefix + $1) : $1, module::CurrentModule());
		$$->SetAsChildOf(bot);
		$$->Fix();
		INWARN(PARM->mach, $$->GetProperName() << " <- [bot]. is assumed implicitly");
		procedure::NewTypeNotify();
		if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
			prog_types.Follow(hd).Coerce($$);
			prog_types = prog_types.Follow(tl);
		}
	}
}
          | modulename atom_or_atomop
{
          module *m = module::SearchModule($1);
	  if(m == NULL)
	    {
	      INERR(PARM->mach,"Could not find module " << $1 << ".");
	      errored = true;
	      $$ = NULL;
	    }
	  else 
	    {
	      type * m_type = m->Search($2);
	      if(m_type == NULL)
		{
		  INERR(PARM->mach,"Could not find atom " << $1 << ":" << $2 <<".");
		  errored = true;
		  $$ = NULL;
		}
	      else $$ = m_type;
	    }
	}
;

feature_with_module:     /* add modulename if feature does not have it */
          TOK_FEAT
			{
				type *f =  module::CurrentModule()->Search($1);

				if (f == NULL)
				{
					INERR(PARM->mach,"Unknown feature name '" << $1 << "'.");
					errored = true;
					$$ = NULL;
				}
				else 
					$$ = (type *)f;
			}
          | modulename TOK_FEAT
			{
				module * m = module::SearchModule($1);
				if(m ==NULL)
				{
					INERR(PARM->mach,"Unknown module name '"<< $1 << "'.");
					errored = true;
					$$ = NULL;
				}
				else 
				{
				type *f = m->Search($2);
					if(f == NULL)
					{
						INERR(PARM->mach,"Unknown feature name '"<< $1 << ":"<< $2 <<"'.");
						errored = true;
						$$ = NULL;
					}
					else
						$$ = f;
				}
			}
;

optional_module_qualifier:
		/* empty */  { $$ = module::CurrentModule(); }
	| modulename { $$ = module::SearchModule($1); };

newfeat:
/*	  feature_with_module apptype precedence */
	  optional_module_qualifier TOK_FEAT optional_equal apptype
		{ 
			string fname = (!bi_type($2)) ? $2 : (progalter_fprefix + $2);
			if( ($$.feat = (feature *)feature::Search(module::CurrentModule()->Search(fname))) == NULL )
			{
				$$.feat = new feature(fname, $1, 0/*$4*/,module::CurrentModule()->GetInterfaceMode()); 
				if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
					prog_types.Follow(hd).Coerce($$.feat->GetRepType());
					prog_types = prog_types.Follow(tl);
				}
			}
			$$.app = $4;
		}
	| optional_module_qualifier TOK_FEAT optional_equal apptype '#' TOK_INTEGER
		{ 
			string fname = (!progalter || bi_feature($2)) ? $2 : (progalter_fprefix + $2);
			if( ($$.feat = (feature *)feature::Search((module::CurrentModule()->Search(fname)))) == NULL )
			{
				$$.feat = new feature(fname, $1, $6,module::CurrentModule()->GetInterfaceMode()); 
				if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
					prog_types.Follow(hd).Coerce($$.feat->GetRepType());
					prog_types = prog_types.Follow(tl);
				}
			}
			$$.app = $4;
		}
	| optional_module_qualifier TOK_FEAT optional_equal apptype optional_sharp '(' TOK_INTEGER ')'
		{ 
			string fname = (!progalter || bi_feature($2)) ? $2 : (progalter_fprefix + $2);
			if( ($$.feat = (feature *)feature::Search((module::CurrentModule()->Search(fname)))) == NULL )
			{
				$$.feat = new feature(fname, $1, $7,module::CurrentModule()->GetInterfaceMode()); 
				if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
					prog_types.Follow(hd).Coerce($$.feat->GetRepType());
					prog_types = prog_types.Follow(tl);
				}
			}
			$$.app = $4;
		}
	| optional_module_qualifier TOK_FEAT optional_equal TOK_STARTP TOK_INTEGER ')'
		{ 
			string fname = (!progalter || bi_feature($2)) ? $2 : (progalter_fprefix + $2);
			if( ($$.feat = (feature *)feature::Search((module::CurrentModule()->Search(fname)))) == NULL )
				$$.feat = new feature(fname, $1, $5,module::CurrentModule()->GetInterfaceMode()); 
			type *tt;
			if( progalter && !bi_type($4) )
			{
				tt = (type *)($1->Search(progalter_tprefix + $4));
				if( tt == NULL )
				{
					tt = (type *)($1->Search($4));
					if( tt != NULL && ! tt->IsBuiltIn() )
						tt = NULL;
				}
			}
			else
				tt = (type *)$1->Search($4);

			if( tt == NULL ) 
			{
//				INWARN(PARM->mach, "Type '" << $4 << "' is undefined; treated as a subtype of 'bot'");
				tt = new type($4, $1);
				tt->SetAsChildOf(bot);
				tt->Fix();
				INWARN(PARM->mach, tt->GetProperName() << " <- [bot]. is assumed implicitly");
				procedure::NewTypeNotify();
				if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
					prog_types.Follow(hd).Coerce(tt);
					prog_types = prog_types.Follow(tl);
				}
			}
			if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
				prog_types.Follow(hd).Coerce($$.feat->GetRepType());
				prog_types = prog_types.Follow(tl);
			}
			$$.app = tt;
		}
;


newtype:
	 TOK_ATOM			
	 	{ 
			module::UseCurrentModule();
			string tname = !progalter ? $1 : (progalter_tprefix + $1);
			const type *t = module::CurrentModule()->Search(tname);
			if( t != NULL && t->GetModule() == module::CurrentModule() )
			{
				INERR(PARM->mach,"Type '" << $1 << "' is defined twice");
				errored = true;
				$$ = NULL;
			}
			else
			{
				$$ = new type(tname,module::CurrentModule(),module::CurrentModule()->GetInterfaceMode()); 
			}
		}
;

/************************************************************************/

command: c_desc period
	{
//		hook();
//		int slot = PARM->mach->GetCurrentSlot();
		bool say_no = false;
		lilfesFlexLexer *prevlexer = PARM->mach->getLexer();
		
		if( $1 == CORE_P_INVALID )
		{
			if( ! errored )
				INERR(PARM->mach,"Cannot generate a feature structure");
			errored = true;
		}
		else 
		{
			FSP rfs(PARM->mach, $1);	// root feature structure
			const type *t = rfs.GetType();

			if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() )
			{
//				std::cout << "---" << std::endl;
//				std::cout << rfs.DisplayAVM();
//				std::cout << (ptr_int)rfs.GetType() << std::endl;
//				std::cout << (ptr_int)type::Search(":-/1") << std::endl;

                          // include/1 is no more implemented
// 				if( rfs.GetType() == module::CoreModule()->Search(":-/1") &&
// 					( rfs.Follow(f_cbody).GetType()->IsSubType(module::BuiltinModule()->Search("include")) ) )
// 				{
// 					// Pasted from include/1
// 					FSP arg1 = rfs.Follow(f_cbody).Follow(f_arg[1]);
// 					if( ! arg1.IsString() )
// 					{
// 						INERR(PARM->mach,"include/1 requires a file name");
// 					}
// 					else
// 					{
// 						char *buf = new char[strlen(arg1.ReadString()) + strlen(PARM->mach->getLexer()->InFile())];
// 						char *p;
						
// 						strcpy(buf, PARM->mach->getLexer()->InFile());
// 					#ifdef WIN32
// 						p = strrchr(buf, '\\');
// 					#else
// 						p = strrchr(buf, '/');
// 					#endif
// 						if( p == NULL )
// 							strcpy(buf, arg1.ReadString());
// 						else
// 							strcpy(p+1, arg1.ReadString());
						
// 					//	std::cout << buf << std::endl;
// 						if( (! pushfile( PARM->mach, buf, PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) ) )
// 							if( (! pushfile( PARM->mach, arg1.ReadString(), PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() )) )
// 							{
// 								INERR(PARM->mach,"Cannot open include file: '" << arg1.ReadString() << "'");
// 							}
// 						delete buf;
// 					}
// 				}
//                              else
                          if( rfs.GetType() == module::CoreModule()->Search(":-/1") &&
					(rfs.Follow(f_cbody).GetType()->GetModule() == module::DirectiveModule()) )
				{
					FSP arg1 = rfs.Follow(f_cbody);
					const type *t = arg1.GetType();

					procedure *pc = procedure::Search(t);
					procthunk *pt = pc->GetThunk();
					ASSERT( pt != NULL );

					int nf = t->GetNFeatures();
					PARM->mach->SetRP(nf);
					if( arg1.IsLeaf() )
					{
						for( int i=0; i<nf; i++ )
							PARM->mach->WriteLocal(i, PTR2c(FSP(PARM->mach).GetAddress()));
					}
					else
					{
						for( int i=0; i<nf; i++ )
							PARM->mach->WriteLocal(i, PTR2c(arg1.FollowNth(i).GetAddress()));
					}

					pt->exec(*PARM->mach);
				}
				else {
//				std::cout << "---" << std::endl;
					if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() != 2 )	// 2 means ensure_in_prog...
					{
						prog_list.Follow(hd).Unify(rfs);
						prog_list.Follow(f_lilfesfileinfo)
								 .Follow(f_lilfesfilename)
								 .Unify(FSP(PARM->mach,(PARM->mach->getLexer()->InFile())));
						prog_list.Follow(f_lilfesfileinfo)
								 .Follow(f_lilfesfileline)
								 .Unify(FSP(PARM->mach,(mint)(PARM->mach->getLexer()->Lineno())));
						prog_list = prog_list.Follow(tl);
					}
					if( PARM->mach->getLexer()->ProgVars() )
					{
						FSP varlist = prog_vars.Follow(hd);
						for( _HASHMAP<string, parsevar *>::const_iterator it = vars.begin();
						it != vars.end(); ++it )
						{
							parsevar *pv = it->second;
							if( it->second->GetName()[0] != '_')
							{
								FSP v = varlist.Follow(hd);
								v.Follow(hd).Unify(FSP(PARM->mach, pv->GetName())); 
								v.Follow(tl).Follow(hd).Unify(FSP(PARM->mach, pv->addr())); 
								v.Follow(tl).Follow(tl).Coerce(nil);
								varlist = varlist.Follow(tl);
							}
						}
						varlist.Coerce(nil);
						prog_vars = prog_vars.Follow(tl);
					}
				}
			}
			else if( rfs.IsLeaf() && t->GetIndex(f_cbody) != -1 )
			{
				INERR(PARM->mach,"Missing clause body");
				errored = true;
			}
			else
			{
				if( rfs.GetType()->IsSubType(t_query) )	// Add DispResult
				{
					FSP varlist(PARM->mach), varlisttop = varlist;
					for( _HASHMAP<string, parsevar *>::const_iterator it = vars.begin();
						 it != vars.end(); ++it )
					{
						parsevar *pv = it->second;
						if( pv->GetName()[0] != '_')
						{
							FSP v = varlist.Follow(hd);
							v.Follow(hd).Unify(FSP(PARM->mach, pv->GetName())); 
							v.Follow(tl).Follow(hd).Unify(FSP(PARM->mach, pv->addr())); 
							v.Follow(tl).Follow(tl).Coerce(nil);
							varlist = varlist.Follow(tl);
						}
					}
					varlist.Coerce(nil);

					FSP newrfs(PARM->mach);
					newrfs.Coerce(module::CoreModule()->Search(":-/1"));
					newrfs.Follow(f_cbody).Coerce(module::CoreModule()->Search(","));
					newrfs.Follow(f_cbody).Follow(f_arg[1]).Unify(rfs.Follow(f_cbody));
					newrfs.Follow(f_cbody).Follow(f_arg[2]).Coerce(module::BuiltinModule()->Search("disp_result"));
					newrfs.Follow(f_cbody).Follow(f_arg[2]).Follow(f_arg[1]).Unify(varlisttop);

					rfs = newrfs;
					say_no = true;
				}
				else
				{
					if( t->GetIndex(f_chead) != -1 )
						CheckSingleton(PARM->mach,rfs.Follow(f_chead).GetType()->GetProperName());
					else if( t->IsSubType(t_pred[0]) )
					{
						if( t == t_cut || t == t_comma || t == t_semicolon || t->IsSubType(t_equal) )
						{
							INERR(PARM->mach,t->GetProperName() << " - not redefinable");
							goto ERR_EXIT;
						}
						else
							CheckSingleton(PARM->mach, rfs.GetType()->GetName());
					}
				}

//				std::cout << rfs.DisplayAVM();
				{
					if( t->GetIndex(f_chead) != -1 )
					{
						// p :- q ... Optimize equals
						//PARM->mach->EnableConstraint(true);
						PARM->mach->EnableConstraint(constraint_status);
						while(1) {
						//while(0) {
							FSP f = rfs.Follow(f_cbody);
							if( f.GetType()->IsSubType(t_equal) )
								if( f.Follow(f_arg[1]).Unify(f.Follow(f_arg[2])) )
								{
									rfs = FSP(rfs.Follow(f_chead));
									break;
								}
								else
								{
									// warn?
									break;
								}
							else 
								if( f.GetType()->IsSubType(t_comma) &&
									f.Follow(f_arg[1]).GetType()->IsSubType(t_equal) )
								{
									FSP g = f.Follow(f_arg[1]);
									if( g.Follow(f_arg[1]).Unify(g.Follow(f_arg[2])) )
									{
										FSP h(PARM->mach);
										h.Follow(f_chead).Unify(rfs.Follow(f_chead));
										h.Follow(f_cbody).Unify(f.Follow(f_arg[2]));
										rfs = h;
										continue;
									}
									else
									{
										// warn?
										break;
									}
								}
								else
									break;
						}
						PARM->mach->EnableConstraint(false);
					}
				}
				
				ShareList sl;
				sl.AddMark(rfs);
				sl.MakeShare();
				
				ptree *pt = GenPTreeOfClause(sl, rfs);
				extern ptree *parsetop; parsetop = pt;
				if( pt != NULL ) 
				{
	#ifdef DEBUG
//					std::cout << pt->OutputTree();
	#endif
					parseinfo *info = new parseinfo(PARM->mach);
					pt->CollectInfo(0, info);
					codelist *l = pt->Compile(0, info);
					l->EndCode();
	#ifdef DEBUG
					if( DEB )
					{
						l->DebugOutput();
						std::cout << "toparg : " << (info->toparg ? info->toparg->GetName() : "(none)") << std::endl;
					}
	#endif
//					if (! strcmp(((pclause *)pt)->GetRepType()->GetSimpleName(), "assert_cky_lexical") )
//						l->DebugOutput();
//					if (! strcmp(((pclause *)pt)->GetRepType()->GetSimpleName(), "unary_rule_check") )
//						l->DebugOutput();

					if( t->GetIndex(f_chead) != -1 || t->IsSubType(pred))	// in case of p :- q...
					{
						procedure *p = procedure::New(((pclause *)pt)->GetRepType(), info->ndefarg);
						if     ( info->is_toparg && info->toparg == t_int )
							p->AddCodelistINT(*l, info->toparg, info->toparg_int);
						else if( info->is_toparg && info->toparg == t_stg )
							p->AddCodelistSTG(*l, info->toparg, info->toparg_stg);
						else
							p->AddCodelist(*l, info->toparg);
					}
					else if( pt->GetClassName() == pclause::className )
					{
	#ifdef DEBUG
						if( DEB )
						{
							std::cout << "@@VARS@@" << std::endl;
							for( _HASHMAP<string, parsevar *>::const_iterator it = vars.begin();
								 it != vars.end(); ++it )
							{
							 	parsevar *it = it->second;
								std::cout << pv->GetName() << ": " << std::hex << cp2int(pv->addr()) << std::dec << std::endl;
							}
						}
	#endif
	#if defined(PROFILE)
						delete profx;
	#endif	/* defined(PROFILE) */
						vars.clear();
						bool result;
						{
							P1("Running");
							PARM->mach->EnableConstraint(constraint_status);
							result = PARM->mach->Execute(*l);
							constraint_status = PARM->mach->GetConstraintEnabled();
							PARM->mach->EnableConstraint(false);
						}
						if( say_no )
							(*output_stream) << (result ? "yes" : "no") << std::endl;
	#if defined(PROFILE)
						profx = new prof("Compiling");
	#endif	/* defined(PROFILE) */
					delete[] ((code *)*l);
					}
					delete l;
					delete pt;
					for (PInfoMap::const_iterator it = info->hash.begin(); it != info->hash.end(); ++it) {
						delete it->second;
					}
					delete info;
				}
				else
				{
					INERR(PARM->mach,"Cannot generate parse tree");
				}
				if( PARM->mach->getLexer() && ! PARM->mach->getLexer()->ProgInput() )
				{
					PARM->mach->SwitchOldHeap(save);
					PARM->mach->ClearStack();
					save = PARM->mach->SwitchNewHeap();
				}
			}
		}
ERR_EXIT:
		vars.clear();
		if( PARM->mach->getLexer() == prevlexer && ! PARM->mach->getLexer()->IsSkip())
			module::UseCurrentModule();
	};

x_desc:  feature_with_module %prec FEATURE i_desc
			{ 
				const feature *f;

				if( $1 == NULL || (f = feature::Search($1)) == NULL || $2 == CORE_P_INVALID )
				{
					$$ = CORE_P_INVALID;
				} 
				else {
					$$ = PARM->mach->AllocNew(); 
					core_p p = PARM->mach->FollowFeature($$, f);
					if( PARM->mach->Unify(p, $2) == false )
					{
						INERR(PARM->mach,"Feature '" << f->GetName() << "' is restricted to '" << f->GetBaseType()->GetAppType(f)->GetName() << "', and cannot take '" << FSP(PARM->mach,$2).GetType()->GetName() << "'");
						$$ = CORE_P_INVALID;
					      }
				      }
			      }
		| TOK_VAR
			{
				if( $1[0] == '_' && $1[1] == '\0' )
				{
					$$ = PARM->mach->AllocNew();
				}
				else
				{
					_HASHMAP<string, parsevar *>::const_iterator it = vars.find($1);
					if( it == vars.end() )
					{
						it = vars.insert(make_pair($1, new parsevar($1, PARM->mach->AllocNew()))).first;
					}
					it->second->IncCount();
					$$ = it->second->addr();
				}
			}
		| atom
			{
				if( $1 == NULL )
					$$ = CORE_P_INVALID;
				else
					$$ = PARM->mach->AllocNew(VAR2c($1));
//				{ $$ = PARM->mach->AllocNew(); FSP(PARM->mach, $$).Coerce($1);}
			}
		| startp arglist ')'
			{
				$$ = $2;
				if( $1 == NULL || $2 == CORE_P_INVALID )
				  {
				    $$ = CORE_P_INVALID;
				  }
				else 
				{
					int i;
					type *t1 = $1;
					type *t2 = (type *)c2STR(PARM->mach->ReadHeap(PARM->mach->Deref($2)));
					type *t3 = (type *)t1->TypeUnify(t2);
					// Search the arity
					cell c = PARM->mach->ReadHeap(PARM->mach->Deref($2));
					for( i=0; i<MAXARGS; i++ )
						if( c == STR2c(t_pred[i]) )
							break;

					if( i >= MAXARGS )
						ABORT("Arglist contains unknown type??");
					
					if( t3 == NULL )
					{
						if( t1->TypeUnify(pred) == NULL )
						{
							INERR(PARM->mach,"Type '" << t1->GetName() << "' used as a " << i << "-arity predicate");
							$$ = CORE_P_INVALID;
						}
						else if( t1->TypeUnify(t_pred[1]) == NULL )
						{
							std::ostringstream oss;
							oss << t1->GetSimpleName() << "/" << i;

							if( t1->GetModule()->Search(oss.str().c_str()) )
							{
								INERR(PARM->mach,"Functor " << t1->GetProperName() << " cannot be used;  Existing type " << (t1->GetModule()->Search(oss.str().c_str()))->GetProperName() << " prevents internal type generation");
								$$ = CORE_P_INVALID;
							} else {
								t3 = new type(oss.str().c_str(),t1->GetModule(),t1->IsExported()); t3->SetAsChildOf(t1); t3->SetAsChildOf(t2);
								t3->Fix();
								procedure::NewTypeNotify();
								if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
									prog_types.Follow(hd).Coerce(t3);
									prog_types = prog_types.Follow(tl);
								}
							}
						}
						else
						{ // we have no type for a correct arity
							INERR(PARM->mach,"Predicate '" << t1->GetName() << "' cannot take " << i << "-arity");
							$$ = CORE_P_INVALID;
						}
					}
					else if( i < MAXARGS-1 && t1->TypeUnify(t_pred[i+1]) == t3 )
					{
						// we have no type for a correct arity
						INERR(PARM->mach,"Predicate '" << t1->GetName() << "' cannot take " << i << "-arity");
						$$ = CORE_P_INVALID;
					}
					else if( i==3 && (t1==t_plus || t1==t_minus || t1==t_aster || t1==t_slash || t1==t_intdiv || t1==t_modulo ) )
					{
						INWARN(PARM->mach, "Obsolete Usage of '" << t1->GetName() << "'; use 'is'-expression instead");
					}
					
					if( $$ != CORE_P_INVALID )
						if( PARM->mach->CoerceType($2, $1) == false )
						{
							if( $1 != NULL && $2 != CORE_P_INVALID )
								ABORT("arglist failed?");
							$$ = CORE_P_INVALID;
						}
				      }
			      }
			      
		| '[' lists ']'
			{
				$$ = $2;
			}
		| TOK_INTEGER
			{
				FSP F(PARM->mach, (mint)$1);
				$$ = F.GetAddr();
			}
		| TOK_DOUBLE
			{
				FSP F(PARM->mach, (mfloat)$1);
				$$ = F.GetAddr();
			}
		| TOK_STRING
			{
				FSP F(PARM->mach, $1);
				$$ = F.GetAddr();
			}
;

desc:     x_desc
		| '(' c_desc ')'
			{
				$$ = $2;
			}
;

i_desc:	  desc
        | ix_desc;

ix_desc:  i_desc '+' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_plus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '-' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_minus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '^' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_circm));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '*' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_aster));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '/' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_slash));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc BIT_AND i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitand));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc BIT_OR i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitor));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '#' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitxor));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| '\\' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitnot));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '=' i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_equal));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc NOT_UNIFIABLE i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_notuni));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| '+' %prec USIGN i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_plus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| '-' %prec USIGN i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_minus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc '&' i_desc
			{
				$$ = $1;
				if( PARM->mach->Unify($1, $3) == false )
				{
					if( $1 != CORE_P_INVALID && $3 != CORE_P_INVALID )
					{
						INERR(PARM->mach,"Cannot unify these two TFSs");
						outform of("Cannot unify: ");
						    of.AddAtRight(FSP(PARM->mach,$1).DisplayAVM());
						of.AddAtRight(outform(" & "));
						    of.AddAtRight(FSP(PARM->mach,$3).DisplayAVM());
						std::cout << of;
					}
					$$ = CORE_P_INVALID;
				}
			}
		| i_desc MEANS i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_means));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_chead), $1) == false
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_cbody), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc MACRO_DEF i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_macro));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| MEANS %prec UMEANS i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_means));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_cbody), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| QUERY %prec UMEANS i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_query));
				if( PARM->mach->Unify(PARM->mach->FollowFeature(PARM->mach->FollowFeature($$, f_cbody), hd), $2) == false
				 || PARM->mach->CoerceType(PARM->mach->FollowFeature(PARM->mach->FollowFeature($$, f_cbody), tl), nil) == false )
					$$ = CORE_P_INVALID;
			}
		| NOT i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_not));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc TOK_IS i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_is));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc TOK_MOD i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_modulo));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc INTDIV i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_intdiv));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc CMP_L i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpl));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc CMP_LE i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmple));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc CMP_G i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpg));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc CMP_GE i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpge));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc CMP_EQ i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpeq));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| i_desc CMP_NE i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpne));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
;

c_desc:	  desc
		| cx_desc;
cx_desc:
		   c_desc '+' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_plus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '-' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_minus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '^' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_circm));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '*' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_aster));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '/' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_slash));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc BIT_AND c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitand));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc BIT_OR c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitor));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '#' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitxor));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| '\\' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_bitnot));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '=' c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_equal));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc NOT_UNIFIABLE c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_notuni));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| '+' %prec USIGN c_desc 
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_plus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| '-' %prec USIGN c_desc 
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_minus));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc '&' c_desc
			{
				$$ = $1;
				if( PARM->mach->Unify($1, $3) == false )
				{
					if( $1 != CORE_P_INVALID && $3 != CORE_P_INVALID )
					{
						INERR(PARM->mach,"Cannot unify these two TFSs");
						outform of("Cannot unify: ");
						of.AddAtRight(FSP(PARM->mach,$1).DisplayAVM());
						of.AddAtRight(outform(" & "));
						of.AddAtRight(FSP(PARM->mach,$3).DisplayAVM());
						std::cout << of;
					}
					$$ = CORE_P_INVALID;
				}
			}
		| c_desc MEANS c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_means));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_chead), $1) == false
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_cbody), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc MACRO_DEF c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_macro));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| MEANS %prec UMEANS c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_means));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_cbody), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| QUERY %prec UMEANS c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_query));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_cbody), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| NOT i_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_not));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $2) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc ',' c_desc
			{
				if( $1 == CORE_P_INVALID || $3 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else {
					$$ = PARM->mach->AllocNew(VAR2c(t_comma));
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, f_arg[1]), $1);
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, f_arg[2]), $3);
				}
			}
		| c_desc ';' c_desc
			{
				if( $1 == CORE_P_INVALID || $3 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else {
					$$ = PARM->mach->AllocNew(VAR2c(t_semicolon));
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, f_arg[1]), $1);
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, f_arg[2]), $3);
				}
			}
		| c_desc IF_THEN c_desc
			{
				if( $1 == CORE_P_INVALID || $3 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else {
					$$ = PARM->mach->AllocNew(VAR2c(t_ifthen));
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, f_arg[1]), $1);
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, f_arg[2]), $3);
				}
			}
		| c_desc TOK_IS c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_is));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc TOK_MOD c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_modulo));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc INTDIV c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_intdiv));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc CMP_L c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpl));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc CMP_LE c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmple));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc CMP_G c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpg));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc CMP_GE c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpge));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc CMP_EQ c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpeq));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
		| c_desc CMP_NE c_desc
			{
				$$ = PARM->mach->AllocNew(VAR2c(t_cmpne));
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false 
				 || PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[2]), $3) == false )
					$$ = CORE_P_INVALID;
			}
;

arglist:
		  i_desc
			{
				$$ = PARM->mach->AllocNew();
				if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[1]), $1) == false )
					$$ = CORE_P_INVALID;
			}
		| arglist ',' i_desc
			{
				int i;
				if( $1 == CORE_P_INVALID || $3 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else
				{
					cell c = PARM->mach->ReadHeap(PARM->mach->Deref($1));
					for( i=0; i<MAXARGS; i++ )
						if( c == STR2c(t_pred[i]) )
							break;
					if( i >= MAXARGS )
						ABORT("Arglist contains unknown type?");
					else if( i == MAXARGS-1 )
						INERR(PARM->mach,"Argument list length over " << MAXARGS);
					else
					{
						$$ = $1;
						if( PARM->mach->Unify(PARM->mach->FollowFeature($$, f_arg[i+1]), $3) == false )
							$$ = CORE_P_INVALID;
					}
				}
			}
;

lists:
		  /* empty */
		    { $$ = PARM->mach->AllocNew(VAR2c(nil)); }
		| nelistp
			{ $$ = $1; }
;

nelistp:
		  i_desc %prec LISTEND  
			{
				if( $1 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else {
					$$ = PARM->mach->AllocNew();
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, hd), $1);
					PARM->mach->CoerceType(PARM->mach->FollowFeature($$, tl), nil);
				}
			}
		| i_desc '|' i_desc
			{
				if( $1 == CORE_P_INVALID || $3 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else {
					$$ = PARM->mach->AllocNew();
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, hd), $1);
					if( PARM->mach->Unify     (PARM->mach->FollowFeature($$, tl), $3) == false )
					{
						INERR(PARM->mach,"Tail of a list is restricted to 'list', and cannot take '" << FSP(PARM->mach,$3).GetType()->GetName() << "'");
//						outform of("Containee: ");
//						of.AddAtRight(FSP(PARM->mach, $3).DisplayAVM());
//						std::cout << of;
						$$ = CORE_P_INVALID;
					}
				}
			}
		| i_desc ',' nelistp
			{
				if( $1 == CORE_P_INVALID || $3 == CORE_P_INVALID )
					$$ = CORE_P_INVALID;
				else {
					$$ = PARM->mach->AllocNew();
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, hd), $1);
					PARM->mach->Unify     (PARM->mach->FollowFeature($$, tl), $3);
				}
			}
;



atom:
		  '+' %prec ATOMP		{ $$= (type *)t_plus; }
		| '-' %prec ATOMP		{ $$= (type *)t_minus; }
		| '*' %prec ATOMP		{ $$= (type *)t_aster; }
		| '/' %prec ATOMP		{ $$= (type *)t_slash; }
		| '=' %prec ATOMP		{ $$= (type *)t_equal; }
		| '^' %prec ATOMP		{ $$= (type *)t_circm; }
		| MACRO_DEF %prec ATOMP		{ $$= (type *)t_macro; }
		| MEANS %prec ATOMP		{ $$= (type *)t_means; }
		| QUERY %prec ATOMP		{ $$= (type *)t_query; }
		| atomtok  %prec ATOMP  { $$ = $1; }
		| feature_with_module %prec ATOMP	{ $$ = $1; }
;

atomtok:
	  atom_with_module  %prec ATOMP
	  ;

startp:
		  TOK_STARTP
			{ 
				if( progalter && !bi_type($1) )
				{
					$$ = (type *)module::CurrentModule()->Search(progalter_tprefix + $1);
					if( $$ == NULL )
					{
						$$ = (type *)module::CurrentModule()->Search($1);
						if( $$ != NULL && ! $$->IsBuiltIn() )
							$$ = NULL;
					}
				}
				else
					$$ = (type *)module::CurrentModule()->Search($1);

				if( $$ == NULL ) 
				{ // This is predicate; make the entry
//					INWARN(PARM->mach, "Type '" << $1 << "' is undefined; treated as a subtype of 'pred'");
					$$ = new type(progalter ? (progalter_tprefix + $1) : $1,module::CurrentModule());
					$$->SetAsChildOf(pred);
					$$->Fix();
					INWARN(PARM->mach, $$->GetProperName() << " <- [pred]. is assumed implicitly");
					procedure::NewTypeNotify();
					if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {

						prog_types.Follow(hd).Coerce($$);
						prog_types = prog_types.Follow(tl);
					}
				}
				else
				{
					if( $$->TypeUnify(pred) == NULL )
					{
						INERR(PARM->mach,"Type '" << $1 << "' is not a subtype of 'pred' but used as a predicate");
						$$ = NULL;
					}
				}
			}
                  | modulename TOK_STARTP
                        {
                                module *m = module::SearchModule($1);
                                if (m == NULL)
                                        {
					  INERR(PARM->mach,"Unknown module name \"" << $1 <<"\"");
					  errored = true;
					  $$ = NULL;
					}
				if( progalter )
				{
					$$ = (type *)m->Search(progalter_tprefix + $2);
					if( $$ == NULL )
					{
						$$ = (type *)m->Search($2);
						if( $$ != NULL && ! $$->IsBuiltIn() )
							$$ = NULL;
					}
				}
				else
					$$ = (type *)m->Search($2);

				if( $$ == NULL ) 
				{ // This is predicate; make the entry
//					INWARN(PARM->mach, "Type '" << $2 << "' is undefined; treated as a subtype of 'pred'");
					$$ = new type(progalter ? (progalter_tprefix + $2) : $2,m);
					$$->SetAsChildOf(pred);
					$$->Fix();
					INWARN(PARM->mach, $$->GetProperName() << " <- [pred]. is assumed implicitly");
					procedure::NewTypeNotify();
					if( PARM->mach->getLexer() && PARM->mach->getLexer()->ProgInput() ) {
						prog_types.Follow(hd).Coerce($$);
						prog_types = prog_types.Follow(tl);
					}
				}
				else
				{
					if( $$->TypeUnify(pred) == NULL )
					{
						INERR(PARM->mach,"Type " << $2 << " is not a subtype of 'pred' but used as a predicate");
						$$ = NULL;
					}
				}
			};


modulename:
		  TOK_MODULENAME { $$ = $1; }
		| TOK_MODULENAME modulename { 
				$$ = my_malloc( strlen($1)+strlen($2)+2 );
				strcpy($$, $1);
				strcat($$, ":");
				strcat($$, $2);
			};

%%

struct token_key tokens[] =
{
		{"+", '+'},
		{"-", '-'},
		{"*", '*'},
		{"/", '/'},
		{"=", '='},
		{"&", '&'},
		{"\\", '\\'},
		{"#", '#'},
		{";", ';'},
		{":", ':'},
		{"^", '^'},
		{"<-", SUBSUMED_BY },
		{"?-", QUERY },
		{":-", MEANS },
		{"<=", CMP_LE },
		{"<",  CMP_L },
		{">=", CMP_GE },
		{">", CMP_G },
		{"=:=", CMP_EQ },
		{"=\\=", CMP_NE },
		{":=", MACRO_DEF },
		{"//", INTDIV },
		{"->", IF_THEN },
		{"\\+", NOT },
		{"\\=", NOT_UNIFIABLE },
		{"\\/", BIT_OR },
		{"/\\", BIT_AND },
		{"./",  ATTR_DEF },
		{NULL, 0}
};

_HASHMAP<string, token_key *> *token_hash;


static void init() 
{ 
#ifdef DEBUG
	std::cout << "Initialize " << __FILE__ << std::endl;
#endif
//	input_stream = & std::cin;
//	output_stream = & std::cout;
//	error_stream = & std::cerr;

	token_hash = new _HASHMAP<string, token_key *>;
	for( int i=0; tokens[i].str != NULL; i++ )
	{
//		std::cout << i << " " << tokens[i].str << " " << tokens[i].token_code << std::endl;
		token_hash->insert(make_pair(tokens[i].GetKey(), &tokens[i]));
	}
}
static void term() 
{ 
/*
	if( input_stream != &std::cin )
		delete input_stream;
	if( output_stream != &std::cout )
		delete output_stream;
	if( error_stream != &std::cerr )
		delete error_stream;
*/
	delete token_hash;
}
static InitFunction IF(init, 100);
static TermFunction TF(term, 100);

void yyerror(void *parm, const char* s) {
        INERR(((parser_control *)parm)->mach, s);
	//INERR(m, s);
}

void machine::parse(lexer_t lex)
{
#ifdef PROFILE
#if PROFILE >= 1
	profx = new prof("Compiling");
#endif
#endif
	save = SwitchNewHeap();
	lexer_t oldlex = getLexer();
#if YYDEBUG
	yydebug = 1;
#endif
	setLexer(lex);
	parser_control parm( this );
        constraint_status = GetConstraintEnabled();
        EnableConstraint( false );  // disable complex constraint during parsing
	while( yyparse(&parm) )
	{
		getLexer()->Flush(); 
		getLexer()->SetFirstRun();
	}
        EnableConstraint( constraint_status );
#ifdef PROFILE
#if PROFILE >= 1
	delete profx;
#endif
#endif
	setLexer(oldlex);
}

void machine::parse()
{
#ifdef PROFILE
#if PROFILE >= 1
	profx = new prof("Compiling");
#endif
#endif
	save = SwitchNewHeap();
	parser_control parm( this );
        constraint_status = GetConstraintEnabled();
        EnableConstraint( false );  // disable complex constraint during parsing
	while( yyparse(&parm) )
	{
		getLexer()->Flush(); 
		getLexer()->SetFirstRun();
	}
        EnableConstraint( constraint_status );
#ifdef PROFILE
#if PROFILE >= 1
	delete profx;
#endif
#endif
}


void CheckSingleton(machine *m, const string &where)
{
	P2("Check Singleton");

	ShareList sl;
	std::ostringstream oss;

	int nsingle = 0;

	for( _HASHMAP<string, parsevar *>::const_iterator it = vars.begin();
		 it != vars.end(); ++it )
	{
		parsevar *p = it->second;
		if( p->GetCount() == 1 && p->GetName()[0] != '_' )
		{
			if( nsingle == 0 )
			{
				oss << "[" << p->GetName();
			}
			else
			{
				oss << ", " << p->GetName();
			}
			nsingle++;
		}
	}

	if( nsingle != 0 )
	{
		oss << "]";

		if( nsingle == 1 )
		{
			INWARN(m, oss.str() << " - singleton variable in " << where);
		}
		else
		{
			INWARN(m, oss.str() << " - singleton variables in " << where);
		}
	}
}

#define STRBUFSIZE 2000

struct strbuf
{
	struct strbuf *next;
	char buf[STRBUFSIZE];
};

static struct strbuf *root = NULL;
static char *bufp = NULL;
static int bufremain = 0;

char *my_malloc(int len)
{
	char *ret;
	struct strbuf *newbuf;
	int buflen;
	
	if( bufremain >= len )
	{
		ret = bufp;
		bufp += len;
		bufremain -= len;
/*		printf("A:%08p\n", ret);*/
		return ret;
	}

	buflen = sizeof(struct strbuf);
	if( len > STRBUFSIZE )
	{
		buflen = buflen - STRBUFSIZE + len;
	}
	
	newbuf = (struct strbuf *)new char[buflen];
	newbuf->next = root;
	root = newbuf;
	bufp = newbuf->buf;
	bufremain = buflen - (sizeof(struct strbuf) - STRBUFSIZE);
	return my_malloc(len);
}


char *my_strdup(char *str)
{
	int len = strlen(str) + 1;

	return strcpy(my_malloc(len), str);
}

void free_dups()
{
/*	printf("F\n");*/
	struct strbuf *next;
	while( root != NULL )
	{
		next = root->next;
		delete[] (char *)root;
		root = next;
	}
	bufp = NULL;
	bufremain = 0;
}

void error_open_file(char *s)
{
	std::cout << "Error: Cannot open input file '" << s << "'" << std::endl;
}

void success_open_file(char *s)
{
	std::cout << s << ":" << std::endl;
}

int token_search(char *str)
{
	_HASHMAP<string, token_key *>::const_iterator it = token_hash->find(str);
// std::cout << "TOKEN: " << str << " " << (tk==NULL ? 0 : tk->token_code) << std::endl;
	if( it == token_hash->end() )
	{
		return TOK_ATOM;
	}
	else
	{
		return it->second->token_code;
	}
}

} // namespace
