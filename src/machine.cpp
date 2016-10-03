/* $Id: machine.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "machine.h"
#include "builtin.h"
#include "structur.h"
#include "in.h"

#include <ios>
#include <iostream>
#include <ostream>
#include <string>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::string;

static const char rcsid[] = "$Id: machine.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $";

////////////////////////////////////////////////////////////////////////////
//
//  class core
//

// Static member variable.
const uint core::initCoreSize = ALLOC_STEP / 8;
const uint core::maxCoreSize = HEAPSTRIDE - ALLOC_STEP;

#define USE_REALLOC

core::core(const string &s, uint is) : name(s)
{
#ifdef USE_REALLOC
	size = is;
	mem = (cell *)malloc(size * sizeof(cell));
#else
	mem = new cell[size = is];
#endif
}

core::~core()
{
#ifdef USE_REALLOC
	free(mem);
#else
	delete[] mem;
#endif
}

void core::Resize(uint newsize)
{
	P1("core::Resize");
#ifndef NOMEMCHK
//	cout << "size=" << size << ", newsize=" << newsize << endl;
	if( newsize > maxCoreSize )
	{
		if( size >= maxCoreSize )
		{
			RUNERR("Internal error: Memory address exceeded [" << name << "]");
			exit(1);
		}
		else
			newsize = maxCoreSize;

//		cout << "fixed:size=" << size << ", newsize=" << newsize << endl;
	}
#endif
	if( newsize <= size ) // Core reducing is detected
	{
#if defined(DEBUG) || defined(DOASSERT)
		ASSERT(1);        // print error in debug mode
#else
		return;           // do nothing otherwise
#endif
	}

#ifdef USE_REALLOC
	cell *newmem = (cell *)realloc(mem, newsize * sizeof(cell));
	if( newmem == NULL )
	{
		RUNERR("Cannot obtain memory for heap [" << name << "]");
		exit(1);
	}
	mem = newmem;
	size = newsize;

#else
	cell *newmem = new cell[newsize];
	memcpy(newmem, mem, size * sizeof(cell));
	delete[] mem;
	mem = newmem;
	size = newsize;
#endif
	return;
}

////////////////////////////////////////////////////////////////////////////
//
//  class machine
//

// machine::code2func is on code.cpp

core_p machine::FollowFeature(core_p cp, const feature *f)
{
	if( cp == CORE_P_INVALID || f == NULL )
		return CORE_P_INVALID;

#ifdef DEBUG
	if( DEB )
		cout << "FollowFeature " << hex << cp2int(cp) << dec << ", " << f->GetName() << endl;
#endif

	fserial	fSN = f->GetSerialNo();

	cp = Deref(cp);
	cell c = ReadHeap(cp);
	if( !IsSTRorVAR(c) )
	{
		return CORE_P_INVALID;		// Fail to unify
	}

	tserial tSN = c2STRSorVARS(c);

	if( coerceData(f->GetBaseTypeS(),tSN).result_s != tSN )
	{
		bool r = CoerceType(cp, f->GetBaseType());
		if( r == false )
			return CORE_P_INVALID;
		cp = Deref(cp);
		c = ReadHeap(cp);
		tSN = c2STRSorVARS(ReadHeap(cp));
	}

	featcode &p = featureCode[tSN][fSN];

	if( IsSTR(c) )
	{
		return cp + featureCode[tSN][fSN].index;
	}
	else //if( IsVAR(c) )// case of VAR
	{
//		if( p.index == S_INVALID )
//		{
//			return CORE_P_INVALID;		// Fail to unify
//		}
		WriteHeap(cp, PTR2c(HP));
		core_p ret = HP + p.index;
		const type *r = type::Serial(p.result);
		CheckHeapLimit(HP+1+r->GetNFeatures());
		PutHeap(STR2c(p.result));
		for( int i=0; i<r->GetNFeatures(); i++ )
			PutHeap(VAR2c(r->GetAppTypeS(i)));

#ifdef DEBUG
		if( DEB )
			Dump();
#endif
		return ret;
	}
//	else
//	{
//		return CORE_P_INVALID;
//	}
}

// OBSOLETE
bool machine::CoerceType(core_p cp, const type *t)
{
	if( cp == CORE_P_INVALID || t == NULL )
		return false;

#ifdef DEBUG
	if( DEB )
		cout << "CoerceType " << hex << cp2int(cp) << dec << ", " << t->GetName() << endl;
#endif

	StartUnifProf();
	tserial tSN = t->GetSerialNo();
	core_p ha = Deref(cp);
	cell c = ReadHeap(ha);
	tserial tSN2 = c2STRSorVARS(c);
	const coercode &cc = coerceData(tSN2,tSN);

	if( !ConstraintEnabled )
	{
		if( IsSTR(c) )
		{
			UIP = NULL;             // Return Address
			TRACE("call to coerceData(" << c2STR(c)->GetName() << "," << type::Serial(tSN)->GetName() << ")");
			HU1 = ha;                                       // Saved for afterworks
			core_p HAback = HA;
			HA = 0;
			code * oldIP = IP;
                        core_p oldOP = OP;   // in order to disable trail back (2004/10/8)
                        OP = 0;              // in order to disable trail back (2004/10/8)
			Execute(cc.coercion);
                        OP = oldOP;          // in order to disable trail back (2004/10/8)
			IP = oldIP;
			if( HA == CORE_P_INVALID )
			{
				HA=HAback;
				return false;
			}
			HA=HAback;
			return true;
		}
		else if( IsVAR(c) )
		{
			ushort result = cc.result_s;
			if( result == S_INVALID )
			{
				StopUnifProf();
				return false;
			}
			else
				WriteHeap(ha, VAR2c(result));
			StopUnifProf();
			return true;
		}
	}
	else
	{
		if( IsSTR(c) )
		{
			core_p HAback = HA;	HA = 0;
			code * oldIP = IP;	IP = 0;
			core_p cutp = GetCutPoint();
			SetTrailPoint();

			HU1 = ha;
			UIP = 0;

			Execute(cc.coercion);

			if( HA == CORE_P_INVALID )
			{
				TrailBack();
				IP = oldIP;
				HA=HAback;
				return false;
			}
			DoCut(cutp);
			HA=HAback;
			IP=oldIP;
			return true;
		}
		else if( IsVAR(c) )
		{
			ushort result = cc.result_s;
			if( result == S_INVALID )
			{
				StopUnifProf();
				return false;
			}
			else
			{
				WriteHeap(ha, VAR2c(result)); 
				if( coerceData(tSN2,result).constraint )
				{
					core_p HAback = HA;	HA = 0;
					code * oldIP = IP;	IP = 0;
					core_p cutp = GetCutPoint();
					SetTrailPoint();

					HU1 = ha;
					UIP = 0;

					Execute(coerceData(tSN2,result).constraint);
					if( HA == CORE_P_INVALID )
					{
						TrailBack();
						IP = oldIP;
						HA=HAback;
						StopUnifProf();
						return false;
					}
					DoCut(cutp);
					HA=HAback;
					IP=oldIP;
				}
			}
			StopUnifProf();
			return true;
		}
	}
	{
		if( t == bot ||
			( IsINT(c) && t == t_int) ||
			( IsFLO(c) && t == t_flo) ||
			( IsCHR(c) && t == t_chr) ||
			( IsSTG(c) && t == t_stg) )
			{
				StopUnifProf();
				return true;
			}
		StopUnifProf();
		return false;
	}
}

bool machine::Unify(core_p cp1, core_p cp2)
{
	if( cp1 == CORE_P_INVALID || cp2 == CORE_P_INVALID )
		return false;

#ifdef DEBUG
	if( DEB )
		cout << "Unify " << hex << cp2int(cp1) << ", " << cp2int(cp2) << dec << endl;
#endif

	core_p HAback = HA;
	HA = 0;
	code * oldIP = IP;
	IP = 0;

	core_p cutp = GetCutPoint();
	SetTrailPoint();

	HU1 = cp1;
	HU2 = cp2;
	UIP = 0;
	BeginUnify();	// Initializing machine...
	if( IP != NULL )
		ExecLoop();

	if( HA == CORE_P_INVALID )
	{
		TrailBack();
		IP = oldIP;
		HA=HAback;
		return false;
	}
	DoCut(cutp);
	HA=HAback;
	IP=oldIP;
#ifdef DEBUG
	if( DEB )
		Dump();
#endif
	return true;
}

#if 0
void machine::ClearHeap()
{
	HB = int2cp(0);
	HP = int2cp(0);
	HA = CORE_P_INVALID;
	SP = int2cp(STACK_BASE);
	FP = int2cp(STACK_BASE);
	RP = 0;
#ifdef TRAIL
	TP = int2cp(0);
	OP = CORE_P_INVALID;
#endif
}
#endif

bool machine::Execute(code *cp)
{
  //try {
	P2("Execute");
	code *reserveIP = IP;
	code *reserveCIP = CIP;
	code *reserveUIP = UIP;
	IP = cp;
	CIP = NULL;
	UIP = NULL;
	ExecLoop();
	IP = reserveIP;
	CIP = reserveCIP;
	UIP = reserveUIP;
	return (HA != CORE_P_INVALID);
  //} catch (...) {	cout << "THROW machine::Execute" << endl; throw; }
}	

void machine::Fail()
{
	P5("machine::Fail");
#ifdef TRAIL
	if( OP == 0 || OP == CORE_P_INVALID )
	{
		TRACE(" Totally failed (No more choice-frame); go back to top-level");
		HA = CORE_P_INVALID; // tell caller that the operation has been failed
		IP = 0;
		return;
	}

#ifdef PROF_UNIFY
	StopUnifProf();
	profiler::DisableProfiling();
	profuser *p;
	p = profuser::Search("Static Unify");
	if( p != NULL )
		delete p;
	p = profuser::Search("Static Unify (caller arg)");
	if( p != NULL )
		delete p;
	p = profuser::Search("Static Unify (callee arg)");
	if( p != NULL )
		delete p;
	profiler::EnableProfiling();
#endif

	IP = ReadStackIP(OP+CF_RETP_OFFSET) ;

	if( IP == 0 )
	{
		TRACE(" Totally failed (Next TRY address is NULL); go back to top-level");
		HA = CORE_P_INVALID; // tell caller that the operation has been failed
		return;
	}
	
	TRACE(" Resume from " << hex << (ptr_int)IP << dec << "...");

	ASSERT( *IP == C_RETRY || *IP == C_LASTTRY || *IP == C_RETRYAT || *IP == C_LASTAT );

#else
	ABORT("The instruction is not supported");
#endif
}

//////////////////////////////////////////////////////////////////////////////
// heap managing functions

void machine::ClearHeap(int slot)
{
	WriteHeapDirect(MakeCoreP(slot, int2core_p(0)), PTR2c(MakeCoreP(slot, int2core_p(0)+1)));
  /// ??? slotで表されるスロットの先頭のセルに、次のセル(?)を指すcellオブジェクトを代入
	if( slot == GetSlot(HP) )
		HP = MakeCoreP(slot, int2core_p(0)+1);
	if( slot == GetSlot(HA) )
	{
		HA = CORE_P_INVALID;
	}
}

void machine::GoToHeap(int slot)
{
	WriteHeapDirect(MakeCoreP(GetSlot(HP), 0), PTR2c(HP));
	HP = c2PTR(ReadHeap(MakeCoreP(slot, int2core_p(0))));
#ifdef N1
	n1currentslot = GetSlot(HP);
#endif	
}
/*
int machine::StartNewHeap()
{
	int newslot = GetCurrentSlot()+1;

//	ASSERT(newslot < NHEAP-1);
	if( newslot >= NHEAP-1 )
	{
		ABORT("Number of heaps exceeded");
	}

	ClearHeap(newslot);
	GoToHeap(newslot);

	return newslot;
}
*/
void machine::EndTheHeap(int slot)
{
	int currentslot = slot-1;
	HP = c2PTR(ReadHeap(MakeCoreP(currentslot, 0)));
#ifdef N1
	n1currentslot = GetSlot(HP);
#endif
}


core_p machine::SwitchNewHeap() 
{ 
//	cout << "SwitchNewHeap" << endl;
//	cout << hex << cell2int(PTR2c(HP)) << dec << endl;
//	cout << GetSlot(HP) << endl;
	WriteHeapDirect(MakeCoreP(GetSlot(HP), 0), PTR2c(HP));
//	cout << hex << cell2int(PTR2c(HP)) << dec << endl;
//	cout << GetSlot(HP) << endl;
	HP = c2PTR(ReadHeap(MakeCoreP(3 - GetSlot(HP), 0))); 
//	cout << hex << cell2int(PTR2c(HP)) << dec << endl;
//	cout << GetSlot(HP) << endl;
//	cout << "SwitchNewHeap end" << endl;
	return HP; 
}

void machine::SwitchOldHeap(core_p old) 
{ 
//	cout << "SwitchOldHeap" << endl;
//	cout << hex << cell2int(PTR2c(old)) << dec << endl;
//	cout << GetSlot(old) << endl;
	WriteHeapDirect(MakeCoreP(GetSlot(old), 0), PTR2c(old));
	HP = ReadHeap(MakeCoreP(3 - GetSlot(HP), 0)); 
//	cout << hex << cell2int(PTR2c(HP)) << dec << endl;
//	cout << GetSlot(HP) << endl;
//	cout << "SwitchNewHeap end" << endl;
}

/*
core_p machine::ReplaceFS(core_p fp)
{
	FSP fs(this, fp);

	FSP copied = fs.Copy();

	return copied.GetAddr();
}
*/

void machine::DoCut(core_p cp)
{ 
#ifdef DEBUG
	if( DEB )
          cout << "DoCut cp= " << hex << cp2int(cp) << dec << endl;
#endif
	if( cp != CORE_P_INVALID )
	{
		HB = c2PTR   (ReadStack  (cp+CF_HP_OFFSET         ));
#ifdef N1
		n1currenthbslot = GetSlot(HB);
#endif
#ifdef DEBUG
	if( DEB )
		cout << hex << "SP= " << cp2int(SP) << "  OPx=" << cp2int(cp_plus(OP+CF_LOCAL_OFFSET, c2PTR(ReadStack(OP+CF_RP_OFFSET)))) << "  cpx= " << cp2int(cp_plus(cp+CF_LOCAL_OFFSET, c2PTR(ReadStack(cp+CF_RP_OFFSET)))) << dec<< endl;
#endif
		if( SP == cp_plus(OP+CF_LOCAL_OFFSET, c2PTR(ReadStack(OP+CF_RP_OFFSET))))
			SP = OP;
	}
	else
	{
#ifdef DEBUG
	if( DEB )
		cout << hex << "SP= " << cp2int(SP) << "  OPx=" << cp2int(cp_plus(OP+CF_LOCAL_OFFSET, c2PTR(ReadStack(OP+CF_RP_OFFSET)))) << endl;
#endif
		HB = MakeCoreP(GetSlot(HP), 1);
#ifdef N1
		n1currenthbslot = GetSlot(HB);
#endif
		if( SP == cp_plus(OP+CF_LOCAL_OFFSET, c2PTR(ReadStack(OP+CF_RP_OFFSET))))
			SP = OP;
	}
	OP = cp;
}


bool machine::call(FSP arg1)
{
	code *reserveIP = IP;
	IP = NULL;
	const type *t = arg1.GetType();

	int nf = t->GetNFeatures();
	SetRP(nf);

	if( arg1.IsLeaf() )
	{
		const type *tt = t->TypeUnify(t_pred[0]);
		if( tt != NULL && tt != t )
			t = tt;
		for( int i=0; i<nf; i++ )
			WriteLocal(i, PTR2c(FSP(this).GetAddress()));
	}
	else
	{
		for( int i=0; i<nf; i++ )
			WriteLocal(i, PTR2c(arg1.FollowNth(i).GetAddress()));
	}

	procedure *pc = procedure::Search(t);
	if( pc == NULL )
	{
		RUNWARN("Non-defined procedure called in call/1: '" << t->GetName() << "'"); // by mitsuisi (1999.04.27)
		return false;
	}
	procthunk *pt = pc->GetThunk();
	ASSERT( pt != NULL );

	pt->exec(*this);
	bool ret = true;
	if( IP != NULL )
		ret = Execute(IP);
	IP = reserveIP;
	return ret;
}	

FSP machine::eval(const string &s)
{
	lilfes::eval(this, s, false);
	return prog_list_top;
}

} // namespace lilfes

