/* $Id: module.h,v 1.6 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: module.h,v 1.6 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>module.h</name>
//
///  <overview>
///  <jpn>モジュール</jpn>
///  <eng>module </eng>
///  </overview>

// added by kaityo 
// for module

#ifndef __module_h
#define __module_h

#include "lconfig.h"
#include "errors.h"

namespace lilfes {

class feature;
class type;
class machine;

} // namespace lilfes

#include <string>
#include <vector>

#include "initial.h"

namespace lilfes {

/////////////////////////////////////////////////////////////////////////
/// <classdef>
/// <name> module </name>
/// <overview> モジュール </overview>
/// <desc>
/// <p>
/// 複数の型をまとめて扱うためのクラスです。<BR>
/// <pre>
///         モジュールA                    モジュールB                     モジュールC
/// |------------------------|--    |------------------------|--    |------------------------|
/// |      interface部       |  --  |      interface部       |  --  |      interface部       |
/// |                        |    ->|                        |    ->|                        |
/// |                        |  --  |                        |  --  |                        |
/// |------------------------|--    |------------------------|--    |------------------------|
/// |      implement部       |      |      implement部       |      |      implement部       |
/// |                        |      |                        |      |                        |
/// |                        |      |                        |      |                        |
/// |------------------------|      |------------------------|      |------------------------|
/// </pre>
/// <BLOCKQUOTE>
/// 上の図において、モジュールCがモジュールBをimportし、モジュールBがモジュールAをimportしているとします。このとき、Aのinterface部にあるものはBから見えますが、Aのimplement部にあるものはBからは見えません。同様に、Bのinterface部にあるものはCから見えますが、Bのimplement部にあるものはCからは見えません。また、Aのinterface部にあるものはCから見えます。<BR>
/// </BLOCKQUOTE>
/// <pre>
///         モジュールA
/// |------------------------|--
/// |      interface部       |  --
/// |                        |    --
/// |                        |  -- |
/// |------------------------|--   |         モジュールC                       モジュールD
/// |      implement部       |     |   |------------------------|--     |------------------------|
/// |                        |     --->|      interface部       |  --   |      interface部       |
/// |                        |         |                        |    -->|                        |
/// |------------------------|         |                        |  --   |                        |
///                                    |------------------------|--     |------------------------|
///         モジュールB            --->|       implement部      |       |      implement部       |
/// |------------------------|--   |   |                        |       |                        |
/// |      interface部       |  -- |   |                        |       |                        |
/// |                        |    --   |------------------------|       |------------------------|
/// |                        |  --
/// |------------------------|--
/// |      implement部       |
/// |                        |
/// |                        |
/// |------------------------|
/// </pre>
/// <BLOCKQUOTE>
/// 上の図において、モジュールCのinterface部にモジュールAがimportされ、モジュールCのimplement部にモジュールBがimportされ、さらにモジュールDのinterface部にモジュールCがimportされているとします。<BR>
/// このとき、Cにとってusableなものは、C自身すべてとAのinterface部のものとBのinterface部のものです。<BR>
/// DからはCのinterface部とAのinterface部のものが見えますが、Bのinterface部のものは見えません。<BR>
/// CのreexportlistにはAが入り、noexportlistにはBが入ります。<BR>
/// Cのprevious module()はD、Aのprevious moduleはC、Bのprevious moduleもDとなります。
/// </BLOCKQUOTE>
/// </p>
/// </desc>
/// <see></see>
/// <body>
class module			// 昔の module_hash
{
public:
	enum new_module_type
          /// そのモジュールの作られ方です。
          { 
		NMT_NONE						= 0, /// ???
		NMT_INCLUDE						= 1, 
		NMT_ENSURE						= 2, 
		NMT_ENSURE_INITIAL				= 3, 
		NMT_ENSURE_IN_PROG				= 4, 
		NMT_CONSULT						= 5, 
		NMT_READ_LILFES					= 6, 
		NMT_READ_LILFES_WITH_PREFIX		= 7, 
		NMT_READ_MODULE					= 8, 
		NMT_INITIALIZE					= 9, 
		NMT_BUILTIN						= 10,
		NMT_ENSURE_WITHOUT_BUILTIN		= 11,
		NMT_ENSURE_WITHOUT_BUILTIN_IN_PROG		= 12,
		NMT_COMMAND						= 13,
		NMT_CREATE_MODULE				= 14
	};

protected:
	std::string name;
  /// そのモジュールの名前です。
	std::string defname;
  /// モジュール定義がない場合のデフォルト名です。
	_HASHMAP<std::string, type *> hash;
  /// 型の名前から対応する型を引くためのハッシュ。
	std::vector<module *> reexportlist;
  /// モジュールの中から再帰的に読み込まれたモジュールのリストです。
  /// このモジュールが import されたときにいっしょにimport されます。
	std::vector<module *> noexportlist;
  /// モジュールの中から再帰的に読み込まれたモジュールのリストです。
  /// このモジュールが import されてもいっしょにimport されません。
	module *prev_module;
  /// 　再帰的に読み込まれたときの、読み込み元（親）のモジュールです。
	bool InterfaceMode;   //It means this module is in interface declaration or not now.
  /// そのモジュールを読み込んでいるときに、interface部を読み込んでいるか、implemente部を読み込んでいるかを表します。前者ならtrue、後者ならfalseになります。
	bool ExtendableMode;
  /// そのモジュールで定義される全ての型について他のモジュールで subtype の定義を許すかどうかを表します。
	new_module_type nmt;
  /// そのモジュールがどのような方法で読み込まれたかということを表す、new_module_type型の変数です。
	module** first_import_modules; // modules which are imported as the builtin
  /// 暗黙のうちにimportされるモジュールのリストです。
	static _HASHMAP<std::string, module *> modulelist_hash;
  /// モジュール名からモジュールを引くためのハッシュです。

	static module *m_user;
  /// 組み込みモジュール。LiLFeSをコマンドラインで使う場合、コマンドラインからの入力をモジュールとして扱います。そのモジュールのことです。
	static module *m_core;
  /// 組み込みモジュール。bottom型などのすべての場所で使われる型を定義しているモジュールです。
	static module *m_builtin;
  /// 組み込みモジュール。組み込み述語を定義しているモジュールです。
	static module *m_directive;
  /// 組み込みモジュール。モジュールの読み込みを制御する型を定義しているモジュールです。
	static module *m_current;
  /// その時点で読み込み中のモジュールです。

	static std::string module_prefix;
  /// 読み込むモジュール名が読み込み元と同じモジュール名とならないように、そのモジュールの頭につける接頭辞です。例えば"__"を付けます。

private:
	static void init();
  /// 開始処理をします。
	static void term();
  /// 終了処理をします。
	static void init2();
  /// 開始処理をします。他の処理の後に実行されます。
	static InitFunction IF;
	static InitFunction IF2;
	static TermFunction TF;
  /// init,init2,termを呼ぶために作ったグローバルオブジェクトです。

public:
	explicit module(const std::string &iname, new_module_type inmt = NMT_NONE, bool Extendable = false);
  /// <var>iname</var>：モジュール名<br>
  /// <var>inmt</var>：<br>
  /// <var>Extendable</var>：extendable(そのモジュールで定義される全ての型について他のモジュールで subtype の定義を許す)かどうか<br>
	~module() { }

	bool IsRegistered() { return !name.empty(); }
  /// そのモジュールがモジュールリストに登録されているかどうかを調べます。
	void Register(const std::string &str = "");
  /// モジュールをモジュールリストに追加します。この時、そのモジュールの名前が決まります。
	const std::string &GetKey() const { return name; }
  /// ハッシュ用のメソッドです。キーとして名前を返します。
	const std::string &GetName() const { return name; }
  /// 名前を取得します。
	const std::string &GetDefName() const { return defname; }
  /// モジュール定義がない場合のデフォルト名を返します。
	void Add(type *t);
  /// 型<var>t</var>をそのモジュールに追加します。
	void Delete(type *t);
  /// 型<var>t</var>をそのモジュールから削除します。
	type *Search(const std::string &str) const
	{
		ASSERT(this != NULL);
		_HASHMAP<std::string, type *>::const_iterator it = hash.find(str);
		return it != hash.end() ? it->second : NULL;
	}
  /// 型の名前が<var>str</var>である型を取得します。
	const feature *SearchFeature(const std::string &str) const;
  /// 型の名前が<var>str</var>である素性を取得します。
	bool IsExporting(const type *t) const;
  /// 型<var>t</var>がexport（上の説明を参照）されているかどうかを調べます。
	bool IsUsable(const type *t) const;
  /// 型<var>t</var>がそのモジュール内でusable（上の説明を参照）かどうか調べます。
	void SetInterfaceMode(bool flag) { InterfaceMode = flag;}
  /// InterfaceMode（そのモジュールを読み込んでいるときに、interface部を読み込んでいるかどうか）を設定します。
	bool GetInterfaceMode(void) const { return InterfaceMode; }
  /// そのモジュールを読み込んでいるときに、interface部を読み込んでいるか、implement部を読み込んでいるかを調べます。前者ならtrue、後者ならfalseを返します。
	void SetExtendableMode(bool flag) { ExtendableMode = flag;}
  /// ExtendableMode（そのモジュールで定義される全ての型について他のモジュールで subtype の定義を許すかどうか）を設定します。
	bool GetExtendableMode(void) const { return ExtendableMode; }
  ///  ExtendableMode（そのモジュールで定義される全ての型について他のモジュールで subtype の定義を許すかどうか）かどうかを調べます。
	void SetFirstImport(module** imports);
  /// モジュール<var>imports</var>をfirst_import_modulesにします。
	module** ExportList(void) const;
  /// reexportlistにあるモジュールを取得します。
	module** NoExportList(void) const;
  /// noexportlistにあるモジュールを取得します。
	new_module_type GetModuleType() { return nmt; }
  /// そのモジュールのタイプを取得します。
	void SetPrevModule(module *mod) { prev_module = mod; }
  /// モジュール<var>mod</var>をそのモジュールの読み込み元(previous module)に設定します。
	module * GetPrevModule() { return prev_module; }
  /// 再帰的に読み込まれたときの、読み込み元（親）のモジュールを取得します。

	static void SetModulePrefix(const std::string &prefixname) { module_prefix = prefixname; }
  /// std::string<var>prefixname</var>をmodule_prefixにします。
	static const std::string &GetModulePrefix(void) { return module_prefix; } 
  /// module_prefixを取得します。
	void Import(module *mod, bool reexport);
  /// そのモジュールに、モジュール<var>mod</var>をimportします。<var>reexport</var>がtrueならそのモジュールの読み込み元から見えます。falseなら見えません。
	void Import(module *mod) { Import(mod, GetInterfaceMode()); }
	void Finish();
  /// そのモジュールの読み込みが終わったときに、モジュール読み込みエンジンに呼ばれます。<BR>
  /// そのモジュールの中で定義された型を呼び込み元にimportします。

	static void SetCurrentModule(module *mod);
  /// モジュール<var>mod</var>を読み込み中のモジュールに設定します。
	static module *SearchModule(const std::string &name)
	{
		_HASHMAP<std::string, module *>::const_iterator it = modulelist_hash.find(module_prefix + name);
		if (it != modulelist_hash.end())
		{
			return it->second;
		}
		_HASHMAP<std::string, module *>::const_iterator it2 = modulelist_hash.find(name);
		return it2 != modulelist_hash.end() ? it->second : NULL;
	}
  /// 名前から対応するモジュールを取得します。
	static module *CoreModule() { return m_core; }
  /// m_coreモジュールを取得します。
	static module *BuiltinModule() { return m_builtin; }
  /// m_builtinモジュールを取得します。
	static module *UserModule() { return m_user; }
  /// m_userモジュールを取得します。
	static module *DirectiveModule() { return m_directive; }
  /// m_directiveモジュールを取得します。
	static module *CurrentModule() { return m_current; }
  /// 読み込み中のモジュールを取得します。
	static void UseCurrentModule();
  /// そのモジュールが宣言されているかどうかを調べて、宣言されていなければエラーを出して、defnameで登録します。
	static module* new_module(machine &m, const std::string &biname, const std::string &modname, new_module_type nmt, module *prev = NULL, module** importlist = NULL);
  /// 新しいモジュールを作ります。すでに同じモジュールが存在すればそのモジュールを返し、無ければ新しく作成します。
  /// <var>m</var>：抽象機械<br>
  /// <var>biname</var>：エラーが起きたときにどこでエラーが起きたかを表します。<br>
  /// <var>modname</var>：仮の名前。本当の名前はファイルの中のモジュール名の宣言部を読み込んだ時に決まります。<br>
  /// <var>nmt</var>：モジュールの作られ方。<br>
  /// <var>prev</var>：そのモジュールの読み込み元のモジュール<br>
  /// <var>importlist</var>：importするモジュール<br>
  
//Added by kaityo for debug

	void ShowList(void);
  /// そのモジュールに含まれる型の一覧を表示します。
};
/// </body></classdef>


} // namespace lilfes


#endif // __module_h
