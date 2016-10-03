/* $Id: code2.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: code2.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $";

#include "code.h"
#include "builtin.h"
#include "machine.h"
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <ostream>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::pair;


// Macro to combine two tags. This is used in switch statements
// where the conditional branch is necessary for tags of two cells.

#define CT(t1,t2) ( ((t1)<<TAG_WIDTH) + (t2) )

// Global variables.

const cell botcell = VAR2c((tserial)0);
//static cell botcell;
static cell intcell;
static cell stgcell;
static cell flocell;
const tserial botserial = 0;
//static tserial botserial;
static tserial intserial;
static tserial stgserial;
static tserial floserial;


#ifdef DEBUG
static cell unknown;
#endif

static void init() 
{
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
//	botcell = VAR2c(bot);
	if( botcell != VAR2c(bot) )
	{
		ABORT( "INTERNAL ERROR!! bot is not serial 0" );
	}
	intcell = VAR2c(t_int);
	stgcell = VAR2c(t_stg);
	flocell = VAR2c(t_flo);
//	botserial = bot->GetSerialNo();
	intserial = t_int->GetSerialNo();
	stgserial = t_stg->GetSerialNo();
	floserial = t_flo->GetSerialNo();
#ifdef DEBUG
	unknown = VAR2c(new type("unknown", module::CoreModule()));
#endif
}
static InitFunction IF(init, 65);

// class machine

void machine::ExpandStructure(tserial result)
{
	const type *r = type::Serial(result);

// Note: Another routine is used in C_FEATADD

//	if( r->IsFeatureConstrained() )
//	{
//		UIP = IP;
//		IP = coerceData(0,result)->coercion;
//	}
//	else
//	{
		CheckHeapLimit(HP+1+r->GetNFeatures());
		PutHeap(STR2c(result));
		for( int i=0; i<r->GetNFeatures(); i++ )
			PutHeap(VAR2c(r->GetAppTypeS(i)));
//	}
}

void machine::Proceed()
{
	P5("machine::Proceed");
	GetUnifyQueue(HU1, HU2);
	if( HU1 != CORE_P_INVALID )		// if there are some unification to do
	{
		TRACE(" ... Unification with " << hex << (cp2int(HU1)) << " and " << (cp2int(HU2)) << dec );
		BeginUnify();				//   go to next unify
	}
	else
	{
//		HA = HA2;					// restore HA
		IP = UIP;
		TRACE(" ... Proceed(); Continue execution " << hex << reinterpret_cast<ptr_int>(UIP) << dec );
	}
}

#ifndef N1
#ifndef INLINE_WRITEHEAP
void machine::WriteHeap(core_p hp, cell c)
{
	P5("machine::WriteHeap");

	if( GetSlot(hp) != GetSlot(HB) || hp < HB )
	{
		CheckTrailLimit(TP+2);
		trail[TP++] = PTR2c(hp);
		trail[TP++] = ReadHeap(hp);
	}
	WriteHeapDirect(hp, c);
}
#endif
#endif
//////////////////////////////////////////////////////////////////////////////
//
// Unification functions
//

inline void machine::PutUnifyQueue(core_p u1, core_p u2)
{
	ASSERT(u1 != CORE_P_INVALID);
	ASSERT(u2 != CORE_P_INVALID);
	queue.push_back(u1);
	queue.push_back(u2);
}

inline void machine::PutUnifyQueueType(core_p u1, tserial t1)
{
	ASSERT(u1 != CORE_P_INVALID);
	queue.push_back(u1);
	queue.push_back( int2core_p(cell2int(VAR2c(t1))) );
}

inline void machine::GetUnifyQueue(core_p &u1, core_p &u2)
{
	if (queue.empty())
	{
		u1 = CORE_P_INVALID;
	}
	else
	{
		u1 = queue.front();
		queue.pop_front();
	}

	if (queue.empty())
	{
		u2 = CORE_P_INVALID;
	}
	else
	{
		u2 = queue.front();
		queue.pop_front();
	}
}

#ifdef N1
#define N0WHEAP(hp,c,cb) if (hp < HB || GetSlot(hp) != GetSlot(HB)) \
				{ CheckTrailLimit(TP+2);trail[TP++] = PTR2c(hp);trail[TP++] = cb; WriteHeapDirect(hp,c); } \
				else WriteHeapDirect(hp,c)
#define N0WRITEHEAP(hp,c) { if (hp < HB || GetSlot(hp) != GetSlot(HB)) \
				{ CheckTrailLimit(TP+2);trail[TP++] = PTR2c(hp);trail[TP++] = ReadHeap(hp); } \
				WriteHeapDirect(hp,c); }
#define N0PUTUNIFYQ(adr1,adr2) if (queue.iswritable()) queue.enqueue(adr1,adr2); else ABORT("Sorry, UNIFQ Overflow")
#define N0GETUNIFYQ(adr1,adr2) if (!queue.isempty())				\
					{ adr1 = queue.dequeue(); adr2 = queue.dequeue(); } \
					else adr1 = adr2 = CORE_P_INVALID
#define N0PUTHEAP(hpslot,hpoffset,val) heap[hpslot][hpoffset++] = val; HP++
#endif
#ifndef N1
//#define DEBUG
//#define DEB 1
//#undef TRACE
//#define TRACE(X) cout << X << endl

////////////////////////////////////////////////////////////////////////////////////////
void machine::BeginUnify()
{ // Jump to the code of unification with HA and HA2
#ifdef DEBUG
	if( DEB )
		cout << "BeginUnify  ";
#endif
	StartUnifProf();

TOP:
#ifdef PROFILE
#if PROFILE >= 3
	prof *_p = new prof("machine::BeginUnify");
#define PROFEND delete _p
#else
#define PROFEND
#endif
#else
#define PROFEND
#endif
	cell c1, c2;

	ASSERT(HU1 != CORE_P_INVALID);
	ASSERT(HU2 != CORE_P_INVALID);

	HU1 = Deref(HU1);
	if( IsVAR(PTR2c(HU2)) )
	{
		// Coercion!!
		c1 = ReadHeap(HU1);
		c2 = PTR2c(HU2);

#ifdef DEBUG
	if( DEB )
		cout << ">[" << hex << cell2int(c1) << "]+[" << cell2int(c2) << dec << "]" << endl;
#endif

		switch( Tag(c1) )
		{
			case T_VAR:
				tserial result;
				{
					P4("machine::BeginUnify VAR-VAR");
					TRACE( "  VAR[" << c2VAR(c1)->GetName() << "], (VAR)[" << c2VAR(c2)->GetName() << "]");
					result = coerceData(c2VARS(c1),c2VARS(c2)).result_s;
				}
				if( result == S_INVALID )
				{
	                                goto FAIL;
	// 				PROFEND;
	// 				Fail();
	// 				return;
				}
				else {
					WriteHeap( HU1, VAR2c(result) );
#ifdef XCONSTR
					IP = coerceData(c2VARS(c1),result).constraint;
					if( IP == NULL )
						goto PROCEED;
					else
					{
						TRACE("Constraint code exists... 1");
						break;
					}
#else
					goto PROCEED;
#endif
				}
			case T_STR:
				TRACE( "  STR[" << c2STR(c1)->GetName() << "], (VAR)[" << c2VAR(c2)->GetName() << "]");
				IP = coerceData(c2STRS(c1),c2VARS(c2)).coercion;
	#ifdef LILFES_U
	                        if( *IP == C_FAIL )
	                             goto FAIL;
	#endif
				TRACE("call to coerceData(" << c2STR(c1)->GetName() << "," << c2VAR(c2)->GetName() << ")");
				break;
			case T_INT:
				if( c2 != botcell && c2 != intcell )
					goto FAIL;
				goto PROCEED;
			case T_FLO:
				if( c2 != botcell && c2 != flocell )
					goto FAIL;
				goto PROCEED;
			case T_STG:
				if( c2 != botcell && c2 != stgcell )
					goto FAIL;
				goto PROCEED;
			default:
				TRACE(" ... fail");
				goto FAIL;
		}
	}
	else
	{
		HU2 = Deref(HU2);

		if( HU1 == HU2 )
			goto PROCEED;

		c1 = ReadHeap(HU1);
		c2 = ReadHeap(HU2);

#ifdef DEBUG
	if( DEB )
		cout << ")[" << hex << cell2int(c1) << "]+[" << cell2int(c2) << dec << "]" << endl;
#endif

		switch( CT(Tag(c1), Tag(c2)) )
		{
			case CT(T_VAR, T_VAR) :
			{
				WriteHeap( HU2, PTR2c(HU1) );
//			COERCE_VAR:
				unsigned short result;
				TRACE( "  VAR[" << c2VAR(c1)->GetName() << "], VAR[" << c2VAR(c2)->GetName() << "]");
#ifdef TYPE_TABLE_HASH
				const coercode *coer = coerceRef(c2VARS(c2),c2VARS(c1));
				if( coer == NULL )
					goto FAIL;
				else 
					result = coer->result_s;
#else
				result = coerceData(c2VARS(c2),c2VARS(c1)).result_s;
				if( result == S_INVALID )
				{
	                                goto FAIL;
				}
#endif
				if( c1 != result )	// This prevents executing VAR+ADDNEW constraint code
				{
					WriteHeap( HU1, VAR2c(result) );
#ifdef XCONSTR
					IP = coerceData(c2VARS(c2),c2VARS(c1)).constraint;
					if( IP == NULL )
						goto PROCEED;
					else
					{
						TRACE("Constraint code exists... 2");
						break;
					}
#else
					goto PROCEED;
#endif
				}
				else
				{
					goto PROCEED;
				}
			}
			break;
			case CT(T_VAR, T_STR) :
			{
				P4("machine::BeginUnify VAR-STR");
				TRACE( "  VAR[" << c2VAR(c1)->GetName() << "], STR[" << c2STR(c2)->GetName() << "]");
#ifdef TYPE_TABLE_HASH
				const coercode *coer = coerceRef(c2STRS(c2),c2VARS(c1));
				if( coer == NULL )
					goto FAIL;
				else 
					IP = coer->coercion_var;
#else
				IP = coerceData(c2STRS(c2),c2VARS(c1)).coercion_var;
#endif
	#ifdef LILFES_U
	                        if( *IP == C_FAIL )
	                             goto FAIL;
	#endif
				WriteHeap( HU1, PTR2c(HU2) );
				HU1 = HU2;	// HU1 is a address to coerce 
				TRACE("call to coerceData(" << c2STR(c2)->GetName() << "," << c2VAR(c1)->GetName() << ")");
				break;
			}
			case CT(T_STR, T_VAR) :
			{
				P4("machine::BeginUnify STR-VAR");
				TRACE( "  STR[" << c2STR(c1)->GetName() << "], VAR[" << c2VAR(c2)->GetName() << "]");
				WriteHeap( HU2, PTR2c(HU1) );
				//HU1 = HU1;	// HU1 is a address to coerce 
#ifdef TYPE_TABLE_HASH
				const coercode *coer = coerceRef(c2STRS(c1),c2VARS(c2));
				if( coer == NULL )
					goto FAIL;
				else 
					IP = coer->coercion_var;
#else
				IP = coerceData(c2STRS(c1),c2VARS(c2)).coercion_var;
#endif
	#ifdef LILFES_U
	                        if( *IP == C_FAIL )
	                             goto FAIL;
	#endif
				TRACE("call to coerceData(" << c2STR(c1)->GetName() << "," << c2VAR(c2)->GetName() << ")");
				break;
			}
			case CT(T_STR, T_STR) :
			{
				P4("machine::BeginUnify STR-STR");
				TRACE( "  STR[" << c2STR(c1)->GetName() << "], STR[" << c2STR(c2)->GetName() << "]");
#ifdef TYPE_TABLE_HASH
				const coercode *coer = coerceRef(c2STRS(c1),c2STRS(c2));
				if( coer == NULL )
					goto FAIL;
				else 
					IP = coer->unification;
#else
				IP = coerceData(c2STRS(c1),c2STRS(c2)).unification;
#endif
	#ifdef LILFES_U
	                        if( *IP == C_FAIL )
	                             goto FAIL;
	#endif
				TRACE("call to unifyCode[" << c2STR(c1)->GetName() << "][" << c2STR(c2)->GetName() << "]");
				break;
			}
			case CT(T_VAR, T_INT) :
			{
				if( c1 != botcell && c1 != intcell )
					goto FAIL;
				WriteHeap(HU1, PTR2c(HU2));
				goto PROCEED;
			}
			case CT(T_INT, T_VAR) :
			{
				if( c2 != botcell && c2 != intcell )
					goto FAIL;
				WriteHeap(HU2, PTR2c(HU1));
				goto PROCEED;
			}
			case CT(T_VAR, T_FLO) :
			{
				if( c1 != botcell && c1 != flocell )
					goto FAIL;
				WriteHeap(HU1, PTR2c(HU2));
				goto PROCEED;
			}
			case CT(T_FLO, T_VAR) :
			{
				if( c2 != botcell && c2 != flocell )
					goto FAIL;
				WriteHeap(HU2, PTR2c(HU1));
				goto PROCEED;
			}
#if 0
			case CT(T_VAR, T_CHR) :
			{
				if( c2VARS(c1) != bot->GetSerialNo() && c2VARS(c1) != t_chr->GetSerialNo() )
					goto FAIL;
				WriteHeap(HU1, PTR2c(HU2));
				goto PROCEED;
			}
			case CT(T_CHR, T_VAR) :
			{
				if( c2VARS(c2) != bot->GetSerialNo() && c2VARS(c2) != t_chr->GetSerialNo() )
					goto FAIL;
				WriteHeap(HU2, PTR2c(HU1));
				goto PROCEED;
			}
#endif
			case CT(T_VAR, T_STG) :
			{
				if( c1 != botcell && c1 != stgcell )
					goto FAIL;
				WriteHeap(HU1, PTR2c(HU2));
				goto PROCEED;
			}
			case CT(T_STG, T_VAR) :
			{
				if( c2 != botcell && c2 != stgcell )
					goto FAIL;
				WriteHeap(HU2, PTR2c(HU1));
				goto PROCEED;
			}
			case CT(T_INT, T_INT) :
			case CT(T_CHR, T_CHR) :
			case CT(T_FLO, T_FLO) :
			case CT(T_STG, T_STG) :
			{
				if( c1 != c2 )
					goto FAIL;
				WriteHeap(HU2, PTR2c(HU1));	// MAY BE SIMPLIFIED?
				goto PROCEED;
			}
			default:
			{
	FAIL:
	#ifdef LILFES_U
	                        FA1 = HU1;
	                        FA2 = HU2;
	#endif // LILFES_U
				PROFEND;
				Fail();
				StopUnifProf();
				return;
			}
		}
	}
	PROFEND;
	return;
	// contents of Proceed()
PROCEED:
	PROFEND;
	GetUnifyQueue(HU1, HU2);
	if( HU1 != CORE_P_INVALID )		// if there are some unification to do
	{
		ASSERT(HU2 != CORE_P_INVALID);
		TRACE(" ... Proceed (BeginUnify); Unification with " << hex << (cp2int(HU1)) << " and " << (cp2int(HU2)) << dec );
		goto TOP;
	}
	else
	{
//		HA = HA2;					// restore HA
		IP = UIP;
		StopUnifProf();
		TRACE(" ... Proceed (BeginUnify); Continue execution " << hex << reinterpret_cast<ptr_int>(UIP) << dec );
		// IP may be 0; the caller must check IP
	}
} // end of BeginUnify()

#undef PROFEND
#endif
//#undef DEBUG
//#undef TRACE
//#define TRACE(X) 

#ifdef TRAIL
core_p machine::SetTrailPoint(code *retp)
{
	P3("machine::SetTrailPoint");
	core_p NewOP = SP;
	core_p NewSP = cp_plus(NewOP+CF_LOCAL_OFFSET,RP);

	CheckStackLimit(NewSP);
	// Make Choice-Point Frame

	WriteStack  (NewOP+CF_OP_OFFSET  , PTR2c(OP));	// B
	WriteStackIP(NewOP+CF_RETP_OFFSET, retp);		// IP
	WriteStackIP(NewOP+CF_CIP_OFFSET , CIP);		// CP
	WriteStack  (NewOP+CF_FP_OFFSET  , PTR2c(FP));	// E
	WriteStack  (NewOP+CF_TP_OFFSET  , PTR2c(TP));	// TR (trail pointer)
	WriteStack  (NewOP+CF_HA_OFFSET  , PTR2c(HA));	// A0
	WriteStack  (NewOP+CF_HP_OFFSET  , PTR2c(HP));	// H
	WriteStack  (NewOP+CF_RP_OFFSET  , PTR2c(RP));	// n
	WriteStack  (NewOP+CF_B0_OFFSET  , PTR2c(B0));	// B0

	for( core_p i=0; i<RP; i++ )
		WriteStack  (cp_plus(NewOP+CF_LOCAL_OFFSET,i), ReadLocal(i));

	OP = NewOP;
	HB = HP;
#ifdef N1
	n1currenthbslot = GetSlot(HB);
#endif
	SP = NewSP;

	return OP;
}

void machine::TrailAgain(core_p cp, code *retp)
{
#ifdef DOPASS
	TRAILBACK++;
#endif
	P3("machine::TrailAgain");
	core_p NewTP;
	
	ASSERT(cp != CORE_P_INVALID);

	queue.clear();	// Clear Unification PDL in case of dynamic-unify
	ConstraintQueue.clear();

	CIP    =          ReadStackIP(cp+CF_CIP_OFFSET        ) ;
	FP     = c2PTR   (ReadStack  (cp+CF_FP_OFFSET         ));
	NewTP  = c2PTR   (ReadStack  (cp+CF_TP_OFFSET         ));
	HA     = c2PTR   (ReadStack  (cp+CF_HA_OFFSET         ));
	RP     = c2PTR   (ReadStack  (cp+CF_RP_OFFSET         ));
	B0     = c2PTR   (ReadStack  (cp+CF_B0_OFFSET         ));
	HP     = c2PTR   (ReadStack  (cp+CF_HP_OFFSET         ));	// == HB
#ifdef N1
	n1currentslot = GetSlot(HP);
#endif
	WriteStackIP(cp+CF_RETP_OFFSET, retp);			// IP

	for( core_p i=0; i<RP; i++ )
		WriteLocal(i, ReadStack  (cp_plus(cp+CF_LOCAL_OFFSET,i)));

	SP = cp_plus(cp+1+CF_LOCAL_OFFSET,RP);

	while( TP > NewTP )
	{
		TRACE(" Untrail....");
		TP -= 2;
		WriteHeapDirect( c2PTR(trail[TP]), trail[TP+1] );
	}
	OP = cp;
}

////////////
// Trail back to the choice point frame cp.
// IP is not restored.

void machine::TrailBack(core_p cp)
{
#ifdef DOPASS
	TRAILBACK++;
#endif
	P3("machine::TrailBack");
	core_p NewTP;

	ASSERT(cp != CORE_P_INVALID);

	queue.clear();	// Clear Unification PDL in case of dynamic-unify
	ConstraintQueue.clear();

	OP     = c2PTR   (ReadStack  (cp+CF_OP_OFFSET         ));
	CIP    =          ReadStackIP(cp+CF_CIP_OFFSET        ) ;
	FP     = c2PTR   (ReadStack  (cp+CF_FP_OFFSET         ));
	NewTP  = c2PTR   (ReadStack  (cp+CF_TP_OFFSET         ));
	HA     = c2PTR   (ReadStack  (cp+CF_HA_OFFSET         ));
	HP     = c2PTR   (ReadStack  (cp+CF_HP_OFFSET         ));	// == HB
	B0     = c2PTR   (ReadStack  (cp+CF_B0_OFFSET         ));
	RP     = c2PTR   (ReadStack  (cp+CF_RP_OFFSET         ));

#ifdef N1
	n1currentslot = GetSlot(HP);
#endif

	if( OP == CORE_P_INVALID )
		HB = 0;
	else
		HB = c2PTR   (ReadStack  (OP+CF_HP_OFFSET         ));
#ifdef N1
	n1currenthbslot = GetSlot(HB);
#endif
	for( core_p i=0; i<RP; i++ )
		WriteLocal(i, ReadStack  (cp_plus(cp+CF_LOCAL_OFFSET,i)));

	SP = cp;			// Dispose the stack after the choice point

	while( TP > NewTP )
	{
		TRACE(" Untrail....");
		TP -= 2;
		WriteHeapDirect( c2PTR(trail[TP]), trail[TP+1] );
	}
}
#endif

#define NOSWITCH 0
#define NOSWITCHADDNEW 1

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void machine::ExecLoop()
{
#ifdef N1
  int _n1tmpslot;
  core_p _n1tmpoffset;
#endif

	tserial tSN;

	ConstraintQueue.clear();

	for(;;)
	{
#ifdef N1
EXECLOOPTOP:
#define N0WHEAP(hp,c,cb) { _n1tmpslot = GetSlot(hp); _n1tmpoffset = GetOffset(hp); \
			     if (hp < HB || _n1tmpslot != n1currenthbslot) { \
			       CheckTrailLimit(TP+2); \
			       trail[TP++] = PTR2c(hp); \
			       trail[TP++] = cb; \
			     } \
			     N0WriteHeapDirect(_n1tmpslot,_n1tmpoffset,c); }
#define N0WRITEHEAP(hp,c) { _n1tmpslot = GetSlot(hp); _n1tmpoffset = GetOffset(hp); \
      			     if (hp < HB || _n1tmpslot != n1currenthbslot) { \
			       CheckTrailLimit(TP+2); \
			       trail[TP++] = PTR2c(hp); \
			       trail[TP++] = N0ReadHeapDirect(_n1tmpslot,_n1tmpoffset); \
			     } \
			     N0WriteHeapDirect(_n1tmpslot,_n1tmpoffset,c); }
#endif
#if defined(DEBUG) || defined(DOASSERT) || defined(DOPASS)
		++PASSCOUNT;
#if defined(DEBUG)
		if( DEB )
		{
			cout << "Inst " << PASSCOUNT << ": [" << hex << (int)(*IP) << dec << "]";

//#define BREAKCOUNT 220567192
#ifdef BREAKCOUNT
			if( PASSCOUNT >= BREAKCOUNT )
			{
				static core_p cp;
				static cell c;
				if( PASSCOUNT == BREAKCOUNT )
				{
					cp = int2cp(cp2int(FP) + 6)+(SF_LOCAL_OFFSET-1);
					c = ReadStack(cp);
					cout << "**** GetVal " << hex << cp2int(c2PTR(c)) << " stacksize=0x" << STACK.GetSize() << dec << endl;
				}
				else
				{
					if( c != ReadStack(cp) )
					{
						c = ReadStack(cp);
						cout << "**** Rewrite " << hex << cp2int(c2PTR(c)) << dec << endl;
					}
					else
						cout << "Current " << hex << cp2int(c2PTR(c)) << dec << endl;
				}
			}
#endif	// BREAKCOUNT
		}
#endif
//		if( PASSCOUNT > 48350 )
//			DEB = true;
#endif

#ifdef DEBUG
		if(DEB == true)
		{
			cout << endl;
			Dump();
//			if( HA != CORE_P_INVALID && !(GetSlot(HA) == GetSlot(HP) && HA >= HP) && GetOffset(HA) != 0 )
//			cout << FSP(this,HA).DisplayAVM();
		}
#endif
		ASSERT( IP != NULL );

		switch(*(IP++))
		{
		case C_SETINTPRO:
		case C_SETFLOPRO:
		case C_SETCHRPRO:
			
		case C_SETINT1ST:
			{
				mint v = GetMInt();
				TRACE("C_SETINT1ST " << v);
				HA = HP;
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,INT2c(v));
#else
				CheckHeapLimit(HP+1);
				PutHeap(INT2c(v));
#endif
				break;
			}
		case C_SETFLO1ST:
			{
				mfloat v = GetMFloat();
				TRACE("C_SETFLO1ST " << v);
				HA = HP;
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,FLO2c(v));
#else
				CheckHeapLimit(HP+1);
				PutHeap(FLO2c(v));
#endif
				break;
			}
		case C_SETCHR1ST:
			{
				mchar v = GetMChar();
				TRACE("C_SETCHR1ST " << v);
				HA = HP;
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,CHR2c(v));
#else
				CheckHeapLimit(HP+1);
				PutHeap(CHR2c(v));
#endif
				break;
			}
		case C_SETSTG1ST:
			{
				sserial s = GetStringSN();
				TRACE("C_SETCHR1ST " << s);
				HA = HP;
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,STG2c(s));
#else
				CheckHeapLimit(HP+1);
				PutHeap(STG2c(s));
#endif
				break;
			}
		case C_SETINT:
			{
				mint v = GetMInt();
				TRACE("C_SETINT " << v);
#ifdef N1
				core_p ha = HA;
				cell c;
				N0DEREF(ha,c);
#else
				core_p ha = Deref(HA);
				cell c = ReadHeap(ha);
#endif
				switch( Tag(c) )
				{
					case T_VAR:
						{
#ifdef N1
							tserial t = c2VARS(c);
							if (t == n1intserial || t == n1botserial) {
							  N0WHEAP(ha, INT2c(v), c);
							}
#else
							if( c == botcell || c == intcell )
								WriteHeap(ha, INT2c(v));
#endif
							else
								goto L_FAIL;
						}
						break;
					case T_INT:
						if( INT2c(v) != c )
							goto L_FAIL;
						break;
					default:
						goto L_FAIL;
				}
				break;
			}

		case C_SETFLO:
			{
				mfloat v = GetMFloat();
				TRACE("C_SETFLO " << v);
#ifdef N1
				core_p ha = HA;
				cell c;
				N0DEREF(ha,c);
#else
				core_p ha = Deref(HA);
				cell c = ReadHeap(ha);
#endif
				switch( Tag(c) )
				{
					case T_VAR:
						{
#ifdef N1
							tserial t = c2VARS(c);
							if (t == n1floserial || t == n1botserial) {
							  N0WHEAP(ha, FLO2c(v), c);
							}
#else
							if( c == botcell || c == flocell )
								WriteHeap(ha, FLO2c(v));
#endif
							else
								goto L_FAIL;
						}
						break;
					case T_FLO:
						if( FLO2c(v) != c )
							goto L_FAIL;
						break;
					default:
						goto L_FAIL;
				}
				break;
			}
		// C_SETCHR : removed
		case C_SETSTG:
			{
				sserial v = GetStringSN();
				TRACE("C_SETSTG " << v);
#ifdef N1
				core_p ha = HA;
				cell c;
				N0DEREF(ha,c);
#else
				core_p ha = Deref(HA);
				cell c = ReadHeap(ha);
#endif
				switch( Tag(c) )
				{
					case T_VAR:
						{
#ifdef N1
							tserial t = c2VARS(c);
							if (t == n1stgserial || t == n1botserial) {
							  N0WHEAP(ha, STG2c(v), c);
							}
#else
							if( c == botcell || c == stgcell )
								WriteHeap(ha, STG2c(v));
#endif
							else
								goto L_FAIL;
						}
						break;
					case T_STG:
						if( STG2c(v) != c )
							goto L_FAIL;
						break;
					default:
						goto L_FAIL;
				}
				break;
			}


		case C_SETREG:
			{
			  P4("machine::cSetReg");
			  TRACE("C_SETREG");
			  varno s = GetVarNo();
			  cell c = GetCell();
			  WriteLocal(s,c);
			}
			break;

		case C_RETURN:	////////////////////////////////////////////////////
			{
				TRACE("RETURN");
				
				IP = CIP;
				TRACE(" ... Return; Continue execution " << hex << reinterpret_cast<ptr_int>(CIP) << dec );
				if( IP == NULL )
					goto L_SUCCEED;
			}
			break;

		case C_SUCCEED:	////////////////////////////////////////////////////
			{
L_SUCCEED:
				TRACE("SUCCEED");
				HA = int2cp(0);
				IP = 0;
				goto EXITLOOP;
			//	IP = PopIP();
			}
//			break;

		case C_SHALCUT:	////////////////////////////////////////////////////
			{
				TRACE("SHALCUT");

				OP = B0;
				if( OP != CORE_P_INVALID ) {
					HB = c2PTR   (ReadStack  (OP+CF_HP_OFFSET         ));
#ifdef N1
					n1currenthbslot = GetSlot(HB);
#endif
				      }
			#ifdef PROCTRACE
				cout << "Shallow Cut; Next Choice Point is OP=0x" << cp2int(OP) << dec << endl;
			#endif
			}
			break;

		case C_GETLEVEL:////////////////////////////////////////////////////
			{
				varno s = GetVarNo();
				TRACE("GETLEVEL " << s);
				WriteLocal(s,PTR2c(B0));
			}
			break;

		case C_GETCP:////////////////////////////////////////////////////
			{
				varno s = GetVarNo();
				TRACE("GETLEVEL " << s);
				WriteLocal(s,PTR2c(OP));
			}
			break;

		case C_SETLEVEL:
			{
				TRACE("SETLEVEL");
				B0 = OP;
			}
			break;

		case C_DEEPCUT:	////////////////////////////////////////////////////
			{
				varno s = GetVarNo();
				TRACE("DEEPCUT " << s);
				OP = c2PTR(ReadLocal(s));
#ifdef BREAKCOUNT
	if( PASSCOUNT >= BREAKCOUNT )
		cout << "**** DEEPCUT 0x" << hex << cp2int(OP) << " stacksize=0x" << STACK.GetSize() << dec << endl;
#endif
				if( OP != CORE_P_INVALID ) {
					HB = c2PTR   (ReadStack  (OP+CF_HP_OFFSET         ));
#ifdef N1
					n1currenthbslot = GetSlot(HB);
#endif
				      }
			#ifdef PROCTRACE
				cout << "Deep Cut; Next Choice Point is OP=0x" << cp2int(OP) << dec << endl;
			#endif
			}
			break;

		case C_FRESH:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("FRESH " << type::Serial(tSN)->GetName());
				PutHeap(VAR2c(tSN));
			}
			break;

		case C_BIND:	/////////////////////////////////////////////////////
			{
				TRACE("BIND");
				PutHeap(PTR2c(HU1++));
			}
			break;

		case C_ADDSTR:	/////////////////////////////////////////////////////
			{ 
				tserial tSN = GetTypeSN();
				TRACE("ADDSTR " << type::Serial(tSN)->GetName());
#ifdef N1
				N0WRITEHEAP(HU1,PTR2c(HP)); HU1++;
#else
				WriteHeap(HU1++, PTR2c(HP));
#endif
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+type::Serial(tSN)->GetNFeatures()+1);
				N0PUTHEAP(n1currentslot,n1offset,STR2c(tSN));
#else
				CheckHeapLimit(HP+type::Serial(tSN)->GetNFeatures()+1);
				PutHeap(STR2c(tSN));
#endif
			#ifdef DEBUG
				for( int i=0; i<type::Serial(tSN)->GetNFeatures(); i++ )
				{
					WriteHeap(HP+i, unknown);
				}
			#endif
			}
			break;

		case C_BINDEOT:	/////////////////////////////////////////////////////
			{
				// Not obeyed to Binding Rule
				TRACE("BINDEOT");

				int t1SN = c2STRS(ReadHeap(HU1));
				int t2SN = c2STRS(ReadHeap(HU2));
				ASSERT( coerceData(t1SN,t2SN).result_s != S_INVALID );
#ifdef N1
				N0WRITEHEAP(HU2, STR2c( coerceData(t1SN,t2SN).result_s));
				N0WRITEHEAP(HU1, PTR2c(HU2));
#else
				WriteHeap(HU2, STR2c( coerceData(t1SN,t2SN).result_s));
				WriteHeap(HU1, PTR2c(HU2));
#endif
				HU1++;	HU2++;
			}
			break;

		case C_BIND1:	/////////////////////////////////////////////////////
			{
				// Not obeyed to Binding Rule
				TRACE("BIND1");
#ifdef N1
				N0WRITEHEAP(HU2, PTR2c(HU1));
#else
				WriteHeap(HU2, PTR2c(HU1));
#endif
				HU1++;	HU2++;
			}
			break;

		case C_BIND1T:
			{
				// Not obeyed to Binding Rule
				tserial tSN = GetTypeSN();
				TRACE("BIND1T " << type::Serial(tSN)->GetName());
#ifdef N1
				N0WRITEHEAP(HU1, STR2c(tSN));
				N0WRITEHEAP(HU2, PTR2c(HU1));
#else
				WriteHeap(HU1, STR2c(tSN));
				WriteHeap(HU2, PTR2c(HU1));
#endif
				HU1++;	HU2++;
			}
			break;

		case C_BINDEO:	/////////////////////////////////////////////////////
		case C_BIND2:	/////////////////////////////////////////////////////
			{
				// Not obeyed to Binding Rule
				TRACE("BIND2");
#ifdef N1
				N0WRITEHEAP(HU1, PTR2c(HU2));
#else
				WriteHeap(HU1, PTR2c(HU2));
#endif

				HU1++;	HU2++;
			}
			break;

		case C_BIND2T:	/////////////////////////////////////////////////////
			{
				// Not obeyed to Binding Rule
				tserial tSN = GetTypeSN();
				TRACE("BIND2T " << type::Serial(tSN)->GetName());
#ifdef N1
				N0WRITEHEAP(HU2, STR2c(tSN));
				N0WRITEHEAP(HU1, PTR2c(HU2));
#else
				WriteHeap(HU2, STR2c(tSN));
				WriteHeap(HU1, PTR2c(HU2));
#endif
				HU1++;	HU2++;
			}
			break;

		case C_BINDNEW:	/////////////////////////////////////////////////////
			{
				// Not obeyed to Binding Rule
				tserial tSN = GetTypeSN();
				TRACE("BINDNEW " << type::Serial(tSN)->GetName());
#ifdef N1
				N0WRITEHEAP(HU2, PTR2c(HP));
				N0WRITEHEAP(HU1, PTR2c(HP));
#else
				WriteHeap(HU2, PTR2c(HP));
				WriteHeap(HU1, PTR2c(HP));
#endif
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+type::Serial(tSN)->GetNFeatures()+1);
				N0PUTHEAP(n1currentslot,n1offset,STR2c(tSN));
#else
				CheckHeapLimit(HP+type::Serial(tSN)->GetNFeatures()+1);
				PutHeap(STR2c(tSN));	// HP++
#endif
				HU1++;	HU2++;
			#ifdef DEBUG
				for( int i=0; i<type::Serial(tSN)->GetNFeatures(); i++ )
				{
					WriteHeap(HP+i, unknown);
				}
			#endif
			}
			break;

		case C_TCOERCE:	/////////////////////////////////////////////////////
			{
				TRACE("TCOERCE");

				HU1++;
			}
			break;

		case C_TCOERCET:	////////////////////////////////////////////////////
			{ 
				tserial tSN = GetTypeSN();
				TRACE("TCOERCET " << type::Serial(tSN)->GetName());
#ifdef N1
				N0WRITEHEAP(HU1, STR2c(tSN));
#else
				WriteHeap(HU1, STR2c(tSN));
#endif
				HU1++;
			}
			break;
			
		case C_CHGSTR:	////////////////////////////////////////////////////
			{ 
				tserial tSN = GetTypeSN();
				TRACE("CHGSTR " << type::Serial(tSN)->GetName());
#ifdef N1
				N0WRITEHEAP(HU1, STR2c(tSN));
#else
				WriteHeap(HU1, STR2c(tSN));
#endif
			}

		// fall thru...
		
		case C_PROCEED:	/////////////////////////////////////////////////////
			{
				TRACE("PROCEED");
#ifdef N1
N1PROCEED:
				if (queue.isempty()) {
				  HU1 = HU2 = CORE_P_INVALID;
				  if ((IP = UIP) == NULL)
				    goto EXITLOOP;
				} else {
				  HU1 = queue.dequeue();
				  HU2 = queue.dequeue();
				  goto BEGINUNIFY;
				}
#else
				GetUnifyQueue(HU1, HU2);
				if( HU1 != CORE_P_INVALID )		// if there are some unification to do
				{
					TRACE(" ... Unification with " << hex << (cp2int(HU1)) << " and " << (cp2int(HU2)) << dec );
					ASSERT( HU2 != CORE_P_INVALID );
					StopUnifProf();
					BeginUnify();				//   go to next unify
				}
				else
				{
			//		HA = HA2;					// restore HA
					IP = UIP;
					StopUnifProf();
					TRACE(" ... Proceed; Continue execution " << hex << reinterpret_cast<ptr_int>(UIP) << dec );
				}
				if( IP == NULL )
					goto EXITLOOP;
#endif
			      }

			break;

		case C_UNIFY:	/////////////////////////////////////////////////////
			{
				TRACE("UNIFY");
#ifdef N1
				N0PUTUNIFYQ(HU1,HU2);
#else
				PutUnifyQueue(HU1, HU2);
#endif
				HU1++; HU2++;
			}
			break;

		case C_UNIFY_S:	/////////////////////////////////////////////////////
			{
				TRACE("UNIFY_S");

				PutHeap(PTR2c(HU1));	// HP++
#ifdef N1
				N0PUTUNIFYQ(HU1,HU2);
#else
				PutUnifyQueue(HU1, HU2);
#endif
				HU1++; HU2++;
			}
			break;

		case C_COPY1:	/////////////////////////////////////////////////////
			{
				TRACE("COPY1");

				PutHeap(PTR2c(HU1));	// HP++
				HU1++;
			}
			break;

		case C_COPY2:	/////////////////////////////////////////////////////
			{
				TRACE("COPY2");

				PutHeap(PTR2c(HU2));	// HP++
				HU2++;
			}
			break;

		case C_SKIP1:	/////////////////////////////////////////////////////
			{
				TRACE("SKIP1");

				HU1++;
			}
			break;

		case C_SKIP2:	/////////////////////////////////////////////////////
			{
				TRACE("SKIP2");

				HU2++;
			}
			break;


		case C_BINDF:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("BINDF " << type::Serial(tSN)->GetName());
				PutUnifyQueueType(HU1, tSN);
				PutHeap(PTR2c(HU1++));
			}
			break;

		case C_UNIFYF:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("UNIFYF " << type::Serial(tSN)->GetName());
				PutUnifyQueueType(HU1, tSN);
#ifdef N1
				N0PUTUNIFYQ(HU1,HU2);
#else
				PutUnifyQueue(HU1, HU2);
#endif
				HU1++; HU2++;
			}
			break;

		case C_UNIFYF_S:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("UNIFYF_S " << type::Serial(tSN)->GetName());

				PutHeap(PTR2c(HU1));	// HP++

				PutUnifyQueueType(HU1, tSN);
#ifdef N1
				N0PUTUNIFYQ(HU1,HU2);
#else
				PutUnifyQueue(HU1, HU2);
#endif
				HU1++; HU2++;
			}
			break;

		case C_COPYF1:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("COPYF1 " << type::Serial(tSN)->GetName());

				PutHeap(PTR2c(HU1));	// HP++
				PutUnifyQueueType(HU1, tSN);
				HU1++;
			}
			break;

		case C_COPYF2:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("COPYF2 " << type::Serial(tSN)->GetName());

				PutHeap(PTR2c(HU2));	// HP++
				PutUnifyQueueType(HU2, tSN);
				HU2++;
			}
			break;

		case C_SKIPF1:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("SKIPF1 " << type::Serial(tSN)->GetName());

				PutUnifyQueueType(HU1, tSN);
				HU1++;
			}
			break;

		case C_SKIPF2:	/////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("SKIPF2 " << type::Serial(tSN)->GetName());

				PutUnifyQueueType(HU2, tSN);
				HU2++;
			}
			break;

		case C_POP:	/////////////////////////////////////////////////////
			{
				TRACE("POP");
				HA = c2PTR(Pop());
			}
			break;
		
		case C_TRY:	/////////////////////////////////////////////////////
			{
				TRACE("TRY");
			#ifdef TRAIL
				code * ReturnIP = IP;
				ReturnIP += GetRelJump();

				B0 = OP;
				SetTrailPoint(ReturnIP);

			#ifdef PROCTRACE
				cout << "Choice Point Created OP=0x" << cp2int(OP) << dec << endl;
			#endif
			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_RETRY:	/////////////////////////////////////////////////////
			{
				TRACE("RETRY");
			#ifdef TRAIL
				code * ReturnIP = IP;
				ReturnIP += GetRelJump();

			#ifdef PROCTRACE
				cout << "Choice Point Retried OP=0x" << cp2int(OP) << dec << endl;
			#endif
				TrailAgain(ReturnIP);
			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_LASTTRY:	/////////////////////////////////////////////////////
			{
				TRACE("LASTTRY");

			#ifdef TRAIL
			#ifdef PROCTRACE
				cout << "Choice Point Cleared OP=0x" << cp2int(OP) << dec << endl;
			#endif
				TrailBack();
			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_TRYAT:	/////////////////////////////////////////////////////
			{
				TRACE("TRYAT");
			#ifdef TRAIL
				code *ExecIP = GetInstP();
				SetTrailPoint(IP);

			#ifdef PROCTRACE
				cout << "Choice Point Created OP=0x" << cp2int(OP) << dec << endl;
			#endif
				IP = ExecIP;
			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_RETRYAT:	/////////////////////////////////////////////////////
			{
				TRACE("RETRYAT");
			#ifdef TRAIL
			#ifdef PROCTRACE
				cout << "Choice Point Retried OP=0x" << cp2int(OP) << dec << endl;
			#endif
				TrailAgain(IP+CODE_INSTP_SIZE);
				IP = GetInstP();
			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_LASTAT:	/////////////////////////////////////////////////////
			{
				TRACE("LASTAT");

			#ifdef TRAIL
			#ifdef PROCTRACE
				cout << "Choice Point Cleared OP=0x" << cp2int(OP) << dec << endl;
			#endif
				IP = GetInstP();
				TrailBack();
			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_JUMP:	/////////////////////////////////////////////////////
			{
				TRACE("JUMP");
				
				code * ReturnIP = IP;
				ReturnIP += GetRelJump();

				IP = ReturnIP; 
			}
			break;

		case C_CLRTEMP:	/////////////////////////////////////////////////////
			{
				int ntemp = GetInt16();
				TRACE("CLRTEMP " << ntemp);

			//	HA = CORE_P_INVALID;

			//	for( int i=0; i<ntemp; i++ )
			//		local[i] = botcell;
				RP = (core_p)0+ntemp;
			}
			break;

		case C_START:	/////////////////////////////////////////////////////
			{
				TRACE("START");

				HA = CORE_P_INVALID;
			}
			break;

		case C_ALLOC:	/////////////////////////////////////////////////////
			{
				P5("C_ALLOC");
			//	int i;

				int nperm = GetInt16();
				int ntemp = GetInt16();
				TRACE("ALLOC " << nperm << ", " << ntemp);

//				HA = CORE_P_INVALID;

			//	for( i=cp2int(RP) ; i<ntemp; i++ )
			//	{
			//		WriteLocal(i, botcell);
			//	}
				RP = (core_p)0+ntemp;

				core_p NewFP = SP+CODESIZEONSTACK;
				core_p NewSP = NewFP+nperm+1;
				CheckStackLimit(NewSP);
				
				WriteStackIP(SP,  CIP);
				WriteStack(NewFP, PTR2c(FP));
				FP = NewFP;
				
			//	for( i=0; i<nperm; i++ )
			//	{
			//		WriteStack( FP+i+1, botcell );
			//	}
				SP = NewSP;
			}
			break;

		case C_DEALLOC:	/////////////////////////////////////////////////////
			{
				P5("C_DEALLOC");
				TRACE("DEALLOC");

				CIP = ReadStackIP( FP + SF_CIP_OFFSET );

				if( OP < FP || OP == CORE_P_INVALID )
					SP = FP - CODESIZEONSTACK;
				else
					SP = cp_plus(OP+CF_LOCAL_OFFSET, c2PTR(ReadStack(OP+CF_RP_OFFSET)));

				FP = c2PTR(ReadStack(FP));
//				RP = 0;
			}
			break;

		case C_CALL:	/////////////////////////////////////////////////////
			{
				pserial s = GetProcSN();
				TRACE("CALL " << procedure::Serial(s)->GetName());

				CIP = IP;		// Return Address
				procthunk *pt = procedure::SerialThunk(s);
				ASSERT( pt != NULL );
				pt->exec(*this);
				if( IP == NULL )
					goto EXITLOOP;
			}
			break;

		case C_EXECUTE:	/////////////////////////////////////////////////////
			{
				pserial s = GetProcSN();
				TRACE("EXECUTE " << procedure::Serial(s)->GetName());

				IP = CIP;
				procthunk *pt = procedure::SerialThunk(s);
				ASSERT( pt != NULL );
				pt->exec(*this);
				if( IP == NULL )
					goto EXITLOOP;
			}
			break;

		case C_CALLAT:	/////////////////////////////////////////////////////
			{
				TRACE("CALLAT");

				code *execp = GetInstP();
				int  arity = GetInt16();
				CIP = IP;
				DefaultCall(execp, arity);
			}
			break;

		case C_FAIL:	/////////////////////////////////////////////////////
			{
			L_FAIL:
				P5("C_FAIL");
				TRACE("FAIL");

			#ifdef TRAIL
				if( OP == 0 || OP == CORE_P_INVALID )
				{
					TRACE(" Totally failed (No more choice-frame); go back to top-level");
					HA = CORE_P_INVALID; // tell caller that the operation has been failed
					IP = 0;
					goto EXITLOOP;
				}

				IP = ReadStackIP(OP+CF_RETP_OFFSET) ;

				if( IP == 0 )
				{
					TRACE(" Totally failed (Next TRY address is NULL); go back to top-level");
					HA = CORE_P_INVALID; // tell caller that the operation has been failed
					goto EXITLOOP;
				}
				
				TRACE(" Resume from " << hex << (ptr_int)IP << dec << "...");

#ifdef DOASSERT
				if( ! ( *IP == C_RETRY || *IP == C_LASTTRY || *IP == C_RETRYAT || *IP == C_LASTAT ) )
				{
					cout << "IP = " << hex << reinterpret_cast<ptr_int>(IP) << dec << ", *IP = " << hex << (int)*IP << dec << endl;
					ASSERT( *IP == C_RETRY || *IP == C_LASTTRY || *IP == C_RETRYAT || *IP == C_LASTAT );
				}
#endif

			#else
				ABORT("The instruction is not supported");
			#endif
			}
			break;

		case C_HALT:	/////////////////////////////////////////////////////
			{ 
				TRACE("HALT");
				
				ABORT("SOMEWHY HALT INSTRUCTION IS EXECUTED... ");
			}
			break;

		case C_ADDNEWPRO:////////////////////////////////////////////////////
			{
				TRACE("ADDNEWPRO");
//				tserial tSN = GetTypeSN();
//				TRACE("ADDNEWPRO " << type::Serial(tSN)->GetName());
//				
				if( HA == CORE_P_INVALID )
					goto L_ADDNEW1ST;
				else 
					goto L_ADDNEW;
			}
//			break;		// unreachable

		case C_ADDNEW1ST:////////////////////////////////////////////////////
			{
			L_ADDNEW1ST:
				tserial tSN = GetTypeSN();
				TRACE("ADDNEW1ST " << type::Serial(tSN)->GetName());

				HA = HP;
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,VAR2c(tSN));
#else
				CheckHeapLimit(HP+1);
				PutHeap(VAR2c(tSN));
#endif
			}
			break;

		case C_ADDNEW1ST_C:////////////////////////////////////////////////////
			{
				tserial tSN = GetTypeSN();
				TRACE("ADDNEW1ST_C " << type::Serial(tSN)->GetName());

				HA = HP;
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,VAR2c(tSN));
#else
				CheckHeapLimit(HP+1);
				PutHeap(VAR2c(tSN));
#endif
				HU1 = HA;
				UIP = IP;
				IP = coerceData(botserial,tSN).constraint;
				if( IP == NULL ) 
					IP = UIP;
			}
			break;


		case C_ADDNEW_C:	/////////////////////////////////////////////////////
		case C_ADDNEW:	/////////////////////////////////////////////////////
			{
			L_ADDNEW:
				tSN = GetTypeSN();
				TRACE("ADDNEW " << type::Serial(tSN)->GetName());

#ifdef N1
				core_p ha = HA;
				cell c;
				N0DEREF(ha,c);
#else
				core_p ha = Deref(HA);
				cell c = ReadHeap(ha);
#endif
				switch( Tag(c) )
				{
					case T_VAR:
						{
							ushort result = coerceData(c2VARS(c),tSN).result_s;
							if( result == S_INVALID )
								goto L_FAIL;
							else
#ifdef N1
							  {
							    if (result != c2VARS(c))
							      N0WHEAP(ha, VAR2c(result), c);
							  }
#else
								WriteHeap(ha, VAR2c(result)); 
#endif
							code *xp = coerceData(c2VARS(c),result).constraint;
							if( xp )
							{
								UIP = IP;
								IP = xp;
								HU1 = ha;
							}
							break;
						}
					case T_STR:
						{
							UIP = IP;		// Return Address
							IP = coerceData(c2STRS(c),tSN).coercion;
							// StartUnifProf();	// TEMPORARY
							TRACE("call to coerceData(" << c2STR(c)->GetName() << "," << type::Serial(tSN)->GetName() << ")");
							HU1 = ha;					// Saved for afterworks
							break;
						}
					case T_INT:
#ifdef N1
						if (tSN != n1intserial && tSN != n1botserial)
#else
						if( tSN != intserial && tSN != botserial )
#endif
							goto L_FAIL;
						break;
					case T_FLO:
#ifdef N1
						if (tSN != n1floserial && tSN != n1botserial)
#else
						if( tSN != floserial && tSN != botserial )
#endif
							goto L_FAIL;
						break;
#if 0
					case T_CHR:
#ifdef N1
						if (tSN != n1chrserial && tSN != n1botserial)
#else
						if( tSN != t_chr->GetSerialNo() && tSN != bot->GetSerialNo() )
#endif
							goto L_FAIL;
						break;
#endif
					case T_STG:
#ifdef N1
						if (tSN != n1stgserial && tSN != n1botserial)
#else
						if( tSN != stgserial && tSN != botserial )
#endif
							goto L_FAIL;
						break;
					default:
						ABORT("Internal Error: unknown Tag");
				}
			}
			break;

		case C_PUSHPRO:	/////////////////////////////////////////////////////
			{
				TRACE("PUSHPRO");

				if( HA == CORE_P_INVALID )
					goto L_PUSH1ST;
				else 
					goto L_PUSH;
			}
//			break;		// unreachable

		case C_PUSH1ST:	/////////////////////////////////////////////////////
			{
			L_PUSH1ST:
				P4("machine::cPush1");
				fserial fSN = GetFeatureSN();
				TRACE("PUSH1ST " << feature::Serial(fSN)->GetName());

				// kairyoable

				featcode &p = featureCode[botserial][fSN];
				if( p.result == S_INVALID )
					goto L_FAIL;
				else
				{
					Push(PTR2c(HP));
					HA = HP + p.index;
					ExpandStructure(p.result);
				}
			}
			break;

		case C_PUSH:	/////////////////////////////////////////////////////
			{
			L_PUSH:
				P4("machine::cPush");
				fserial fSN = GetFeatureSN();
				TRACE("PUSH " << feature::Serial(fSN)->GetName());
#ifdef N1
				core_p cp = HA;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HA);
				cell c = ReadHeap(cp);
#endif

				switch( Tag(c) )
				{
				case T_STR:
					{
						tserial tSN = c2STRS(c);
						featcode &p = featureCode[tSN][fSN];
						if( p.coercion == NULL )
						{
							if( p.index == S_INVALID )
								goto L_FAIL;		// Fail to unify
							Push(PTR2c(cp));
							HA = cp + p.index;
							break;
						}
						else
						{
							Push(PTR2c(cp));
							UIP = IP;		// Return Address
							IP = p.coercion;
							// StartUnifProf();	// TEMPORARY
							TRACE("call to featureCode[" << type::Serial(tSN)->GetName() << "][" << feature::Serial(fSN)->GetName() << "]");
							ASSERT(*IP == C_ADDSTR);
							HU1 = cp;					// coercion point
							HA = HP + p.index;
							break;
						}
					}
				case T_VAR:
					{
						tserial tSN = c2VARS(c);
						featcode &p = featureCode[tSN][fSN];
						if( p.index == S_INVALID )
							goto L_FAIL;		// Fail to unify
#ifdef N1
						N0WHEAP(cp, PTR2c(HP),c);
#else
						WriteHeap(cp, PTR2c(HP));
#endif
						Push(PTR2c(HP));
#ifdef N1
						HA = HP + p.index;
						const type *r = type::Serial(p.result);
						int n1nf = r->GetNFeatures();
#ifdef N1
						core_p n1offset = GetOffset(HP);
						N0CheckHeapLimit(n1currentslot,n1offset+1+n1nf);
						N0PUTHEAP(n1currentslot,n1offset,STR2c(p.result));
						for (int i = 0; i < n1nf; i++) {
						  N0PUTHEAP(n1currentslot,n1offset,VAR2c(r->GetAppTypeS(i)));
						}
#else
						CheckHeapLimit(HP+1+n1nf);
						PutHeap(STR2c(p.result));
						for (int i = 0; i < n1nf; i++)
							PutHeap(VAR2c(r->GetAppTypeS(i)));
#endif
#else
						HA = HP + p.index;
						ExpandStructure(p.result);
#endif
						break;
					}
				default:
					goto L_FAIL;
				}
			}
			break;

		case C_FOLLOWPRO:////////////////////////////////////////////////////
			{
				TRACE("FOLLOWPRO");

				if( HA == CORE_P_INVALID )
					goto L_FOLLOW1ST;
				else 
					goto L_FOLLOW;
			}
//			break;		// unreachable

		case C_FOLLOW1ST:////////////////////////////////////////////////////
			{
			L_FOLLOW1ST:
				P4("machine::cFollow");
				fserial fSN = GetFeatureSN();
				TRACE("FOLLOW1ST " << feature::Serial(fSN)->GetName());

				// kairyoable

				featcode &p = featureCode[botserial][fSN];
				if( p.result == S_INVALID )
					goto L_FAIL;
				else
				{
					HA = HP + p.index;
					ExpandStructure(p.result);
				}
			}
			break;
			
		case C_FOLLOW:	/////////////////////////////////////////////////////
			{
			L_FOLLOW:
				P4("machine::cFollow");
				fserial fSN = GetFeatureSN();
				TRACE("FOLLOW " << feature::Serial(fSN)->GetName());
#ifdef N1
				core_p cp = HA;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HA);
				cell c = ReadHeap(cp);
#endif

				switch( Tag(c) )
				{
				case T_STR:
					{
						tserial tSN = c2STRS(c);
						featcode &p = featureCode[tSN][fSN];
						if( p.coercion == NULL )
						{
							if( p.index == S_INVALID )
								goto L_FAIL;		// Fail to unify
							HA = cp + p.index;
							break;
						}
						else
						{
							UIP = IP;		// Return Address
							IP = p.coercion;
							// StartUnifProf();	// TEMPORARY
							TRACE("call to featureCode[" << type::Serial(tSN)->GetName() << "][" << feature::Serial(fSN)->GetName() << "]");
							ASSERT(*IP == C_ADDSTR);
							HU1 = cp;					// coercion point
							HA = HP + p.index;
							break;
						}
					}
				case T_VAR:
					{
						tserial tSN = c2VARS(c);
						featcode &p = featureCode[tSN][fSN];
						if( p.index == S_INVALID )
							goto L_FAIL;		// Fail to unify
#ifdef N1
						N0WHEAP(cp, PTR2c(HP), c);
#else
						WriteHeap(cp, PTR2c(HP));
#endif
						HA = HP + p.index;
#ifdef N1
						const type *r = type::Serial(p.result);
						int n1nf = r->GetNFeatures();
#ifdef N1
						core_p n1offset = GetOffset(HP);
						N0CheckHeapLimit(n1currentslot,n1offset+1+n1nf);
						N0PUTHEAP(n1currentslot,n1offset,STR2c(p.result));
						for (int i = 0; i < n1nf; i++) {
						  N0PUTHEAP(n1currentslot,n1offset,VAR2c(r->GetAppTypeS(i)));
						}
#else
						CheckHeapLimit(HP+1+n1nf);
						PutHeap(STR2c(p.result));
						for (int i = 0; i < n1nf; i++)
						  PutHeap(VAR2c(r->GetAppTypeS(i)));
#endif
#else
						HA = HP + p.index;
						ExpandStructure(p.result);
#endif
						break;
					}
				default:
					goto L_FAIL;
				}
			}
			break;

		case C_LINKVAR:	/////////////////////////////////////////////////////
			{
				TRACE("LINKVAR");

				if( HA == CORE_P_INVALID )
					goto L_LINK1ST;
				else
					goto L_LINK2ND;
			}
//			break;		// unreachable

		case C_LINK1ST:	/////////////////////////////////////////////////////
			{
			L_LINK1ST:
				varno s = GetVarNo();
				TRACE("LINK1ST " << s);

				HA = HP;
				WriteLocal(s, PTR2c(HP));
#ifdef N1
				core_p n1offset = GetOffset(HP);
				N0CheckHeapLimit(n1currentslot,n1offset+1);
				N0PUTHEAP(n1currentslot,n1offset,n1botcell);
#else
				CheckHeapLimit(HP+1);
				PutHeap(botcell);
#endif
			}
			break;

		case C_LINK2ND:	/////////////////////////////////////////////////////
			{
			L_LINK2ND:
				varno s = GetVarNo();
				TRACE("LINK2ND " << s);

				WriteLocal(s,PTR2c(HA));
			}
			break;

		case C_UNIFYVAR://///////////////////////////////////////////////////
			{
				TRACE("UNIFYVAR");
				
				if( HA == CORE_P_INVALID )
					goto L_UNIFY1ST;
				else
					goto L_UNIFY2ND;
			}
//			break;		// unreachable

		case C_UNIFY1ST://///////////////////////////////////////////////////
			{
			L_UNIFY1ST:
				varno s = GetVarNo();
				TRACE("UNIFY1ST " << s);
				
				HA = c2PTR(ReadLocal(s));
			}
			break;

		case C_UNIFY2ND://///////////////////////////////////////////////////
			{
			L_UNIFY2ND:
				varno s = GetVarNo();
				TRACE("UNIFY2ND " << s);
				
				HU1 = HA;
				HU2 = c2PTR(ReadLocal(s));
				UIP = IP; // Return Address
				ASSERT( HU1 != CORE_P_INVALID );
				ASSERT( HU2 != CORE_P_INVALID );
#ifdef N1
				goto BEGINUNIFY;
#else
				BeginUnify();
				if( IP == NULL )
					goto EXITLOOP;
#endif
			}
			break;

		case C_TESTVAR:	/////////////////////////////////////////////////////
			{
				TRACE("TESTVAR");

				if( HA == CORE_P_INVALID )
					goto L_TEST1ST;
				else
					goto L_TEST2ND;
			}
//			break;		// unreachable

		case C_TEST1ST:	/////////////////////////////////////////////////////
			{
			L_TEST1ST:
				varno s = GetVarNo();
				TRACE("TEST1ST " << s);
				
				if( ReadLocal(s) == botcell )
				{
					HA = HP;
					WriteLocal(s, PTR2c(HP));
					CheckHeapLimit(HP+1);
					PutHeap(botcell);
				}
				else
					HA = c2PTR(ReadLocal(s));
			}
			break;

		case C_TEST2ND:	/////////////////////////////////////////////////////
			{
			L_TEST2ND:
				varno s = GetVarNo();
				TRACE("TEST2ND " << s);
				
				if( ReadLocal(s) == botcell )
				{
					WriteLocal(s,PTR2c(HA));
				}
				else {
					HU1 = HA;
					HU2 = c2PTR(ReadLocal(s));
					UIP = IP; // Return Address
					ASSERT( HU1 != CORE_P_INVALID );
					ASSERT( HU2 != CORE_P_INVALID );
#ifdef N1
					goto BEGINUNIFY;
#else
					BeginUnify();
					if( IP == NULL )
						goto EXITLOOP;
#endif

				}
			}
			break;

		case C_FEATLINK://///////////////////////////////////////////////////
			{ 
			//	code *oldIP = IP - 1;
				P4("machine::cFeat");
				fserial fSN = GetFeatureSN();
				varno s = GetVarNo();
				TRACE("FEATLINK " << feature::Serial(fSN)->GetName() << " " << s);
#ifdef N1
				core_p cp = HA;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HA);
				cell c = ReadHeap(cp);
#endif

				switch( Tag(c) )
				{
					case T_VAR:
						{
							tserial tSN = c2VARS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.result == (ushort)(-1) )
								goto L_FAIL;
							else
							{
#ifdef N1
							  N0WHEAP(cp, PTR2c(HP), c);
#else
								WriteHeap(cp, PTR2c(HP));
#endif
								HA = HP;
#ifdef N1
								const type *r = type::Serial(p.result);
							  int n1nf = r->GetNFeatures();
#ifdef N1
							  core_p n1offset = GetOffset(HP);
							  N0CheckHeapLimit(n1currentslot,n1offset+1+n1nf);
							  N0PUTHEAP(n1currentslot,n1offset,STR2c(p.result));
							  for (int i = 0; i < n1nf; i++) {
							    N0PUTHEAP(n1currentslot,n1offset,VAR2c(r->GetAppTypeS(i)));
							  }
#else
							  CheckHeapLimit(HP+1+n1nf);
							  PutHeap(STR2c(p.result));
							  for (int i=0; i < n1nf; i++ )
								PutHeap(VAR2c(r->GetAppTypeS(i)));
#endif
#else
									ExpandStructure(p.result);
#endif

								WriteLocal(s,PTR2c(HA+p.index));
							}
						}
						break;
					case T_STR:
						{
							tserial tSN = c2STRS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.coercion != NULL )
							{
								UIP = IP;		// Return Address
							// StartUnifProf();	// TEMPORARY
								IP = p.coercion;
								TRACE("call to featureCode[" << type::Serial(tSN)->GetName() << "][" << feature::Serial(fSN)->GetName() << "]");
								ASSERT(*IP == C_ADDSTR);
								HU1 = cp;					// coercion point
								HA = HP;
								WriteLocal(s,PTR2c(HA+p.index));
							}
							else
							{
								if( p.index == (ushort)(-1) )
									goto L_FAIL;		// Fail to unify
								else
									WriteLocal(s,PTR2c(cp+p.index));
			//					WriteLocal(s,PTR2c(Deref(cp+p.index))); // which is better?
							}
						}
						break;
					default:
						goto L_FAIL;
//						ABORT("Internal Error: unknown Tag");
				}
			}
			break;

		case C_FEATTEST://///////////////////////////////////////////////////
			{ 
			ABORT("Unsupported instruction");
			}
			break;

		case C_FEATUNIFY:////////////////////////////////////////////////////
			{
			//	code *oldIP = IP - 1;
				P4("machine::cFeat");
				fserial fSN = GetFeatureSN();
				varno s = GetVarNo();
				TRACE("FEATUNIFY " << feature::Serial(fSN)->GetName() << " " << s);
#ifdef N1
				core_p cp = HA;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HA);
				cell c = ReadHeap(cp);
#endif

				switch( Tag(c) )
				{
					case T_VAR:
						{
							tserial tSN = c2VARS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.result == (ushort)(-1) )
								goto L_FAIL;
							else
							{
#ifdef N1
							  N0WHEAP(cp, PTR2c(HP), c);
#else
								WriteHeap(cp, PTR2c(HP));
#endif
								HA = HP;
#ifdef N1
								const type *r = type::Serial(p.result);
							  int n1nf = r->GetNFeatures();
#ifdef N1
							  core_p n1offset = GetOffset(HP);
							  N0CheckHeapLimit(n1currentslot,n1offset+1+n1nf);
							  N0PUTHEAP(n1currentslot,n1offset,STR2c(p.result));
							  for (int i = 0; i < n1nf; i++) {
							    N0PUTHEAP(n1currentslot,n1offset,VAR2c(r->GetAppTypeS(i)));
							  }
#else
							  CheckHeapLimit(HP+1+n1nf);
							  PutHeap(STR2c(p.result));
							  for(int i = 0; i < n1nf; i++ )
							    PutHeap(VAR2c(r->GetAppTypeS(i)));
#endif
#else
									ExpandStructure(p.result);
#endif

								HU1 = HA+p.index;
								HU2 = c2PTR(ReadLocal(s));
								UIP = IP; // Return Address
								ASSERT( HU1 != CORE_P_INVALID );
#ifdef N1
#else
								if( HU2 == CORE_P_INVALID )
									cout << "BAD" << endl;
#endif
#ifdef N1
								goto BEGINUNIFY;
#else
								BeginUnify();
								if( IP == NULL )
									goto EXITLOOP;
#endif
							}
						}
						break;
					case T_STR:
						{
							tserial tSN = c2STRS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.coercion != NULL )
							{
								UIP = IP;		// Return Address
								IP = p.coercion;
							// StartUnifProf();	// TEMPORARY
								TRACE("call to featureCode[" << type::Serial(tSN)->GetName() << "][" << feature::Serial(fSN)->GetName() << "]");
								ASSERT(*IP == C_ADDSTR);
								HU1 = cp;					// coercion point
								HA = HP;
#ifdef N1
								core_p n1adr1 = HA + p.index;
								core_p n1adr2 = c2PTR(ReadLocal(s));
								N0PUTUNIFYQ(n1adr1, n1adr2);
#else
								PutUnifyQueue(HA+p.index, c2PTR(ReadLocal(s)) );
#endif
							}
							else
							{
								if( p.index == (ushort)(-1) )
									goto L_FAIL;		// Fail to unify
								else
								{
									HU1 = cp+p.index;
									HU2 = c2PTR(ReadLocal(s));
									UIP = IP; // Return Address
									ASSERT( HU1 != CORE_P_INVALID );
									ASSERT( HU2 != CORE_P_INVALID );
#ifdef N1
									goto BEGINUNIFY;
#else
									BeginUnify();
									if( IP == NULL )
										goto EXITLOOP;
#endif
								}
							}
						}
						break;
					default:
						goto L_FAIL;
//						ABORT("Internal Error: unknown Tag");
				}
			}
			break;

		case C_FEATADD:	/////////////////////////////////////////////////////
			{
				code *oldIP = IP - 1;
				P4("machine::cFeat");
				fserial fSN = GetFeatureSN();
				tserial newtSN = GetTypeSN();
				TRACE("FEATADD " << feature::Serial(fSN)->GetName() << " " << type::Serial(newtSN)->GetName());
#ifdef N1
				core_p cp = HA;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HA);
				cell c = ReadHeap(cp);
#endif

				switch( Tag(c) )
				{
					case T_VAR:
						{
							tserial tSN = c2VARS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.result == TS_INVALID )
								goto L_FAIL;
							else
							{
								const type *r = type::Serial(p.result);
#ifdef TYPE_TABLE_HASH
								const coercode *coer = coerceRef(r->GetAppTypeS(p.index-1),newtSN);
								if( coer == NULL )
									goto L_FAIL;
								tserial ser = coer->result_s;
#else
								tserial ser = coerceData(r->GetAppTypeS(p.index-1),newtSN).result_s;
								if( ser == TS_INVALID )
									goto L_FAIL;
#endif

#ifdef N1
								N0WHEAP(cp, PTR2c(HP),c);
#else
								WriteHeap(cp, PTR2c(HP));
#endif

								HA = HP;
#ifdef N1
								int n1nf = r->GetNFeatures();
								CheckHeapLimit(HP+1+n1nf);
#else
								CheckHeapLimit(HP+1+r->GetNFeatures());
#endif
								PutHeap(STR2c(p.result));
								int i;

								// Should call ExpandStructure
								for( i=1; i<p.index; i++ )
									PutHeap(VAR2c(r->GetAppTypeS(i-1)));
								PutHeap(VAR2c(ser));
#ifdef N1
								for(    ; i < n1nf; i++ )
									PutHeap(VAR2c(r->GetAppTypeS(i)));
#else
								for(    ; i<r->GetNFeatures(); i++ )
									PutHeap(VAR2c(r->GetAppTypeS(i)));
#endif
							}
						}
						break;
					case T_STR:
						{
							tserial tSN = c2STRS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.coercion != NULL )
							{
								UIP = oldIP;		// Return Address; Execute This Once More
								IP = p.coercion;
							// StartUnifProf();	// TEMPORARY
								TRACE("call to featureCode[" << type::Serial(tSN)->GetName() << "][" << feature::Serial(fSN)->GetName() << "]");
								ASSERT(*IP == C_ADDSTR);
								HU1 = cp;					// coercion point
								HA = HP;
							}
							else
							{
								if( p.index == (ushort)(-1) )
									goto L_FAIL;		// Fail to unify

								// ADDNEW CODE
#ifdef N1
								core_p ha = cp+p.index;
								cell c;
								N0DEREF(ha,c);
#else
								core_p ha = Deref(cp+p.index);
								cell c = ReadHeap(ha);
#endif				
								switch( Tag(c) )
								{
									case T_VAR:
										{

#ifdef TYPE_TABLE_HASH
											tserial result;
											const coercode *coer = coerceRef(c2VARS(c),newtSN);
											if( coer == NULL )
												goto L_FAIL;
#else
											tserial result = coerceData(c2VARS(c),newtSN).result_s;
											if( result == S_INVALID )
												goto L_FAIL;
#endif
											else
											{
#ifdef TYPE_TABLE_HASH
												result = coer->result_s;
#endif
#ifdef N1
											    if (tc != result)
											      { N0WHEAP(ha, VAR2c(result), c); }
#else
												WriteHeap(ha, VAR2c(result)); 
#endif
											}
											code *constrcode = coerceData(c2VARS(c),result).constraint;
											if( constrcode )
											{
												UIP = IP;
												IP = constrcode;
												HU1 = ha;
											}
											break;
										}
									case T_STR:
										{
											UIP = IP;		// Return Address
											IP = coerceData(c2STRS(c),newtSN).coercion;
							// StartUnifProf();	// TEMPORARY
											TRACE("call to coerceData(" << c2STR(c)->GetName() << "," << type::Serial(newtSN)->GetName() << ")");
											HU1 = ha;					// Saved for afterworks
											break;
										}
									case T_INT:
#ifdef N1
										if (newtSN != n1intserial && newtSN != n1botserial)
#else
										if (newtSN != intserial && newtSN != botserial)
#endif
											goto L_FAIL;
										break;

									case T_FLO:
#ifdef N1
										if (newtSN != n1floserial && newtSN != n1botserial)
#else
										if (newtSN != floserial && newtSN != botserial)
#endif
											goto L_FAIL;
										break;
// 									case T_CHR:
// #ifdef N1
// 										if (newtSN != n1chrserial && newtSN != n1botserial)
// #else
// 										if( newtSN != t_chr->GetSerialNo() && newtSN != bot->GetSerialNo() )
// #endif
// 											goto L_FAIL;
// 										break;
									case T_STG:
#ifdef N1
										if (newtSN != n1stgserial && newtSN != n1botserial)
#else
										if (newtSN != stgserial && newtSN != botserial)
#endif
											goto L_FAIL;
										break;
									default:
										ABORT("Internal Error: unknown Tag");
								}
							}
						}
						break;
					default:
						goto L_FAIL;
//						ABORT("Internal Error: unknown Tag");
				}
			}
			break;
			
		case C_FEATADD_C:	/////////////////////////////////////////////////////
			{
				code *oldIP = IP - 1;
				P4("machine::cFeat");
				fserial fSN = GetFeatureSN();
				tserial newtSN = GetTypeSN();
				TRACE("FEATADD_C " << feature::Serial(fSN)->GetName() << " " << type::Serial(newtSN)->GetName());
#ifdef N1
				core_p cp = HA;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HA);
				cell c = ReadHeap(cp);
#endif

				switch( Tag(c) )
				{
					case T_VAR:
						{
							tserial tSN = c2VARS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.result == TS_INVALID )
								goto L_FAIL;
							else
							{
								const type *r = type::Serial(p.result);
#ifdef TYPE_TABLE_HASH
								const coercode *coer = coerceRef(r->GetAppTypeS(p.index-1),newtSN);
								if( coer == NULL )
									goto L_FAIL;
								tserial ser = coer->result_s;
#else
								tserial ser = coerceData(r->GetAppTypeS(p.index-1),newtSN).result_s;
								if( ser == TS_INVALID )
									goto L_FAIL;
#endif

#ifdef N1
								N0WHEAP(cp, PTR2c(HP),c);
#else
								WriteHeap(cp, PTR2c(HP));
#endif

								HA = HP;
#ifdef N1
								int n1nf = r->GetNFeatures();
								CheckHeapLimit(HP+1+n1nf);
#else
								CheckHeapLimit(HP+1+r->GetNFeatures());
#endif
								PutHeap(STR2c(p.result));
								int i;

								// Should call ExpandStructure
								for( i=1; i<p.index; i++ )
									PutHeap(VAR2c(r->GetAppTypeS(i-1)));
								HU1 = HP;
								PutHeap(VAR2c(ser));
#ifdef N1
								for(    ; i < n1nf; i++ )
									PutHeap(VAR2c(r->GetAppTypeS(i)));
#else
								for(    ; i<r->GetNFeatures(); i++ )
									PutHeap(VAR2c(r->GetAppTypeS(i)));
#endif
								UIP = IP;
//								IP = coerceData(r->GetAppTypeS(p.index-1),newtSN).constraint;
								IP = coerceData(botserial,ser).constraint;
								if( IP == NULL ) 
									IP = UIP;
							}
						}
						break;
					case T_STR:
						{
							tserial tSN = c2STRS(c);
							featcode &p = featureCode[tSN][fSN];
							if( p.coercion != NULL )
							{
								UIP = oldIP;		// Return Address; Execute This Once More
								IP = p.coercion;
							// StartUnifProf();	// TEMPORARY
								TRACE("call to featureCode[" << type::Serial(tSN)->GetName() << "][" << feature::Serial(fSN)->GetName() << "]");
								ASSERT(*IP == C_ADDSTR);
								HU1 = cp;					// coercion point
								HA = HP;
							}
							else
							{
								if( p.index == (ushort)(-1) )
									goto L_FAIL;		// Fail to unify

								// ADDNEW CODE
#ifdef N1
								core_p ha = cp+p.index;
								cell c;
								N0DEREF(ha,c);
#else
								core_p ha = Deref(cp+p.index);
								cell c = ReadHeap(ha);
#endif				
								switch( Tag(c) )
								{
									case T_VAR:
										{

#ifdef TYPE_TABLE_HASH
											tserial result;
											const coercode *coer = coerceRef(c2VARS(c),newtSN);
											if( coer == NULL )
												goto L_FAIL;
#else
											tserial result = coerceData(c2VARS(c),newtSN).result_s;
											if( result == S_INVALID )
												goto L_FAIL;
#endif
											else
											{
#ifdef TYPE_TABLE_HASH
												result = coer->result_s;
#endif
#ifdef N1
											    if (tc != result)
											      { N0WHEAP(ha, VAR2c(result), c); }
#else
												WriteHeap(ha, VAR2c(result)); 
#endif
											}

											HU1 = ha;
											UIP = IP;
											IP = coerceData(c2VARS(c),result).constraint;
											if( IP == NULL ) 
												IP = UIP;
											break;
										}
									case T_STR:
										{
											UIP = IP;		// Return Address
											IP = coerceData(c2STRS(c),newtSN).coercion;
							// StartUnifProf();	// TEMPORARY
											TRACE("call to coerceData(" << c2STR(c)->GetName() << "," << type::Serial(newtSN)->GetName() << ")");
											HU1 = ha;					// Saved for afterworks
											break;
										}
									default:
										goto L_FAIL;
								}
							}
						}
						break;
					default:
						goto L_FAIL;
//						ABORT("Internal Error: unknown Tag");
				}
			}
			break;

		case C_FEATCONSTR:	/////////////////////////////////////////////////////
			{
//				code *oldIP = IP - 1;
				P4("machine::cFeat");
				fserial fSN = GetFeatureSN();
				tserial oldtSN = GetTypeSN();
				tserial newtSN = GetTypeSN();
				TRACE("FEATCONSTR " << feature::Serial(fSN)->GetName() << " " << type::Serial(oldtSN)->GetName() << " " << type::Serial(newtSN)->GetName());

#ifdef N1
				core_p cp = HU1;
				cell c;
				N0DEREF(cp,c);
#else
				core_p cp = Deref(HU1);
				cell c = ReadHeap(cp);
#endif

				Push(PTR2c(cp));
				PushIP(UIP);

				if( ConstraintEnabled )
				{
					switch( Tag(c) )
						{
						case T_VAR:
							{
								tserial tSN = c2VARS(c);
								featcode &p = featureCode[tSN][fSN];
								if( p.result == TS_INVALID )
									goto L_FAIL;
								else
								{
									const type *r = type::Serial(p.result);
	#ifdef N1
									N0WHEAP(cp, PTR2c(HP),c);
	#else
									WriteHeap(cp, PTR2c(HP));
	#endif

//									HA = HP;
	#ifdef N1
									int n1nf = r->GetNFeatures();
									CheckHeapLimit(HP+1+n1nf);
	#else
									CheckHeapLimit(HP+1+r->GetNFeatures());
	#endif
									PutHeap(STR2c(p.result));
									int i;

									// Should call ExpandStructure
									for( i=1; i<p.index; i++ )
										PutHeap(VAR2c(r->GetAppTypeS(i-1)));
									HU1 = HP;
									PutHeap(VAR2c(newtSN));
	#ifdef N1
									for(    ; i < n1nf; i++ )
										PutHeap(VAR2c(r->GetAppTypeS(i)));
	#else
									for(    ; i<r->GetNFeatures(); i++ )
										PutHeap(VAR2c(r->GetAppTypeS(i)));
	#endif
								}
							}
							break;
						case T_STR:
							{
								tserial tSN = c2STRS(c);
								featcode &p = featureCode[tSN][fSN];
								if( p.index == (ushort)(-1) )
									goto L_FAIL;		// Fail to unify
								HU1 = Deref(cp+p.index);
							}
							break;
						default:
							goto L_FAIL;
	//						ABORT("Internal Error: unknown Tag");
					}
					UIP = IP;
					IP = coerceData(oldtSN,newtSN).constraint;
					if( IP == NULL ) 
						IP = UIP;
				}
			}
			break;

		case C_FEATCPOP:	/////////////////////////////////////////////////////

			TRACE("FEATCPOP");
			UIP = PopIP();
			HU1 = c2PTR(Pop());
			break;
			
		case C_COMMIT:	/////////////////////////////////////////////////////

			{
				TRACE("COMMIT");
			L_COMMIT:
//				if( ConstraintEnabled )
				{
					if( ! ConstraintQueue.empty() )
					{
						ConstraintPair p = ConstraintQueue.front();
						ConstraintQueue.pop_front();
						int nperm = RP + 2;
						TRACE( "CONSTR1ST ");

						core_p NewFP = SP + CODESIZEONSTACK;
						core_p NewSP = NewFP + nperm + 1 + CODESIZEONSTACK;
						CheckStackLimit( NewSP );

						WriteStackIP( SP, IP );
						WriteStack( NewFP, PTR2c( FP ) );

						// WriteStackIP( NewFP + CODESIZEONSTACK, CIP );
						WriteStackIP( NewFP + 1, CIP );

						FP = NewFP;
						SP = NewSP;

						WriteLocal( -1 - CODESIZEONSTACK, INT2c( RP ) );
						WriteLocal( -2 - CODESIZEONSTACK, PTR2c( HA ) );
						for ( uint i = 0; i < RP; i++ )
							WriteLocal( - i - 3 - CODESIZEONSTACK, ReadLocal( i ) );

						IP = FINALIZE_CONSTRAINT;

						do {
							int nperm = 1;
							TRACE( "CONSTR_MKFRAME ");

							core_p NewFP = SP + CODESIZEONSTACK;
							core_p NewSP = NewFP + nperm + 1;
							CheckStackLimit( NewSP );

							WriteStackIP( SP, IP );
							WriteStack( NewFP, PTR2c( FP ) );

							FP = NewFP;
							SP = NewSP;
							WriteLocal( static_cast<core_p>(-1), PTR2c( p.first ) );

							IP = p.second;

							if (ConstraintQueue.empty())
							{
								p = pair<core_p, code *>(CORE_P_INVALID, NULL);
							}
							else
							{
								p = ConstraintQueue.front();
								ConstraintQueue.pop_front();
							}
						} while( p.second != NULL );
					}
				}
			}
			break;
			
		case C_TRACE:
			{
				TRACE("TRACE");
				int tt = GetInt16();
				const type *t = GetType();
				int x = GetInt16();

				TraceOutput(tt, t, x);
			}
			break;

		case C_INTERRUPTED:
			goto EXITLOOP;

#ifdef XCONSTR

	        case C_CONSTR_HP:
			if( ConstraintEnabled )
			{
				TRACE( "CONSTR_HP (ConstraintEnabled)");
				code *new_SCP = GetInstP();
				ConstraintQueue.push_back(ConstraintPair(HP, new_SCP));
			}
			else
			{
				TRACE( "CONSTR_HP");
				GetInstP();
			}
			break;

	        case C_CONSTR:
			if( ConstraintEnabled )
			{
				TRACE( "CONSTR (ConstraintEnabled)");
				code *new_SCP = GetInstP();
				ConstraintQueue.push_back(ConstraintPair(HU1, new_SCP));
			}
			else
			{
				TRACE( "CONSTR");
				GetInstP();
			}
			break;

		case C_RESTORE_REG:
			{
				TRACE( "RESTORE_REG ");
				RP = c2INT( ReadLocal( -1 - CODESIZEONSTACK ) );
				HA = c2PTR( ReadLocal( -2 - CODESIZEONSTACK ) );
				for ( uint i = 0; i < RP; i++ )
					WriteLocal( i, ReadLocal( - i - 3 - CODESIZEONSTACK ) );

				IP = ReadStackIP( FP + SF_CIP_OFFSET );

				if ( OP < FP || OP == CORE_P_INVALID )
					SP = FP - CODESIZEONSTACK;
				else
					SP = cp_plus( OP + CF_LOCAL_OFFSET,
						      c2PTR( ReadStack( OP + CF_RP_OFFSET ) ) );

				// CIP = ReadStackIP( FP + CODESIZEONSTACK  );
				CIP = ReadStackIP( FP + 1  );
				FP = c2PTR( ReadStack( FP ) );

				if( IP == NULL )
					goto L_SUCCEED;
			}
			break;
#endif // XCONSTR

		case C_PLUS:
			{
				mint v = GetMInt();
				TRACE("C_PLUS " << v);
#ifdef N1
				core_p ha = HA;
				cell c;
				N0DEREF(ha,c);
#else
				core_p ha = Deref(HA);
				cell c = ReadHeap(ha);
#endif
				switch( Tag(c) )
				{
					case T_VAR:
					case T_STR:
						RUNWARN("Instantiation Error in Operator + (or -)");
						goto L_FAIL;
					case T_INT:
						HA = HP;
						CheckHeapLimit(HP+1);
						PutHeap(INT2c(v+c2INT(c)));
						break;
					case T_FLO:
						HA = HP;
						CheckHeapLimit(HP+1);
						PutHeap(FLO2c(v+c2FLO(c)));
						break;
					default:
						goto L_FAIL;
				}
				break;
			}

		case C_MINUS:
			{
				mint v = GetMInt();
				TRACE("C_MINUS " << v);
#ifdef N1
				core_p ha = HA;
				cell c;
				N0DEREF(ha,c);
#else
				core_p ha = Deref(HA);
				cell c = ReadHeap(ha);
#endif
				switch( Tag(c) )
				{
					case T_VAR:
					case T_STR:
						RUNWARN("Instantiation Error in Operator -");
						goto L_FAIL;
					case T_INT:
						HA = HP;
						CheckHeapLimit(HP+1);
						PutHeap(INT2c(v-c2INT(c)));
						break;
					case T_FLO:
						HA = HP;
						CheckHeapLimit(HP+1);
						PutHeap(FLO2c(v-c2FLO(c)));
						break;
					default:
						goto L_FAIL;
				}
				break;
			}

		default:		/////////////////////////////////////////////////////
				cout << "IP = " << hex << reinterpret_cast<ptr_int>(IP-1) << dec << ", *IP = " << hex << (int)*(IP-1) << dec << endl;
				ABORT("Undefined instruction Executed");
		} // end of switch
	} // end of for(;;)
#ifdef N1
	{
	BEGINUNIFY:
	  //  cerr << "BeginUnify" << endl;
	  cell c1, c2;
	  unsigned short result;
	  tserial t1, t2;

	  N0DEREF(HU1,c1);
	  N0DEREF(HU2,c2);
	  //    cerr << c1 << " " << c2 << endl;
	  if (HU1 != HU2) {
	    switch( CT(Tag(c1), Tag(c2)) )
	      {
	      case CT(T_VAR, T_VAR) :
		{
/*
		  if (c1 == n1botcell) {
			N0WHEAP(HU1, PTR2c(HU2), c1);
		  } else if (c2 == n1botcell) {
			N0WHEAP(HU2, PTR2c(HU1), c2);
		  } else {
*/
		    result = coerceData(c2VARS(c2),c2VARS(c1)).result_s;
		    if (result == S_INVALID)
		      goto L_FAIL;
		    if (result == c1) {
			  N0WHEAP(HU2, PTR2c(HU1), c2);
		    } else if (result == c2) {
			  N0WHEAP(HU1,PTR2c(HU2), c1);
		    } else {
			  N0WHEAP(HU1,PTR2c(HU2), c1);
			  N0WHEAP(HU2,VAR2c(result), c2);
		    }
//		  }
		}
		break;
	      case CT(T_VAR, T_STR) :
		{
		  N0WHEAP(HU1,PTR2c(HU2),c1);
		  if (c1 != n1botcell) {
		    t1 = c2VARS(c1);
		    t2 = c2STRS(c2);
		    if (t1 != t2) {
		      HU1 = HU2;
		      IP = coerceData(t2,t1).coercion;
							// StartUnifProf();	// TEMPORARY
		      goto OUT_BEGINUNIFY;
		    }
		  }
		}
		break;
	      case CT(T_STR, T_VAR) :
		{
		  N0WHEAP(HU2,PTR2c(HU1),c2);
		  if (c2 != n1botcell) {
		    t1 = c2STRS(c1);
		    t2 = c2VARS(c2);
		    if (t1 != t2) {
							// StartUnifProf();	// TEMPORARY
		      IP = coerceData(t1,t2).coercion;
		      goto OUT_BEGINUNIFY;
		    }
		  }
		}
		break;
	      case CT(T_VAR, T_INT) :
		{
		  if (c1 != n1intcell && c1 != n1botcell)
		    goto L_FAIL;
		  N0WHEAP(HU1, PTR2c(HU2), c1);
		}
		break;
	      case CT(T_INT, T_VAR) :
		{
		  if (c2 != n1intcell && c2 != n1botcell)
		    goto L_FAIL;
		  N0WHEAP(HU2, PTR2c(HU1), c2);
		}
		break;
	      case CT(T_VAR, T_FLO) :
		{
		  if (c1 != n1flocell && c1 != n1botcell)
		    goto L_FAIL;
		  N0WHEAP(HU1, PTR2c(HU2), c1);
		}
		break;
	      case CT(T_FLO, T_VAR) :
		{
		  if (c2 != n1flocell && c2 != n1botcell )
		    goto L_FAIL;
		  N0WHEAP(HU2, PTR2c(HU1), c2);
		}
		break;
	      case CT(T_VAR, T_CHR) :
		{
		  if (c1 != n1chrcell && c1 != n1botcell )
		    goto L_FAIL;
		  N0WHEAP(HU1, PTR2c(HU2), c1);
		}
		break;
	      case CT(T_CHR, T_VAR) :
		{
		  if (c2 != n1chrcell && c2 != n1botcell )
		    goto L_FAIL;
		  N0WHEAP(HU2, PTR2c(HU1), c2);
		}
		break;
	      case CT(T_VAR, T_STG) :
		{
		  if (c1 != n1stgcell && c1 != n1botcell )
		    goto L_FAIL;
		  N0WHEAP(HU1, PTR2c(HU2), c1);
		}
		break;
	      case CT(T_STG, T_VAR) :
		{
		  if (c2 != n1stgcell && c2 != n1botcell )
		    goto L_FAIL;
		  N0WHEAP(HU2, PTR2c(HU1), c2);
		}
		break;
	      case CT(T_INT, T_INT) :
	      case CT(T_CHR, T_CHR) :
	      case CT(T_FLO, T_FLO) :
	      case CT(T_STG, T_STG) :
		{
		  if (c1 != c2)
		    goto L_FAIL;
		  N0WHEAP(HU2, PTR2c(HU1), c2);
		}
		break;
	      case CT(T_STR, T_STR) :
		{
		  t1 = c2STRS(c1);
		  t2 = c2STRS(c2);
		  if (t1 == t2) {
//		    cerr << "i" << endl;
		    goto N1SIMPLEDYNAMIC;
		  }
		  IP = coerceData(t1,t2).unification;
		  goto OUT_BEGINUNIFY;
		}
		break;
	      default:
		{
		  goto L_FAIL;
		}
	      }
	  }
	  //    cerr << "PROCEED: ";
	  if (queue.isempty()) {
	    //      cerr << "return\n";
	    IP = UIP;
	  OUT_BEGINUNIFY:
	    if (IP == NULL)
	      goto EXITLOOP;
	    goto EXECLOOPTOP;
	  } else {
	    //      cerr << "dequeue\n";
	    HU1 = queue.dequeue();
	    HU2 = queue.dequeue();
	  }
	  goto BEGINUNIFY;

	  N1SIMPLEDYNAMIC:
//	  cerr << "n" << endl;
	  N0WHEAP(HU1, PTR2c(HU2), c1); HU1++; HU2++;
	  switch(type::Serial(t1)->GetNFeatures()) {
	  case 16:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 15:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 14:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 13:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 12:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 11:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 10:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 9:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 8:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 7:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 6:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 5:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 4:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 3:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 2:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
	  case 1:
	    N0PUTUNIFYQ(HU1,HU2); HU1++; HU2++;
// PROCEED;
//	    cerr << type::Serial(t1)->GetNFeatures() << " ";
	    HU1 = queue.dequeue(); HU2 = queue.dequeue();
	    goto BEGINUNIFY;
	  default:
	    {
	      HU1--; HU2--;
	      IP = coerceData(t1,t2).unification;
	      goto OUT_BEGINUNIFY;
	    }
	  }
	}
#endif
      EXITLOOP:
      	if( HA != CORE_P_INVALID && ConstraintEnabled && ! ConstraintQueue.empty() )
      		goto L_COMMIT;
#ifdef N1
//	cerr << "ExitLoop:\n";
#endif	
	;
} // end of ExecLoop()

} // namespace lilfes
