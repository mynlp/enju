/**
 * @module  = module
 * @cvs     = $Id: module.cpp,v 1.18 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Module
 * @jdesc   = モジュール
 * Directives and predicates for module control
 * @japanese =
 * モジュールを制御するためのディレクティブや組み込み述語です．
 * @end_japanese
*/

/* $Id: module.cpp,v 1.18 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    
 *      made by kaityo
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: module.cpp,v 1.18 2011-05-02 10:38:23 matuzaki Exp $";

#include "module.h"
#include "initial.h"
#include "option.h"
#include "builtin.h"
#include "in.h"
#include "search_path.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <list>
#include <ostream>
#include <string>

//#define DEBUG_MODULE

//#ifdef DEBUG
//#define DEBUG_MODULE
//#endif

#define MODLIST_GROW_BITS 3

namespace lilfes {

using std::cout;
using std::endl;
using std::list;
using std::replace;
using std::strcmp;
using std::string;
using std::strrchr;


_HASHMAP<string, module *> module::modulelist_hash;

module *module::m_user;
module *module::m_core;
module *module::m_builtin;
module *module::m_directive;
module *module::m_current;
string module::module_prefix("");

////////////////////////////////////////////////////////////////////////

void get_fileinfo(const string &filename, string *lilname, string *modulename);

////////////////////////////////////////////////////////////////////////
//
//  class module
//

module::module(const string &iname, new_module_type inmt /* = NMT_NONE */, bool Extendable /* = false */) 
	: name(""), defname(module_prefix + iname), ExtendableMode(Extendable)
{ 
	nmt = inmt;
	InterfaceMode = true;
	prev_module = NULL;
	first_import_modules = NULL;
	if( nmt == NMT_BUILTIN )
		Register();
}

void module::Add(type *t)
{
	if( ! IsRegistered() )
	{
		Register();
	}
	hash[t->GetKey()] = t;
}

void module::SetFirstImport(module** mods) {
	if (mods == 0) {
		first_import_modules = new module*[1];
		first_import_modules[0] = 0;
		return;
	}
	int len = 0; module** cur = mods;
	while(*cur != 0) { cur++; len++; }
	first_import_modules = new module*[len+1];
	int i; for (i=0; i<len; i++) first_import_modules[i] = mods[i];
	first_import_modules[i] = 0;
}

void module::Delete(type *t) 
{ 
	if( t->IsDeleted() ) 
		hash.erase(t->GetKey()); 
	else 
		ABORT("INTERNAL ERROR"); 
}

const feature * module::SearchFeature(const string &str) const {
  ASSERT(this != NULL);
  _HASHMAP<string, type *>::const_iterator it = hash.find(str);
  return feature::Search(it->second);
}

bool module::IsUsable(const type *t) const
{ 
	return t == Search(t->GetSimpleName()); 
}

bool module::IsExporting(const type *t) const
{
	if( ! t->IsExported() )
		return false;
	if( t->GetModule() == this )
		return true;
	if( ! IsUsable(t) )
		return false;
	for( uint i=0; i<reexportlist.size(); i++ )
		if( t->GetModule() == reexportlist[i] )
		{
			return true;
		}
	return false;
}

void module::ShowList(void)
{
	if( hash.size() > 0 )
	{
		cout << "Exporting Symbols: " << endl;
		for ( _HASHMAP<string, type *>::const_iterator it = hash.begin();
			  it != hash.end(); ++it)
		{
			type *t = it->second;
			if (t!=NULL && IsExporting(t))
				cout << t->GetName() << " ";
		}
		cout << endl;

		cout << "Non exporting symbols: " << endl;
		for ( _HASHMAP<string, type *>::const_iterator it = hash.begin();
			  it != hash.end(); ++it)
		{
			type *t = it->second;
			if (t!=NULL && t->GetModule() == this && !IsExporting(t))
				cout << t->GetName() << " ";
		}

		cout << endl;
		cout << "All symbols: " << endl;

		for ( _HASHMAP<string, type *>::const_iterator it = hash.begin();
			  it != hash.end(); ++it)
		{
			type *t = it->second;
			if (t!=NULL)
				cout << t->GetProperName() << " ";
		}
		cout << endl;
	}

	if( reexportlist.size() )
	{
		cout << "Re-exporting modules: " << endl;
		for( uint i=0; i<reexportlist.size(); i++ )
			cout << reexportlist[i]->GetName() << " ";
		cout << endl;
	}
	if( noexportlist.size() )
	{
		cout << "Not-exporting modules: " << endl;
		for( uint i=0; i<noexportlist.size(); i++ )
			cout << noexportlist[i]->GetName() << " ";
		cout << endl;
	}

	cout << "List of all modules: " << endl;
	for( _HASHMAP<string, module *>::const_iterator it = modulelist_hash.begin();
		 it != modulelist_hash.end(); ++it)
		if( it->second )
		{
			module *p = it->second;
			cout << p->GetName() << " ";
		}
	cout << endl;
}

module** module::ExportList(void) const
{
	module** mods = new module*[reexportlist.size()+1];
	uint i; for (i=0; i<reexportlist.size(); i++) {
		mods[i] = reexportlist[i];
	}
	mods[i] = 0;
	return mods;
}

module** module::NoExportList(void) const
{
	module** mods = new module*[noexportlist.size()+1];
	uint i; for (i=0; i<noexportlist.size(); i++) {
		mods[i] = noexportlist[i];
	}
	mods[i] = 0;
	return mods;
}

void module::Import(module *mod, bool bexport)
{
	ASSERT(mod != this);

#ifdef DEBUG_MODULE
		cout << "Module " << GetName() << "(" << GetDefName() << ") imports " << mod->GetName() << " (reexport " << bexport << ")" << endl;
#endif

	if( bexport )
	{
		for( uint i=0; i<reexportlist.size(); i++ )
			if( reexportlist[i] == mod )
			{
#ifdef DEBUG_MODULE
				cout << "SAME MODULE IMPORT; ignored" << endl;
#endif
				return;	
			}
		reexportlist.push_back(mod);
		for( uint i=0; i<mod->reexportlist.size(); i++ )
		{
			for( uint j=0; j<reexportlist.size(); j++ )
			{
				if( reexportlist[j] == mod->reexportlist[i] )
					goto CONTINUE;
			}
			reexportlist.push_back(mod->reexportlist[i]);
		CONTINUE:
			;
		}
	} else {
		for( uint i=0; i<noexportlist.size(); i++ )
			if( noexportlist[i] == mod )
			{
#ifdef DEBUG_MODULE
				cout << "SAME MODULE IMPORT; ignored" << endl;
#endif
				return;	
			}
		noexportlist.push_back(mod);
	}
	
	for ( _HASHMAP<string, type *>::const_iterator it = mod->hash.begin();
			  it != mod->hash.end(); ++it)
	{
		type *t = it->second;
		if( t != NULL && mod->IsExporting(t) )
		{
#ifdef DEBUG_MODULE
			if( DEB )
			if( mod != BuiltinModule() && mod != CoreModule() )
			cout << ">>" << t->GetName() << endl;
#endif
			type *oldt = NULL;
			_HASHMAP<string, type *>::const_iterator it = hash.find(t->GetKey());
			if( it != hash.end() )
			{
				oldt = it->second;
			}
			hash[t->GetKey()] = t;

			if( oldt != NULL )
			{
				if( oldt->GetModule() == this )
				{
					hash[oldt->GetKey()] = oldt;		// Replace again
					RUNWARN( "The imported type " << t->GetProperName() << " cannot override the same-named intrinsic type in module " << GetName());
				}
				else if( oldt->GetModule() != t->GetModule() )
				{
					RUNWARN( "The imported type " << t->GetProperName() << " overrides previously imported type " << oldt->GetProperName() << " in module " << GetName());
				}
			}
		}
	}
}


void module::SetCurrentModule(module *mod) 
{ 
	ASSERT(mod != NULL);
#ifdef DEBUG_MODULE
	cout << "current module changed " << (m_current == NULL ? "(null)" : m_current->name) << " -> " << mod->name << endl;
#endif
	m_current = mod; 
}

InitFunction module::IF(module::init, 5);
InitFunction module::IF2(module::init2, 300);

void module::init()		// This initialization should be done 
						// ahead of init() in ftype.cpp
{
	m_core = new module("core", NMT_BUILTIN, true);
	m_user = new module("user", NMT_BUILTIN, true);
	m_builtin = new module("builtin", NMT_BUILTIN);
	m_directive = new module("builtin:directive", NMT_BUILTIN);
	m_current = m_user;

}

void module::init2()
{
	m_builtin->Import(m_directive, true);
	m_user->Import(m_core, false);
	m_user->Import(m_builtin, false);
}


void module::term()		// This initialization should be done 
						// ahead of init() in ftype.cpp
{
	for(_HASHMAP<string, module *>::const_iterator it = modulelist_hash.begin();
		it != modulelist_hash.end(); ++it)
	{
		module *m = it->second;
		if( m )
		{
			delete m;
		}
	}
}

TermFunction module::TF(module::term, 5);

////////////////////////////////////////////////////////////////////////
//
//  Built-in functions for module manipulation
//

//////////////////////////////////////////////////////////////////////////////
// ensure_loaded/1 : include a file


module* module::new_module(machine &m, const string &biname, const string &modname, new_module_type nmt, module *prevmod /* = NULL */, module** import_module /* = NULL */)
{
#ifdef DEBUG_MODULE
	cout << "new_module " << modname << " (type " << nmt << ") in " << biname << endl;
	if( prevmod ) 
		cout << " ( prevmod = " << prevmod->GetName() << " )" << endl;
#endif

        // Unnecessary?  removed by yusuke (2004/09/15)
        /*
	if( nmt == NMT_ENSURE || nmt == NMT_ENSURE_INITIAL || nmt == NMT_ENSURE_IN_PROG || nmt == NMT_ENSURE_WITHOUT_BUILTIN || nmt == NMT_ENSURE_WITHOUT_BUILTIN_IN_PROG)
	{	// ensure 系の場合
		module *mod = module::SearchModule(CurrentModule()->GetModulePrefix()+modname);
//		if( mod && mod->GetModuleType() == NMT_BUILTIN )
		if( mod )	// モジュールが存在していれば
		{
			module::CurrentModule()->Import( mod );	// インポートするだけ
			//return modname;
                        return mod;
		}
	}
        */

	module *prev_current = module::CurrentModule();
	module *mod = NULL;
	
	string lilname, modulename;
	get_fileinfo(modname, &lilname, &modulename);

	if( nmt != NMT_INCLUDE && nmt != NMT_READ_LILFES )
	{	// モジュールに関係ある述語の場合
		mod = new module( modulename, nmt ); 			// 新規モジュールを作る
		mod->Import( module::CoreModule(), false );		// Core モジュールは常にインポート
		if( nmt != NMT_READ_MODULE &&
		    nmt != NMT_ENSURE_WITHOUT_BUILTIN &&
		    nmt != NMT_ENSURE_WITHOUT_BUILTIN_IN_PROG)
		{												// 特定の場合以外は Builtin をインポート
			if ( prev_current->first_import_modules == 0) {		// first_import 指定がなければ
				mod->Import( module::BuiltinModule(), false );	// 普通に builtin をインポート
			} else {											// 指定がある場合は
				mod->Import( module::DirectiveModule(), false );	// Directive だけはインポート
				mod->SetFirstImport(prev_current->first_import_modules);	// first_import を引き継ぐ
				module** cur = prev_current->first_import_modules;
				while(*cur != NULL)								// first_import 処理をする
					mod->Import( *cur++, false);
			}
		}
		else											// Builtin をインポートしない場合
		{
			mod->Import( module::DirectiveModule(), false );	// Directive だけはインポート
			mod->SetFirstImport(import_module);			// import_module をセット (NULL の場合でもそのままセット)
			if (import_module != NULL) {
				while(*import_module != NULL)
					mod->Import( *import_module++, false);
			}
		}
	}

	if( nmt == NMT_CREATE_MODULE )						// create_module の場合はここでおわり
	{
		module::SetCurrentModule( mod );
		return mod;
	}

	if( nmt != NMT_ENSURE_INITIAL && m.getLexer() )	{		// 初期化時以外は
		search_path::ExtractRelativePath(m.getLexer()->InFile());		// 相対パスを検索する
        } else {
          search_path::SetRelativePath("./");
        }

	if( nmt == NMT_READ_LILFES || nmt == NMT_READ_LILFES_WITH_PREFIX || nmt == NMT_READ_MODULE )
	{
		/* readfile() の内容を展開 */
#ifdef DEBUG_INCLUDE
	        cout << "READ_FILE" <<  fname << endl;
#endif
		if(! pushfile( &m, lilname, 1, (nmt == NMT_READ_LILFES_WITH_PREFIX ? "__" : "")) )
		{
      INERR(&m, "Cannot open file '" << modname << "' in " << biname);
			delete mod;
			m.Fail();
			return NULL;	
    }
		else
		{
			module::SetCurrentModule( mod );
			mod->SetPrevModule(prevmod ? prevmod : prev_current);
			readfile( &m );
#ifdef DEBUG_INCLUDE
		        cout << "END_READ_FILE\n";
#endif
		}
	}
	else
	{
		if( (! pushfile( &m, lilname, ( (nmt == NMT_ENSURE_IN_PROG || nmt == NMT_ENSURE_WITHOUT_BUILTIN_IN_PROG) ? 2 : 0) ) ) )
		{
      if( nmt != NMT_INITIALIZE ) {
        INERR(&m, "Cannot open file '" << modname << "' in " << biname);
      }
			delete mod;
			m.Fail();
			return NULL;
		}
		else
		{
			if( mod != NULL )
			{
				module::SetCurrentModule( mod );
				mod->SetPrevModule(prevmod ? prevmod : prev_current);
			}
		}
	}

//		if( nmt == NMT_INITIALIZE || nmt == NMT_ENSURE_IN_PROG )
//		{
//			m.getLexer()->SetDepth(1);
//		}
        return mod;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace builtin {

/**
  * @predicate  = ensure_loaded(+ModulePath)
  * @desc       = Load a module .
  * @param      = +ModulePath/ string  : Module path
  * @example    =
  > :- ensure_loaded("lillib/lists").
  * @end_example

  * @jdesc      = モジュールを読み込みます．
  * @jparam     = +ModulePath/ string: モジュールのパス
  * @jexample   =
  > :- ensure_loaded("lillib/lists").
  * @end_jexample
*/
bool ensure_loaded( machine& m, FSP arg1 )
{
	if( ! arg1.IsString() )
	{
          INERR(&m, "ensure_loaded/1 requires a file name");
          return false;
	}

  module::new_module(m, "ensure_loaded/1", arg1.ReadString(), (m.getLexer()->ProgInput() ? module::NMT_ENSURE_IN_PROG : module::NMT_ENSURE) );
  return true;
}

LILFES_BUILTIN_DIRECTIVE_1(ensure_loaded, ensure_loaded);

/**
  * @predicate  = module(+NAME)
  * @desc       = Declare module name.This is used at the top of module.
  * @param      = +NAME/ string  : Module name
  * @example    = :- module("list:reverse").
  * @end_example

  * @jdesc      = モジュール名を宣言します．この述語はモジュールの先頭で使用されます．
  * @jparam     = +Module/ string: モジュール名
  * @jexample   = :- module("list:reverse").
  * @end_jexample
*/
bool module_directive( machine& m, FSP arg1 )
{
			// 与えられたモジュール名でモジュールを新設。
	if( lilfes::module::CurrentModule()->IsRegistered() )
	{
		INERR(&m, "Module declaration should be appeared on the top of a module, only once");
	}
	else if( ! arg1.IsString() )
	{
		INERR(&m, "The argument of module/1 should be a string");
		lilfes::module::CurrentModule()->Register();
	}
	else if( *arg1.ReadString() == '\0' )
	{
		INERR(&m, "The argument of module/1 should not be an empty string");
		lilfes::module::CurrentModule()->Register();
	}
	else
	{
		lilfes::module *mod =  lilfes::module::SearchModule(arg1.ReadString());

		if(mod)		// 同名モジュールの存在
		{
			if( ! strcmp( arg1.ReadString(), "core" ) || ! strcmp( arg1.ReadString(), "builtin" )  )
			{
                          INERR(&m, "Built-in module \"" << arg1.ReadString() << "\" cannot be redefined");
                          m.getLexer()->Skip();
                          return false;
			}
			if( mod->GetDefName() != lilfes::module::CurrentModule()->GetDefName() && lilfes_flag::Search("warn-module-override")->GetIntVal() )
			{
				INWARN(&m, "Same module name \"" << arg1.ReadString() << "\" with different paths; the old module will be invalidated");
				INWARN(&m, mod->GetDefName() );
				INWARN(&m, lilfes::module::CurrentModule()->GetDefName() );
			}
			else
				if( lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE 
				 || lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE_IN_PROG
				 || lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE_INITIAL
				 || lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE_WITHOUT_BUILTIN
				 || lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE_WITHOUT_BUILTIN_IN_PROG )
				{
					lilfes::module *prevmod = lilfes::module::CurrentModule()->GetPrevModule();
					if( prevmod )
					{
						if( lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE 
						 || lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE_INITIAL
						 || lilfes::module::CurrentModule()->GetModuleType() == lilfes::module::NMT_ENSURE_IN_PROG)
							{prevmod->Import(mod);}
						lilfes::module::CurrentModule()->SetPrevModule(NULL);
					}
					m.getLexer()->Skip();
					return true;
				}
		}
		lilfes::module::CurrentModule()->Register(arg1.ReadString());
	}
  return true;
}

LILFES_BUILTIN_DIRECTIVE_1(module_directive,module);

/**
  * @predicate  = module_interface
  * @desc       = Declare the begining of the module interface part. See how to make module.
  * @example    = :- module_interface.

  * @end_example

  * @jdesc      = モジュールインタフェース部の開始を宣言します． モジュールの作り方を参照してください．
  * @jexample   = :- module_interface.
  * @end_jexample
*/
bool module_interface( machine& )
{
  lilfes::module::CurrentModule()->SetInterfaceMode(true);
  return true;
}

LILFES_BUILTIN_DIRECTIVE_0(module_interface, module_interface);

/**
  * @predicate  = module_implementation
  * @desc       = Declare the begining of the module interface part. See how to make module.
  * @example    = :- module_interface.

  * @end_example

  * @jdesc      = モジュール実装部の開始を宣言します． モジュールの作り方を参照してください．
  * @jexample   = :- module_interface.
  * @end_jexample
*/
bool module_implementation( machine& )
{
  lilfes::module::CurrentModule()->SetInterfaceMode(false);
  return true;
}

LILFES_BUILTIN_DIRECTIVE_0(module_implementation, module_implementation);

/**
  * @predicate  = module_extendable
  * @desc       = Declare that this module can be defined as a type of a subtype.
  * @example    = > :- module_extendable.

  * @end_example

  * @jdesc      = このモジュールで定義された型の subtype を他のモジュールで定義することが可能であることを宣言します．
  * @jexample   = > :- module_extendable.
  * @end_jexample
*/
bool module_extendable( machine& )
{
  lilfes::module::CurrentModule()->SetExtendableMode(true);
  return true;
}

LILFES_BUILTIN_DIRECTIVE_0(module_extendable, module_extendable);

} // namespace builtin

/**
  * @predicate  = get_module(+Type, -Module)
  * @desc       = Get the module name that contains an specified type.
  * @param      = +Type/ bot  : Type of the object
  * @param      = -Module/ string : Module name
  * @example    =
  > ?- get_module(list, X).
  X: "core"
  * @end_example

  * @jdesc      = Type の所属する Module を取得します．
  * @jparam     = +Type/ bot : 対象の型
  * @jparam     = -Module/ string : Type が所属するモジュール名
  * @jexample   =
  > ?- get_module(list, X).
  X: "core"
  * @end_jexample
*/
bool get_module( machine& m, FSP arg1, FSP arg2 )
{
  return arg2.Unify(FSP(m,arg1.GetType()->GetModule()->GetName()));
}

LILFES_BUILTIN_PRED_2(get_module, get_module)

/**
  * @predicate  = is_exported(+Type)
  * @desc       = Can discover if a module can accessed from another.
  * @param      = +Type/ bot : Type of the target
  * @example    =
  > ?- is_exported(list).
  yes
  * @end_example

  * @jdesc      = Type が他のモジュールから見えるかどうか判定します．
  * @jparam     = +Type/ bot : 対象の型
  * @jexample   =
  > ?- is_exported(list).
  yes
  * @end_jexample
*/
bool is_exported( machine&, FSP arg1 )
{
  return arg1.GetType()->IsExported();
}

LILFES_BUILTIN_PRED_1(is_exported, is_exported)

/**
  * @predicate  = change_module(+NAME)
  * @desc       = Change the present module.
  * @param      = +NAME/ string  : Module name
  * @example    = > :- change_module("lillib:lists").
  * @end_example

  * @jdesc      = 現在のモジュールを変更します．
  * @jparam     = +NAME/ string: モジュール名
  * @jexample   = > :- change_module("lillib:lists").
  * @end_jexample
*/
bool change_module( machine&, FSP arg1 )
{
	if (! arg1.IsString() ) {
          RUNERR("change_module/1 requires a module name");
          return false;
	}
	module* mod = module::SearchModule( arg1.ReadString() );
	module* prevmod = module::CurrentModule();
	if (mod == NULL) {
          RUNWARN("Warning: module '"<<arg1.ReadString() << "' has not been loaded yet");
          return false;
	}

//	prevmod->Import(mod);
	mod->SetPrevModule(prevmod);
	module::SetCurrentModule( mod );
//	mod->SetInterfaceMode(true);
	mod->SetInterfaceMode(false);
  return true;
}

LILFES_BUILTIN_PRED_1(change_module, change_module)

/**
  * @predicate  = return_module
  * @desc       = Restore the previous module
  * @param      = +NAME/ string  : Module name
  * @example    = > :- return_module.
  * @end_example

  * @jdesc      = 前のモジュールに戻ります．
  * @jexample   = > :- return_module.
  * @end_jexample
*/
bool return_module( machine& )
{
	module* mod = module::CurrentModule()->GetPrevModule();
	if (mod == 0) {
          RUNERR("return_module/0 : current module does not have previous module.");
          return false;
	}
	module::SetCurrentModule(mod);
  return true;
}

LILFES_BUILTIN_PRED_0(return_module, return_module)

/**
  * @predicate  = current_module(-NAME)
  * @desc       = Get name of present module.
  * @param      = -NAME/ string  : Module name
  * @example    =
  * @end_example

  * @jdesc      = 現在のモジュール名を取得します．
  * @jparam     = -NAME/ string: モジュール名
  * @jexample   =
  * @end_jexample
*/
bool current_module( machine& m, FSP arg1 )
{
  return arg1.Unify(FSP(m, module::CurrentModule()->GetName()));
}

LILFES_BUILTIN_PRED_1(current_module, current_module)

#define EXT_LENGTH 4

void get_fileinfo(const string &filename, string *lilname, string *modulename)
//get filename return filename with ".lil" and extract it's modulename
{
	if( filename ==  "-" )
	{
		*lilname = "-";
		*modulename = "stdin";
		return;
	}

	const char *buf = filename.c_str();
	bool backslash = false;
	const char *p;

	p = strrchr(buf, '/');
	if( p == NULL )
	{
		p = strrchr(buf, '\\');
		if( p != NULL )
		{
			backslash = true;
		}
		else
		{
			p = buf;
		}
	}

    string temp;
	if( !strrchr(p, '.') )
	{
		*lilname = filename + ".lil";
        temp.assign(buf);
	}
	else
	{
		*lilname = filename;
		temp.assign(buf, p - buf);
	}
	replace(temp.begin(), temp.end(), backslash ? '\\' : '/', ':');
	
	if( temp[0] == ':' )
	{
		*modulename = temp.substr(1);
	}
	else
	{
		*modulename = temp;
	}
}


void module::Finish() 
{ 
	ASSERT( this == CurrentModule() );
#ifdef DEBUG_MODULE
	cout << "Finish " << name << endl;
#endif
	if( IsRegistered() ) 
	{
		if( prev_module ) 
		{ 
#ifdef DEBUG_MODULE
			cout << "Import " << prev_module->name << endl;
#endif
			if( GetModuleType() != NMT_READ_MODULE && prev_module != this )
				if( GetModuleType() != module::NMT_ENSURE_WITHOUT_BUILTIN
				 && GetModuleType() != module::NMT_ENSURE_WITHOUT_BUILTIN_IN_PROG )
					prev_module->Import(this); 
			prev_module = NULL;
		}
	}
	else 
	{
		delete this; 
		m_current = NULL;
	}
}


void module::UseCurrentModule()
{
	if( ! module::CurrentModule()->IsRegistered() )
	{
		RUNERR("The first line must be a module directive");
		module::CurrentModule()->Register();
	}
}


#define LEFT_ALIGN 35
void module::Register(const string &str /*= ""*/) 
{ 
	if( IsRegistered() ) 
		ABORT("Duplicated module name register"); 

	name = module_prefix + (str.empty() ? defname : str); 
	modulelist_hash[this->GetKey()] = this;
}





} // namespace lilfes

