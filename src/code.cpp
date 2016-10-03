/* $Id: code.cpp,v 1.9 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: code.cpp,v 1.9 2011-05-02 10:38:23 matuzaki Exp $";

#include "code.h"
#include "codelist.h"
#include "proc.h"
#include <cstring>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::setw;
using std::memcpy;


#ifdef BIG_ENDIAN_
#if SIZEOF_INTP == 4
  void *GetPointer(const code *p) { ptr_int ret = (((((p[0] << 8) + p[1]) << 8) + p[2]) << 8) + p[3]; return reinterpret_cast<void*>(ret); }
#elif SIZEOF_INTP == 8
void *GetPointer(const code *p) { ptr_int ret = ((((((((((((((ptr_int)p[0] << 8) + p[1]) << 8) + p[2]) << 8) + p[3]) << 8) + p[4]) << 8) + p[5]) << 8) + p[6]) << 8) + p[7]; return reinterpret_cast<void*>(ret); }
#else
#error Can not find ptr size.
#endif
#else
#if SIZEOF_INTP == 4
void *GetPointer(const code *p) { ptr_int ret = (((((p[3] << 8) + p[2]) << 8) + p[1]) << 8) + p[0]; return reinterpret_cast<void*>(ret); }
#elif SIZEOF_INTP == 8
void *GetPointer(const code *p) { ptr_int ret = ((((((((((((((ptr_int)p[7] << 8) + p[6]) << 8) + p[5]) << 8) + p[4]) << 8) + p[3]) << 8) + p[2]) << 8) + p[1]) << 8) + p[0]; return reinterpret_cast<void*>(ret); }
#else
#error Can not find ptr size.
#endif
#endif

#define CODE_GROW_STEP 128

//////////////////////////////////////////////////////////////////////////////
//
// class LabelRefCode
//

const char *LabelDef::className     = "LabelDef";
const char *LabelDefCode::className = "LabelDefCode";
const char *LabelRef::className     = "LabelRef";
const char *LabelRefCode::className = "LabelRefCode";
const char *codelist::className = "codelist";

bool LabelRefCode::Match(LabelDef *r)
{ 
	return r->ClassName() == LabelDefCode::className 
	    && LabelRef::Match(r); 
}

void LabelRefCode::Write(codelist &cl) 
{ 
//	ASSERT(strcmp(ld->ClassName(), "LabelDefCode") == 0)
//	TRACE("LABEL " << name << " AT " << storeplace << " IS RESOLVED WITH " << (ld->GetValue() - storeplace));
	cl.WriteCode(storeplace, (uint32)(ld->GetValue() - storeplace)); 
}


TermFunction SharedPtr::TF(SharedPtr::term, 5);
_HASHMAP<code_ptr, SharedPtr *> SharedPtr::hash;

void SharedPtr::term()
{
#ifdef DEBUG
	cout << "Terminate " << __FILE__ << endl;
#endif
	for(_HASHMAP<code_ptr, SharedPtr *>::const_iterator it = hash.begin();
		it != hash.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// class codelist
//

codelist::codelist() 
{
	cp = NULL; len = 0; freezed = MELTED; 
	Refs = NULL; Defs = NULL;
}

codelist::~codelist() 
{
	if( !freezed )
	{
		delete[] cp;
	}

// CleanLabel() may perform ResolveLabel; it is not neccesary
	while( Refs != NULL )
	{
		LabelRef *r = Refs; Refs = Refs->Next(); delete r;
	}
	while( Defs != NULL )
	{
		LabelDef *d = Defs; Defs = Defs->Next(); delete d;
	}
}

void codelist::SetLabelDef(LabelDef *ld)
{
	ld->Prepend(Defs);
}

void codelist::EndCode()
{
	ASSERT(freezed != RESOLVED);

#ifdef DOASSERT
	AddCode( C_HALT );
#endif

// Resolve labels
	ResolveLabel();

// Copy the code to codeMem
	code *cpa = new code[len];

	memcpy(cpa, cp, len * sizeof(code));
	delete[] cp;

	cp = cpa;
	freezed = FREEZED;
}

void codelist::Insert(int place, int n)
{
	ASSERT( ! freezed );
	ASSERT( n > 0 );
	if( n == 0 )
	{
		return;
	}

	if( ((len-1) & ~(CODE_GROW_STEP-1)) != ((len+n-1) & ~(CODE_GROW_STEP-1)) )
	{
		code *nc = new code[((len+n-1) & ~(CODE_GROW_STEP-1)) + CODE_GROW_STEP];
		memcpy(nc, cp, len*sizeof(code));
		delete[] cp;
		cp = nc;
	}

#ifdef HAS_MEMMOVE
	memmove(cp+place+n, cp+place, len-place);
#else
	for( int i=len-1; i>=place; i-- )
	{
		cp[i+n] = cp[i];
	}
#endif

#ifdef DEBUG
	for( int x=0; x<n; x++ )
	{
		cp[place+x] = C_HALT;
	}
#endif

	len += n;

// Move Refs & Defs 

	LabelRef *r = Refs;
	while( r != NULL )
	{
		r->InsertAt(place, n);
		r = r->Next();
	}

	LabelDef *d = Defs;
	while( d != NULL )
	{
		d->InsertAt(place, n);
		d = d->Next();
	}
}

void codelist::Delete(int place, int n)
{
	ASSERT( ! freezed );
	ASSERT( n > 0 );
	if( n == 0 )
	{
		return;
	}

#ifdef HAS_MEMMOVE
	memmove(cp+place, cp+place+n, len-(place+n));
#else
	for( int i=place; i<len-n; i++ )
	{
		cp[i] = cp[i+n];
	}
#endif

#ifdef DEBUG
	for( int x=0; x<n; x++ )
	{
		cp[place+x] = C_HALT;
	}
#endif

	len -= n;

// Move Refs & Defs 

	LabelRef *r = Refs;
	while( r != NULL )
	{
		r->InsertAt(place, -n);
		r = r->Next();
	}

	LabelDef *d = Defs;
	while( d != NULL )
	{
		d->InsertAt(place, -n);
		d = d->Next();
	}
}

void codelist::ResolveLabel()
{
	ASSERT( freezed == MELTED || freezed == CLEANED );
	if( freezed == CLEANED )
	{
		return;
	}

	LabelRef *r = Refs;
	
	while( r != NULL )
	{
		LabelDef *d = Defs;
		
		while( d != NULL )
		{
			if( r->Match(d) )
			{
				break;
			}
			d = d->Next();
		}
		
		if( d == NULL )
		{
			r->LabelUndefined();
		}
		r = r->Next(); 
	}

// And Write labels
	r = Refs;
	while( r != NULL )
	{
		r->Write(*this);
		r = r->Next(); 
	}

	freezed = RESOLVED;
}

void codelist::CleanLabel()
{
	if( freezed == MELTED )
	{
		ResolveLabel();
	}
		
	while( Refs != NULL )
	{
		LabelRef *r = Refs; Refs = Refs->Next(); delete r;
	}
	while( Defs != NULL )
	{
		LabelDef *d = Defs; Defs = Defs->Next(); delete d;
	}
	
	if( freezed == RESOLVED )
	{
		freezed = CLEANED;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
//  AddCode family.

void codelist::AddCode(code c)
{
	ASSERT( ! freezed );
	if((len & (CODE_GROW_STEP-1)) == 0 )
	{
		code *nc = new code[len + CODE_GROW_STEP];
		memcpy(nc, cp, len*sizeof(code));
		delete[] cp;
		cp = nc;
	}

	cp[len++] = c;
}

void codelist::AddCode(const type *t)
{
	int i = t->GetSerialNo();
	AddCode( (ushort) i );
}

void codelist::AddCode(const feature *f)
{
	int i = f->GetSerialNo();
	AddCode( (ushort) i );
}

void codelist::AddCode(const procedure *p)
{
	int i = p->GetSerialNo();
	AddCode( (ushort) i );
}

void codelist::AddCode(LabelRef *lr)
{
	lr->SetStorePlace(Current());
	lr->Prepend(Refs);

	for( int i=0; i<lr->DefaultSize(); i++ )
	{
		AddCode(C_HALT);
	}
}

void codelist::AddCode(uint32 i)
{
#ifdef BIG_ENDIAN_
	AddCode( (code)((i >> 24) & 0xff) );
	AddCode( (code)((i >> 16) & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)( i        & 0xff) );
#else
	AddCode( (code)( i        & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)((i >> 16) & 0xff) );
	AddCode( (code)((i >> 24) & 0xff) );
#endif
}

#ifdef HAS_INT64
void codelist::AddCode(uint64 i) {
#ifdef BIG_ENDIAN_
	AddCode( (code)((i >> 56) & 0xff) );
	AddCode( (code)((i >> 48) & 0xff) );
	AddCode( (code)((i >> 40) & 0xff) );
	AddCode( (code)((i >> 32) & 0xff) );
	AddCode( (code)((i >> 24) & 0xff) );
	AddCode( (code)((i >> 16) & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)( i        & 0xff) );
#else
	AddCode( (code)( i        & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)((i >> 16) & 0xff) );
	AddCode( (code)((i >> 24) & 0xff) );
	AddCode( (code)((i >> 32) & 0xff) );
	AddCode( (code)((i >> 40) & 0xff) );
	AddCode( (code)((i >> 48) & 0xff) );
	AddCode( (code)((i >> 56) & 0xff) );
#endif
}
#endif // HAS_INT64

// void codelist::AddCode(cell c) {
// 	uint32 i = cell2int(c);
// #ifdef BIG_ENDIAN_
// 	AddCode( (code)((i >> 24) & 0xff) );
// 	AddCode( (code)((i >> 16) & 0xff) );
// 	AddCode( (code)((i >>  8) & 0xff) );
// 	AddCode( (code)( i        & 0xff) );
// #else
// 	AddCode( (code)( i        & 0xff) );
// 	AddCode( (code)((i >>  8) & 0xff) );
// 	AddCode( (code)((i >> 16) & 0xff) );
// 	AddCode( (code)((i >> 24) & 0xff) );
// #endif
// }

void codelist::AddCode(code *cp)
{
	ptr_int i = reinterpret_cast<ptr_int>(cp);
#ifdef BIG_ENDIAN_
#if SIZEOF_INTP == 8
	AddCode( (code)((i >> 56) & 0xff) );
	AddCode( (code)((i >> 48) & 0xff) );
	AddCode( (code)((i >> 40) & 0xff) );
	AddCode( (code)((i >> 32) & 0xff) );
#endif
	AddCode( (code)((i >> 24) & 0xff) );
	AddCode( (code)((i >> 16) & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)( i        & 0xff) );
#else
	AddCode( (code)( i        & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)((i >> 16) & 0xff) );
	AddCode( (code)((i >> 24) & 0xff) );
#if SIZEOF_INTP == 8
	AddCode( (code)((i >> 32) & 0xff) );
	AddCode( (code)((i >> 40) & 0xff) );
	AddCode( (code)((i >> 48) & 0xff) );
	AddCode( (code)((i >> 56) & 0xff) );
#endif
#endif
}

void codelist::AddCode(ushort i)
{
#ifdef BIG_ENDIAN_
	AddCode( (code)((i >>  8) & 0xff) );
	AddCode( (code)( i        & 0xff) );
#else
	AddCode( (code)( i        & 0xff) );
	AddCode( (code)((i >>  8) & 0xff) );
#endif
}

void codelist::AddCodelist(codelist &cl)
{
	ASSERT( ! freezed );

	if( ((len-1) & ~(CODE_GROW_STEP-1)) != ((len+cl.len-1) & ~(CODE_GROW_STEP-1)) )
	{
		code *nc = new code[((len+cl.len-1) & ~(CODE_GROW_STEP-1)) + CODE_GROW_STEP];
		memcpy(nc, cp, len*sizeof(code));
		delete[] cp;
		cp = nc;
	}

	memcpy( cp+len, cl.cp, cl.len*sizeof(code) );
	
// Catenate Refs & Defs 

	if( !cl.freezed )
	{
		LabelRef *r = cl.Refs;
		if( r != NULL )
		{
			LabelRef *newRefs = NULL;
			LabelRef **refchain = &newRefs;
			while( r != NULL )
			{
				LabelRef *rr = r->Clone();
				rr->InsertAt(-1, len);
				rr->AppendTo(refchain);
				r = r->Next();
			}
			Refs->AppendTo(refchain);
			Refs = newRefs;
		}

		LabelDef *d = cl.Defs;
		if( d != NULL )
		{
			LabelDef *newDefs = NULL;
			LabelDef **defchain = &newDefs;
			while( d != NULL )
			{
				LabelDef *dd = d->Clone();
				dd->InsertAt(-1, len);
				dd->AppendTo(defchain);
				d = d->Next();
			}
			Defs->AppendTo(defchain);
			Defs = newDefs;
		}
	}

	len += cl.len;
}

//////////////////////////////////////////////////////////////////////////////
//
//  WriteCode family.

void codelist::WriteCode(int place, code c)
{
	ASSERT( ! freezed );

	cp[place] = c;
}

void codelist::WriteCode(int place, const type *t)
{
	int i = t->GetSerialNo();
	WriteCode( place, (ushort) i );
}

void codelist::WriteCode(int place, const feature *f)
{
	int i = f->GetSerialNo();
	WriteCode( place, (ushort) i );
}

void codelist::WriteCode(int place, const procedure *p)
{
	int i = p->GetSerialNo();
	WriteCode( place, (ushort) i );
}

void codelist::WriteCode(int place, LabelRef *lr)
{
	lr->SetStorePlace(place);
	lr->Prepend(Refs);
}

void codelist::WriteCode(int place, uint16 i)
{
//	cout << place << "=" << i << endl;
#ifdef BIG_ENDIAN_
	cp[place  ] = (code)((i >>  8) & 0xff);
	cp[place+1] = (code)( i        & 0xff);
#else
	cp[place  ] = (code)( i        & 0xff);
	cp[place+1] = (code)((i >>  8) & 0xff);
#endif
}

void codelist::WriteCode(int place, uint32 i)
{
//	cout << place << "=" << i << endl;
#ifdef BIG_ENDIAN_
	cp[place  ] = (code)((i >> 24) & 0xff);
	cp[place+1] = (code)((i >> 16) & 0xff);
	cp[place+2] = (code)((i >>  8) & 0xff);
	cp[place+3] = (code)( i        & 0xff);
#else
	cp[place  ] = (code)( i        & 0xff);
	cp[place+1] = (code)((i >>  8) & 0xff);
	cp[place+2] = (code)((i >> 16) & 0xff);
	cp[place+3] = (code)((i >> 24) & 0xff);
#endif
}

void codelist::InsertCode(int place, codelist &cl)
{
	ASSERT( ! freezed );

	Insert(place, cl.len);

	memcpy( cp+place, cl.cp, cl.len*sizeof(code) );
	
// Move Refs & Defs 

	if( !cl.freezed )
	{
		LabelRef *r = cl.Refs;
		while( r != NULL )
		{
			LabelRef *rr = r->Clone();
			rr->InsertAt(-1, place);
			rr->Prepend(Refs);
			r = r->Next();
		}

		LabelDef *d = cl.Defs;
		while( d != NULL )
		{
			LabelDef *dd = d->Clone();
			dd->InsertAt(-1, place);
			dd->Prepend(Defs);
			d = d->Next();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//
//  Debug output of codelist
//

enum args_t
{
	X,			// Bad
	_,			// No arg
	T,			// Type
	F,			// Feature
	N,			// Number
	V,			// Variable
	L,			// Jump (Label)
	P,			// Procedure
	C,			// Code
	M,			// Memory Cell
	CN,			// Code and Number
	NN,			// N N N
	FT,			// Feature & Type
	FTT,		// Feature & Type & Type
	FV,			// Feature & Variable
	VM,			// Variable & Memory Cell
	KT,			// Trace kind & type
	I,			// Integer
	D,			// Float
	H,			// Char
	S,			// StrSymbol
};

struct debug_table {
	const char *name;
	args_t arg;
} DebugTable[256] = 
{
	{"RETURN",	_},	{"BIND",	_},	{"FRESH",	T},	{"ADDSTR",	T},	
	{"SUCCEED",	_},	{"CHGSTR",	T},	{"",		X},	{"",		X},	
	{"SHALCUT",	_},	{"GETLEVEL",V},	{"DEEPCUT",	V},	{"SETLEVEL",_},	
	{"GETCP",	V},	{"",		X},	{"",		X},	{"",		X},	

	{"BINDEO",	_},	{"BINDEOT",	_},	{"BIND1",	_},	{"BIND1T",	T},	
	{"BIND2",	_},	{"BIND2T",	T},	{"BINDNEW",	T},	{"PROCEED",	_},	
	{"UNIFY",	_},	{"UNIFY_S",	_},	{"COPY1",	_},	{"COPY2",	_},	
	{"SKIP1",	_},	{"SKIP2",	_},	{"",		_},	{"",		_},	

	{"",		X},	{"POP",		_},	{"",		X},	{"",		X},	
	{"TRY",	    L},	{"RETRY",	L},	{"LASTTRY",	_},	{"JUMP",	L},	
	{"TRYAT",   C},	{"RETRYAT",	C},	{"LASTAT",	C},	{"START",	_},	
	{"CLRTEMP",	N},	{"ALLOC",  NN},	{"DEALLOC",	_},	{"",		X},	

	{"CALL",	P},	{"EXECUTE",	P},	{"CALLAT", CN},	{"",		X},	
	{"ADDNEW1ST",T},{"ADDNEWPRO",T},{"ADDNEW",	T},	{"",		X},	
	{"PUSH1ST",	F},	{"PUSHPRO",	F},	{"PUSH",	F},	{"",		X},	
	{"FOLLOW1ST",F},{"FOLLOWPRO",F},{"FOLLOW",	F},	{"",		X},	

	{"LINK1ST", V},	{"TEST1ST",	V},	{"UNIFY1ST",V}, {"",		X},
	{"LINKVAR", V},	{"TESTVAR",	V},	{"UNIFYVAR",V}, {"",		X},
	{"LINK2ND", V},	{"TEST2ND",	V},	{"UNIFY2ND",V}, {"",		X},
	{"",		X},	{"",		X},	{"",		X},	{"",		X},	
	{"SETINT1ST",I},{"SETFLO1ST",D},{"SETCHR1ST",H},{"SETSTG1ST",S},
	{"SETINTPRO",I},{"SETFLOPRO",D},{"SETCHRPRO",H},{"SETSTGPRO",S},
	{"SETINT",	I},	{"SETFLO",	D},	{"SETCHR",	H}, {"SETSTG",	S},
	{"",		X},	{"",		X},	{"",		X},	{"",		X},	

	{"FEATADD",FT},	{"FEATLINK",FV},{"FEATTEST",FV},{"FEATUNIFY",FV},	
	{"SETREG", VM},	{"PLUS",	I},	{"TCOERCE",	X},	{"TCOERCET", T},	
	{"COPYF1",	T},	{"COPYF2",	T},	{"SKIPF1",	T},	{"SKIPF2",	T},	
	{"UNIFYF",	T},	{"UNIFYF_S",T},	{"BINDF",	T},	{"",		X},	

	{"PLUS",	I},	{"MINUS",	I},	{"",		X},	{"",		X},	
	{"",		X},	{"",		X},	{"",		X},	{"",		X},	
	{"",		X},	{"",		X},	{"",		X},	{"",		X},	
#ifdef PROF_UNIFY
	{"",		X},	{"",		X},	{"PROFS",	S},	{"PROFE",	S},	
#else
	{"",		X},	{"",		X},	{"",		X},	{"",		X},	
#endif

	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 

	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, {"",X}, 
	{"FEATCONSTR",	FTT},{"FEATCPOP",	_},	{"COMMIT",	_},	{"",		X},	
	{"ADDNEW1ST_C",	T},	{"ADDNEWPRO_C",	T},	{"ADDNEW_C",	T},	{"FEATADD_C",FT},	
	{"",X},	{"CONSTR_HP",	C},	{"CONSTR",	C},	{"RESTORE_REG",	X},	
	{"INTERRUPTED",	X},	{"TRACE",  KT},	{"FAIL",	_},	{"HALT",	_},	
};

void codelist::DebugOutput()
{
	ASSERT(freezed);
	
	LabelRef *r;
	LabelDef *d;
	
// output label defs

	cout << "---------------------------------" << endl;
	cout << "Base addr: " << hex << reinterpret_cast<ptr_int>(cp) << dec << endl;

	cout << "Labels values: " << endl;
	int aaa = 0;
	d = Defs;
	while( d != NULL )
	{
		aaa++;
		d->DebugOutputTop();
		d = d->Next();
	}
	cout << "Total " << aaa << " labels " << endl;
	
	int p = 0, lastp = -1;
	int tmp;
	
	while( p < len )
	{
		d = Defs;
		while( d != NULL )
		{
			if( d->GetValue() > lastp && d->GetValue() <= p )
			{
				d->DebugOutputLabel(d->GetValue());
			}
			d = d->Next();
		}
		
		lastp = p;
		code c = cp[p];

		cout << setw(5) << p << "\t\t" << DebugTable[c].name;

		p++;

		switch( DebugTable[c].arg )
		{
			case X:
				cout << "(unknown " << hex << setw(2) << std::setfill('0') << (int)c << dec << std::setfill(' ') << ")";
				break;
			case _:
				break;
			case FTT:
				cout << " " << GetFeature(cp+p)->GetName(); p += CODE_FEATURE_SIZE; 
				cout << " " << GetType(cp+p)->GetName(); p += CODE_TYPE_SIZE; 
				cout << " " << GetType(cp+p)->GetName(); p += CODE_TYPE_SIZE; 
				break;
			case FT:
				cout << " " << GetFeature(cp+p)->GetName(); p += CODE_FEATURE_SIZE; 
				/* fall thru */
			case T:
				cout << " " << GetType(cp+p)->GetName(); p += CODE_TYPE_SIZE; 
				break;
			case F:
				cout << " " << GetFeature(cp+p)->GetName(); p += CODE_FEATURE_SIZE; 
				break;
			case NN:
				cout << " " << GetInt16(cp+p); p += CODE_INT16_SIZE; 
				cout << " " << GetInt16(cp+p); p += CODE_INT16_SIZE; 
				break;
			case I:
				cout << " " << GetMInt(cp+p); p += CODE_MINT_SIZE; 
				break;
			case D:
				cout << " " << GetMFloat(cp+p); p += CODE_MFLOAT_SIZE; 
				break;
			case H:
				cout << " " << GetMChar(cp+p); p += CODE_MCHAR_SIZE; 
				break;
			case S:
				cout << " \"" << strsymbol::Serial(GetStringSN(cp+p))->GetString() << "\""; p += CODE_STRING_SIZE; 
				break;
			case N:
				cout << " " << GetInt16(cp+p); p += CODE_INT16_SIZE; 
				break;
			case FV:
				cout << " " << GetFeature(cp+p)->GetName(); p += CODE_FEATURE_SIZE; 
				/* fall thru */
			case V:
				tmp = GetVarNo(cp+p); p += CODE_VARNO_SIZE;
				if( tmp >= 0 )
				{
					cout << " TEMP-" << tmp;
				}
				else
				{
					cout << " PERM-" << -tmp;
				}
				break;
			case VM:
				tmp = GetVarNo(cp+p); p += CODE_VARNO_SIZE;
				if( tmp >= 0 )
				{
					cout << " TEMP-" << tmp;
				}
				else
				{
					cout << " PERM-" << -tmp;
				}
				cout << ", ";
				/* fall thru */
			case M:
				cout << "cell(" << hex << cell2int(GetCell(cp+p)) << dec << ")";
				p += CODE_CELL_SIZE;
				break;
			case L:
				cout << " @" << (p + GetRelJump(cp+p)); p += CODE_RELJUMP_SIZE; 
				break;
			case P:
				{
					const procedure *pr = procedure::Serial(GetProcSN(cp+p));
					p += CODE_PROC_SIZE; 
					cout << " " << pr->GetName() << "/" << pr->GetArity(); 
				}
				break;
			case C:
				{
					ptr_int where = reinterpret_cast<ptr_int>(GetInstP(cp+p));
					p += CODE_INSTP_SIZE; 
					cout << " #" << hex << where << dec;
				}
				break;
			case CN:
				{
					ptr_int where = reinterpret_cast<ptr_int>(GetInstP(cp+p));
					p += CODE_INSTP_SIZE; 
					cout << " #" << hex << where << dec;
					cout << "," << GetInt16(cp+p); p += CODE_INT16_SIZE; 
				}
				break;
			case KT:
				{
					cout << " ";
					int tt = GetInt16(cp+p); p += CODE_INT16_SIZE;
					const type *t = GetType(cp+p); p += CODE_TYPE_SIZE;
					int x = GetInt16(cp+p); p += CODE_INT16_SIZE;
					cout << ( tt==0 ? "ENTER" :
						      tt==1 ? "SUCCESS" :
						      tt==2 ? "REENTER" :
						      tt==3 ? "FAIL" : "UNKNOWN" )
						 << " " << t->GetName()
						 << "(" << x << ")";
				}
				break;
			default:
				ABORT("Unknown code type");
		}
		
		r = Refs;
		while( r != NULL )
		{
			if( r->GetStorePlace() >= lastp && r->GetStorePlace() < p )
			{
				r->DebugOutput();
			}
			r = r->Next();
		}
		cout << endl;
	}
	cout << "---------------------------------" << endl;
}


void LabelRefCode::DebugOutput()
{
	cout << "\t(@" << ld->GetValue() << " = " << name << ")";
}

void LabelDef::DebugOutputTop()
{
	cout << name << " = " << value << endl;
}

void LabelDefCode::DebugOutputLabel(int)
{
	cout << setw(5) << value << "\t" << name << ":" << endl;
}

#ifdef DEBUG_LABEL
void LabelDefDebug::DebugOutputLabel(int l)
{
//	if( value == l )
		cout << setw(5) << l << "\t[" << filename << " " << line << ":]" << endl;
}

const char *LabelDefDebug::className = "LabelDefDebug"; 

void LabelDefDebugStr::DebugOutputLabel(int l)
{
//	if( value == l )
		cout << setw(5) << l << "\t<" << str << ">" << endl;
}

const char *LabelDefDebugStr::className = "LabelDefDebugStr"; 
#endif

} // namespace lilfes
