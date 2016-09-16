/* $Id: proc.cpp,v 1.11 2011-05-02 10:38:24 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

///  <ID>$Id: proc.cpp,v 1.11 2011-05-02 10:38:24 matuzaki Exp $</ID>
//
///  <COLLECTION> lilfes-bytecode </COLLECTION>
//
///  <name>proc.cpp</name>
//
///  <overview>
///  <jpn>述語を扱うためのルーチン</jpn>
///  <eng>??? </eng>
///  </overview>
/// <see>proc.h</see>
#include "proc.h"

#include "machine.h"
#include "builtin.h"
#include "codelist.h"

#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::make_pair;
using std::map;
using std::ostringstream;
using std::vector;


static const char rcsid[] = "$Id: proc.cpp,v 1.11 2011-05-02 10:38:24 matuzaki Exp $";

#define SSERIAL_INVALID (-1)
//#define MINT_INVALID (-134217719)
static const mint MINT_INVALID = ( static_cast<mint>(-1) << TAG_SHIFT );  // outside of "mint" range

//////////////////////////////////////////////////////////////////////////////
//
// proc thunks
//

////////////////////////////////////////////////////////////////////////////
// Selection Thunk
/// 情報収集をするthunk
/// <classdef>
/// <name> proc_selectthunk </name>
/// <overview> 情報収集をするthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_selectthunk : public procthunk
{
	procedure *p;
public:
	proc_selectthunk(procedure *ip, uint ar);
	virtual ~proc_selectthunk();
	virtual void execwork(machine &m);

	static const char *className;
  /// このクラスの名前です。つまり"proc_selectthunk"が入ります。
	virtual const char *ClassName() { return className; }
  /// クラス名を取得します。
};
/// </body></classdef>

proc_selectthunk::proc_selectthunk(procedure *ip, uint ar)
	: procthunk(ar)
{
	p = ip;
}

proc_selectthunk::~proc_selectthunk()
{
	// do nothing
}

void proc_selectthunk::execwork(machine &m)
{
	P1("Compile during run");
	procedure *pp = p;

	p->Compile();	// Note that Compile() calls SetThunk(), which deletes 
					// the current thunk. Thus it has the same effect of
					// delete this;  so I copy the member p to the stack.
	pp->GetThunk()->execwork(m);
}

////////////////////////////////////////////////////////////////////////////
// Simple Thunk
/// <classdef>
/// <name> proc_simplethunk </name>
/// <overview> コードを実行するだけのthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_simplethunk : public procthunk
{
	code *cl;
public:
	proc_simplethunk(code *icl, uint ar);
	virtual ~proc_simplethunk();
	virtual void execwork(machine &m);

	virtual bool NeedNewTypeRevise() { return true; }

	static const char *className;
  /// このクラスの名前です。つまり"proc_simplethunk"が入ります。
	virtual const char *ClassName() { return className; }
  /// クラス名を取得します。
};
/// </body></classdef>

proc_simplethunk::proc_simplethunk(code *icl, uint ar)
	: procthunk(ar)
{
	cl = icl;
}

proc_simplethunk::~proc_simplethunk()
{
	// do nothing
}

void proc_simplethunk::execwork(machine &m)
{
	P5("proc_simplethunk");
	ASSERT(cl != NULL); 

	m.SetLevel();		// May be avoidable ... 
	m.DefaultCall(cl, arity);
}

////////////////////////////////////////////////////////////////////////////
// Fail Thunk
/// <classdef>
/// <name> proc_failthunk </name>
/// <overview> 必ず失敗することがわかっているときに割り当てられるthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_failthunk : public procthunk
{
public:
	proc_failthunk(uint ar);
	virtual ~proc_failthunk();
	virtual void execwork(machine &m);

	static const char *className;
  /// このクラスの名前です。つまり"proc_failthunk"が入ります。
	virtual const char *ClassName() { return className; }
  /// クラス名を取得します。
};
/// </body></classdef>

proc_failthunk::proc_failthunk(uint ar)
	: procthunk(ar)
{
	// do nothing
}

proc_failthunk::~proc_failthunk()
{
	// do nothing
}

void proc_failthunk::execwork(machine &m)
{
	P5("proc_failthunk");
	m.Fail();
}

////////////////////////////////////////////////////////////////////////////
// True Thunk
/// <classdef>
/// <name> proc_truethunk </name>
/// <overview> 必ず成功することがわかっているときに割り当てられるthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_truethunk : public procthunk
{
public:
	proc_truethunk(uint ar);
	virtual ~proc_truethunk();
	virtual void execwork(machine &m);

	static const char *className;
  /// このクラスの名前です。つまり"proc_truethunk"が入ります。
	virtual const char *ClassName() { return className; }
  /// クラス名を取得します。
};
/// </body></classdef>

proc_truethunk::proc_truethunk(uint ar)
	: procthunk(ar)
{
	// do nothing
}

proc_truethunk::~proc_truethunk()
{
	// do nothing
}

void proc_truethunk::execwork(machine &)
{
	P5("proc_truethunk");
}

////////////////////////////////////////////////////////////////////////////
// Binary Thunk
/// <classdef>
/// <name> proc_binarythunk </name>
/// <overview> 第１引数の型によって実行するか必ず失敗するかのどちらかになるthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_binarythunk : public procthunk
{
	code *cl;
	Bitmap typemap;
public:
	proc_binarythunk(code *icl, Bitmap &typemap, uint ar);
	virtual ~proc_binarythunk();
	virtual void execwork(machine &m);

	virtual bool NeedNewTypeRevise() { return true; }

	static const char *className;
  /// このクラスの名前です。つまり"proc_binarythunk"が入ります。
	virtual const char *ClassName() { return className; }
    /// クラス名を取得します。
};
/// </body></classdef>

proc_binarythunk::proc_binarythunk(code *icl, Bitmap &it, uint ar)
	: procthunk(ar), typemap(it)
{
	cl = icl;
}

proc_binarythunk::~proc_binarythunk()
{
	// do nothing
}

void proc_binarythunk::execwork(machine &m)
{
	P5("proc_binarythunk");
#ifdef DOASSERT
	m.SetRP(1);
#endif
	cell c = m.DerefCell(m.ReadLocal(0));		// First Argument

	tserial ts;
	
	if( IsSTRorVAR(c) )
	{
		ts = c2STRSorVARS(c);
	}
	else
	switch( Tag(c) )
	{
	case T_INT:
		ts = t_int->GetSerialNo();	break;
	case T_FLO:
		ts = t_flo->GetSerialNo();	break;
	case T_CHR:
		ts = t_chr->GetSerialNo();	break;
	case T_STG:
		ts = t_stg->GetSerialNo();	break;
//	case T_STR:
//	case T_VAR:
//		ts = c2STRSorVARS(c);	break;
	default:
		ABORT("IT IS IMPOSSIBLE");
	}
	if( typemap.Test( ts ) )
	{
		m.SetLevel();		// May be avoidable ... 
		m.DefaultCall(cl, arity);
	}
	else
		m.Fail();
}

////////////////////////////////////////////////////////////////////////////
// Multiple Thunk
/// <classdef>
/// <name> proc_multithunk </name>
/// <overview> 第１引数の型によって別のコードが実行されるthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_multithunk : public procthunk
{
	code **cll;

public:
	proc_multithunk(code **icll, uint ar);
	virtual ~proc_multithunk();
	virtual void execwork(machine &m);

	virtual bool NeedNewTypeRevise() { return true; }

	static const char *className;
  /// このクラスの名前です。つまり"proc_multithunk"が入ります。
	virtual const char *ClassName() { return className; }
  /// クラス名を取得します。
};
/// </body></classdef>

proc_multithunk::proc_multithunk(code **icll, uint ar)
	: procthunk(ar)
{
	cll = icll;
}

proc_multithunk::~proc_multithunk()
{
	delete[] cll;
}

void proc_multithunk::execwork(machine &m)
{
	P5("proc_multithunk");
#ifdef DOASSERT
	m.SetRP(1);
#endif
	cell c = m.DerefCell(m.ReadLocal(0));		// First Argument
	
	m.SetLevel();		// May be avoidable ... 
	switch( Tag(c) )
	{
	case T_INT:
		m.DefaultCall(cll[t_int->GetSerialNo()], arity);	break;
	case T_FLO:
		m.DefaultCall(cll[t_flo->GetSerialNo()], arity);	break;
	case T_CHR:
		m.DefaultCall(cll[t_chr->GetSerialNo()], arity);	break;
	case T_STG:
		m.DefaultCall(cll[t_stg->GetSerialNo()], arity);	break;
	case T_STR:
	case T_VAR:
		m.DefaultCall(cll[c2STRSorVARS(c)], arity);		break;
	default:
		ABORT("IT IS IMPOSSIBLE");
	}
}

////////////////////////////////////////////////////////////////////////////
// Full Thunk
/// <classdef>
/// <name> proc_fullthunk </name>
/// <overview> 第１引数の型と値によって別のコードが実行されるthunkクラス</overview>
/// <desc>
/// <p>
/// 
/// 
/// </p>
/// </desc>
/// <see></see>
/// <body>
class proc_fullthunk : public procthunk
{
	code **cll;
	_HASHMAP<sserial, code *> shash;
	_HASHMAP<mint, code *> ihash;

public:
	proc_fullthunk(code **cll, _HASHMAP<sserial, code *> &ishash, _HASHMAP<mint, code *> &iihash, uint ar);
	virtual ~proc_fullthunk();
	virtual void execwork(machine &m);

	virtual bool NeedNewTypeRevise() { return true; }

	static const char *className;
  /// このクラスの名前です。つまり"proc_fullthunk"が入ります。
	virtual const char *ClassName() { return className; }
  /// クラス名を取得します。
};

proc_fullthunk::proc_fullthunk(code **icll, _HASHMAP<sserial, code *> &ishash, _HASHMAP<mint, code *> &iihash, uint ar)
	: procthunk(ar)
{
	P1("proc_fullthunk constructor");

	cll = icll;

	shash = ishash;
	ihash = iihash;
}
/// </body></classdef>

proc_fullthunk::~proc_fullthunk()
{
	delete[] cll;
}

void proc_fullthunk::execwork(machine &m)
{
	P5("proc_fullthunk");
#ifdef DOASSERT
	m.SetRP(1);
#endif
	cell c = m.DerefCell(m.ReadLocal(0));		// First Argument
	
	m.SetLevel();		// May be avoidable ... 
	switch( Tag(c) )
	{
	case T_INT:
		{
		_HASHMAP<mint, code *>::const_iterator it = ihash.find(c2INT(c));
		if( it != ihash.end() )
		{
			m.DefaultCall(it->second, arity);
		}
		else
		{
			m.DefaultCall(ihash[MINT_INVALID], arity);
		}
		break;
		}
	case T_FLO:
		m.DefaultCall(cll[t_flo->GetSerialNo()], arity);	break;
	case T_CHR:
		m.DefaultCall(cll[t_chr->GetSerialNo()], arity);	break;
	case T_STG:
		{
		_HASHMAP<sserial, code *>::const_iterator it = shash.find(c2STGS(c));
		if( it != shash.end() )
		{
			m.DefaultCall(it->second, arity);
		}
		else {
			m.DefaultCall(shash[(sserial)SSERIAL_INVALID], arity);
		}
		break;
		}
	case T_STR:
	case T_VAR:
		m.DefaultCall(cll[c2STRSorVARS(c)], arity);		break;
	default:
		ABORT("IT IS IMPOSSIBLE");
	}
}

const char * procthunk::className		 = "procthunk";
const char * proc_selectthunk::className = "proc_selectthunk";
const char * proc_simplethunk::className = "proc_simplethunk";
const char * proc_binarythunk::className = "proc_binarythunk";
const char * proc_multithunk::className	 = "proc_multithunk";
const char * proc_failthunk::className	 = "proc_failthunk";
const char * proc_truethunk::className	 = "proc_truethunk";
const char * proc_fullthunk::className	 = "proc_fullthunk";

////////////////////////////////////////////////////////////////////////////
// Procedure Fail definition

procedure *fail;
code *failcode;

static void init() 
{ 
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
	type *t = new type("fail", module::CoreModule()); t->SetAsChildOf(t_pred[0]); t->Fix();
	fail = new proc_builtin(t, 0, new proc_failthunk(0)); 

	codelist fails;
	fails.AddCode(C_FAIL);
	fails.EndCode();
	failcode = fails;

	t = new type("true", module::CoreModule()); t->SetAsChildOf(t_pred[0]); t->Fix();
	new proc_builtin(t, 0, new proc_truethunk(0)); 
}

static void term() 
{
	procedure::terminate();
//	delete fail; 
}
static InitFunction IF(init, 80);
static TermFunction TF(term, 80);

vector<procpair> procedure::serialtable;
_HASHMAP<tserial, procedure *> procedure::hash;

procedure::procedure(const type *inireptype, uint ia) 
{ 
	reptype = inireptype;
	hash[this->GetKey()] = this;
	arity = ia;
	serialno = serialtable.size();
	serialtable.push_back(procpair());
	procpair *pp = &serialtable.back();
	pp->pc = this;
	pp->pt = new proc_selectthunk(this, arity);
	trace = false;
	special = false;
	multifile = false;
	dynamic = false;
}

procedure::~procedure() 
{ 
//	delete cl;
}

procedure *procedure::SearchByThunk(procthunk *p)
{
	P1("procedure::SearchByThunk");
	for( uint i=0; i<serialtable.size(); i++ )
	{
		if( serialtable[i].pt == p )
			return serialtable[i].pc;
	}
	return NULL;
}

void procedure::AddCodelist(code *c, const type *toptype /*= NULL*/) 
{ 
	if( special )
	{
		RUNERR( "cannot assert predicate '" << GetName() << "'" );
		return;
	}
#ifdef DEBUG
	if( DEB )
		cout << "procedure::AddCodelist " << GetName() << ", addr = " << hex << reinterpret_cast<ptr_int>(c) << dec << endl;
#endif
	cls.push_back(proc_t());
	proc_t &p = cls.back();
	p.cl = c; 
	p.tt = toptype; 
	p.instant = false;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::AddCodelistSTG(code *c, const type *toptype, sserial ss) 
{ 
	if( special )
	{
		RUNERR( "cannot assert predicate '" << GetName() << "'" );
		return;
	}
#ifdef DEBUG
	if( DEB )
		cout << "procedure::AddCodelistSTG " << GetName() << ", addr = " << hex << reinterpret_cast<ptr_int>(c) << dec << endl;
#endif
	cls.push_back(proc_t());
	proc_t &p = cls.back();
	p.cl = c; 
	p.tt = toptype; 
	p.instant = true;
	p.ts = ss;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::AddCodelistINT(code *c, const type *toptype, mint i) 
{ 
	if( special )
	{
		RUNERR( "cannot assert predicate '" << GetName() << "'" );
		return;
	}
#ifdef DEBUG
	if( DEB )
		cout << "procedure::AddCodelistINT " << GetName() << ", addr = " << hex << reinterpret_cast<ptr_int>(c) << dec << endl;
#endif
	cls.push_back(proc_t());
	proc_t &p = cls.back(); 
	p.cl = c; 
	p.tt = toptype; 
	p.instant = true;
	p.ti = i;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::AddCodelistTop(code *c, const type *toptype /*= NULL*/) 
{ 
	if( special )
	{
		RUNERR( "cannot assert predicate '" << GetName() << "'" );
		return;
	}
#ifdef DEBUG
	if( DEB )
		cout << "procedure::AddCodelist " << GetName() << ", addr = " << hex << reinterpret_cast<ptr_int>(c) << dec << endl;
#endif
	uint s = cls.size();
	cls.push_back(proc_t());
	for( uint j=s; j>0; j-- )
		cls[j] = cls[j-1];
	
	proc_t &p = cls[0]; 
	p.cl = c; 
	p.tt = toptype; 
	p.instant = false;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::AddCodelistTopSTG(code *c, const type *toptype, sserial ss) 
{ 
	if( special )
	{
		RUNERR( "cannot assert predicate '" << GetName() << "'" );
		return;
	}
#ifdef DEBUG
	if( DEB )
		cout << "procedure::AddCodelistSTG " << GetName() << ", addr = " << hex << reinterpret_cast<ptr_int>(c) << dec << endl;
#endif
	uint s = cls.size();
	cls.push_back(proc_t());
	for( uint j=s; j>0; j-- )
		cls[j] = cls[j-1];
	
	proc_t &p = cls[0]; 
	p.cl = c;
	p.tt = toptype; 
	p.instant = true;
	p.ts = ss;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::AddCodelistTopINT(code *c, const type *toptype, mint i) 
{ 
	if( special )
	{
		RUNERR( "cannot assert predicate '" << GetName() << "'" );
		return;
	}
#ifdef DEBUG
	if( DEB )
		cout << "procedure::AddCodelistINT " << GetName() << ", addr = " << hex << reinterpret_cast<ptr_int>(c) << dec << endl;
#endif
	uint s = cls.size();
	cls.push_back(proc_t());
	for( uint j=s; j>0; j-- )
		cls[j] = cls[j-1];
	
	proc_t &p = cls[0]; 
	p.cl = c; 
	p.tt = toptype; 
	p.instant = true;
	p.ti = i;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::SetTrace(bool tr)
{
	if( special )
	{
		RUNERR( "cannot trace predicate '" << GetName() << "'" );
		return;
	}
	trace = tr;
	SetThunk(new proc_selectthunk(this, arity));
}

void procedure::SetSpecialThunk(procthunk *p) 
{ 
	special = true; SetThunk(p); 
}

void procedure::ClearSpecialThunk() 
{ 
	special = false; SetThunk(new proc_selectthunk(this, arity)); 
}


proc_builtin::proc_builtin(const type *ininame, uint ia, procthunk *pt)
	: procedure(ininame, ia)
{
	SetThunk(pt);
}

void proc_builtin::AddCodelist(code *, const type * /*toptype*/ /*= NULL*/) 
{ 
	RUNERR("Built-in procedure '" << GetName() << "' cannot be redefined");
}
void proc_builtin::AddCodelistINT(code *, const type * /*toptype*/ /*= NULL*/, mint) 
{ 
	RUNERR("Built-in procedure '" << GetName() << "' cannot be redefined");
}
void proc_builtin::AddCodelistSTG(code *, const type *, sserial )
{ 
	RUNERR("Built-in procedure '" << GetName() << "' cannot be redefined");
}
void proc_builtin::AddCodelistTop(code *, const type * /*toptype*/ /*= NULL*/) 
{ 
	RUNERR("Built-in procedure '" << GetName() << "' cannot be redefined");
}
void proc_builtin::AddCodelistTopINT(code *, const type * /*toptype*/ /*= NULL*/, mint) 
{ 
	RUNERR("Built-in procedure '" << GetName() << "' cannot be redefined");
}
void proc_builtin::AddCodelistTopSTG(code *, const type *, sserial ) 
{ 
	RUNERR("Built-in procedure '" << GetName() << "' cannot be redefined");
}

void proc_builtin::SetTrace(bool /*tr*/)
{
	RUNERR("Built-in procedure '" << GetName() << "' cannot be traced");
}

//////////////////////////////////////////////////////////////////////////////

void procedure::SetThunk(procthunk *pt)
{
	delete serialtable[serialno].pt;
	serialtable[serialno].pt = pt;
}

void procedure::Compile()
{
	uint i,j;
	_HASHMAP<sserial, Bitmap *> pshash;
	_HASHMAP<mint, Bitmap *> pihash;
	bool instant = false;

#ifdef DEBUG
	if( DEB )
		cout << "Compiling " << GetName() << "  clauses = " << cls.GetSize();
#endif
//	cout << reinterpret_cast<ptr_int>(execwork) << endl;
//	cout << reinterpret_cast<ptr_int>(&_defwork) << endl;
#if 0
	if( GetThunk() != NULL )
	{
#ifdef DEBUG
	if( DEB )
		cout << "... built-in" << endl;
#endif
		return; // nothing to do
	}
#endif
#ifdef DEBUG
	if( DEB )
		cout << endl;
#endif
	if( cls.size() == 0 )
	{
		RUNWARN("no goal of " << reptype->GetName() << " is defined");

		SetThunk(new proc_failthunk(arity));
	}
	else 
	{
		code *failCode = failcode;
		const type *tracetype = reptype;
		ASSERT( tracetype != NULL );
		if( trace )
		{
			codelist cl;
			REC(cl);
			cl.AddCode(C_TRACE, (ushort)TRACE_ENTER);
			cl.AddCode(tracetype);
			cl.AddCode((ushort)-1);
			cl.AddCode(C_TRACE, (ushort)TRACE_FAIL);
			cl.AddCode(tracetype);
			cl.AddCode((ushort)-1);
			cl.AddCode(C_FAIL);
			cl.EndCode();
			failCode = cl;
		}

		uint ncls = cls.size();
		// TODO: By checking whether ncls==1, we can improve the compiling speed

		uint ntypes = type::GetSerialCount();
		uint *map = new uint[ntypes];
		int *flg = new int[ntypes];
		Bitmap **clx = new Bitmap *[ntypes];
		uint nclx = 1;
		clx[0] = new Bitmap(ncls);
		for( i=0; i<ntypes; i++ )
		{
			map[i] = 0;
		}
		Bitmap *defint = new Bitmap(ncls);
		Bitmap *defstg = new Bitmap(ncls);
		
		for( i=0; i<ncls; i++ )  // for each clause, 
		{
			for( j=0; j<nclx; j++ )
				flg[j] = -1;			// initialize flags

			for( j=0; j<ntypes; j++ )  // for each type, 
			{
				const type *r;
				if( cls[i].tt == NULL )
					r = type::Serial(j);
				else 
				{
					tserial ser = coerceData(cls[i].tt->GetSerialNo(),j).result_s;
					if( ser != S_INVALID )
						r = type::Serial(ser);
					else
						r = NULL;
				}

				if( flg[map[j]] == -1 )
				{	// Now start with new
					if( r != NULL )
						clx[map[j]]->Set(i);
					flg[map[j]] = 0;
				}	
				else
				{
					// There is already one or more 
					if( clx[map[j]]->Test(i) == (r != NULL) )
					{  // If they coincide...
						// Do nothing
					}
					else
					{  // otherwise
						if( flg[map[j]] == 0 )
						{
							flg[map[j]] = nclx;
//								flg[nclx] = 0;
							clx[nclx] = new Bitmap(*clx[map[j]]);
							if( r != NULL )
								clx[nclx]->Set(i);
							else
								clx[nclx]->Reset(i);
							nclx++;
						}
						map[j] = flg[map[j]];
					}
				}
			}
			if( cls[i].instant )
			{
				instant = true;
				if( cls[i].tt == t_int )
				{
					_HASHMAP<mint, Bitmap *>::const_iterator it = pihash.find(cls[i].ti);
					if( it == pihash.end() )
					{
						pihash.insert(make_pair(cls[i].ti, new Bitmap(ncls)));
					}
					pihash[cls[i].ti]->Set(i);
				}
				else if( cls[i].tt == t_stg )
				{
					_HASHMAP<sserial, Bitmap *>::const_iterator it = pshash.find(cls[i].ts);
					if( it == pshash.end() )
					{
						pshash.insert(make_pair(cls[i].ts, new Bitmap(ncls)));
					}
					pshash[cls[i].ts]->Set(i);
				}
				else
					ABORT("UNKNOWN INSTANCE");
			}
			else
			{
				const type *t = cls[i].tt;
				if( t == NULL || t == bot || t == t_int )
				{
					defint->Set(i);
				}
				if( t == NULL || t == bot || t == t_stg )
				{
					defstg->Set(i);
				}
			}
		}

		delete[] flg;
		
		code **cll = new code *[nclx];
		for( i=0; i<nclx; i++ )
		{
#ifdef DEBUG
			if( DEB )
			{
				cout << "code type (" << i << ") : ";
				clx[i]->Output(cout); cout << endl;
				cout << "Code of " << GetName() << "(" << i << ")" << endl;
			}
#endif
			cll[i]  = MakeCaller(ncls, clx[i], trace, failCode, tracetype);
		}

		// parr is the array of the map type -> function_need.
		// Now select thunk
		if( nclx == 1 )
		{
			if( cll[0] == failCode && ! trace )
			{
#ifdef DEBUG
				if( DEB )
					cout << "thunk : failthunk" << endl;
#endif
				SetThunk(new proc_failthunk(arity));
			}
			else
			{
#ifdef DEBUG
				if( DEB )
					cout << "thunk : simplethunk" << endl;
#endif
#ifdef PROCTRACE
				SetThunk(new proc_simplethunk_trace(cll[0], arity));
#else
				SetThunk(new proc_simplethunk(cll[0], arity));
#endif
			}
		}
//#ifdef BINARYTHUNK
		else if( nclx == 2 && (cll[0] == failCode || cll[1] == failCode) && !instant )
		{
			Bitmap bp(ntypes);
			for( i=0; i<ntypes; i++ )
				if( cll[map[i]] != failCode )
					bp.Set(i);

			if( cll[0] == failCode )
			{
#ifdef DEBUG
				if( DEB )
					cout << "thunk : binarythunk(1)" << endl;
#endif
#ifdef PROCTRACE
				SetThunk(new proc_binarythunk_trace(cll[1], bp, arity));
#else
				SetThunk(new proc_binarythunk(cll[1], bp, arity));
#endif
			}
			else
			{
#ifdef DEBUG
				if( DEB )
					cout << "thunk : binarythunk(0)" << endl;
#endif
#ifdef PROCTRACE
				SetThunk(new proc_binarythunk_trace(cll[0], bp, arity));
#else
				SetThunk(new proc_binarythunk(cll[0], bp, arity));
#endif
			}
		}
//#endif
		else if( !instant )
		{
#ifdef DEBUG
			if( DEB )
				cout << "thunk : multithunk" << endl;
#endif
			code **parr = new code *[ntypes];

			for( i=0; i<ntypes; i++ )
			{
#ifdef DEBUG
				if( DEB )
					cout << type::Serial(i)->GetName() << " : " << GetName() << "(" << map[i] << ")" << endl;
#endif
				parr[i] = cll[map[i]];
			}

#ifdef PROCTRACE
			SetThunk(new proc_multithunk_trace(parr, arity));
#else
			SetThunk(new proc_multithunk(parr, arity));
#endif
		}
		else
		{
#ifdef DEBUG
			if( DEB )
				cout << "thunk : fullthunk" << endl;
#endif

			pshash.insert(make_pair((sserial)SSERIAL_INVALID, new Bitmap(ncls)));
			pihash.insert(make_pair((mint)MINT_INVALID, new Bitmap(ncls)));

			code **parr = new code *[ntypes];
			_HASHMAP<sserial, code *> ishash;
			_HASHMAP<mint, code *> iihash;

			for( i=0; i<ntypes; i++ )
			{
#ifdef DEBUG
				if( DEB )
					cout << type::Serial(i)->GetName() << " : " << GetName() << "(" << map[i] << ")" << endl;
#endif
				parr[i] = cll[map[i]];
			}
			for( _HASHMAP<sserial, Bitmap *>::const_iterator it = pshash.begin();
				 it != pshash.end(); ++it )
			{
				Bitmap *bm = it->second;
				bm->Merge(*defstg);
				ishash.insert(make_pair(it->first, MakeCaller(ncls, bm, trace, failCode, tracetype)));
			}
			
			for( _HASHMAP<mint, Bitmap *>::const_iterator it = pihash.begin();
				 it != pihash.end(); ++it )
			{
				Bitmap *bm = it->second;
				bm->Merge(*defint);
				iihash.insert(make_pair(it->first, MakeCaller(ncls, bm, trace, failCode, tracetype)));
			}

			SetThunk(new proc_fullthunk(parr, ishash, iihash, arity));
		}
		for( i=0; i<nclx; i++ )
			delete clx[i];
		delete[] clx;
		delete[] cll;
		delete[] map;
		delete defint;
		delete defstg;
	}
}

void procedure::NewTypeNotify()
{
	for( uint i=0; i<procedure::GetSerialCount(); i++ )
	{
		if( procedure::SerialThunk(i)->NeedNewTypeRevise() )
			procedure::Serial(i)->SetThunk(new proc_selectthunk(procedure::Serial(i), procedure::Serial(i)->GetArity()));
	}
}

#if 0
void CompileProcedures()
{
	for( int i=0; i<procedure::GetSerialCount(); i++ )
	{
		procedure::Serial(i)->Compile();
	}
}
#endif

code *procedure::MakeCaller(uint ncls, Bitmap *clx, bool trace, code *failCode, const type *tracetype)
{
	code *result;
	if( clx->IsAllZero() )
	{
		result = failCode;
	}
	else
	{
		codelist cl;
		int nclauseadded = 0;
		
		for( uint j=0; j<ncls; j++ )
		{
			if( clx->Test(j) )
			{
				clx->Reset(j);
				
				if( trace )
				{
					ostringstream oss;
					oss << "_tr_" << j;
					
					if( nclauseadded == 0 ) // the first
					{
						REC(cl);
						cl.AddCode(C_TRY);
						cl.AddCode(new LabelRefCode("_tr_fail"));
						cl.AddCode(C_SETLEVEL);
						if( ! clx->IsAllZero() )
						{
							REC(cl);
							cl.AddCode(C_TRY);
							cl.AddCode(new LabelRefCode(oss.str().c_str()));
						}
						REC(cl);
						cl.AddCode(C_TRACE, (ushort)TRACE_ENTER);
						cl.AddCode(tracetype);
						cl.AddCode((ushort)j);
					}
					else
					{
						if( ! clx->IsAllZero() )
						{
							REC(cl);
							cl.AddCode(C_RETRY);
							cl.AddCode(new LabelRefCode(oss.str().c_str()));
						}
						else
						{
							REC(cl);
							cl.AddCode(C_LASTTRY);
						}
						REC(cl);
						cl.AddCode(C_TRACE, (ushort)TRACE_REENTER);
						cl.AddCode(tracetype);
						cl.AddCode((ushort)j);
					}
					REC(cl);
					cl.AddCode(C_ALLOC);
					cl.AddCode((ushort)GetArity());
					cl.AddCode((ushort)GetArity());
					uint k;
					for( k=0; k<GetArity(); k++ )
					{
						REC(cl);
						cl.AddCode(C_UNIFY1ST, (varno)k);
						cl.AddCode(C_LINK2ND, (varno)(-(k+1)));
					}
					REC(cl);
					cl.AddCode(C_CALLAT, cls[j].cl, GetArity());
					cl.AddCode(C_CLRTEMP, (ushort)GetArity());
					for( k=0; k<GetArity(); k++ )
					{
						REC(cl);
						cl.AddCode(C_UNIFY1ST, (varno)(-(k+1)));
						cl.AddCode(C_LINK2ND, (varno)k);
					}
					REC(cl);
					cl.AddCode(C_TRACE, (ushort)TRACE_SUCCEED);
					cl.AddCode(tracetype);
					cl.AddCode((ushort)j);
					cl.AddCode(C_DEALLOC);
					cl.AddCode(C_RETURN);
					cl.SetLabelDefCode(new LabelDefCode(oss.str().c_str()));
				}
				else 
				{
					if( clx->IsAllZero() )
						if( nclauseadded == 0 ) // the first
						{ // The case clause is only one to run
							result = cls[j].cl;
							goto resulted; // bypass EndCode
						}
						else
						{
							cl.AddCode(C_LASTAT, cls[j].cl);
							break;
						}
					else
						if( nclauseadded == 0 ) // the first
							cl.AddCode(C_TRYAT, cls[j].cl);
						else
							cl.AddCode(C_RETRYAT, cls[j].cl);
				}
				nclauseadded++;
			}
		}
		if( trace )
		{
			REC(cl);
			cl.SetLabelDefCode(new LabelDefCode("_tr_fail"));
			cl.AddCode(C_LASTTRY);
			cl.AddCode(C_TRACE, (ushort)TRACE_FAIL);
			cl.AddCode(tracetype);
			cl.AddCode((ushort)-1);
			cl.AddCode(C_FAIL);
		}
		cl.EndCode();

#ifdef DEBUG
	if( DEB )
		cl.DebugOutput();
#endif
		result = cl;
resulted: ;
	}
	return result;
}


void procedure::terminate()
{
	for( uint i=0; i<GetSerialCount(); i++ )
	{
		delete serialtable[i].pc;
		delete serialtable[i].pt;
	}
}


bool procedure::Call(machine &m)
{
	bool ret;

	procthunk *pt = this->GetThunk();
	ASSERT( pt != NULL );

	m.SetRP(arity);

	code * oldIP = m.GetIP();
	m.SetIP(0);

	core_p cutp = m.GetCutPoint();
	m.SetTrailPoint();

	pt->exec(m);

	if( m.GetIP() == m.GetCIP() )
	{
		ret = true;
	}
	else
	{
		ret = false;
		if( m.GetIP() != NULL )
			if( m.Execute(m.GetIP()) )
				ret = true;
	}

	if( ret == false )
	{
		m.TrailBack();
		m.SetIP(oldIP);
		return false;
	}
	m.DoCut(cutp);
	m.SetIP(oldIP);
	
	return true;
}

bool procedure::Call(machine &m, FSP arg1)
{
	m.SetRP(arity);
	m.WriteLocal(0, PTR2c(arg1.GetAddress()));
	return Call(m);
}

bool procedure::Call(machine &m, FSP arg1, FSP arg2)
{
	m.SetRP(arity);
	m.WriteLocal(0, PTR2c(arg1.GetAddress()));
	m.WriteLocal(1, PTR2c(arg2.GetAddress()));
	return Call(m);
}

bool procedure::Call(machine &m, FSP arg1, FSP arg2, FSP arg3)
{
	m.SetRP(arity);
	m.WriteLocal(0, PTR2c(arg1.GetAddress()));
	m.WriteLocal(1, PTR2c(arg2.GetAddress()));
	m.WriteLocal(2, PTR2c(arg3.GetAddress()));
	return Call(m);
}

bool procedure::Call(machine &m, FSP arg1, FSP arg2, FSP arg3, FSP arg4)
{
	m.SetRP(arity);
	m.WriteLocal(0, PTR2c(arg1.GetAddress()));
	m.WriteLocal(1, PTR2c(arg2.GetAddress()));
	m.WriteLocal(2, PTR2c(arg3.GetAddress()));
	m.WriteLocal(3, PTR2c(arg4.GetAddress()));
	return Call(m);
}

bool procedure::Call(machine &m, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5)
{
	m.SetRP(arity);
	m.WriteLocal(0, PTR2c(arg1.GetAddress()));
	m.WriteLocal(1, PTR2c(arg2.GetAddress()));
	m.WriteLocal(2, PTR2c(arg3.GetAddress()));
	m.WriteLocal(3, PTR2c(arg4.GetAddress()));
	m.WriteLocal(4, PTR2c(arg5.GetAddress()));
	return Call(m);
}

bool procedure::Call(machine &m, FSP arg1, FSP arg2, FSP arg3, FSP arg4, FSP arg5, FSP arg6)
{
	m.SetRP(arity);
	m.WriteLocal(0, PTR2c(arg1.GetAddress()));
	m.WriteLocal(1, PTR2c(arg2.GetAddress()));
	m.WriteLocal(2, PTR2c(arg3.GetAddress()));
	m.WriteLocal(3, PTR2c(arg4.GetAddress()));
	m.WriteLocal(4, PTR2c(arg5.GetAddress()));
	m.WriteLocal(5, PTR2c(arg6.GetAddress()));
	return Call(m);
}

bool procedure::Call(machine &m, FSP args[ /*arity*/ ])
{
	m.SetRP(arity);
	for( uint i=0; i<arity; i++ )
		m.WriteLocal(i, PTR2c(args[i].GetAddress()));
	return Call(m);
}

} // namespace lilfes
