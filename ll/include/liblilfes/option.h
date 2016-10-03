/* $Id: option.h,v 1.14 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1998-2000, Takaki Makino
 *
 *    You may distribute this file under the terms of the Artistic License.
 */

///  <ID>$Id: option.h,v 1.14 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>option.h</name>
//
///  <overview>
///  <jpn>コマンドラインやLiLFeSプログラムの中で使用できるオプションに関するルーチン</jpn>
///  <eng>??? </eng>
///  </overview>

#ifndef	__option_h
#define __option_h

#include "lconfig.h"
#include "structur.h"

#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace lilfes {

////////////////////////////////////////
/// <classdef>
/// <name> lilfes_flag </name>
/// <overview> オプション</overview>
/// <desc>
/// <p>
/// lilfes_flagインスタンス１つが、１つのオプション（例えば-rとか-fとか）に対応します。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_flag
{
 protected:
	uchar opt;
	/// 短いオプション名です。optが'\0'の時は、短いオプション名が存在しないということです。 
	std::string name;
	/// 長いオプション名です。
	std::string usage;
	/// 使い方の説明です。

	static _HASHMAP<std::string, lilfes_flag *> flagCollection;
	/// 長いオプション名(name)から 対応するlilfes_flagオブジェクトを引くためのハッシュです。
	static lilfes_flag* flagmap[];
	/// 短いオプション名(opt)から 対応するlilfes_flagオブジェクトを引くためのテーブルです。

 public:
	lilfes_flag(const std::string iname, const std::string iusage) : opt('\0'), name(iname), usage(iusage)
	{
		flagCollection.insert(std::make_pair(name, this));
	}
	/// <var>iname</var>：長いオプション名<br>
	/// <var>iusage</var>：使い方の説明<br>

	lilfes_flag(uchar iopt, const std::string iname, const std::string iusage) : opt(iopt), name(iname), usage(iusage)
	{
		flagmap[opt] = this;
		flagCollection.insert(std::make_pair(name, this));
	}
	/// <var>iopt</var>：短いオプション名<br>
	/// <var>iname</var>：長いオプション名<br>
	/// <var>iusage</var>：使い方の説明<br>
	virtual ~lilfes_flag() { }

	const std::string GetKey() const { return name; }
	/// ハッシュ用のメソッドです。キーとして名前を返します。
	virtual int GetIntVal() const { return 0; }
	/// <ref>lilfes_int_flag</ref>など整数値の引数をとることができるオプションから、整数値を取得する仮想関数です。
	virtual void SetIntVal(int) { }
	/// <ref>lilfes_int_flag</ref>など整数値の引数をとることができるオプションに、整数値を設定する仮想関数です。
	/// 文字列の引数をとることができるオプションに、文字列を設定する仮想関数です。
	virtual bool SetFlagFromFSP(FSP f) = 0;
	/// そのオプションにFSPオブジェクト<var>f</var>をオプションの引数として渡します。引数として使われればtrueが、使われなければfalseが返ります。LiLFeSプログラム中から渡すときに使われます。
	virtual bool SetFlagFromString(const char *s) = 0;	// return true if the string is consumed
	/// そのオプションに文字列<var>s</var>をオプションの引数として渡します。引数として使われればtrueが、使われなければfalseが返ります。
	virtual FSP GetFlagToFSP(machine &m) const { return FSP(m); }
	/// 素性構造の引数をとることができるオプションから、素性構造を取得する仮想関数です。抽象機械<var>m</var>の上に素性構造が作られます。
	virtual void PrintUsage(std::ostream &o);
	/// そのオプションの使い方の説明を表示します。

	static void ParseOptions(machine *mach, int argc, const char **argv);
	/// 最初に渡されたオプション文字列を解析し、適切なlilfes_flagオブジェクトに値を渡すメソッド。

	static int ExecOptions(machine &mach);
	/// -eオプションで指定されたコマンドを解析し、実行するメソッド。

	static lilfes_flag * Search(const std::string s) 
	{
		_HASHMAP<std::string, lilfes_flag *>::const_iterator it = flagCollection.find(s);
		return it != flagCollection.end() ? it->second : NULL;
	}
	/// 長いオプション名<var>s</var>に対応するlilfes_flagオブジェクトを取得します。
	static lilfes_flag * GetFlagMap(uchar c) { return flagmap[c];}
	/// 短いオプション名<var>c</var>に対応するlilfes_flagオブジェクトを取得します。
};
/// </body></classdef>

////////////////////////////////////////
/// <classdef>
/// <name> lilfes_int_flag </name>
/// <overview> 整数値の引数をとることができるオプションのクラス</overview>
/// <desc>
/// <p>
/// オプションの引数として整数値をとることができるようなオプションのクラスです。<br>
/// 例：-d80
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_int_flag : public lilfes_flag
{
	int val;
	bool is_changeable;

 public:
	lilfes_int_flag(const char *iname, int ival, bool flag, const char *iusage) : lilfes_flag(iname, iusage), val(ival), is_changeable(flag) { }
	/// <var>iname</var>：長いオプション名<br>
	/// <var>ival</var>：オプションの引数の初期値<br>
	/// <var>iusage</var>：使い方の説明<br>
  
	lilfes_int_flag(uchar iopt, const char *iname, int ival, bool flag, const char *iusage) : lilfes_flag(iopt, iname, iusage), val(ival), is_changeable(flag) { }
	/// <var>iopt</var>：短いオプション名<br>
	/// <var>ival</var>：オプションの引数の初期値<br>
	/// <var>iusage</var>：使い方の説明<br>
  
	virtual int GetIntVal() const { return val; }
	/// オプションの引数（整数型）を取得します。
  
	virtual void SetIntVal(int i) { val = i; }
	/// オプションの引数に整数<var>i</var>を設定します。
  
	virtual bool SetFlagFromFSP(FSP f);
	/// ???
	virtual bool SetFlagFromString(const char *s);
	/// そのオプションに文字列sをオプションの引数として渡します。 引数として使われればtrueが、使われなければfalseが返ります。
	virtual FSP  GetFlagToFSP(machine &m) const { return FSP(m, (mint)val); }
	/// ???
};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_quiet_flag </name>
/// <overview> オプション　'q',"quiet"で使われるクラス</overview>
/// <desc>
/// <p>
/// 'q',"quiet"オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_quiet_flag : public lilfes_flag
{
 public:
	lilfes_quiet_flag(const char *iname, const char *iusage) : lilfes_flag(iname, iusage) { }
	lilfes_quiet_flag(uchar iopt, const char *iname, const char *iusage) : lilfes_flag(iopt, iname, iusage) { }
    
	virtual bool SetFlagFromFSP(FSP);
	virtual bool SetFlagFromString(const char *);
	virtual FSP GetFlagToFSP(machine &m) const;
};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_help_flag </name>
/// <overview> オプション　'h',"help"で使われるクラス</overview>
/// <desc>
/// <p>
/// 'h',"help"オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_help_flag : public lilfes_flag
{
 private:
	static int help_flag;

 public:
	lilfes_help_flag(const char *iname, const char *iusage) : lilfes_flag(iname, iusage) { }
	lilfes_help_flag(uchar iopt, const char *iname, const char *iusage) : lilfes_flag(iopt, iname, iusage) { }
  
	virtual bool SetFlagFromFSP(FSP)
	{
		RUNERR("LiLFeS Flag \"help\" is read-only");
		return false; 
	};
	virtual bool SetFlagFromString(const char *s); 
	virtual FSP GetFlagToFSP(machine &m) const; 
  
	static void printHelp(std::string &str);

	static int getHelpFlag() { return help_flag; }
};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_load_flag </name>
/// <overview> オプション　'l',"load"で使われるクラス</overview>
/// <desc>
/// <p>
/// 'l',"load"オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_load_flag : public lilfes_flag
{
 private:
	module::new_module_type nmt;
	static std::vector<std::string> stov;
	static std::vector<module::new_module_type> ston;

 public:
	lilfes_load_flag(const char *iname, const char *iusage, module::new_module_type inmt) : lilfes_flag(iname, iusage), nmt(inmt) { }
	lilfes_load_flag(uchar iopt, const char *iname, const char *iusage, module::new_module_type inmt) : lilfes_flag(iopt, iname, iusage), nmt(inmt) { }
  
	virtual bool SetFlagFromFSP(FSP) { 
		RUNERR("LiLFeS Flag \"load\" is read-only");
		return false; 
	}
	virtual bool SetFlagFromString(const char *s); 
	virtual FSP GetFlagToFSP(machine &m) const;

	static void pushNewModule( const std::string& name, module::new_module_type type ) {
		stov.push_back( name );
		ston.push_back( type );
	}

	static int getNumLoadModule() { return stov.size(); }
	static std::string& getLoadModule( size_t i ) { return stov[ i ]; }
	static module::new_module_type getLoadModuleType( size_t i ) { return ston[ i ]; }
};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_path_flag </name>
/// <overview> オプション　"include-path"で使われるクラス</overview>
/// <desc>
/// <p>
/// "include-path"オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_path_flag : public lilfes_flag
{
 public:
	lilfes_path_flag(const char *iname, const char *iusage) : lilfes_flag(iname, iusage){ }
	lilfes_path_flag(uchar iopt, const char *iname, const char *iusage) : lilfes_flag(iopt, iname, iusage) { }

	virtual bool SetFlagFromFSP(FSP);
	virtual bool SetFlagFromString(const char *s);
	virtual FSP GetFlagToFSP(machine &m) const;
};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_arg_flag </name>
/// <overview> オプション　'a' で使われるクラス</overview>
/// <desc>
/// <p>
/// 'a' オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_arg_flag : public lilfes_flag
{
 private:
	static std::vector<std::string> comargs;

 public:
	lilfes_arg_flag(const char *iname, const char *iusage) : lilfes_flag(iname, iusage){ }
	lilfes_arg_flag(uchar iopt, const char *iname, const char *iusage) : lilfes_flag(iopt, iname, iusage) { }

	virtual bool SetFlagFromFSP(FSP){ 
		RUNERR("LiLFeS Flag \"argv\" is read-only");
		return false; 
	}
	virtual bool SetFlagFromString(const char *){ return false; }
	virtual FSP GetFlagToFSP(machine &m) const;

	static bool SetFlagFromStringArray(int argc, const char **argv);
	static bool SetFlagFromStringArray(const std::vector<std::string>& args);

	static int getArgc() { return comargs.size(); }
	static const std::string& getArgv( size_t i ) { return comargs[i]; }
	static const std::vector<std::string>& getArgs() { return comargs; }

};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_eval_flag </name>
/// <overview> オプション　'e' で使われるクラス</overview>
/// <desc>
/// <p>
/// 'e' オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_eval_flag : public lilfes_flag
{
 private:
	static std::vector<std::string> cmd_list;

 public:
	lilfes_eval_flag(const char *iname, const char *iusage) : lilfes_flag(iname, iusage){ }
	lilfes_eval_flag(uchar iopt, const char *iname, const char *iusage) : lilfes_flag(iopt, iname, iusage) { }

	virtual bool SetFlagFromFSP(FSP){ 
		RUNERR("LiLFeS Flag \"eval\" is read-only");
		return false; 
	}
	virtual bool SetFlagFromString(const char *);
	virtual FSP GetFlagToFSP(machine &m) const;

	static const std::vector<std::string>& getCommandList() { return cmd_list; }
};
/// </body></classdef>

/// <classdef>
/// <name> lilfes_package_dir_flag </name>
/// <overview> オプション　"package-dir" で使われるクラス</overview>
/// <desc>
/// <p>
/// "package-dir" オプションが指定されたとき、このクラスのオブジェクトが使われます。<br>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class lilfes_package_dir_flag : public lilfes_flag
{
 public:
	static std::string LILFES_PACKAGE_DIR;

 public:
	lilfes_package_dir_flag(const char *iname, const char *iusage) : lilfes_flag(iname, iusage) { }

	virtual bool SetFlagFromFSP(FSP);
	virtual bool SetFlagFromString(const char *);
	virtual FSP GetFlagToFSP(machine &m) const;

};
/// </body></classdef>

} // namespace lilfes

#endif	// __option_h
