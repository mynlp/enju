/* $Id: machine2.cpp,v 1.6 2011-05-02 10:38:23 matuzaki Exp $ 
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "machine.h"
#include "proc.h"
#include "structur.h"
#include "codelist.h"
#include "builtin.h"
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::ostringstream;
using std::setw;
using std::string;



static const char rcsid[] = "$Id: machine2.cpp,v 1.6 2011-05-02 10:38:23 matuzaki Exp $";

machine::machine()
	: local("local", 1024)
{
	for( int i=0; i<NHEAP-1; i++ )
	{
		ostringstream oss;
		oss << "heap " << i;
		heap[i].SetName(oss.str().c_str());
	}
	heap[NHEAP-1].SetName("stack");
	trail.SetName("trail");
	
	
	HB = MakeCoreP(1, 0);
	HP = MakeCoreP(1, 1);
	HA = CORE_P_INVALID;
	HU1 = CORE_P_INVALID;
	HU2 = CORE_P_INVALID;
	SP = int2cp(STACK_BASE);
	FP = int2cp(0);
	RP = int2cp(0);
#ifdef TRAIL
	TP = int2cp(0);
	OP = CORE_P_INVALID;
	B0 = CORE_P_INVALID;
#endif
	IP = NULL;
	CIP = NULL;
	for( int i=0; i<1024; i++ )
		local[i] = VAR2c(bot);
	CheckHeapLimit(HP);
	ClearHeap(0);
	WriteHeap(MakeCoreP(1, 0), PTR2c(HP));
	WriteHeap(MakeCoreP(2, 0), PTR2c(MakeCoreP(2, 1)));
#ifdef N1
	n1currentslot = GetSlot(HP);
	n1currenthbslot = GetSlot(HB);
#endif
        ConstraintEnabled = true;
}

machine::~machine()
{
	
}

void machine::TraceOutput( int tt, const type *t, int x )
{
	const string s = t->GetName();
	
	cout << "TRACE: "
		 << ( tt==0 ? "ENTER" :
		      tt==1 ? "SUCCESS" :
		      tt==2 ? "REENTER" :
		      tt==3 ? "FAIL" : "UNKNOWN" )
		 << " " << s;
	if( tt != 3 )
		cout << "(" << (x+1) << ")";
	cout << endl;
	procedure *p = procedure::Search(t);
	ASSERT( p != NULL );
	int arity = p->GetArity();

	outform of;

        for( int i=0; i<arity; i++ ) {
	    of.AddAtRight(FSP(this, c2PTR(ReadLocal(i))).DisplayAVM());
	    if( i != arity-1 )
	        of.AddAtRight(outform(", "));
        }
	cout << of;
}

codelist * machine::Interpret(FSPointer f)
{ 
	codelist *cl = new codelist;
	REC(*cl);
	cl->AddCode(C_ALLOC);
	cl->AddCode((ushort)1);
	cl->AddCode((ushort)15);
	cl->AddCode(C_GETLEVEL, (ushort)-1);
	codelist *c = Interpret_internal(f.GetAddress());
	cl->AddCodelist(c);
	cl->AddCode(C_DEALLOC);
	cl->AddCode(C_RETURN);
	delete c;
	return cl; 
}

codelist * machine::Interpret_internal(core_p lst) 
{
	codelist *cl = new codelist;
	const type* c;
	int i;
	core_p h = Deref(lst);

	if( !IsSTR(ReadHeap(h)) && !IsVAR(ReadHeap(h)) )
	{
		cout << "strange" << endl;
		return cl;	
	}
	c = c2STRorVAR(ReadHeap(h));
	if( c == t_comma )
	{
		codelist *l1 = Interpret_internal(Deref(lst)+1);
		codelist *l2 = Interpret_internal(Deref(lst)+2);
		cl->AddCodelist(l1);
		cl->AddCodelist(l2);
		delete l1;
		delete l2;
	}
	else if( c == t_cut )
	{
		cl->AddCode(C_DEEPCUT, (varno)-1);
	}
	else
	{
		for (i = 0; i < c->GetNFeatures(); i++) 
		{
			REC(*cl);
			cl->AddCode(C_SETREG,(varno) i,PTR2c(h+i+1));
		}
		REC(*cl);
		cl->AddCode(C_CALL,procedure::New(c,c->GetNFeatures()));
		cl->AddCode(C_CLRTEMP,(ushort)15);
	}

	return cl;
}

void machine::DisplayStatistics()
{
	cout << "; (Current occupation" 
	     ": " << cp2int(GetOffset(HP)) << " heap" 
	     ", " << cp2int(GetOffset(SP))  << " stack" 
	     ", " << ( cp2int(TP) ) << " trail" 
		 " cell(s)"
#if defined(DEBUG) || defined(DOASSERT) || defined(DOPASS)
		 ", " << PASSCOUNT << " instructions"
#endif 
		 ")" << endl;
}

outform core::OutputCore (int start, int end)
{
	outform ret;
	for( int i=start; i<end; i++)
	{
                ostringstream oss;
		oss << setw(5) << hex << i << dec;
		oss << "  " << (*this)[i];
		ret.AddAtBottom(outform(oss.str().c_str()));
	}
	return ret;
}

outform core::OutputStack (int start, int end, int fp, int op)
{
	outform ret;
	for( int i=end-1; i>=start; i--)
	{
                ostringstream oss;
		if( i >= op && i < op+CF_LOCAL_OFFSET )
		{
			switch( i - op )
			{
				case CF_OP_OFFSET:		oss << "OP-->"; break;
				case CF_RETP_OFFSET:	oss << " RET>"; break;
				case CF_CIP_OFFSET:		oss << " CIP>"; break;
				case CF_FP_OFFSET:		oss << " FP->"; break;
				case CF_TP_OFFSET:		oss << " TP->"; break;
				case CF_HA_OFFSET:		oss << " HA->"; break;
				case CF_HP_OFFSET:		oss << " HP->"; break;
				case CF_B0_OFFSET:		oss << " B0->"; break;
				case CF_RP_OFFSET:		oss << " RP->"; break;
				default:                oss << "     "; break;
			}
		}
		else if( i == fp )
			oss << "FP-->";
		else
			oss << setw(5) << hex << i << dec;
		oss << "  " << setw(8) << hex << cell2int((*this)[i]) << dec;
		ret.AddAtBottom(outform(oss.str().c_str()));
	}
	return ret;
}

void machine::Dump()
{
	P1("machine::Dump()");
	cout << hex
	     <<   "HP=" << setw(8) << cp2int(HP) 
	     << ", HA=" << setw(8) << cp2int(HA)
//	     << ",HA2=" << setw(8) << cp2int(HA2)
	     << ",HU1=" << setw(8) << cp2int(HU1)
	     << ",HU2=" << setw(8) << cp2int(HU2)
	     << ", HB=" << setw(8) << cp2int(HB)
	     << endl
	     <<   "SP=" << setw(8) << cp2int(SP)
#ifdef TRAIL
	     << ", OP=" << setw(8) << cp2int(OP)
	     << ", TP=" << setw(8) << cp2int(TP)
	     << ", B0=" << setw(8) << cp2int(B0)
#endif
	     << ", FP=" << setw(8) << cp2int(FP)
	     << ", RP=" << setw(8) << cp2int(RP)
	     << endl
	     <<   "IP=" << setw(16) << reinterpret_cast<ptr_int>(IP)
	     << ", CIP=" << setw(16) << reinterpret_cast<ptr_int>(CIP)
	     << ", UIP=" << setw(16) << reinterpret_cast<ptr_int>(UIP)
	     << "." << dec << endl;
	
#if 0
//	outform hpout  ( "     PREV     " );
//	hpout.AddAtBottom(heap[GetSlot(HP)-1].OutputCore(0, cp2int(1+GetOffset(heap[GetSlot(HP)-1][0]))));
	outform /*n*/hpout  ( "     HEAP     " );
	/*n*/hpout.AddAtBottom(heap[GetSlot(HP)].OutputCore(0, cp2int(GetOffset(HP))));
	outform trout  ( "     TRAIL    " );
	trout.AddAtBottom(trail.OutputCore(0, (cp2int(TP))));
	outform stout  ( "     STACK    " );
	stout.AddAtBottom(STACK.OutputStack(0, (cp2int(SP)-STACK_BASE), (cp2int(FP)-STACK_BASE), (cp2int(OP)-STACK_BASE)));
	outform reout  ( "     LOCAL    " );
	reout.AddAtBottom(local.OutputCore(0, (cp2int(RP)+8)));
	
// 	hpout.AddAtRightTop(nhpout);
 	hpout.AddAtRightTop(trout);
	hpout.AddAtRightTop(stout);
	hpout.AddAtRightTop(reout);
	cout << hpout;
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// PrettyPrint functions
//


static code interrupted[] = {
	C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED,
	C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED,
	C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED,
	C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED, C_INTERRUPTED };

void machine::Interrupt()
{
	IP = interrupted;
	CIP = interrupted;
	UIP = interrupted;
	OP = CORE_P_INVALID;
	B0 = CORE_P_INVALID;

	SetTrailPoint(interrupted);
	
	SP = int2cp(STACK_BASE);

				core_p NewFP = SP+CODESIZEONSTACK;
				core_p NewSP = NewFP+1;
				CheckStackLimit(NewSP);
				
				WriteStackIP(SP,  interrupted);
				WriteStack(NewFP, PTR2c(FP));
				FP = NewFP;

	RP = int2cp(0);
	queue.clear();
}

} // namespace lilfes
