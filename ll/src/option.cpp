/**
 * @module  = option
 * @cvs     = $Id: option.cpp,v 1.31 2011-05-02 10:38:23 matuzaki Exp $
 * @copyright = Copyright (c) 1997-1998, Makino Takaki
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = Option operations
 * Predicates for operating command-line options
 * @jdesc   = オプション操作
 * @japanese  = コマンドラインやLiLFeSプログラムの中で使用できるオプションに関する述語です。
 * @end_japanese
*/

#include "builtin.h"
#include "errors.h"
#include "option.h"
#include "search_path.h"
#include <cstring>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif


namespace lilfes {

using std::cout;
using std::endl;
using std::istringstream;
using std::map;
using std::ostream;
using std::ostringstream;
using std::set;
using std::strchr;
using std::strcmp;
using std::string;
using std::vector;


static const char rcsid[] = "$Id: option.cpp,v 1.31 2011-05-02 10:38:23 matuzaki Exp $";

/////////////////////////////////////////////////////////////////////////////
// LiLFeS flags
  
_HASHMAP<string, lilfes_flag *> lilfes_flag::flagCollection;
lilfes_flag* lilfes_flag::flagmap[UCHAR_MAX];

// helper function
bool SetFlagFromStringHelper(const char *s, int *val)
{
	if( s == NULL )
	{
		*val = 1;
		return false;
	}
	if( *s == '=' )
	{
		s++;
	}
		
	if( *s == '+' )
	{
		*val = 1;
	}
	else if ( isdigit(*s) || (*s == '-' && isdigit(s[1])))
	{
		*val = atoi(s);
	}
	else
	{
		*val = 1;
		return false;
	}
	return true;
}
  
/////////////////////////////////////////////////////////////////////////////
// LiLFeS quiet flag
bool lilfes_quiet_flag::SetFlagFromString(const char *s)
{
	int val = 0;
	bool ret = SetFlagFromStringHelper(s, &val);
#ifdef PROFILE
	profiler::SetQuiet(!!val);
#endif // PROFILE
	return ret;
}


bool lilfes_quiet_flag::SetFlagFromFSP(FSP f)
{
	if( ! f.IsInteger())
	{
		RUNERR("LiLFeS Flag \"quiet\" requires 1 or 0");
		return false;
	}

#ifdef PROFILE
	int b = f.ReadInteger();
	if(b == 0)
	{
		profiler::SetQuiet(false);
		return true;
	}
	else if(b == 1)
	{
		profiler::SetQuiet(true); return true;
	}
	else
	{ 
		RUNERR("LiLFeS Flag \"quiet\" requires 1 or 0");
		return false; 
	}
#else // PROFILE
	return true;
#endif // PROFILE
}

FSP lilfes_quiet_flag::GetFlagToFSP(machine &m) const
{
#ifdef PROFILE
	int i = (int)profiler::GetQuiet();
#else // PROFILE
	int i = 1;
#endif // PROFILE
	return FSP(m, static_cast<mint>(i));
}
	
/////////////////////////////////////////////////////////////////////////////
// LiLFeS help flag

int lilfes_help_flag::help_flag = 0;

void lilfes_help_flag::printHelp(string &str)
{
	extern const char *LastCompiled;
	string last(LastCompiled);
	str =  "Last Compiled on " + last;
	
#if defined( WITH_TNT )
	str += " [TNT]";
#endif
#if defined( WITH_GDBM )
	str += " [GDBM]";
#endif
#if defined( WITH_ASSOC )
	str += " [ASSOC]";
#endif
#if defined( WITH_ITSDB )
	str += " [ITSDB]";
#endif

#if defined( DEBUG )
	str += " [DEBUG]";
#elif defined( DOASSERT )
	str += " [DOASSERT]";
#endif

#if defined( NOMEMCHK )
	str += " [NOMEMCHK]";
#endif
#if defined( XCONSTR )
	str += " [XCONSTR]";
#endif
#if defined( INLINE_WRITEHEAP )
	str += " [IN_WH]";
#endif
#if defined( TYPE_TABLE_HASH )
	str += " [TTHASH]";
#endif
#if defined( PROFILE_HASH )
	str += " [PROFHASH]";
#endif
#if defined( PROF_UNIFY )
	str += " [PROFUNIFY]";
#endif
#if defined( PROF_COPY )
	str += " [PROFCOPY]";
#endif
#if defined( FORCE_DYNAMIC_UNIFY )
	str += " [FORCE_DU]";
#endif
#if defined( FORCE_NORMAL_COPY )
	str += " [FORCE_NC]";
#endif
#if defined( USE_STRING_GC )
	str += " [STRING_GC]";
#endif
#if defined( AMAVL_MODE )
	str += " [AMAVL]";
#endif
#if defined( PROFILE )
	ostringstream str1;
	str1 << PROFILE;
	str += " [PROF=" + str1.str() + "]";
#endif
	str += "\n";

	str += "Usage:\n" ;

	set<string> sorted_set;

	for( _HASHMAP<string, lilfes_flag *>::const_iterator it = flagCollection.begin();
		 it != flagCollection.end(); ++it )
	{
		ostringstream oss;
		it->second->PrintUsage(oss); 
		sorted_set.insert(oss.str());
	}

	for( set<string>::const_iterator it = sorted_set.begin();
		 it != sorted_set.end(); ++it )
	{
		str += *it;
	}
}

bool lilfes_help_flag::SetFlagFromString(const char *)
{
	help_flag = 1;
	return false;

}

FSP lilfes_help_flag::GetFlagToFSP(machine &m) const
{
	string s;
	lilfes_help_flag::printHelp(s);

	return FSP(m, s.c_str());
}

//////////////////////////////////////////////////////////////////////////////
// LiLFeS load flag

vector< string > lilfes_load_flag::stov;
vector< module::new_module_type > lilfes_load_flag::ston;

bool lilfes_load_flag::SetFlagFromString(const char *s)
{
	if( s == NULL || *s == '\0' )
	{
		RUNERR( "-l option requires a file name" );
		return false;
	}

	stov.push_back( s );
	ston.push_back( nmt );

	return true;
}

FSP lilfes_load_flag::GetFlagToFSP(machine &m) const
{
	FSP f( m );
	FSP froot( m, f.GetAddress() );
	for ( size_t i = 0; i < stov.size(); ++i )
	{
		FSP g( m, stov[i].c_str() );
		f.Follow( hd ).Unify( g );
		f = f.Follow( tl );
	}
	f.Coerce( nil );

	return froot;
}

//////////////////////////////////////////////////////////////////////////////
// LiLFeS path flag

bool lilfes_path_flag::SetFlagFromFSP(FSP f)
{
	if( !f.IsString() )
	{
		RUNERR("LiLFeS Flag \"include-path\" requires 1 or 0");
		return false;
	}

	string path = f.ReadString();
	search_path::AddSearchPath(path);

	return true;
}

bool lilfes_path_flag::SetFlagFromString(const char *s)
{
	if( s == NULL || *s == '\0' )
	{
		RUNERR( "-I option requires a pathname" );
		return false;
	}

	search_path::AddSearchPath(s);
	return true;
}

FSP lilfes_path_flag::GetFlagToFSP(machine &m) const
{
	FSP f( m );
	FSP froot( m, f.GetAddress() );

	search_path *s = search_path::SearchPath();
	while( s )
	{
		f.Follow(hd).Unify(FSP(m,s->GetName() ));
		f = f.Follow(tl);
		s = s->Next();
	}
  
	f.Coerce( nil );
	return froot;
}

//////////////////////////////////////////////////////////////////////////////
// LiLFeS int flag

bool lilfes_int_flag::SetFlagFromFSP(FSP f)
{
	if( ! f.IsInteger() )
	{
		RUNERR("LiLFeS Flag \"" << name << "\" requires integer");
		return false;
	}
	else if( !is_changeable )
	{
		RUNERR("LiLFeS Flag \"" << name << "\" is read-only");
		return false;
	}
	else
	{
		val = f.ReadInteger();
		return true;
	}
}

bool lilfes_int_flag::SetFlagFromString(const char *s)
{
	return SetFlagFromStringHelper(s, &this->val);
}

//////////////////////////////////////////////////////////////////////////////
// LiLFeS arg flag

vector< string > lilfes_arg_flag::comargs;

bool lilfes_arg_flag::SetFlagFromStringArray(int argc, 
					     const char **argv)
{
	if( argc <= 0 )
	{
		RUNERR( "-a option requires arguments" );
		return false;
	}
  
	for(int j=0; j<argc; j++)
	{
		comargs.push_back( argv[j] );
	}

	return true;
}

bool lilfes_arg_flag::SetFlagFromStringArray(const vector< string >& args)
{
	if( args.empty() )
	{
		RUNERR( "-a option requires arguments" );
		return false;
	}
  
	for(vector< string >::size_type j=0; j<args.size(); j++)
	{
		comargs.push_back( args[j] );
	}

	return true;
}

FSP lilfes_arg_flag::GetFlagToFSP(machine &m) const
{
	FSP f( m );
	FSP froot( m, f.GetAddress() );
	for ( size_t i = 0; i < comargs.size(); ++i )
	{
		FSP g( m, comargs[i].c_str() );
		f.Follow( hd ).Unify( g );
		f = f.Follow( tl );
	}
	f.Coerce( nil );
  
	return froot;
}

//////////////////////////////////////////////////////////////////////////////
// LiLFeS eval flag

vector< string > lilfes_eval_flag::cmd_list;

bool lilfes_eval_flag::SetFlagFromString(const char *s)
{
	if( s == NULL )
	{
		RUNERR( "-e option requires arguments" );
		return false;
	}

	cmd_list.push_back( s );
	return true;

}

FSP lilfes_eval_flag::GetFlagToFSP(machine &m) const
{
	RUNERR("LiLFeS Flag \"eval\" is only available from command-line.");
	return FSP(m);
}

//////////////////////////////////////////////////////////////////////////////
// LiLFeS package directory flag

string lilfes_package_dir_flag::LILFES_PACKAGE_DIR = PKGDATADIR;

bool lilfes_package_dir_flag::SetFlagFromFSP( FSP f )
{
	if ( ! f.IsString() )
	{
		RUNERR( "package-dir option requires string" );
		return false;
	}
	LILFES_PACKAGE_DIR = f.ReadString();
	return true;
}

bool lilfes_package_dir_flag::SetFlagFromString(const char *s)
{
	if( s == NULL )
	{
		RUNERR( "package-dir option requires arguments" );
		return false;
	}

	LILFES_PACKAGE_DIR = s;
	return true;
}

FSP lilfes_package_dir_flag::GetFlagToFSP(machine &m) const
{
	return FSP(m, LILFES_PACKAGE_DIR.c_str());
}

//////////////////////////////////////////////////////////////////////

static void init3()
{
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << " (init3)" << endl;
#endif

	new lilfes_int_flag ('n',"no_initialize"  , 0, false, 
		"Do not interpret global initialize files");
	new lilfes_int_flag ('u',"no_user_initialize"  , 0, false, 
		"Do not interpret user initialize files");
	new lilfes_int_flag ('w',"warning"		 , 1, true,
		"Display all warnings");
	new lilfes_quiet_flag ('q',"quiet"		   ,
		"Suppress verbose messages");
	new lilfes_int_flag (	"warn-module-override" , 0, true,
		"Warn module override (same module name with different paths)");
	new lilfes_load_flag('l',"load",
		"Load module", module::NMT_ENSURE_INITIAL);
	new lilfes_path_flag('I',"include-path",
		"Add search path of modules");
	new lilfes_arg_flag('a',"argv",
		"Set command-line arguments");
	new lilfes_eval_flag('e',"eval",
		"Execute commands from command-line");
	new lilfes_help_flag('h',"help",
		"Show this help");
	new lilfes_package_dir_flag("package-dir",
		"Directory of LiLFeS data");

#ifdef DEBUG
	cout << "End Initialize " << __FILE__ << " (init3)" << endl;
#endif
}

static InitFunction IF3(init3, 200);

/**
  * @predicate	= lilfes_flag(+OPT,-ARG) 
  * @desc	= Get an argument of option. 
  * @param	= +OPT/string :option mode
  * @param	= -ARG/bot :argument of option
  * @note	= List of option
  * @note	= <div align="conter">
  * @note	=   <table>
  * @note	=	 <tr>
  * @note	=	   <td>abbreviation</td>
  * @note	=	   <td>name</td>
  * @note	=	   <td>context</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'n'</td>
  * @note	=	   <td>"no_initialize"</td>
  * @note	=	   <td>Do not interpret global initialize files</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'u'</td>
  * @note	=	   <td>"no_user_initialize"</td>
  * @note	=	   <td>Do not interpret user initialize files</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td height="10" width="23">'w'</td>
  * @note	=	   <td>"warning"</td>
  * @note	=	   <td>Display all warnings</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'q'</td>
  * @note	=	   <td>"quiet"</td>
  * @note	=	   <td>Suppress verbose messages</td> 
  * @note	=	</tr>
  * @note	=	 <tr>
  * @note	=	   <td></td>
  * @note	=	   <td>"warn-module-override"</td>
  * @note	=	   <td>Warn module override (same module name with different paths)</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'l'</td>
  * @note	=	   <td>&quot;load&quot;</td>
  * @note	=	   <td>Load module</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'I'</td>
  * @note	=	   <td>"include-path"</td>
  * @note	=	   <td>Add search path of modules</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	 <tr>
  * @note	=	   <td>'a'</td>
  * @note	=	   <td>"argv"</td>
  * @note	=	   <td>Set command-line arguments</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'e'</td>
  * @note	=	   <td>"eval"</td>
  * @note	=	   <td>Exec a LiLFeS program from command-line</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'h'</td>
  * @note	=	   <td>"help"</td>
  * @note	=	   <td>Show this help</td> 
  * @note	=	 </tr>
  * @note	=   </table>
  * @note	= </div>
  * @example	= 
  * @end_example
  * @jdesc	= オプションの引数を取得します．
  * @jparam	= +OPT/string :オプションモード
  * @jparam	= -ARG/bot :オプションの引数
  * @jnote	= オプションのリスト
  * @jnote	= <div align="conter">
  * @jnote	=   <table>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>省略形</td>
  * @jnote	=	   <td>オプション名</td>
  * @jnote	=	   <td>内容</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'n'</td>
  * @jnote	=	   <td>"no_initialize"</td>
  * @jnote	=	   <td>グローバルな初期化ファイルを解釈しません。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'u'</td>
  * @jnote	=	   <td>"no_user_initialize"</td>
  * @jnote	=	   <td>ユーザーの初期化ファイルを解釈しません。</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td height="10" width="23">'w'</td>
  * @jnote	=	   <td>"warning"</td>
  * @jnote	=	   <td>全ての警告を表示します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'q'</td>
  * @jnote	=	   <td>"quiet"</td>
  * @jnote	=	   <td>冗長なメッセージの出力を抑制します。</td> 
  * @jnote	=	</tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td></td>
  * @jnote	=	   <td>"warn-module-override"</td>
  * @jnote	=	   <td>モジュールのoverride（異なるパス名で同じ名前のモジュールを読み込むこと）を警告します。</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'l'</td>
  * @jnote	=	   <td>&quot;load&quot;</td>
  * @jnote	=	   <td>モジュールを読み込みます。</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'I'</td>
  * @jnote	=	   <td>"include-path"</td>
  * @jnote	=	   <td>モジュールの検索パスを追加します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'a'</td>
  * @jnote	=	   <td>"argv"</td>
  * @jnote	=	   <td>コマンドライン引数を指定します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'e'</td>
  * @jnote	=	   <td>"eval"</td>
  * @jnote	=	   <td>コマンドラインからLiLFeSプログラムを実行します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'h'</td>
  * @jnote	=	   <td>"help"</td>
  * @jnote	=	   <td>オプションのヘルプを表示します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=   </table>
  * @jnote	= </div>
  * @jexample	= 
  * @end_jexample
*/
/**
  * @predicate	= prolog_flag(+OPT,-ARG) 
  * @desc	= Get an argument of option. 
  * @param	= +OPT/string :option mode
  * @param	= -ARG/bot :argument of option
  * @note	= List of option
  * @note	= <div align="conter">
  * @note	=   <table>
  * @note	=	 <tr>
  * @note	=	   <td>abbreviation</td>
  * @note	=	   <td>name</td>
  * @note	=	   <td>context</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'n'</td>
  * @note	=	   <td>"no_initialize"</td>
  * @note	=	   <td>Do not interpret global initialize files</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'u'</td>
  * @note	=	   <td>"no_user_initialize"</td>
  * @note	=	   <td>Do not interpret user initialize files</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td height="10" width="23">'w'</td>
  * @note	=	   <td>"warning"</td>
  * @note	=	   <td>Display all warnings</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'q'</td>
  * @note	=	   <td>"quiet"</td>
  * @note	=	   <td>Suppress verbose messages</td> 
  * @note	=	</tr>
  * @note	=	 <tr>
  * @note	=	   <td></td>
  * @note	=	   <td>"warn-module-override"</td>
  * @note	=	   <td>Warn module override (same module name with different paths)</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'l'</td>
  * @note	=	   <td>&quot;load&quot;</td>
  * @note	=	   <td>Load module</td>
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'I'</td>
  * @note	=	   <td>"include-path"</td>
  * @note	=	   <td>Add search path of modules</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	 <tr>
  * @note	=	   <td>'a'</td>
  * @note	=	   <td>"argv"</td>
  * @note	=	   <td>Set command-line arguments</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'e'</td>
  * @note	=	   <td>"eval"</td>
  * @note	=	   <td>Exec a LiLFeS program from command-line</td> 
  * @note	=	 </tr>
  * @note	=	 <tr>
  * @note	=	   <td>'h'</td>
  * @note	=	   <td>"help"</td>
  * @note	=	   <td>Show this help</td> 
  * @note	=	 </tr>
  * @note	=   </table>
  * @note	= </div>
  * @example	= 
  * @end_example
  * @jdesc	= オプションの引数を取得します．
  * @jparam	= +OPT/string :オプションモード
  * @jparam	= -ARG/bot :オプションの引数
  * @jnote	= オプションのリスト
  * @jnote	= <div align="conter">
  * @jnote	=   <table>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>省略形</td>
  * @jnote	=	   <td>オプション名</td>
  * @jnote	=	   <td>内容</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'n'</td>
  * @jnote	=	   <td>"no_initialize"</td>
  * @jnote	=	   <td>グローバルな初期化ファイルを解釈しません。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'u'</td>
  * @jnote	=	   <td>"no_user_initialize"</td>
  * @jnote	=	   <td>ユーザーの初期化ファイルを解釈しません。</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td height="10" width="23">'w'</td>
  * @jnote	=	   <td>"warning"</td>
  * @jnote	=	   <td>全ての警告を表示します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'q'</td>
  * @jnote	=	   <td>"quiet"</td>
  * @jnote	=	   <td>冗長なメッセージの出力を抑制します。</td> 
  * @jnote	=	</tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td></td>
  * @jnote	=	   <td>"warn-module-override"</td>
  * @jnote	=	   <td>モジュールのoverride（異なるパス名で同じ名前のモジュールを読み込むこと）を警告します。</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'l'</td>
  * @jnote	=	   <td>&quot;load&quot;</td>
  * @jnote	=	   <td>モジュールを読み込みます。</td>
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'I'</td>
  * @jnote	=	   <td>"include-path"</td>
  * @jnote	=	   <td>モジュールの検索パスを追加します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'a'</td>
  * @jnote	=	   <td>"argv"</td>
  * @jnote	=	   <td>コマンドライン引数を指定します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'e'</td>
  * @jnote	=	   <td>"eval"</td>
  * @jnote	=	   <td>コマンドラインからLiLFeSプログラムを実行します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=	 <tr>
  * @jnote	=	   <td>'h'</td>
  * @jnote	=	   <td>"help"</td>
  * @jnote	=	   <td>オプションのヘルプを表示します。</td> 
  * @jnote	=	 </tr>
  * @jnote	=   </table>
  * @jnote	= </div>
  * @jexample	= 
  * @end_jexample
*/
namespace builtin {

bool lilfes_flag_2( machine& m, FSP arg1, FSP arg2 )
{
	if( ! arg1.IsString() )
	{
		RUNERR("Error: lilfes_flag/2 requires a string as a first argument");
		return false;
	}
	else
	{
		lilfes::lilfes_flag *lf = lilfes::lilfes_flag::Search(arg1.ReadString());
		if( lf == NULL )
		{
			RUNERR("Error: Unknown lilfes flag \"" << arg1.ReadString() << "\"");
			return false;
		}
		else
		{
			return arg2.Unify(lf->GetFlagToFSP(m));
		}
	}
}
  
LILFES_BUILTIN_PRED_2(lilfes_flag_2, lilfes_flag);
LILFES_BUILTIN_PRED_2(lilfes_flag_2, prolog_flag);

} // namespace builtin


//////////////////////////////////////////////////////////////////////////////
// lilfes_flag/3 : 

/**
  * @predicate	= lilfes_flag(+OPT, -OLD, -NEW) 
  * @desc	= Set option.
  * @param	= +OPT/string : option mode
  * @param	= -OLD/bot : an old value of the option
  * @param	= -NEW/bot : a new value of the option
  * @example	= 
  * @end_example
  * @jdesc	  = オプションの引数を新たに設定します。
  * @jparam	= +OPT/string : オプションのモード
  * @jparam	= -OLD/bot : 引数の古い値
  * @jparam	= -NEW/bot : 引数の新しい値
  * @jexample	= 
  * @end_jexample
*/
/**
  * @predicate	= prolog_flag(+OPT, -OLD, -NEW) 
  * @desc	= Set option.
  * @param	= +OPT/string : option mode
  * @param	= -OLD/bot : an old value of the option
  * @param	= -NEW/bot : a new value of the option
  * @example	= 
  * @end_example
  * @jdesc	  = オプションの引数を新たに設定します。
  * @jparam	= +OPT/string : オプションのモード
  * @jparam	= -OLD/bot : 引数の古い値
  * @jparam	= -NEW/bot : 引数の新しい値
  * @jexample	= 
  * @end_jexample
*/
namespace builtin {

bool lilfes_flag_3( machine& m, FSP arg1, FSP arg2, FSP arg3 )
{
	if( !lilfes_flag_2(m, arg1, arg2))
	{
		return false;
	}

	if( ! arg1.IsString() )
	{
		RUNERR("Error: lilfes_flag/3 requires a string as a first argument");
		return false;
	}
	else
	{
		lilfes_flag *lf = lilfes_flag::Search(arg1.ReadString());
		if( lf == NULL )
		{
			RUNERR("Error: Unknown lilfes flag \"" << arg1.ReadString() << "\"");
			return false;
		}
		else
		{
			return lf->SetFlagFromFSP(arg3);
		}
	}
}

LILFES_BUILTIN_PRED_OVERLOAD_3(lilfes_flag_3, lilfes_flag_3, lilfes_flag);
LILFES_BUILTIN_PRED_OVERLOAD_3(lilfes_flag_3, prolog_flag_3, prolog_flag);

} // namespace builtin

///////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= get_argc(-ARGC) 
  * @desc	= Get the number of command-line arguments.
  * @param	= -ARGC/integer : the number of arguments
  * @example	= 
  * % lilfes -a arg1 arg2
  * > ?- get_argc(X).
  * X: 2
  * @end_example
  * @jdesc	= コマンドライン引数の数を取得します。
  * @jparam	= 
  * @jexample	= 
  * @end_jexample
*/
namespace builtin{
bool get_argc( machine& m, FSP arg1 )
{
	FSP result = FSP( m, static_cast<mint>(lilfes_arg_flag::getArgc()) );
  
	if( ! arg1.Unify( result ))
	{		
		return false;
	}
	return true;
}

LILFES_BUILTIN_PRED_1(get_argc, get_argc);
}

///////////////////////////////////////////////////////////////////////////////

/**
  * @predicate	= get_argv(+IND, -ARG) 
  * @desc	= Get a commandline argument.
  * @param	= +IND/integer : index
  * @param	= -ARG/string : commandline argument
  * @example	= 
  * % lilfes -a arg1 arg2
  * > ?- get_argv(1, X).
  * X: "arg2"
  * @end_example
  * @jdesc	= コマンドライン引数を取得します。
  * @jparam	= 
  * @jexample	= 
  * @end_jexample
*/
namespace builtin{
bool get_argv( machine& m, FSP arg1, FSP arg2 )
{
	if ( ! arg1.IsInteger() )
	{
		RUNWARN( "get_argv/2: arg1 requires integer" );
		return false;
	}

	int i = arg1.ReadInteger();

	if( i >= lilfes_arg_flag::getArgc() )
	{
		RUNWARN( "get_argv/2: arg1 is too big" );
		return false;
	}

	FSP result = FSP( m, lilfes_arg_flag::getArgv( i ).c_str() );

	if( ! arg2.Unify( result ) )
	{
		return false;
	}
	return true;
}

LILFES_BUILTIN_PRED_2(get_argv, get_argv);

} // namespace builtin

void lilfes_flag::PrintUsage(ostream &out)
{
	out << "   ";
	if( opt )
	{
		out << "-" << opt;
	}
	else
	{
		out << "  ";
	}

	if( !name.empty() )
	{
		out << " --" << name;
	}
	out << "  " << usage << endl;
}

void lilfes_flag::ParseOptions(machine *mach, int argc, const char **argv)
{
#ifdef DEBUG
	cout << "ParseOptions" << argc << endl;
#endif

	int warn = 0;

#ifdef DEBUG
	cout << "ParseOptions..." << argc << endl;
#endif

#ifdef DEBUG
	cout << "ParseOptions   " << argc << endl;
#endif

	for( int i=1; i<argc; i++ )
	{
#ifdef DEBUG
	cout << "ParseOptions " << i << "/" << argc << "  " << argv[i] << endl;
#endif
		if( argv[i][0] == '-' && argv[i][1] )
		{
			if( argv[i][1] == '-' )  /// オプションが　"--なんとか"の場合
			{
				const char *p = strchr(argv[i]+2, '='); //最も末尾側に出現する'='へのポインタを取得
				if( p ) //'='があれば・・・
				{
					lilfes_flag *f = lilfes_flag::Search(string(argv[i]+2, p - (argv[i] + 2)));
					if( f == NULL )
					{
						RUNERR( "Unknown option: --" << argv[i]+2 );
					}
					else
					{
						f->SetFlagFromString(p+1);
					}
				}
				else
				{
					lilfes_flag *f = lilfes_flag::Search(argv[i]+2);

					if( f == NULL )
					{
						RUNERR( "Unknown option: --" << argv[i]+2 );
					}
					else if( strcmp( (argv[i]+2), "argv" ) == 0 )
					{
						//lilfes_arg_flag::SetFlagFromStringArray(argc, argv, i);
						//i += comargc - 1;
						lilfes_arg_flag::SetFlagFromStringArray(argc-i-1, argv+i+1);
						break;
					}
					else
					{
						bool ret = f->SetFlagFromString(argv[i+1]);
						if( ret )
						{
							i++;
						}
					}
				}
			}
			else  /// オプションが　"-なんとか"の場合
			{
				lilfes_flag *f = lilfes_flag::GetFlagMap(argv[i][1]);
				if( f == NULL )
				{
					RUNERR( "Unknown option: -" << argv[i][1] );
				}
				else if( argv[i][1] == 'a' )
				{
					lilfes_arg_flag::SetFlagFromStringArray(argc-i-1, argv+i+1);
					break;
				}
				else if( argv[i][2] )
				{
					f->SetFlagFromString(argv[i]+2);
				}
				else
				{
					bool ret = f->SetFlagFromString(argv[i+1]);
					if( ret ) // 次のオプション文字列を消費した場合は、そいつの処理をとばしてその次へ。
					{
						i++;
					}
				}
			}
		}
		else
		{
			if( !warn )
			{
				RUNWARN( "obsolete commandline" );
				warn = 1;
			}
			lilfes_load_flag::pushNewModule( argv[ i ], module::NMT_INCLUDE );
		}
	}

#ifdef DEBUG
	cout << "ParseOptions q" << endl;
#endif
	for ( int i = lilfes_load_flag::getNumLoadModule() - 1; i >= 0; --i )
	{
		module::new_module(*mach, "command line",
						   lilfes_load_flag::getLoadModule( i ).c_str(),
						   lilfes_load_flag::getLoadModuleType( i ));
	}

#ifdef DEBUG
	cout << "ParseOptions r" << endl;
#endif
	if( ! lilfes_flag::Search("no_user_initialize")->GetIntVal() )
	{
		string path;

		const char *init = getenv("LILFESRC");
		if( init != NULL )
		{
			path = init;
		}
		else
		{
			const char *path_ptr = getenv(LOCAL_INITIALIZATION_PATH_ENV);
			if (path_ptr)
			{
				path = path_ptr;
				path += PATH_SEPARATOR;
			}

			path += LOCAL_INITIALIZATION_FILE;
		}
		module::new_module(*mach, "initialization phase", path, module::NMT_INITIALIZE, module::UserModule());
	}

#ifdef DEBUG
	cout << "ParseOptions s" << endl;
#endif
	if( ! lilfes_flag::Search("no_initialize")->GetIntVal() )
	{
		module::new_module(*mach, "initialization phase", GLOBAL_INITIALIZATION_FILE, module::NMT_INITIALIZE, module::UserModule());
	}

#ifdef DEBUG
	cout << "ParseOptions t" << endl;
#endif

#ifdef DEBUG
	cout << "ParseOptions x" << endl;
#endif
}

int lilfes_flag::ExecOptions(machine &m)
{
  
	if( lilfes_eval_flag::getCommandList().empty() && lilfes_help_flag::getHelpFlag() == 0 )
	{
		return 0;
	}
	else
	{
		if( lilfes_help_flag::getHelpFlag() )
		{ 
			string s;
			lilfes_help_flag::printHelp(s);
			*error_stream << s;
		}
		if( ! lilfes_eval_flag::getCommandList().empty() )
		{
			for ( vector< string >::const_iterator it = lilfes_eval_flag::getCommandList().begin();
				  it != lilfes_eval_flag::getCommandList().end(); ++it )
			{
				istringstream iss(*it);
				lilfes::lexer_t lexer(&iss, "eval_flag");
				m.setLexer(lexer);
				m.parse();
			}
		}
		return 1;
	}
	return 0;
}

} // namespace lilfes



