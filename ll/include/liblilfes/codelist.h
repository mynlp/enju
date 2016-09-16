/* $Id: codelist.h,v 1.10 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __codelist_h
#define __codelist_h

#include "lconfig.h"
#include "code.h"

#include <string>

namespace lilfes {

// Forward definitions
class codelist;
class procedure;

class LabelDef
{
private:
	LabelDef *next;
protected:
	std::string name;
	int value;

public:
	LabelDef(const std::string &in, int iv=0)
		{ next = NULL; name = in; value = iv; }
	virtual ~LabelDef() { }

	LabelDef *Next() { return next; }
	void Prepend(LabelDef *&x) { next = x; x = this; }
	void AppendTo(LabelDef **&x) { *x = this; x = &next; }

	const std::string &GetName() { return name; }
	void SetValue(int iv) { value = iv; }
	int GetValue() { return value; }

	virtual void InsertAt(int, int) { }
	virtual void DebugOutputTop();
	virtual void DebugOutputLabel(int) {  }

	static const char *className;
	virtual const char * ClassName() { return className; }
	virtual LabelDef *Clone() { return new LabelDef(*this); }
};

class LabelDefCode : public LabelDef
{
public:
	LabelDefCode(const std::string &in, int iv=0) : LabelDef(in, iv) { }

	virtual void InsertAt(int place, int len) 
	{
		if( value >= place )
		{
			value += len;
		}
	}
	virtual void DebugOutputTop() {  }
	virtual void DebugOutputLabel(int l);

	static const char *className;
	virtual const char * ClassName() { return className; }
	virtual LabelDef *Clone() { return new LabelDefCode(*this); }
};

#ifdef DEBUG_LABEL
class LabelDefDebug : public LabelDefCode
{
	std::string filename;
	int line;
public:
	LabelDefDebug(const std::string &f, int l) : LabelDefCode("") { filename = f; line = l; }

	virtual void DebugOutputLabel(int l);

	static const char *className;
	virtual const char * ClassName() { return className; }
	virtual LabelDef *Clone() { return new LabelDefDebug(*this); }
};

class LabelDefDebugStr : public LabelDefCode
{
	std::string str;
public:
	LabelDefDebugStr(const std::string &f) : LabelDefCode("") { str = f; }

	virtual void DebugOutputLabel(int l);

	static const char *className;
	virtual const char * ClassName() { return className; }
	virtual LabelDef *Clone() { return new LabelDefDebugStr(*this); }
};
#endif

class LabelRef
{
private:
	LabelRef *next;
protected:
	std::string name;
	int storeplace;
	LabelDef *ld;

public:
	LabelRef(const char *in) 
		{ next = NULL; name = in; storeplace = -1; ld = NULL; }
	virtual ~LabelRef() { }

	void SetStorePlace(int sp) { storeplace = sp; }
	int GetStorePlace() { return storeplace; }
	LabelRef *Next() { return next; }
	void Prepend(LabelRef *&x) { next = x; x = this; }
	void AppendTo(LabelRef **&x) { *x = this; x = &next; }

	bool LabelUndefined() { ABORT( ClassName() << " undefined: " << name ); return false; }

	const std::string &GetName() { return name; }

	void SetLabel(codelist &cl, LabelDef *l) { ld = l; Resolve(cl); }

	virtual int  DefaultSize() { return 1; }
	virtual void InsertAt(int place, int len)
	{
		if( storeplace > place )
		{
			storeplace += len;
		}
	}
	virtual void Resolve(codelist &) { }
	virtual void Write(codelist &) = 0;
	virtual bool Match(LabelDef *r) { if ( r->GetName() == name) { ld = r; return true; } else return false; }
	virtual void DebugOutput() = 0;

	static const char *className;
	virtual const char * ClassName() { return className; }
	virtual LabelRef *Clone() = 0; 
};

class LabelRefCode : public LabelRef
{
	int value;
public:
	LabelRefCode(const char *in) : LabelRef(in) { }
	virtual ~LabelRefCode() { }

	virtual int  DefaultSize() { return CODE_RELJUMP_SIZE; }
	virtual void Write(codelist &cl);
	virtual bool Match(LabelDef *r);
	virtual void DebugOutput();

	static const char *className;
	virtual const char * ClassName() { return className; }
	virtual LabelRef *Clone() { return new LabelRefCode(*this); }
};

////////////////////////////////////////
//
//  class codelist
//

class codelist
{
protected:
	code * cp;
	int len;
	unsigned memp;
	LabelRef *Refs;
	LabelDef *Defs;

	enum { MELTED=0, RESOLVED, CLEANED, FREEZED } freezed;
//	bool freezed;

public:
	codelist();
	virtual ~codelist();

	operator code *() { ASSERT(freezed); return cp; }

	void Insert(int place, int len);
	void Delete(int place, int len);

	virtual void EndCode();

	int GetLen() { return len; }
	int Current() { return len; }

// Labelling function.
	void SetLabelDef(LabelDef *ld);
	void SetLabelDefCode(LabelDefCode *ld) 
	{
		ld->SetValue(Current());
		SetLabelDef(ld);
	}
	void SetLabel(const char *name)
	{
		SetLabelDefCode(new LabelDefCode(name));
	}

	void ResolveLabel();
	void CleanLabel();

// Debug function.

	void DebugOutput();

// AddCode family.

	void AddCode(code c);

	void AddCode(const type *t);
	void AddCode(const feature *f);
	void AddCode(const procedure *p);
	void AddCode(uint16 n);
	void AddCode(uint32 n);
#ifdef HAS_INT64
	void AddCode(uint64 n);
#endif // HAS_INT64
	void AddCode(LabelRef *lr);
	void AddCode(const char *label) { AddCode( new LabelRefCode(label) ); }
	void AddCode(code *cp);
	void AddCode(cell c) { AddCode( cell2int( c ) ); }
	void AddCode(varno n) { AddCode( (uint16)n ); }
#ifdef CELL_64BIT
	void AddCode(mint n)  { AddCode( (uint64)n ); }
#else // CELL_64BIT
	void AddCode(mint n)  { AddCode( (uint32)n ); }
#endif // CELL_64BIT
#ifdef CELL_64BIT
	void AddCode(mfloat n){ AddCode( static_cast<uint64>(float2int(n)) ); }
#else // CELL_64BIT
	void AddCode(mfloat n){ AddCode( static_cast<uint32>(float2int(n)) ); }
#endif // CELL_64BIT
//	void AddCode(mchar n) { AddCode( (uint16)n ); }
//	void AddCode(int16 n) { AddCode( (uint16)n ); }


	void AddCode(code c, const type *   a) { AddCode(c); AddCode(a);}
	void AddCode(code c, const feature *a) { AddCode(c); AddCode(a);}
	void AddCode(code c, const procedure *a) { AddCode(c); AddCode(a);}
	void AddCode(code c, uint16         a) { AddCode(c); AddCode(a);}
	void AddCode(code c, varno          a) { AddCode(c); AddCode(a);}
	void AddCode(code c, uint32         a) { AddCode(c); AddCode(a);}
	void AddCode(code c, const char *   a) { AddCode(c); AddCode(a);}
	void AddCode(code c, LabelRef *     a) { AddCode(c); AddCode(a);}
	void AddCode(code c, code *         a) { AddCode(c); AddCode(a);}
	void AddCode(code c, const char *a1, ushort a2) { AddCode(c); AddCode(a1); AddCode(a2); }
	void AddCode(code c, const char *a1, LabelRef * a2) { AddCode(c); AddCode(a1); AddCode(a2); }
	void AddCode(code c, LabelRef *a1, ushort a2) { AddCode(c); AddCode(a1); AddCode(a2); }
	void AddCode(code c, LabelRef *a1, LabelRef * a2) { AddCode(c); AddCode(a1); AddCode(a2); }
	void AddCode(code c, code *         a1, ushort a2) { AddCode(c); AddCode(a1); AddCode(a2); }
	void AddCode(code c, uint16 a1, cell a2) { AddCode(c); AddCode(a1); AddCode(a2); }

	virtual void AddCodelist(codelist &cl);
	void AddCodelist(codelist *cl) { AddCodelist(*cl); }

// WriteCode family.
	void WriteCode(int place, code c);

	void WriteCode(int place, const type *t);
	void WriteCode(int place, const feature *f);
	void WriteCode(int place, const procedure *p);
	void WriteCode(int place, uint16 n);
	void WriteCode(int place, uint32 n);
	void WriteCode(int place, varno n) { WriteCode(place, (uint16)n); }
	void WriteCode(int place, LabelRef *lr);
	void WriteCode(int place, const char *label) { WriteCode( place, new LabelRefCode(label) ); }
//	void WriteCode(int place, code *cp);
//	void WriteCode(int place, int   n) { WriteCode( place, (ushort)n ); }
//	void WriteCode(int place, short n) { WriteCode( place, (ushort)n ); }

// InsertCode family.

	void InsertCode(int place, code c)
		{ Insert(place, 1); WriteCode(place, c); }

	void InsertCode(int place, const type *t)
		{ Insert(place, CODE_TYPE_SIZE); WriteCode(place, t); }

	void InsertCode(int place, const feature *f)
		{ Insert(place, CODE_FEATURE_SIZE); WriteCode(place, f); }

	void InsertCode(int place, const procedure *p)
		{ Insert(place, CODE_PROC_SIZE); WriteCode(place, p); }

	void InsertCode(int place, uint16 n)
		{ Insert(place, CODE_INT16_SIZE); WriteCode(place, n); }

	void InsertCode(int place, LabelRef *lr)
		{ Insert(place, lr->DefaultSize() ); WriteCode(place, lr); }

	void InsertCode(int place, const char *label) { InsertCode( place, new LabelRefCode(label) ); }

//	void InsertCode(int place, code *cp);
//	void InsertCode(int place, int   n) { InsertCode( place, (ushort)n ); }
//	void InsertCode(int place, short n) { InsertCode( place, (ushort)n ); }

	void InsertCode(int place, codelist &cl);

	static const char *className;
	virtual const char * ClassName() { return className; }
};

#ifdef DEBUG_LABEL
#define REC(x)  (x).SetLabelDefCode(new LabelDefDebug(__FILE__, __LINE__))
#define RECDATA(x, d)  (x).SetLabelDefCode(new LabelDefDebugStr((d)))
#else
#define REC(x) 
#define RECDATA(x, d)
#endif

} // namespace lilfes

#endif // __codelist_h
