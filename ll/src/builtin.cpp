/**
 * @module  = exec
 * @cvs     = $Id: builtin.cpp,v 1.23 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Execution control
 * @jdesc   = 実行制御
 * Basic builtin predicates for execution control
 * @japanese =
 * 実行制御のための基本的な組み込み述語です．
 * @end_japanese
*/

static const char rcsid[] = "$Id: builtin.cpp,v 1.23 2011-05-02 10:38:23 matuzaki Exp $";

#include "builtin.h"
#include "in.h"
#include "codelist.h"
#include "parse.h"
#include "module.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <cmath>

// Collection of builtin predicates in each file
// --- used to ensure these files are linked
namespace lilfes {

using std::copy;
using std::cout;
using std::endl;
using std::fstream;
using std::list;
using std::map;
using std::ostringstream;
using std::string;
using std::vector;

  namespace builtin {
    bool print(machine&, FSP);
    bool floor(machine&, FSP, FSP);
    bool load_so(machine&, FSP);
    bool regex_match(machine&, FSP, FSP);
    bool lilfes_version(machine&, FSP);
    bool ensure_loaded(machine&, FSP);
    bool lilfes_flag_2(machine&, FSP, FSP);
    bool start_prof(machine&, FSP);
    bool write_stream(machine&, FSP, FSP);
    bool strtolist(machine&, FSP, FSP);
    bool getenv(machine&, FSP, FSP);
    bool trace(machine&, FSP);
    bool tfstoxml(machine&, FSP, FSP);
  }
  namespace lildbm {
    bool open_db(machine&, FSP);
  }

  // To ensure all built-in predicates are included,
  // add a reference to this variable
  void* BUILTIN_PRED_SYMBOLS[] = {
    (void*)(lilfes::builtin::dynamic),         // builtin.cpp
    (void*)(lilfes::builtin::copy),            // builtinfs.cpp
    (void*)(lilfes::builtin::print),           // display.cpp
    (void*)(lilfes::builtin::floor),           // fariths.cpp
    (void*)(lilfes::builtin::load_so),         // load_so.cpp
    (void*)(lilfes::builtin::regex_match),     // lregex.cpp
    (void*)(lilfes::builtin::lilfes_version),  // misc.cpp
    (void*)(lilfes::builtin::ensure_loaded),   // module.cpp
    (void*)(lilfes::builtin::lilfes_flag_2),   // option.cpp
    (void*)(lilfes::builtin::start_prof),      // profile.cpp
    (void*)(lilfes::builtin::write_stream),    // stream.cpp
    (void*)(lilfes::builtin::strtolist),       // string.cpp
    (void*)(lilfes::builtin::getenv),          // system.cpp
    (void*)(lilfes::builtin::trace),           // trace.cpp
    (void*)(lilfes::builtin::tfstoxml),        // xml.cpp
    (void*)(lilfes::lildbm::open_db),          // lildbm-interface.cpp
    NULL
  };
}

//////////////////////////////////////////////////////////////////////
//
//  Built-in types
//

namespace lilfes {

const type * t_list, *cons, *nil;
const feature * hd, *tl;

const type * t_cut;
const type * t_equal;
const type * t_means;
const type * t_query;
const type * t_defclause;

const type * t_int;
const type * t_flo;
const type * t_chr;
const type * t_stg;

      type * pred;
      type * t_pred[MAXARGS];
const feature * f_arg[MAXARGS];

const feature * f_chead;
const feature * f_cbody;
const feature * f_lilfesfilename;
const feature * f_lilfesfileline;
const feature * f_lilfesfileinfo;
const type * t_not;
const type * t_notuni;
const type * t_ifthen;
const type * t_comma;
const type * t_circm;
const type * t_semicolon;

const type * t_macro;

      type * t_feature;

//////////////////////////////////////////////////////////////////////
//
//  Built-in predicates
//

namespace builtin {

/**
  * @predicate	= dynamic(+$PRED) 
  * @desc		= Declare to allow predicate <i>$PRED</i> to assert.
  * @param		= +$PRED/pred :predicate
  * @example	=
  > :- dynamic(cky_table).
  　
  Declare to allow predicate "cky_table" to assert.
  * @end_example

  * @jdesc		= 述語<i>$PRED</i>の assert を許すよう宣言します． 
  * @jparam		= +$PRED/pred :述語
  * @jexample	=
  > :- dynamic(cky_table).
  　
  述語"cky_table"の assert を許すよう宣言します． 
  * @end_jexample
*/
bool dynamic( machine&, FSP arg1 )
{
	const type *tf;

	while( arg1.GetType()->IsSubType(cons) && ! arg1.Follow(tl).GetType()->IsSubType(nil) )
	{
		tf = arg1.Follow(hd).GetType();
		if( ! tf->IsSubType(pred) || tf == pred )
		{
			RUNWARN("Warning: Predicate dynamic/1 cannot process non-predicates");
			return false;
		}
		else
		{
			if (procedure::Search(tf) != 0)
			{
				procedure::Search(tf)->SetDynamic();
			}
			((type*)(tf))->SetDynamic();
		}
		arg1 = arg1.Follow(tl);
	}
	if( arg1.GetType()->IsSubType(cons) )
	{
		arg1 = arg1.Follow(hd);
	}

	tf = arg1.GetType();
	if( ! tf->IsSubType(pred) || tf == pred )
	{
		RUNWARN("Warning: Predicate dynamic/1 cannot process non-predicates");
		return false;
	}
	else
	{
		if (procedure::Search(tf) != 0)
		{
			procedure::Search(tf)->SetDynamic();
		}
		((type*)(tf))->SetDynamic();
	}
  return true;
}

LILFES_BUILTIN_DIRECTIVE_1(dynamic, dynamic);

/**
  * @predicate	= is_dynamic(+$PRED) 
  * @desc		= Return true if predicate <i>$PRED</i> is dynamic.
  * @param		= +$PRED/pred :predicate
  * @example	= 
  > ?- is_dynamic(cky_table).
  yes
  　
  Return true if predicate "cky_table" is dynamic.
  * @end_example
  * @see = #dynamic/1

  * @jdesc		= 述語$PREDが dynamicかどうか判定します． 
  * @jparam		= +$NAME/pred :述語
  * @jexample	=
  > ?- is_dynamic(cky_table).
  yes
  　
  述語"cky_table"が dynamicかどうか判定します． 
  * @end_jexample
*/
bool is_dynamic( machine&, FSP arg1 )
{
	const type* tf;
	tf = arg1.GetType();
	if (tf->GetDynamic() == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

LILFES_BUILTIN_PRED_1(is_dynamic, is_dynamic);

//////////////////////////////////////////////////////////////////////////////
// multifile/1 : set it as multifile

/**
  * @predicate	= multifile(+$PRED) 
  * @desc		= Declare to allow predicate <i>$PRED</i> to describe its main definition on other module.
  * @param		= +$PRED/pred :predicate
  * @example	=
  > :- multifile(newwill_query).
  　
  Declare to allow predicate "newwill_query" to describe its main definition on other module.
  * @end_example

  * @jdesc		= 述語<i>$PRED</i>の本体定義を別のモジュールに記述できることを宣言します．  
  * @jparam		= +$PRED/pred :述語
  * @jexample	=
  > :- multifile(newwill_query).
  　
  述語"newwill_query"の本体定義を別のモジュールに記述できることを宣言します． 
  * @end_jexample
*/
bool multifile( machine&, FSP arg1 )
{
	const type *tf;

	while( arg1.GetType()->IsSubType(cons) && ! arg1.Follow(tl).GetType()->IsSubType(nil) )
	{
		tf = arg1.Follow(hd).GetType();
		if( ! tf->IsSubType(pred) || tf == pred )
		{
			RUNWARN("Warning: Predicate dynamic/1 cannot process non-predicates");
			return false;
		}
		else
		{
			if (procedure::Search(tf) != 0)
			{
				procedure::Search(tf)->SetMultifile();
			}
			((type*)(tf))->SetMultifile();
		}
		arg1 = arg1.Follow(tl);
	}
	if( arg1.GetType()->IsSubType(cons) )
	{
		arg1 = arg1.Follow(hd);
	}

	tf = arg1.GetType();
	if( ! tf->IsSubType(pred) || tf == pred )
	{
		RUNWARN("Warning: Predicate dynamic/1 cannot process non-predicates");
		return false;
	}
	else
	{
		if (procedure::Search(tf) != 0)
		{
			procedure::Search(tf)->SetMultifile();
		}
		((type*)(tf))->SetMultifile();
	}
	return true;
}

LILFES_BUILTIN_DIRECTIVE_1(multifile, multifile);

/**
  * @predicate	= is_multifile(+$PRED) 
  * @desc		= Return true if predicate <i>$PRED</i> is multifile.
  * @param		= +$PRED/pred :predicate
  * @example	= 
  > ?- is_multifile(newwill_query).
  yes
  　
  Return true if predicate "newwill_query" is dynamic.
  * @end_example
  * @see = #dynamic/1

  * @jdesc		= 述語$PREDが multifileかどうか判定します． 
  * @jparam		= +$NAME/pred :述語
  * @jexample	=
  > ?- is_multifile(newwill_query).
  yes
  　
  述語"newwill_query"が multifileかどうか判定します． 
  * @end_jexample
*/
bool is_multifile( machine&, FSP arg1 )
{
	const type* tf;
	tf = arg1.GetType();
	if (tf->GetMultifile() == true) {
		return true;
	}
	else
	{
		return false;
	}
}

LILFES_BUILTIN_PRED_1(is_multifile, is_multifile);

/**
  * @predicate	= disp_result(+FS)
  * @desc	= Display the result of a query and wait for input.
  * @param	= +FS/bot : feature structure
  * @example	= 
  * @end_example

  * @jdesc	= クエリーの結果を表示し入力を待ちます．
  * @jparam	= +FS/bot : 素性構造
  * @jexample	=
  * @end_jexample
*/
bool disp_result( machine& m, FSP arg1 )
{
	FSP f = arg1;
	ShareList sl; sl.AddMark(arg1); sl.MakeShare();

	int display = 0;
	
	while( f.GetType()->IsSubType(cons) )
	{
		FSP f1 = f.Follow(hd);
		FSP varname = f1.Follow(hd);
		FSP value = f1.Follow(tl).Follow(hd);

		outform of;
		if( varname.IsString() )
		{
			of = outform(varname.ReadString());
		}
		else
		{
			of = varname.DisplayAVM(sl);
		}
		of.AddAtRight(outform(": "));
		of.AddAtRight(value.DisplayAVM(sl));

		*output_stream << of;

		f = f.Follow(tl);

		display++;
	}

	if( display )
	{
		if (! m.getLexer()->Isatty())
		{
			*output_stream << endl;
		}
		else
		{
			m.getLexer()->Flush();
			while( input_stream->good() )
			{
				*output_stream << "Enter ';' for more choices, otherwise press ENTER --> ";
				string buf;
				getline(*input_stream, buf);
                                //cerr << (int)buf[0] << endl;
				if( buf[0]==';' )
				{
					m.getLexer()->Flush();
					return false;
				}
				else if( buf.size() == 0 )
				{
					m.getLexer()->Flush();
					return true;
				}
			}
		}
	}
  return true;
}

LILFES_BUILTIN_PRED_1(disp_result, disp_result);

/**
  * @predicate  = eval(+STRING, -PREDLIST)
  * @desc       = Analyse LiLFeS program STRING and return list of predicate PREDLIST.
  * @param      = +STRING/string ：LiLFeS program(string)
  * @param      = -PREDLIST/list ：list of predicate
  * @example    =
  > ?- eval("strprintln(""a"").",X).
     |~lilfesfilecons                          ~|
     | hd:|~strprintln~|                        |
     |    |_arg1:"a"  _|                        |
  X: | tl:< >                                   |
     |                |~lilfesfileinfo       ~| |
     | lilfesfileinfo:| lilfesfilename:"eval" | |
     |_               |_lilfesfileline:1     _|_|
  * @end_example

  * @jdesc      = LiLFeSプログラムSTRINGを解析して述語のリストPREDLISTを返します．
  * @jparam     = +STRING/string ：LiLFeSプログラム(文字列)
  * @jparam     = -PREDLIST/list ：述語のリスト
  * @jexample   =
  > ?- eval("strprintln(""a"").",X).
     |~lilfesfilecons                          ~|
     | hd:|~strprintln~|                        |
     |    |_arg1:"a"  _|                        |
  X: | tl:< >                                   |
     |                |~lilfesfileinfo       ~| |
     | lilfesfileinfo:| lilfesfilename:"eval" | |
     |_               |_lilfesfileline:1     _|_|
  * @end_jexample
*/
bool lilfes_eval(machine& m, FSP arg1, FSP arg2)
{
	if( ! arg1.IsString() )
	{
          RUNERR("1st argument of eval/2 requires a string");
          return false;
	}

	FSP result = m.eval( arg1.ReadString() );

	if( !result.IsValid() )
	{
		return false;
	}
	return arg2.Unify( result );
}

LILFES_BUILTIN_PRED_2(lilfes_eval, eval);

/**
  * @predicate  = eval(+STRING, -PREDLIST,-VARLIST)
  * @desc       = Analyse LiLFeS program STRING and return list of predicate PREDLIST and list of variables VARLIST.
  * @param      = +STRING/string ：LiLFeS program(string)
  * @param      = -PREDLIST/list ：list of predicate
  * @param      = -VARLIST/list ：list of variables
  * @example    =
  > ?- eval("strprintln(""a"").",X,Y).
     |~lilfesfilecons                          ~|
     | hd:|~strprintln~|                        |
     |    |_arg1:"a"  _|                        |
  X: | tl:< >                                   |
     |                |~lilfesfileinfo       ~| |
     | lilfesfileinfo:| lilfesfilename:"eval" | |
     |_               |_lilfesfileline:1     _|_|
  Y: < < > >
  * @end_example

  * @jdesc      = LiLFeSプログラムSTRINGを解析して述語のリストPREDLISTと変数のリストVARLISTを返します．
  * @jparam     = +STRING/string ：LiLFeSプログラム(文字列)
  * @jparam     = -PREDLIST/list ：述語のリスト
  * @jparam     = -VARLIST/list ：変数のリスト
  * @jexample   =
  > ?- eval("strprintln(""a"").",X,Y).
     |~lilfesfilecons                          ~|
     | hd:|~strprintln~|                        |
     |    |_arg1:"a"  _|                        |
  X: | tl:< >                                   |
     |                |~lilfesfileinfo       ~| |
     | lilfesfileinfo:| lilfesfilename:"eval" | |
     |_               |_lilfesfileline:1     _|_|
  Y: < < > >
  * @end_jexample
*/
bool lilfes_eval_3(machine& m, FSP arg1, FSP arg2, FSP arg3)
{
	if( ! arg1.IsString() )
	{
          RUNERR("1st argument of eval/3 requires a string");
          return false;
	}

	lilfes::eval(&m, arg1.ReadString(), true);

        return prog_list_top.IsValid() &&
			prog_vars_top.IsValid() &&
			arg2.Unify( prog_list_top ) &&
			arg3.Unify( prog_vars_top );
}

LILFES_BUILTIN_PRED_OVERLOAD_3(lilfes_eval_3, eval_3, eval);

/**
  * @predicate	= call(+$X) 
  * @desc	= execute predicate indicate feature structure <i>$X</i>.
  * @param	= +$X/(pred) : feature structure(predicate)
  * @example	= 
  > :- call(strprintln("a")).
  a
  > ?- call(X = person).
  X: person
  * @end_example

  * @jdesc		= 素性構造構造 <i>X</i> で表される述語を実行します。  
  * @jparam		= +$X/(pred) : 素性構造(述語)
  * @jexample	=
  > :- call(strprintln("a")).
  a
  > ?- call(X = person).
  X: person
  * @end_jexample
*/
bool lilfes_call( machine& m, FSP arg1 )
{
	const type *t = arg1.GetType();

	int nf = t->GetNFeatures();
	m.SetRP(nf);

	if( arg1.IsLeaf() )
	{
		const type *tt = t->TypeUnify(t_pred[0]);
		if( tt != NULL && tt != t )
		{
			t = tt;
		}
		for( int i=0; i<nf; i++ )
		{
			m.WriteLocal(i, PTR2c(FSP(m).GetAddress()));
		}
	}
	else
	{
		for( int i=0; i<nf; i++ )
		{
			m.WriteLocal(i, PTR2c(arg1.FollowNth(i).GetAddress()));
		}
	}

	procedure *pc = procedure::Search(t);
	if( pc == NULL )
	{
		RUNWARN("Non-defined procedure called in call/1: '" << t->GetName() << "'"); // by mitsuisi (1999.04.27)
		return false;
	}
	procthunk *pt = pc->GetThunk();
	ASSERT( pt != NULL );

	pt->exec(m);
  return true;
}

LILFES_BUILTIN_PRED_1(lilfes_call, call);

static void init()
{
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
	type *t0, *t1, *t2, *t3, *t4, *t5, *t6;

	t_int = t0 = new type("integer", module::CoreModule()); t0->SetAsChildOf(bot); t0->Fix();
	t_flo = t0 = new type("float", module::CoreModule());   t0->SetAsChildOf(bot); t0->Fix();
	t_chr = t0 = new type("char", module::CoreModule());    t0->SetAsChildOf(bot); t0->Fix();
	t_stg = t0 = new type("string", module::CoreModule());  t0->SetAsChildOf(bot); t0->Fix();

	// t_feature is initialized in ftype.cpp

	type *l, *c, *n;
	t_list = l = new type("list", module::CoreModule()); l->SetAsChildOf(bot); l->Fix();
	feature *h, *t;
	hd = h = new feature("hd\\", module::CoreModule());
	tl = t = new feature("tl\\", module::CoreModule());
	cons = c = new type("cons", module::CoreModule()); c->SetAsChildOf(l); c->AddFeature(h); c->AddFeature(t, t_list);
	c->Fix();
	nil  = n = new type("nil" , module::CoreModule()); n->SetAsChildOf(l);
	n->Fix();

////////////////////////////////////////////////////////////////////////////////
	feature *f;
	const feature **ff;
	pred = new type("pred", module::CoreModule()); pred->SetAsChildOf(bot); pred->Fix();
	t_pred[0] = new type("pred/0", module::CoreModule()); t_pred[0]->SetAsChildOf(pred); t_pred[0]->Fix();
	f_arg[0] = NULL;
	{
		ff = &f_arg[1];
		for( int i=1; i<MAXARGS; i++ )
		{
			ostringstream ost;
			ostringstream osf;

			ost << "pred/" << i;
			osf << "arg" << i << "\\";

			t_pred[i] = new type(ost.str().c_str(), module::CoreModule()); 
			*ff++ = f = new feature(osf.str().c_str(), module::CoreModule(), i); 
			t_pred[i]->SetAsChildOf(t_pred[i-1]);
			t_pred[i]->AddFeature(f);
			t_pred[i]->Fix();
			//delete[] ost.str();
			//delete[] osf.str();
		}
	}
#if 0
	t_plus  = t0 = new type("+"  ); t0->SetAsChildOf(pred); 							t0->Fix();
	          t1 = new type("+/1"); t1->SetAsChildOf(t0); t1->SetAsChildOf(t_pred[1]);	t1->Fix();
	          t2 = new type("+/2"); t2->SetAsChildOf(t1); t2->SetAsChildOf(t_pred[2]);	t2->Fix();

	t_minus = t0 = new type("-"  ); t0->SetAsChildOf(pred);								t0->Fix();
	          t1 = new type("-/1"); t1->SetAsChildOf(t0); t1->SetAsChildOf(t_pred[1]);	t1->Fix();
	          t2 = new type("-/2"); t2->SetAsChildOf(t1); t2->SetAsChildOf(t_pred[2]);	t2->Fix();

	t_aster = t0 = new type("*"  ); t0->SetAsChildOf(pred);								t0->Fix();
	          t2 = new type("*/2"); t2->SetAsChildOf(t0); t2->SetAsChildOf(t_pred[2]);	t2->Fix();

	t_slash = t0 = new type("/"  ); t0->SetAsChildOf(pred);								t0->Fix();
	          t2 = new type("//2"); t2->SetAsChildOf(t0); t2->SetAsChildOf(t_pred[2]);	t2->Fix();
#endif

	t_equal = t0 = new type("=" , module::CoreModule()); t0->SetAsChildOf(pred);								t0->Fix();
	          t2 = new type("=/2",module::CoreModule()); t2->SetAsChildOf(t0); t2->SetAsChildOf(t_pred[2]);	t2->Fix();

	t_cut   = t0 = new type("!"  ,module::CoreModule()); t0->SetAsChildOf(pred);								t0->Fix();

	t_query = t1 = new type("?-",module::CoreModule()); t1->SetAsChildOf(bot); 								t1->Fix();
	t_means = t2 = new type(":-",module::CoreModule()); t2->SetAsChildOf(bot); 								t2->Fix();
	f_cbody = f = new feature("cbody\\",module::CoreModule(), 1);
	t_defclause=t3=new type("defclause",module::CoreModule()); t3->SetAsChildOf(bot); t3->AddFeature(f);		t3->Fix();
	          t4 = new type("?-/1",module::CoreModule()); t4->SetAsChildOf(t1); t4->SetAsChildOf(t3); 		t4->Fix();
	          t5 = new type(":-/1",module::CoreModule()); t5->SetAsChildOf(t2); t5->SetAsChildOf(t3); 		t5->Fix();
	f_chead = f = new feature("chead\\", module::CoreModule(),0);
	          t6 = new type(":-/2",module::CoreModule()); t6->SetAsChildOf(t5); t6->AddFeature(f);			t6->Fix();

	t_comma     = t0 = new type("," ,module::CoreModule() ); t0->SetAsChildOf(t_pred[2]);						t0->Fix();
	t_semicolon = t0 = new type(";" ,module::CoreModule() ); t0->SetAsChildOf(t_pred[2]);						t0->Fix();
	t_ifthen    = t0 = new type("->" ,module::CoreModule()); t0->SetAsChildOf(t_pred[2]);						t0->Fix();
	t_circm     = t0 = new type("^" ,module::CoreModule());  t0->SetAsChildOf(t_pred[2]);						t0->Fix();
	t_not       = t0 = new type("\\+",module::CoreModule()); t0->SetAsChildOf(t_pred[1]);						t0->Fix();
	t_notuni    = t0 = new type("\\=",module::CoreModule()); t0->SetAsChildOf(t_pred[2]);						t0->Fix();
	t_macro     = t0 = new type(":=",module::CoreModule());  t0->SetAsChildOf(t_pred[2]);						t0->Fix();

	type *t_lilfesfileinfo;
	feature *ln, *ll, *li;
	f_lilfesfilename = ln = new feature("lilfesfilename\\", module::BuiltinModule(), 10);
	f_lilfesfileline = ll = new feature("lilfesfileline\\", module::BuiltinModule(), 11);
	t_lilfesfileinfo = t0 = new type("lilfesfileinfo",module::BuiltinModule());  t0->SetAsChildOf(bot); t0->AddFeature(ln, t_stg); t0->AddFeature(ll, t_int); t0->Fix();
	f_lilfesfileinfo = li = new feature("lilfesfileinfo\\", module::BuiltinModule(), 1);
	t0 = new type("lilfesfilecons",module::BuiltinModule());  t0->SetAsChildOf(c); t0->AddFeature(li, t_lilfesfileinfo); t0->Fix();

	{
		procedure *p = new procedure(module::CoreModule()->Search("=/2"), 2);
		codelist cl;
		cl.AddCode(C_UNIFY1ST, (ushort)0);
		cl.AddCode(C_UNIFY2ND, (ushort)1);
		cl.AddCode(C_RETURN);
		cl.EndCode();
		p->AddCodelist(cl);
	}
	{
		procedure *p = new procedure(module::CoreModule()->Search("\\="), 2);
		codelist cl;
		cl.AddCode(C_TRY);
		cl.AddCode("last");
		cl.AddCode(C_UNIFY1ST, (ushort)0);
		cl.AddCode(C_UNIFY2ND, (ushort)1);
		cl.AddCode(C_SHALCUT);
		cl.AddCode(C_FAIL);
		cl.SetLabel("last");
		cl.AddCode(C_LASTTRY);
		cl.AddCode(C_RETURN);
		cl.EndCode();
		p->AddCodelist(cl);
	}
	feature::SetBuiltInLimit();
	type::SetBuiltInLimit();
}

static void term() 
{ 

}

static InitFunction IF(init, 60);
static TermFunction TF(term, 60);

#ifdef BUILTIN_TYPE_SAVE
#define CALL_NAME "call"
#else
#define CALL_NAME "call/1"
#endif

static void init2()
{ // 99/6/9 mak
	const type *call_type = module::BuiltinModule()->Search(CALL_NAME);
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << " (init2)" << endl;
#endif
	{
		procedure *p = new procedure(module::CoreModule()->Search(","), 2);
		codelist cl;
		cl.AddCode(C_ALLOC);
		cl.AddCode((ushort)1);
		cl.AddCode((ushort)2);
		cl.AddCode(C_UNIFY1ST, (varno)1);
		cl.AddCode(C_LINK2ND, (varno)-1);
		cl.AddCode(C_CALL, procedure::Search(call_type));
		cl.AddCode(C_UNIFY1ST, (ushort)-1);
		cl.AddCode(C_LINK2ND, (ushort)0);
		cl.AddCode(C_DEALLOC);
		cl.AddCode(C_EXECUTE, procedure::Search(call_type));
		cl.EndCode();
//		cl.DebugOutput();
		p->AddCodelist(cl);
	}
	{
		procedure *p = new procedure(module::CoreModule()->Search(";"), 2);
		codelist cl;
		cl.AddCode(C_TRY, "2ND");
		cl.AddCode(C_EXECUTE, procedure::Search(call_type));
		cl.SetLabel("2ND");
		cl.AddCode(C_LASTTRY);
		cl.AddCode(C_UNIFY1ST, (ushort)1);
		cl.AddCode(C_LINK2ND, (ushort)0);
		cl.AddCode(C_EXECUTE, procedure::Search(call_type));
		cl.EndCode();
//		cl.DebugOutput();
		p->AddCodelist(cl);
	}
	{
		procedure *p = new procedure(module::CoreModule()->Search("->"), 2);
		codelist cl;
		cl.AddCode(C_ALLOC);
		cl.AddCode((ushort)2);
		cl.AddCode((ushort)2);
		cl.AddCode(C_UNIFY1ST, (varno)1);
		cl.AddCode(C_LINK2ND, (varno)-2);
		cl.AddCode(C_GETCP, (varno)-1);
		cl.AddCode(C_CALL, procedure::Search(call_type));
//		cl.AddCode(C_CLRTEMP, (ushort)1);
		cl.AddCode(C_DEEPCUT, (varno)-1);
		cl.AddCode(C_UNIFY1ST, (varno)-2);
		cl.AddCode(C_LINK2ND, (varno)0);
		cl.AddCode(C_DEALLOC);
		cl.AddCode(C_EXECUTE, procedure::Search(call_type));
		cl.EndCode();
		p->AddCodelist(cl);
	}
	{
		procedure *p = new procedure(module::CoreModule()->Search("\\+"), 1);
		codelist cl;
		cl.AddCode(C_TRY, "2ND");
		cl.AddCode(C_ALLOC);
		cl.AddCode((varno)1);
		cl.AddCode((varno)2);
		cl.AddCode(C_GETLEVEL, (varno)-1);
		cl.AddCode(C_CALL, procedure::Search(call_type));
		cl.AddCode(C_DEEPCUT, (varno)-1);
		cl.AddCode(C_FAIL);
		cl.SetLabel("2ND");
		cl.AddCode(C_LASTTRY);
		cl.AddCode(C_RETURN);
		cl.EndCode();
		p->AddCodelist(cl);
	}
}

static InitFunction IF2(init2, 160);

/**
  * @predicate	= findall(+$TEMPLATE, +$GENARATOR, -$BAG) 
  * @desc		= Make a list which result from backtracking .
  * @param		= +$TEMPLATE/ : template of wanted form
  * @param		= +$GENARATOR/ : predicate to examine
  * @param		= -$BAG/list : result list
  * @example	= 
  （Basic type） 
  > x <- [bot]. y <- [bot]. z <- [bot].
  > t <- [pred]. u <- [pred].
  > t(x).
  > t(y).
  > t(z).
  > ?- findall(X, t(X), S).
  S: < x, y, z >
  X: bot
  （Get the pairs of success.） 
  > u(X,Y) :- t(X), t(Y).
  > ?- findall([A,B], u(A,B), S).
  A: bot
  B: bot
  S: < < x, x >, < x, y >, < x, z >, < y, x >, < y, y >, < y, z >, 
       < z, x >, < z, y >, < z, z > >
  （Put the result in a feature structure.） 
  > xt <- [bot] + [ONE\, TWO\].
  > ?- findall( (ONE:A, TWO:B), u(A,B), S).
  A: bot
  B: bot
       |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  
  S: < | ONE:x |, | ONE:x |, | ONE:x |, | ONE:y |, | ONE:y |, | ONE:y |,
       |_TWO:x_|  |_TWO:y_|  |_TWO:z_|  |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
      |~xt   ~|  |~xt   ~|  |~xt   ~|
      | ONE:z |, | ONE:z |, | ONE:z | >
      |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
  * @end_example

  * @jdesc		= バックトラックの結果をまとめてリストにします． 
  * @jparam		= +$TEMPLATE/ : 得たい形のテンプレート
  * @jparam		= +$GENARATOR/ : 調べる述語
  * @jparam		= -$BAG/list : 結果のリスト 
  * @jexample	=
  （最も基本的な形） 
  > x <- [bot]. y <- [bot]. z <- [bot].
  > t <- [pred]. u <- [pred].
  > t(x).
  > t(y).
  > t(z).
  > ?- findall(X, t(X), S).
  S: < x, y, z >
  X: bot
  （成功する型の組み合わせを得る） 
  > u(X,Y) :- t(X), t(Y).
  > ?- findall([A,B], u(A,B), S).
  A: bot
  B: bot
  S: < < x, x >, < x, y >, < x, z >, < y, x >, < y, y >, < y, z >, 
       < z, x >, < z, y >, < z, z > >
  （結果を素性構造に入れる） 
  > xt <- [bot] + [ONE\, TWO\].
  > ?- findall( (ONE:A, TWO:B), u(A,B), S).
  A: bot
  B: bot
       |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  
  S: < | ONE:x |, | ONE:x |, | ONE:x |, | ONE:y |, | ONE:y |, | ONE:y |,
       |_TWO:x_|  |_TWO:y_|  |_TWO:z_|  |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
      |~xt   ~|  |~xt   ~|  |~xt   ~|
      | ONE:z |, | ONE:z |, | ONE:z | >
      |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
  * @end_jexample
*/
bool findall( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	FSP f(m);
	FSP froot(m, f.GetAddress());

	arg2.VAR2STR();

	const type *t = arg2.GetType();

	procedure *pc = procedure::Search(t);
	if( pc == NULL )
	{
		RUNWARN("Non-defined procedure called in findall/3: '" << t->GetName() << "'"); // by mitsuisi (1999.04.27)
		return false;
	}
	procthunk *pt = pc->GetThunk();
	ASSERT( pt != NULL );

	int nf = t->GetNFeatures();
	m.SetRP(nf);

	for( int i=0; i<nf; i++ )
		m.WriteLocal(i, PTR2c(arg2.FollowNth(i).GetAddress()));

//m.Dump();
	int oldheap = m.GetCurrentSlot();
	core_p save = m.SwitchNewHeap();
	int newheap = m.GetCurrentSlot();
//m.Dump();

#ifdef DEBUG_BAGOF
	int i;
	for( i=0; i<oldheap; i++ )
		cout << "  ";
	cout << "Enter Bagof " << arg2.GetType()->GetName() << endl;
	int nsolution = 0;
#endif

	code *oldIP = m.GetIP();
	core_p TrailP = m.SetTrailPoint(NULL);
//m.Dump();

	pt->exec(m);

	if( m.GetIP() == m.GetCIP() )
	{
		// when a built-in function is bagof'ed

		m.GoToHeap(oldheap);

		// To avoid trailing back the results, here we use untrailable
		// functions to list the results, WriteHeapDirect(), PutHeap() and Copy()
		FSP x = arg1.Copy();
		m.WriteHeapDirect(f.Deref().GetAddress(), PTR2c(m.GetHP()) );

		m.CheckHeapLimit(m.GetHP()+3);
		
		m.PutHeap(STR2c(cons));
		m.PutHeap(PTR2c(x.GetAddress()));
		f = FSP(m, m.GetHP() );
		m.PutHeap(VAR2c(bot));

		m.GoToHeap(newheap);
	}
	else
	{
		bool loop = false;
		if( m.GetIP() != NULL )
		{
			do {
//cerr << froot.DisplayAVM();
	#ifdef DEBUG_BAGOF
				for( i=0; i<oldheap; i++ )
				{
					cout << "  ";
				}
				cout << "  Solution " << ++nsolution;
				cout.flush();
	#endif
				loop = loop==false ? m.Execute(m.GetIP())   // The first answer
								   : m.NextAnswer(); // Quest for the next answer

				if( loop )
				{
	#ifdef DEBUG_BAGOF
					cout << " found: Copying...";
					cout.flush();
	#endif
//m.Dump();
					m.GoToHeap(oldheap);

					// To avoid trailing back the results, here we use untrailable
					// functions to list the results, WriteHeapDirect(), PutHeap() and Copy()
					FSP x = arg1.Copy();
					m.WriteHeapDirect(f.Deref().GetAddress(), PTR2c(m.GetHP()) );

					m.CheckHeapLimit(m.GetHP()+3);
				
					m.PutHeap(STR2c(cons));
					m.PutHeap(PTR2c(x.GetAddress()));
					f = FSP(m, m.GetHP() );
					m.PutHeap(VAR2c(bot));

					m.GoToHeap(newheap);
	#ifdef DEBUG_BAGOF
					cout << "done" << endl;
	#endif
//m.Dump();
				}
//cerr << froot.DisplayAVM();
			} while( loop );
		}
	} // non-built-in

#ifdef DEBUG_BAGOF
	cout << " not found" << endl;
#endif
	
//cerr << "1" << froot.DisplayAVM();
//m.Dump();
	m.TrailBack(TrailP);
//m.Dump();
//cerr << "2" << froot.DisplayAVM();
	
	m.SwitchOldHeap(save);
//cerr << "4" << froot.DisplayAVM();
	f.Coerce(nil);

//cerr << froot.DisplayAVM();

	bool result = arg3.Unify(froot);
	
	m.SetIP(oldIP);
  
#ifdef DEBUG_BAGOF
	for( i=0; i<oldheap; i++ )
	{
		cout << "  ";
	}
	cout << "Leave Bagof " << arg2.GetType()->GetName() << endl;
#endif
  
	if ( ! result )
	{
		return false;
	}
	return true;
}

LILFES_BUILTIN_PRED_3(findall, findall);

/**
  * @predicate	= findall(+$TEMPLATE, +$GENARATOR, -$BAG,-$TAIL) 
  * @desc		= Make a list which result from backtracking.
  * @param		= +$TEMPLATE/ : template of wanted form
  * @param		= +$GENARATOR/ : predicate to examine
  * @param		= -$BAG/list : result list
  * @param		= -$TAIL/list : the end of result list
  * @example	= 
  （Basic type） 
  > x <- [bot]. y <- [bot]. z <- [bot].
  > t <- [pred]. u <- [pred].
  > t(x).
  > t(y).
  > t(z).
  > ?- findall(X, t(X), S).
  S: < x, y, z >
  X: bot
  （Get the pairs of success.） 
  > u(X,Y) :- t(X), t(Y).
  > ?- findall([A,B], u(A,B), S).
  A: bot
  B: bot
  S: < < x, x >, < x, y >, < x, z >, < y, x >, < y, y >, < y, z >, 
       < z, x >, < z, y >, < z, z > >
  （Put the result in a feature structure.） 
  > xt <- [bot] + [ONE\, TWO\].
  > ?- findall( (ONE:A, TWO:B), u(A,B), S).
  A: bot
  B: bot
       |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  
  S: < | ONE:x |, | ONE:x |, | ONE:x |, | ONE:y |, | ONE:y |, | ONE:y |,
       |_TWO:x_|  |_TWO:y_|  |_TWO:z_|  |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
      |~xt   ~|  |~xt   ~|  |~xt   ~|
      | ONE:z |, | ONE:z |, | ONE:z | >
      |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
  * @end_example

  * @jdesc		= バックトラックの結果をまとめて差分リストにします． 
  * @jparam		= +$TEMPLATE/ : 得たい形のテンプレート
  * @jparam		= +$GENARATOR/ : 調べる述語
  * @jparam		= -$BAG/list : 結果のリスト 
  * @jparam		= -$TAIL/list : 結果のリストの最後尾
  * @jexample	=
  （最も基本的な形） 
  > x <- [bot]. y <- [bot]. z <- [bot].
  > t <- [pred]. u <- [pred].
  > t(x).
  > t(y).
  > t(z).
  > ?- findall(X, t(X), S).
  S: < x, y, z >
  X: bot
  （成功する型の組み合わせを得る） 
  > u(X,Y) :- t(X), t(Y).
  > ?- findall([A,B], u(A,B), S).
  A: bot
  B: bot
  S: < < x, x >, < x, y >, < x, z >, < y, x >, < y, y >, < y, z >, 
       < z, x >, < z, y >, < z, z > >
  （結果を素性構造に入れる） 
  > xt <- [bot] + [ONE\, TWO\].
  > ?- findall( (ONE:A, TWO:B), u(A,B), S).
  A: bot
  B: bot
       |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  |~xt   ~|  
  S: < | ONE:x |, | ONE:x |, | ONE:x |, | ONE:y |, | ONE:y |, | ONE:y |,
       |_TWO:x_|  |_TWO:y_|  |_TWO:z_|  |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
      |~xt   ~|  |~xt   ~|  |~xt   ~|
      | ONE:z |, | ONE:z |, | ONE:z | >
      |_TWO:x_|  |_TWO:y_|  |_TWO:z_|
  * @end_jexample
*/
bool findall_4( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 )
{
	FSP f(m);
	FSP froot(m, f.GetAddress());

	arg2.VAR2STR();

	const type *t = arg2.GetType();

	procedure *pc = procedure::Search(t);
	if( pc == NULL )
	{
		RUNWARN("Non-defined procedure called in findall/4: '" << t->GetName() << "'"); // by mitsuisi (1999.04.27)
		return false;
	}
	procthunk *pt = pc->GetThunk();
	ASSERT( pt != NULL );

	int nf = t->GetNFeatures();
	m.SetRP(nf);

	for( int i=0; i<nf; i++ )
	{
		m.WriteLocal(i, PTR2c(arg2.FollowNth(i).GetAddress()));
	}

//m.Dump();
	int oldheap = m.GetCurrentSlot();
	core_p save = m.SwitchNewHeap();
	int newheap = m.GetCurrentSlot();
//m.Dump();

#ifdef DEBUG_BAGOF
	int i;
	for( i=0; i<oldheap; i++ )
	{
		cout << "  ";
	}
	cout << "Enter Bagof " << arg2.GetType()->GetName() << endl;
	int nsolution = 0;
#endif

	code *oldIP = m.GetIP();
	core_p TrailP = m.SetTrailPoint(NULL);
//m.Dump();

	pt->exec(m);

	if( m.GetIP() == m.GetCIP() )
	{
		// when a built-in function is bagof'ed

		m.GoToHeap(oldheap);

		// To avoid trailing back the results, here we use untrailable
		// functions to list the results, WriteHeapDirect(), PutHeap() and Copy()
		FSP x = arg1.Copy();
		m.WriteHeapDirect(f.Deref().GetAddress(), PTR2c(m.GetHP()) );

		m.CheckHeapLimit(m.GetHP()+3);
		
		m.PutHeap(STR2c(cons));
		m.PutHeap(PTR2c(x.GetAddress()));
		f = FSP(m, m.GetHP() );
		m.PutHeap(VAR2c(bot));

		m.GoToHeap(newheap);
	}
	else
	{
		bool loop = false;
		if( m.GetIP() != NULL ) {
			do
			{
//cerr << froot.DisplayAVM();
	#ifdef DEBUG_BAGOF
				for( i=0; i<oldheap; i++ )
				{
					cout << "  ";
				}
				cout << "  Solution " << ++nsolution;
				cout.flush();
	#endif
				loop = loop==false ? m.Execute(m.GetIP())   // The first answer
								   : m.NextAnswer(); // Quest for the next answer

				if( loop )
				{
	#ifdef DEBUG_BAGOF
					cout << " found: Copying...";
					cout.flush();
	#endif
//m.Dump();
					m.GoToHeap(oldheap);

					// To avoid trailing back the results, here we use untrailable
					// functions to list the results, WriteHeapDirect(), PutHeap() and Copy()
					FSP x = arg1.Copy();
					m.WriteHeapDirect(f.Deref().GetAddress(), PTR2c(m.GetHP()) );

					m.CheckHeapLimit(m.GetHP()+3);

					m.PutHeap(STR2c(cons));
					m.PutHeap(PTR2c(x.GetAddress()));
					f = FSP(m, m.GetHP() );
					m.PutHeap(VAR2c(bot));

					m.GoToHeap(newheap);
	#ifdef DEBUG_BAGOF
					cout << "done" << endl;
	#endif
//m.Dump();
				}
//cerr << froot.DisplayAVM();
			} while( loop );
		}
	} // non-built-in

#ifdef DEBUG_BAGOF
	cout << " not found" << endl;
#endif
	
//cerr << "1" << froot.DisplayAVM();
//m.Dump();
	m.TrailBack(TrailP);
//m.Dump();
//cerr << "2" << froot.DisplayAVM();
	
	m.SwitchOldHeap(save);
//cerr << "4" << froot.DisplayAVM();


//cerr << froot.DisplayAVM();

	bool result = arg3.Unify(froot);
	
	m.SetIP(oldIP);
  
	if( f.Unify(arg4) == false ) // by mitsuisi
	{
		return false;
	}
  
#ifdef DEBUG_BAGOF
	for( i=0; i<oldheap; i++ )
		cout << "  ";
	cout << "Leave Bagof " << arg2.GetType()->GetName() << endl;
#endif
  
	if ( ! result )
	{
		return false;
	}
	return true;
}

LILFES_BUILTIN_PRED_OVERLOAD_4(findall_4, findall_4, findall);

LILFES_BUILTIN_PRED_4(findall_4, dfindall);

/**
  * @predicate	= assert(+$C) 
  * @desc	= Add a new definition clause <i>$C</i>.
  * @param	= +$C/ :definition clause
  * @note       = <i>$C</i> must be described without period.For example,"assert(p(a))." or "assert(p(X):- q(X)). "  
  * @example	= 
  > p <- [pred]. a <- [bot]. b <- [bot].
  > p(a).
  > :- assert(p(b)).
  > ?- p(X).
  X: a
  Enter ';' for more choices, otherwise press ENTER --> ;
  X: b
  　
  "p(b)" is added after definition of predicate p "p(a)" by assert.
  * @end_example

  * @jdesc		= 新たな確定節<i>$C</i>を追加します。
  * @jparam		= +$C/ :確定節
  * @jnote      =  <i>$C</i> には、ピリオドを含まない形で入力します。例えば、assert(p(a)). や、assert(p(X):- q(X)).のように記述します。 
  * @jexample	=
  > p <- [pred]. a <- [bot]. b <- [bot].
  > p(a).
  > :- assert(p(b)).
  > ?- p(X).
  X: a
  Enter ';' for more choices, otherwise press ENTER --> ;
  X: b
  　
  例では、assertによって、述語 p の定義 p(a) の後に p(b) が追加されています。
  * @end_jexample
*/

bool lilfes_assert( machine& m, FSP arg1 )
{

#ifdef DEBUG
	if( DEB )
	{
          cout << "@@@@@@ ASSERT entered" << endl;
          cout << arg1.DisplayLiLFeS();
	}
#endif

	ShareList sl;
	sl.AddMark(arg1);
	sl.MakeShare();
	
	ptree *pt = GenPTreeOfClause(sl, arg1);
	if( pt != NULL ) 
	{
		if( pt->GetClassName() == pclause::className )
		{
#ifdef DEBUG
			if( DEB )
			{
				cout << pt->OutputTree();
			}
#endif
			parseinfo info(&m);
			pt->CollectInfo(0, &info);
			codelist *l = pt->Compile(0, &info);
			l->EndCode();

#ifdef DEBUG
			if( DEB )
			{
				l->DebugOutput();
			}
#endif
			
			procedure *p = procedure::New((((pclause *)pt)->GetRepType()), info.ndefarg);
			if( p == NULL )
			{
				RUNERR("assert/1 cannot obtain procedure info");
			}
			else
			{
				FSP head ;
				if( arg1.GetType()->IsSubType(t_means) )
				{
					head = arg1.Follow(f_chead);
				}
				else
				{
					head = arg1;
				}
				if( head.GetType()->IsSubType(t_pred[1]) )
				{
					if( head.Follow(f_arg[1]).IsInteger() )
					{
						p->AddCodelistINT(*l, info.toparg, head.Follow(f_arg[1]).ReadInteger() );
//						cout << "@@@@@@ ASSERT leaved 1" << endl;
						return true;
					}
					else if( head.Follow(f_arg[1]).IsString() )
					{
						p->AddCodelistSTG(*l, info.toparg, head.Follow(f_arg[1]).ReadStringSN());
//						cout << "@@@@@@ ASSERT leaved 2" << endl;
						return true;
					}
				}
				p->AddCodelist(*l, info.toparg);
//				cout << "@@@@@@ ASSERT leaved 3" << endl;
				return true;
			}
		}
		else
		{
			RUNERR("assert/1 cannot take a query");
		}
	}
	else
	{
		RUNERR("assert/1 cannot compile the given clause");
	}
//	cout << "@@@@@@ ASSERT leaved" << endl;
  return true;
}

LILFES_BUILTIN_PRED_1(lilfes_assert, assert);

/**
  * @predicate	= asserta(+$C) 
  * @desc	= Add a new definition clause <i>$C</i> to the top of predicate definition.
  * @param	= +$C/ :definition clause
  * @note       = <i>$C</i> must be described without period.For example,"assert(p(a))." or "assert(p(X):- q(X)). "  
  * @example	= 
  > p <- [pred]. a <- [bot]. b <- [bot].
  > p(a).
  > :- asserta(p(b) :- !).
  > ?- p(X).
  X: b
  Enter ';' for more choices, otherwise press ENTER --> ;
  no
  　
  "p(b):-!" is added after definition of predicate p "p(a)" by asserta.On call of p in query,the definition clause added by asserta has priority,so p(b) success and restrain backtrack by "!",so the old definition p(a) is not called.
  * @end_example

  * @jdesc		= 新たな確定節<i>$C</i>を追加します。
  * @jparam		= +$C/ :確定節
  * @jnote      =  <i>$C</i> には、ピリオドを含まない形で入力します。例えば、assert(p(a)). や、assert(p(X):- q(X)).のように記述します。 
  * @jexample	=
  > p <- [pred]. a <- [bot]. b <- [bot].
  > p(a).
  > :- asserta(p(b) :- !).
  > ?- p(X).
  X: b
  Enter ';' for more choices, otherwise press ENTER --> ;
  no
  　
  例では、assertaによって、述語 p の定義 p(a) の前に p(b) :- ! が追加されています。クエリーでの pの呼び出しは、asserta で追加されたものが優先されるため、 p(b) が成功して ! でバックトラックが抑制されるため、元の p(a) は呼び出されなくなります。 
  * @end_jexample
*/
bool lilfes_asserta( machine& m, FSP arg1 )
{
	ShareList sl;
	sl.AddMark(arg1);
	sl.MakeShare();
	
	ptree *pt = GenPTreeOfClause(sl, arg1);
	if( pt != NULL )
	{
		if( pt->GetClassName() == pclause::className )
		{
			parseinfo info(&m);
			pt->CollectInfo(0, &info);
			codelist *l = pt->Compile(0, &info);
			l->EndCode();
			procedure *p = procedure::New(((pclause *)pt)->GetRepType(), info.ndefarg);

			if( p == NULL )
			{
				RUNERR("asserta/1 cannot obtain procedure info");
			}
			else
			{
				FSP head ;
				if( arg1.GetType()->IsSubType(t_means) )
				{
					head = arg1.Follow(f_chead);
				}
				else
				{
					head = arg1;
				}
				if( head.GetType()->IsSubType(t_pred[1]) )
				{
					if( head.Follow(f_arg[1]).IsInteger() )
					{
						p->AddCodelistTopINT(*l, info.toparg, head.Follow(f_arg[1]).ReadInteger() );
//						cout << "@@@@@@ ASSERT leaved 1" << endl;
						return true;
					}
					else if( head.Follow(f_arg[1]).IsString() )
					{
						p->AddCodelistTopSTG(*l, info.toparg, head.Follow(f_arg[1]).ReadStringSN());
//						cout << "@@@@@@ ASSERT leaved 2" << endl;
						return true;
					}
				}
				p->AddCodelistTop(*l, info.toparg);
			}
		}
		else
		{
			RUNERR("asserta/1 cannot take a query");
		}
	}
	else
	{
		RUNERR("asserta/1 cannot compile the given clause");
	}
  return true;
}

LILFES_BUILTIN_PRED_1(lilfes_asserta, asserta);

/**
  * @predicate	= halt
  * @desc	= Halt lilfes with return code 0
  * @see        = #halt/1
  * @example	= 
  * :- halt.
  * @end_example
  * @jdesc	= lilfes を停止します．
  */
bool halt( machine& m )
{
  throw halt_exception( &m );
}

LILFES_BUILTIN_PRED_0(halt, halt);

/**
  * @predicate	= halt(+$C)
  * @desc	= Halt lilfes
  * @param	= +$C/integer : return code
  * @example	= 
  * :- halt(1).
  * @end_example
  * @jdesc	= lilfes を停止します．
  */
bool halt_1( machine& m, FSP arg1 )
{
	if ( ! arg1.IsInteger() )
	{
		RUNERR( "argument of halt/1 must be integer" );
		return false;
	}
	throw halt_exception( &m, arg1.ReadInteger() );
}

LILFES_BUILTIN_PRED_OVERLOAD_1(halt_1, halt_1, halt);

} // namespace builtin

} // namespace lilfes

