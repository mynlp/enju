/* $Id: parse.h,v 1.5 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: parse.h,v 1.5 2011-05-02 08:48:59 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>parse.h</name>
//
///  <overview>
///  <jpn>構文解析ルーチン</jpn>
///  <eng>parse routine </eng>
///  </overview>


/// <notiondef>
///
/// <name> parse_overview </name>
/// <title> parse </title>
/// <desc>
///  parse.hは入力されたLiLFeSプログラムを構文解析し、抽象機械が処理できるようなコードへと翻訳するためのプログラム群です。
/// </desc>
/// <see>  </see>
/// </notiondef>

#ifndef __parse_h
#define __parse_h

#include <string>
#include <vector>

#include "lconfig.h"
#include "structur.h"
#include "codelist.h"

// Definition of flags.

namespace lilfes {

const unsigned int PF_INHERITMASK	    = 0x00ff;
const unsigned int PF_NEED_DEALLOC      = 0x0001;
const unsigned int PF_NEED_EQUAL_CLRTEMP= 0x0002;
const unsigned int PF_LASTDESC		 	= 0x0100;
const unsigned int PF_LASTCALL		 	= 0x0200;
const unsigned int PF_LASTPROC		 	= 0x0200;
const unsigned int PF_INDISJ		 	= 0x0400;
const unsigned int PF_PUSH_COMBINED 	= 0x0800;
const unsigned int PF_FIRSTARG      	= 0x1000;
const unsigned int PF_INCONSTR      	= 0x2000;

extern const char * const clause_cut_label;

class machine;
class FSP;

//////////////////////////////////////////////////////////////////////////////
//
// class ptree .. parse-tree class
//

struct parseinfo;

/// <classdef>
/// <name> ptree </name>
/// <overview> parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてを構文木を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class ptree
{
public:
			ptree() { }
	///constructor
	virtual ~ptree() { }
	///destructor

	virtual void CollectInfo(int, parseinfo *) { }
	///中間コード生成のための情報収集を行います。
	///<var>flg</var> 
	///<var>info</var> 
	virtual codelist *Compile(int, parseinfo *) = 0;
	///中間コードを生成します。
	///<var>flg</var> 
	///<var>info</var> 
	virtual const char *GetClassName() { return className; }
	///ClassNameを返します。
	
	virtual outform OutputTree() = 0;
	///構文木をAVM木の形式で返します。
	
	static const char *className;
	///ClassName(クラスの名前)を保持する変数です。
};

/// <classdef>
/// <name> pempty </name>
/// <overview> empty-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてを空の構文木を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pempty : public ptree
{
public:
			pempty() : ptree() { }
	virtual ~pempty() { }
	
	virtual codelist *Compile(int, parseinfo *) { return new codelist; }
	virtual const char *GetClassName() { return className; }

	virtual outform OutputTree() { return outform("<empty>"); }
	
	static const char *className;
};

/// <classdef>
/// <name> pcut </name>
/// <overview> cut-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pcut : public ptree
{
	std::string label;
	/// 
public:
			pcut(const char *ilabel = clause_cut_label) : ptree(), label(ilabel) {}
	virtual ~pcut() { }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual const char *GetClassName() { return className; }
	virtual outform OutputTree();
	
	static const char *className;
};

/// <classdef>
/// <name> pfeature </name>
/// <overview> feature-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて素性構造の構文木を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pfeature : public ptree
{
	const feature *feat;
	///対象となる素性
	ptree *desc;
	///description tree
public:
			pfeature(const feature *f, ptree *d) : ptree() { feat = f; desc = d; }
	virtual ~pfeature() { delete desc; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual const char *GetClassName() { return className; }
	virtual outform OutputTree();
	
	static const char *className;
};

/// <classdef>
/// <name> ptype </name>
/// <overview> type-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて型の構文木を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class ptype : public ptree
{
	const type *newtype;
	///
public:
			ptype(const type *t) : ptree() { newtype = t; }
	virtual ~ptype() { }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pvariable </name>
/// <overview> variable-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pvariable : public ptree
{
	std::string varname;
	///variable name(inner)
public:
	pvariable(const char *str) : ptree(), varname(str) { }
	virtual ~pvariable() { }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};


/// <classdef>
/// <name> pconj </name>
/// <overview> conjuctive-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてptreeを連結するクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pconj : public ptree
{
	ptree *desc1;
	///対象となるptree
	ptree *desc2;
	///対象となるptree
public:
			pconj(ptree *d1, ptree *d2) : ptree() { desc1 = d1; desc2 = d2; }
	virtual ~pconj() { delete desc1; delete desc2; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pdisj </name>
/// <overview> disjunctive-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてptreeを分離するクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pdisj : public ptree
{
	ptree *desc1;
	///対象となるptree
	ptree *desc2;
	///対象となるptree
public:
			pdisj(ptree *d1, ptree *d2) : ptree() { desc1 = d1; desc2 = d2; }
	virtual ~pdisj() { delete desc1; delete desc2; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pequal </name>
/// <overview> equal-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて"="を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pequal : public ptree
{
	ptree *desc1;
	/// 対象となるptree
	ptree *desc2;
	/// 対象となるptree
public:
			pequal(ptree *d1, ptree *d2) : ptree() { desc1 = d1; desc2 = d2; }
	virtual ~pequal() { delete desc1; delete desc2; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};


/// <classdef>
/// <name> pcallarg </name>
/// <overview> call-argument-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pcallarg : public ptree
{
	ptree *rest;
	ptree *desc;
	
public:
			pcallarg(ptree *r, ptree *d) : ptree() { desc = d; rest = r; }
	virtual ~pcallarg() { delete desc; delete rest; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pcall </name>
/// <overview> call-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pcall : public ptree
{
	const type *reptype;
	ptree *arg;
	
public:
			pcall(const type *t, ptree *a) : ptree(), reptype(t) { arg = a; }
	virtual ~pcall() { delete arg; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pgoaldesc </name>
/// <overview> goal-description-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pgoaldesc : public ptree
{
	ptree *desc;
	
public:
			pgoaldesc(ptree *d) : ptree() { desc = d; }
	virtual ~pgoaldesc() { delete desc; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pgoalconj </name>
/// <overview> goal-conjunctive-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pgoalconj : public ptree
{
	ptree *goal1;
	ptree *goal2;
	
public:
			pgoalconj(ptree *g1, ptree *g2) : ptree() { goal1 = g1; goal2 = g2; }
	virtual ~pgoalconj() { delete goal1; delete goal2; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pgoaldisj </name>
/// <overview> goal-disjunctive-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pgoaldisj : public ptree
{
	ptree *goal1;
	ptree *goal2;
	
public:
			pgoaldisj(ptree *g1, ptree *g2) : ptree() { goal1 = g1; goal2 = g2; }
	virtual ~pgoaldisj() { delete goal1; delete goal2; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pdefarg </name>
/// <overview> definition-argument-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pdefarg : public ptree
{
	ptree *rest;
	/// 今現在のargument
	ptree *desc;
	/// 追加するargument
	
public:
			pdefarg(ptree *r, ptree *d) : ptree() { desc = d; rest = r; }
	virtual ~pdefarg() { delete desc; delete rest; }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;

	ptree *GetRest() { return rest; }
	///restを返します。
	ptree *GetDesc() { return desc; }
	///descを返します。

	int NArg() { if( rest->GetClassName() == pempty::className ) return 0; 
	             else return ((pdefarg *)rest)->NArg()+1; }
	///restのclassnameがpemptyであれば0を、そうでなければrest
};

/// <classdef>
/// <name> pprocdef </name>
/// <overview> procedure-definition-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて述語の...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pprocdef : public ptree
{
	const type *reptype;
	///type
	ptree *defarg;
	///argument
	
public:
			pprocdef(const type *t, ptree *d) : ptree(), reptype(t) { defarg = d; }
	virtual ~pprocdef() { delete defarg; }

	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;

	const type *GetRepType() { return reptype; }
	///reptypeを返します。
	ptree *GetDefarg() { return defarg; }
	///defargを返します。
};

/// <classdef>
/// <name> pclause </name>
/// <overview> clause-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pclause : public ptree
{
protected:
	pprocdef *procdef;
	///definition tree
	ptree *goals;
	///goal tree
	
public:
			pclause(pprocdef *p, ptree *g) : ptree() { procdef = p; goals = g; }
	virtual ~pclause() { delete procdef; delete goals; }
	void ResolveVariable(parseinfo *);
	///
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;

	const type *GetRepType() { ASSERT(procdef != NULL); return procdef->GetRepType(); }
	///return reptype of definition tree 
};

/// <classdef>
/// <name> pquery </name>
/// <overview> query-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pquery : public ptree
{
	ptree *goals;
	int nvars;
	struct _varlist {
		std::string name;
		int varno;
	} *varlist;
	bool display;
	
public:
			pquery(ptree *g, bool id = true) : ptree() { goals = g; varlist = NULL; display = id; }
	virtual ~pquery() { delete goals; delete[] varlist; }
	void ResolveVariable(parseinfo *);
	bool GetDisplayFlag() { return display; }
	bool DisplayResult(machine &m);
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pcutregion </name>
/// <overview> cut-region-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pcutregion : public ptree
{
	std::string label;
	///
	ptree *goals;
	///
	
public:
			pcutregion(const char *ilabel, ptree *g) : ptree(), label(ilabel), goals(g) { }
	virtual ~pcutregion() { delete goals; }

	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pinterpret </name>
/// <overview> interpret-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pinterpret : public ptree
{
	FSP fs;
public:
		pinterpret(FSP &ifs) : ptree(), fs(ifs) { }

	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pcalc </name>
/// <overview> calc-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pcalc : public ptree
{
	ptree *p1;
	ptree *result;
	FSP p2;
	const type *op;

public:
			pcalc(ptree *ip1, const type *iop, FSP ip2, ptree *iresult) : ptree() { p1 = ip1; op = iop; p2 = ip2; result = iresult; }
	virtual ~pcalc() { }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pcalcrev </name>
/// <overview> calc-???-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pcalcrev : public ptree
{
	ptree *p1;
	///
	ptree *result;
	FSP p2;
	const type *op;

public:
			pcalcrev(ptree *ip1, const type *iop, FSP ip2, ptree *iresult) : ptree() { p1 = ip1; op = iop; p2 = ip2; result = iresult; }
	virtual ~pcalcrev() { }
	
	virtual void CollectInfo(int flg, parseinfo *info);
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pint </name>
/// <overview> integer-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて整数の構文木（の枝）を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pint : public ptree
{
	mint val;
public:
			pint(mint iv) : ptree() { val = iv; }
	virtual ~pint() { }
	
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pfloat </name>
/// <overview> float-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて実数の構文木（の枝）を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pfloat : public ptree
{
	mfloat val;
public:
			pfloat(mfloat iv) : ptree() { val = iv; }
	virtual ~pfloat() { }
	
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pchar </name>
/// <overview> char-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて文字の構文木（の枝）を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pchar : public ptree
{
	mchar val;
public:
			pchar(mchar iv) : ptree() { val = iv; }
	virtual ~pchar() { }
	
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

/// <classdef>
/// <name> pstring </name>
/// <overview> string-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて文字列の構文木（の枝）を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pstring : public ptree
{
	std::string val;
public:
	pstring(const char *iv) : ptree(), val(iv) { }
	virtual ~pstring() { }
	
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};

#ifdef XCONSTR
/// <classdef>
/// <name> pconstraint </name>
/// <overview> constraint-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pconstraint : public pclause
{
	
public:
			pconstraint(pprocdef *p, ptree *g) : pclause(p, g) { }
	
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }

	static const char *className;
};

/// <classdef>
/// <name> pfeatconstr </name>
/// <overview> feature-constraint-parse-tree クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいて...を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pfeatconstr : public ptree
{
	const type *constrtype;
public:
			pfeatconstr(const type *t) : ptree() { constrtype = t; }
	virtual ~pfeatconstr() { }
	
	virtual codelist *Compile(int flag, parseinfo *info);
	virtual outform OutputTree();
	virtual const char *GetClassName() { return className; }
	
	static const char *className;
};
#endif //XCONSTR


//////////////////////////////////////////////////////////////////////////////
//
// class parseinfo
//

enum VarState { PV_FIRST, PV_SECOND, PV_PROBABLE };
/// 

/// <classdef>
/// <name> pinfo_var </name>
/// <overview> parse-variable-infomation クラス</overview>
/// <desc>
/// <p>
/// 変数の構文木の情報を保持するためのクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
class pinfo_var
{
	std::string name;
	///
	VarState state;
	///
	int firstuse, lastuse, usetime;
	///
	varno vn;
	///
public:
	pinfo_var(const std::string in, int use, VarState st = PV_FIRST ) : name(in) { state = st; firstuse = lastuse = use; usetime = 1; }
	const std::string GetKey() { return name; } // for hash...
	const char *GetName() { return name.c_str(); }
	VarState GetState() { return state; }
	void SetState(VarState s) { state = s; }
	int FirstUse() { return firstuse; }
	int LastUse() { return lastuse; }
	void NewUse(int use) { if( firstuse > use ) firstuse = use; if( lastuse < use ) lastuse = use; usetime++; }
	int Usetime() { return usetime; }
	void SetVarNo(varno i) { vn = i; }
	varno VarNo() { return vn; }
};

/// <classdef>
/// <name> pinfo </name>
/// <overview> parse-call-infomation クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてを..構文解析情報を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
struct pinfo_call
{
	int callargs;
	int nperm;
	int ntemp;
};

typedef _HASHMAP<std::string, pinfo_var *> PInfoMap;
/// <classdef>
/// <name> pinfo </name>
/// <overview> parse-infomation クラス</overview>
/// <desc>
/// <p>
/// LiLFeSにおいてを構文解析情報を扱うクラスです。
/// </p>
/// </desc>
/// <see></see>
/// <body>
struct parseinfo
{
	machine *mach;
	
	PInfoMap hash;
	///
	PInfoMap hashcut;
	///
	int ntemp;
	///
	int maxntemp;
	///
	int maxnperm;
	///
	char disjlabel[30];
	/// disjunctive name
	varno ncallarg;	// use varno...
	///
	VarState ha_state;
	///
// parameters used during collection
	int ncall;
	///
	int nnewchoice;
	///
	int ndefarg;
	///
	std::vector<pinfo_call> calls;
	/// 
	const feature *feat;
	/// change in pfeature

	const type *toparg;
	/// 
	int neednewchoice;
	///change in pvaliable&ptype
	sserial toparg_stg;
	///
	mint	toparg_int;
	///
	bool lastcall;
	///
	bool nonlastcallexist;
	///
	bool is_toparg;		// toparg に文字列や整数の instance があるか
	/// 

// parameters used in tracing

	bool err;
	///
	
	parseinfo(machine *imach) {
		mach = imach; 
		ntemp = 0; 
		*disjlabel = '\0'; 
		ncallarg = 0; 
		lastcall = false; 
		ncall=0; 
		nnewchoice=0;
		ha_state = PV_FIRST; 
		toparg = NULL;
		err = false; 
		is_toparg = false;
	}

	~parseinfo() { }
};

} // namespace lilfes

#endif // __parse_h

