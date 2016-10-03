/* $Id: compile.cpp,v 1.5 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */


static const char rcsid[] = "$Id: compile.cpp,v 1.5 2011-05-02 10:38:23 matuzaki Exp $";

#include "lconfig.h"
#include "ftype.h"
#include "code.h"
#include "codelist.h"
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::ostringstream;


#ifdef DEBUG
//#define DISP_PROGRESS
#define DISP_DEFINES
//#define DEBUGC
//#define DEBUGF
#endif

#ifndef TYPE_TABLE_HASH
coercode **coerceCode = NULL;
#endif
featcode **featureCode = NULL;

#ifdef DEBUG_MEM
#define MAXFEAT 64
#else
#define MAXFEAT 1024
#endif

code *unifyOnly [MAXFEAT+1] = {NULL/*,NULL...*/};
code *unifyOnlyT[MAXFEAT+1] = {NULL/*,NULL...*/};

////////////////////////////////////////
//  type::CompileTypes
//  Generate abstract machine codes for:
//    1) Type coercion
//    2) Adding a feature to a feature structure
//    3) Unification


//static void CompileType(const type *unifiee, const type *unifier);
//static void CompileFeature(const type *t, const feature *f);

//static void CompileCoercion(int ntypes, int nfeats, int maxfeatures);
//static void CompileFeature(int ntypes, int nfeats, int maxfeatures);
//static void CompileUnification(int ntypes, int nfeats, int maxfeatures);

code coerceNone[] = {C_PROCEED, C_HALT};
code onlyFail[] = {C_FAIL, C_HALT};

static void init()
{
	// Default code sequence for 
	//   1) do nothing
	//   2) only fail
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
#if 0
	codelist cn;
	cn.AddCode(C_PROCEED);	cn.EndCode();	coerceNone = cn;  
	new SharedPtr(coerceNone);
	codelist of;
	of.AddCode(C_FAIL);		of.EndCode();	onlyFail = of;    
	new SharedPtr(onlyFail);
#endif
	new SharedPtr(coerceNone, false);
	new SharedPtr(onlyFail, false);
}

static InitFunction IF(init, 1);

static void term() 
{ 
#ifdef DEBUG
	cout << "Terminate " << __FILE__ << endl;
#endif
#if defined(DEBUG_MEM) && ! defined(TYPE_TABLE_HASH)
	for( int i=0; i<type::GetSerialCount(); i++ )
	{
//		cout << "del coerce " << i << endl;
		if( coerceCode[i] != NULL )
		{
			for( int j=0; j<type::GetSerialCount(); j++ )
			{
				DeleteCode(coerceData(i,j).unification);
				DeleteCode(coerceData(i,j).coercion);
			}
			delete[] coerceCode[i];
		}
	}
	delete[] coerceCode;
#endif

//	cout << "del feature " << endl;
	for( int i=0; i<type::GetSerialCount(); i++ )
	{
		delete[] featureCode[i];
	}
	delete[] featureCode;


//	cout << "del cn&of " << endl;
}
static TermFunction TF(term, 6);

#define MAXCONSTRAINTPTYPES 4096
codelist *ConstraintList(const type *unifiee, const type *unifier, const type *unified, bool featchk = false, bool isHP = false)
{
	const type *typelist[MAXCONSTRAINTPTYPES];
	int in = 0;
	int out = 0;

	if( (unified->IsConstrained() == false && (featchk == false || ! unified->IsFeatureConstrained())) || unifiee->IsSubType(unified) || unifier->IsSubType(unified))
#if 0
	{
		codelist *cl = new codelist;
		ostringstream oss;
		oss << "unifiee:" << unifiee->GetName() << ", unifier:" << unifier->GetName() << ", unified:" << unified->GetName();
		RECDATA(*cl, oss.str().c_str());
		return cl;
	}
#else
		return NULL;
#endif

// featchk.
	codelist* cl = NULL;

	if( featchk && unified->IsFeatureConstrained() )
	{
		for( int i=0; i<unified->GetNFeatures(); i++ )
		{
			tserial tx = unifiee->GetAppTypeS(unified->Feature(i));
			tx = (tx == TS_INVALID ? bot->GetSerialNo() : tx);
			if( (unified->GetAppType(i)->IsConstrained() || unified->GetAppType(i)->IsFeatureConstrained())
			 && ( coerceData(tx,unified->GetAppTypeS(i)).constraint != NULL ) )
			{
				if( cl == NULL )
				{
					cl = new codelist;
					REC(*cl);
				}
				cl->AddCode( C_FEATCONSTR );
				cl->AddCode( unified->Feature(i) );
				cl->AddCode( tx );
				cl->AddCode( unified->GetAppType(i) );
				cl->AddCode( C_FEATCPOP );
			}
		}
	}

// main type constraint.
	typelist[out++] = unifiee;
	typelist[out++] = unifier;
	
	while( in < out )
	{
		const type *t = typelist[in++];
		
		for( int k=0; k<t->GetNParents(); k++ )
		{
			typelist[out] = t->Parent(k);		// Sentinel
			int l = 0;
			while( typelist[l] != typelist[out] )
				l++;
			if( l == out )
			{
				out++;
				if( out >= MAXCONSTRAINTPTYPES )
					ABORT("MAXCONSTRAINTPTYPES exceeded");
			}
		}
	};

	int newin = out;
	typelist[out++] = unified;

	while( in < out )
	{
		const type *t = typelist[in++];
		
		for( int k=0; k<t->GetNParents(); k++ )
		{
			typelist[out] = t->Parent(k);		// Sentinel
			int l = 0;
			while( typelist[l] != typelist[out] )
				l++;
			if( l == out )
			{
				out++;
				if( out >= MAXCONSTRAINTPTYPES )
					ABORT("MAXCONSTRAINTPTYPES exceeded");
			}
		}
	};

	for( int k=newin; k<out; k++ )
		if( typelist[k]->GetConstraint() )
		{
			if( cl == NULL )
			{
				cl = new codelist;
				REC(*cl);
				#ifdef DEBUG_LABEL
					ostringstream oss;
					oss << "unifiee:" << unifiee->GetName() << ", unifier:" << unifier->GetName() << ", unified:" << unified->GetName();
					RECDATA(*cl, oss.str().c_str());
					for( int kk=newin-1; kk>=0; kk-- )
						if( typelist[kk]->GetConstraint() )
						{
							ostringstream oss;
							oss << "constraint skipped:" << typelist[kk]->GetName();
							RECDATA(*cl, oss.str().c_str());
						}
				#endif
			}
			for( int kk=k+1; kk<out; kk++ )
			{
#ifdef DEBUG
				//cout << "Check " << typelist[kk]->GetName() << " is subtype of " << typelist[k]->GetName() << endl;
#endif
				if( typelist[kk]->IsSubType(typelist[k]) && typelist[kk]->IsConstrained() )
				{
#ifdef DEBUG
				//	cout << ".. True; Delay " << typelist[k]->GetName() << endl;
#endif
					typelist[out] = typelist[k];
					out++;
					if( out >= MAXCONSTRAINTPTYPES )
						ABORT("MAXCONSTRAINTPTYPES exceeded");
					goto CONTINUE;
				}
			}
			#ifdef DEBUG_LABEL
			{
				ostringstream oss;
				oss << "constraint:" << typelist[k]->GetName();
				RECDATA(*cl, oss.str().c_str());
			}
			#endif
			cl->AddCode(isHP ? C_CONSTR_HP : C_CONSTR, typelist[k]->GetConstraint());
			CONTINUE: ;
		}
	return cl;
}

void CompileType(tserial unifiee, tserial unifier) { CompileType(type::Serial(unifiee), type::Serial(unifier)); }
void CompileType(const type *unifiee, const type *unifier)
{
	serial i = unifiee->GetSerialNo();
	serial j = unifier->GetSerialNo();

	coercode &c = coerceDataAdd(i,j);

#ifdef XCONSTR
//////////////////////////////////////////////////////////////////////////////
// ConstraintCode.
	c.constraint = NULL;
	if( unifier != unifiee )
	{
		const type *unified;
		unified = unifiee->TypeUnify(unifier);
		if( unified != NULL )
		{
			codelist *cl1 = ConstraintList(unifiee, (unifier != unified ? unifier : bot) , unified, true, false);
			if( cl1 != NULL ) 
			{ 
				codelist cl;
				REC(cl);
				cl.AddCodelist(*cl1);
				cl.AddCode(C_PROCEED);
				cl.EndCode();

#ifdef DEBUG
				if( DEB )
				{
					cout << "Constraint " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
					cl.DebugOutput();
				}
#endif
				DeleteCode(c.constraint);
				c.constraint = cl;
				delete cl1; 
			}
		}
	}
#endif

//////////////////////////////////////////////////////////////////////////////
// CoerceCode.

	if( unifiee == unifier )
	{	// CASE 1 : The unifiee and the unifier are the same
		//   Do nothing.
		ASSERT(c.result_s == unifiee->GetSerialNo());
		c.coercion = coerceNone;
		c.coercion_var = coerceNone;
	}
	else {
		const type *unified;

		unified = unifiee->TypeUnify(unifier);
//				c.result_s = unified==NULL ? S_INVALID : unified->GetSerialNo();

		if( unified == NULL )
		{
			// Case 2 : Cannot Unify
			c.coercion = onlyFail;
			c.coercion_var = onlyFail;
		}
		else
		{
			codelist cl;
			bool unifiee_same = (unified->GetNFeatures() == unifiee->GetNFeatures());

			if( unifiee_same )
			{
				int tchange = 0;
				for( int k=0; k<unified->GetNFeatures(); k++ )
				{
					if( unified->GetAppType(k) != unifiee->GetAppType(k) )
						tchange++;
				}
				if( tchange )
				{
					// Case 1'' : No structure changes, but type changes on unifiee
					REC(cl);
					if( unified == unifiee )
						cl.AddCode(C_TCOERCE);
					else {
						cl.AddCode(C_TCOERCET, unified);
					}
					for( int k=0; k<unified->GetNFeatures(); k++ )
					{
						if( unified->GetAppType(k) != unifiee->GetAppType(k) )
						{
							cl.AddCode(C_SKIPF1, unified->GetAppType(k));
							tchange--;
							if( tchange == 0 )
								break;
						}
						else
							cl.AddCode(C_SKIP1);
					}
					cl.AddCode(C_PROCEED);
#ifdef DEBUG
					if( DEB )
					{
						cout << "Coerce " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
					}
#endif
				}
				else
				{
					// Case 1' : No structure changes on unifiee
					//   Do nothing.

					REC(cl);
					if( unified != unifiee )
						cl.AddCode(C_CHGSTR, unified);
					else
						cl.AddCode(C_PROCEED);		// CoerceNone can be used!
#ifdef DEBUG
					if( DEB )
					{
						cout << "Coerce " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
					}
#endif
				}
			}
			else {
				// Case 3' : create new structure
				
				cl.AddCode(C_ADDSTR, unified);
				int unifiee_fp = 0, unified_fp = 0;
				while( unified_fp < unified->GetNFeatures() )
				{
					if( unifiee_fp < unifiee->GetNFeatures() 
					 && unifiee->Feature(unifiee_fp) == unified->Feature(unified_fp) )
					{
						if( unifiee->GetAppType(unifiee_fp) == unified->GetAppType(unified_fp) )
							cl.AddCode(C_BIND);
						else
							cl.AddCode(C_BINDF, unified->GetAppType(unified_fp));
						unifiee_fp++;
						unified_fp++;
					}
					else
					{
#ifdef XCONSTR
						codelist *cl1 = ConstraintList(bot, bot, unified->GetAppType(unified_fp), false, true);
						if( cl1 != NULL ) { REC(cl); cl.AddCodelist(cl1);	delete cl1; }
#endif
						cl.AddCode(C_FRESH, unified->GetAppType(unified_fp));
						unified_fp++;
					}
				}
				if( unifiee->GetNFeatures() != unifiee_fp )
				{
					cout << unifiee->GetNFeatures() << " / " << unifiee_fp;
					ABORT("Internal error: compilation failed");
				}
				cl.AddCode( C_PROCEED );
#ifdef DEBUG
				if( DEB )
				{
					cout << "Coerce " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
				}
#endif
			}
#ifdef XCONSTR
			codelist *cl1 = ConstraintList(unifiee, bot, unified, false);
			codelist *cl2 = ConstraintList(unifiee, unifier, unified, false);

			bool same = ( cl1 == NULL && cl2 == NULL );
				
			if( cl1 == NULL )
				cl1 = new codelist;
			REC(*cl1);
			cl1->AddCodelist(cl);
			cl1->EndCode();
#ifdef DEBUG
			if( DEB ) cl1->DebugOutput();
#endif
			if( c.coercion != c.coercion_var )
				DeleteCode(c.coercion_var);
			DeleteCode(c.coercion);
			c.coercion = (code *)*cl1;

			if( same )
				c.coercion_var = (code *)*cl1;
			else
			{
				if( cl2 == NULL )
					cl2 = new codelist;
				REC(*cl2);
				cl2->AddCodelist(cl);
				cl2->EndCode();
#ifdef DEBUG
				if( DEB ) cl2->DebugOutput();
#endif
				c.coercion_var = (code *)*cl2;

				delete cl2;
			}
			delete cl1;
#else
			cl.EndCode();
			DeleteCode(c.coercion);
			c.coercion = cl;
#endif
		}
	}

//////////////////////////////////////////////////////////////////////////////
// UnifyCode.

	int k;
	if( unifiee == unifier )
	{	// CASE 1 : The unifiee and the unifier are the same
		//   Simply unify all features.
		if( unifyOnly[unifiee->GetNFeatures()] == NULL )
		{
			codelist cl;
			cl.AddCode(C_BINDEO);
			for( k=0; k<unifiee->GetNFeatures(); k++ )
				cl.AddCode(C_UNIFY);
			cl.AddCode(C_PROCEED);
			cl.EndCode();
#ifdef DEBUG
			if( DEB )
			{
				cout << "Unify " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unifiee->GetName() << endl;
				cl.DebugOutput();
			}
#endif
			unifyOnly[unifiee->GetNFeatures()] = (code *)cl;
			new SharedPtr(unifyOnly[unifiee->GetNFeatures()]);
		}
//				if( i==7 )
//					cout << unifiee->GetNFeatures() << " " << reinterpret_cast<int>(unifyOnly[unifiee->GetNFeatures()]) << endl;
		c.unification = unifyOnly[unifiee->GetNFeatures()];
	}
	else {
		const type *unified;

		unified = unifiee->TypeUnify(unifier);
		
		if( unified == NULL )
		{
			// Case 2 : Cannot Unify
			c.unification = onlyFail;
		}
		else
		{
			bool unifiee_same = (unified->GetNFeatures() == unifiee->GetNFeatures());
			bool unifier_same = (unified->GetNFeatures() == unifier->GetNFeatures());

			if( unifiee_same && unifier_same )
			{
				// Case 1' : unified type has the same structure of both
				//   Simply unify all features.
				int tchange = 0;
				for( k=0; k<unified->GetNFeatures(); k++ )
				{
					if( unifiee->GetAppType(k)->TypeUnify(unifier->GetAppType(k)) != unified->GetAppType(k) )
						tchange++;
				}

#ifdef XCONSTR
				codelist *cl1 = ConstraintList(unifiee, unifier, unified);
				if( tchange == 0 && cl1 == NULL)
#else
				if( tchange == 0 )
#endif
				{
					// no AppTypeChange occurs; old routine
					if( unifyOnlyT[unifiee->GetNFeatures()] == NULL )
					{
						codelist cl;
						cl.AddCode(C_BINDEOT);
						for( k=0; k<unifiee->GetNFeatures(); k++ )
							cl.AddCode(C_UNIFY);
						cl.AddCode(C_PROCEED);
						cl.EndCode();
#ifdef DEBUG
						if( DEB )
						{
							cout << "Unify " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
							cl.DebugOutput();
						}
#endif
						unifyOnlyT[unifiee->GetNFeatures()] = cl;
						new SharedPtr(unifyOnlyT[unifiee->GetNFeatures()]);
					}
					DeleteCode(c.unification);
					c.unification = unifyOnlyT[unifiee->GetNFeatures()];
				}
				else
				{
					// AppTypeChange may occurs; new routine
					codelist cl;
#ifdef XCONSTR
					if( cl1 != NULL ) { REC(cl); cl.AddCodelist(cl1);	delete cl1; }
#endif
					cl.AddCode(C_BINDEOT);
					for( k=0; k<unifiee->GetNFeatures(); k++ )
					{
						if( unifiee->GetAppType(k)->TypeUnify(unifier->GetAppType(k)) != unified->GetAppType(k) )
							cl.AddCode(C_UNIFYF, unified->GetAppType(k));
						else
							cl.AddCode(C_UNIFY);
					}
					cl.AddCode(C_PROCEED);
					cl.EndCode();
#ifdef DEBUG
					if( DEB )
					{
						cout << "Unify " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
						cl.DebugOutput();
					}
#endif
					DeleteCode(c.unification);
					c.unification = cl;
				}
			}
			else if( unifiee_same )
			{
				// Case 3 : Unifier subsumes unifiee; gather to unifiee
				codelist cl;

#ifdef XCONSTR
				codelist *cl1 = ConstraintList(unifiee, unifier, unified);
				if( cl1 != NULL ) { REC(cl); cl.AddCodelist(cl1);	delete cl1; }
#endif

				if( unifiee == unified )
					cl.AddCode(C_BIND1);
				else
					cl.AddCode(C_BIND1T, unified);

				int unifiee_fp = 0, unifier_fp = 0, unified_fp = 0;
				while( unifier_fp < unifier->GetNFeatures() )
				{
					if( unifier->Feature(unifier_fp) == unified->Feature(unified_fp) )
					{
						if( unifiee->GetAppType(unifiee_fp)->TypeUnify(unifier->GetAppType(unifier_fp)) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_UNIFYF, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_UNIFY);
						unifiee_fp++;	unifier_fp++;	unified_fp++;
					}
					else
					{
						if( unifiee->GetAppType(unifiee_fp) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_SKIPF1, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_SKIP1);
						unifiee_fp++;					unified_fp++;
					}
				}
				cl.AddCode( C_PROCEED );
				cl.EndCode();
#ifdef DEBUG
				if( DEB )
				{
					cout << "Unify " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
					cl.DebugOutput();
				}
#endif
				DeleteCode(c.unification);
				c.unification = cl;
			}
			else if( unifier_same )
			{
				// Case 4 : Unifiee subsumes unifier; gather to unifier
				codelist cl;
				
#ifdef XCONSTR
				codelist *cl1 = ConstraintList(unifiee, unifier, unified);
				if( cl1 != NULL ) { REC(cl); cl.AddCodelist(cl1);	delete cl1; }
#endif

				if( unifier == unified )
					cl.AddCode(C_BIND2);
				else
					cl.AddCode(C_BIND2T, unified);

				int unifiee_fp = 0, unifier_fp = 0, unified_fp = 0;
				while( unifiee_fp < unifiee->GetNFeatures() )
				{
					if( unifiee->Feature(unifiee_fp) == unified->Feature(unified_fp) ) 
					{
						if( unifiee->GetAppType(unifiee_fp)->TypeUnify(unifier->GetAppType(unifier_fp)) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_UNIFYF, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_UNIFY);
						unifiee_fp++;	unifier_fp++;	unified_fp++;
					} else {
						if( unifier->GetAppType(unifier_fp) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_SKIPF2, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_SKIP2);
										unifier_fp++;	unified_fp++;
					}
				}
				cl.AddCode( C_PROCEED );
				cl.EndCode();
#ifdef DEBUG
				if( DEB )
				{
					cout << "Unify " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
					cl.DebugOutput();
				}
#endif
				DeleteCode(c.unification);
				c.unification = cl;
			}
			else
			{
				// Case 5 : Unified type is a new type; create new feature structure
				codelist cl;
				
#ifdef XCONSTR
				codelist *cl1 = ConstraintList(unifiee, unifier, unified);
				if( cl1 != NULL ) { REC(cl); cl.AddCodelist(cl1);	delete cl1; }
#endif

				cl.AddCode(C_BINDNEW, unified);

				int unifiee_fp = 0, unifier_fp = 0, unified_fp = 0;
				while( unified_fp < unified->GetNFeatures() )
				{
					bool unifiee_eq = ( unifiee_fp < unifiee->GetNFeatures() 
					 && unifiee->Feature(unifiee_fp) == unified->Feature(unified_fp) );
					bool unifier_eq = ( unifier_fp < unifier->GetNFeatures() 
					 && unifier->Feature(unifier_fp) == unified->Feature(unified_fp) );
					 
					if( unifiee_eq && unifier_eq ) {
						if( unifiee->GetAppType(unifiee_fp)->TypeUnify(unifier->GetAppType(unifier_fp)) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_UNIFYF_S, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_UNIFY_S);
						unifiee_fp++;	unifier_fp++;	unified_fp++;
					}
					else if( unifiee_eq )
					{
						if( unifiee->GetAppType(unifiee_fp) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_COPYF1, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_COPY1);
						unifiee_fp++;					unified_fp++;
					}
					else if( unifier_eq )
					{
						if( unifier->GetAppType(unifier_fp) != unified->GetAppType(unified_fp) )
							cl.AddCode(C_COPYF2, unified->GetAppType(unified_fp));
						else
							cl.AddCode(C_COPY2);
										unifier_fp++;	unified_fp++;
					}
					else {
#ifdef XCONSTR
						codelist *cl1 = ConstraintList(bot, bot, unified->GetAppType(unified_fp), false, true);
						if( cl1 != NULL ) { REC(cl); cl.AddCodelist(cl1);	delete cl1; }
#endif
						cl.AddCode(C_FRESH, unified->GetAppType(unified_fp));
														unified_fp++;
					}
				}
				if( unifiee->GetNFeatures() != unifiee_fp || unifier->GetNFeatures() != unifier_fp )
				{
					cout << unifiee->GetNFeatures() << " / " << unifiee_fp << endl;
					cout << unifier->GetNFeatures() << " / " << unifier_fp << endl;
					ABORT("Internal error: compilation failed");
				}
				cl.AddCode( C_PROCEED );
				cl.EndCode();
#ifdef DEBUG
				if( DEB )
				{
					cout << "Unify " << unifiee->GetName() << " , " << unifier->GetName() << " -> " << unified->GetName() << endl;
					cl.DebugOutput();
				}
#endif
				DeleteCode(c.unification);
				c.unification = cl;
			}
		}
	}

//////////////////////////////////////////////////////////////////////////////
// FeatureCode.

	for( serial f=0; f<feature::GetSerialCount(); f++ )
	{
		if( featureCode[i][f].result == S_INVALID && featureCode[j][f].result != S_INVALID )
		{
			int index = unifiee->GetIndex(f);
			
			if( index == -1 )
			{
				const type *coerceTo = unifiee->AcceptableType(feature::Serial(f));
				
				if( coerceTo == NULL )
				{
					// Case 1: Cannot push
					featureCode[i][f].result   = S_INVALID;
					featureCode[i][f].coercion = NULL;
					featureCode[i][f].index    = (ushort)(-1);
				}
				else
				{
					// Case 2: Type coercion is neccesary
					ASSERT(coerceTo->GetIndex(f) != -1);
					featureCode[i][f].result   = coerceTo->GetSerialNo();
					featureCode[i][f].coercion = coerceData(i,coerceTo->GetSerialNo()).coercion;
					featureCode[i][f].index    = (ushort) (coerceTo->GetIndex(feature::Serial(f)) + 1);
		#ifdef DEBUGF
					cout << unifiee->GetName() << "+" << feature::Serial(f)->GetName() << "=" 
						<< coerceTo->GetName() << " ... " << hex << reinterpret_cast<ptr_int>(featureCode[i][f].coercion) << dec << endl;
		#endif
				}
			}
			else
			{
				// Case 3: The feature is on the type already, merely push
					featureCode[i][f].result   = (ushort) (unifiee->GetSerialNo());
					featureCode[i][f].coercion = NULL;
					featureCode[i][f].index    = (ushort) (index + 1);
		#ifdef DEBUGF
					cout << unifiee->GetName() << "+" << feature::Serial(f)->GetName() << "=" 
						<< unifiee->GetName() << " ... merely push" << endl;
		#endif
			}
		}
	}
}

/*
Unification code ...

     Unifiee     Unifier
  E0  STR     R0  STR   
  E1  ...     R1  ...   

* Case 1: The type of the unifier and the type of the unifiee has the same structure

    C_BINDEO	// store PTR R0 on E0 ( or vice versa ) HA++, HA2++ ... types are same
    C_BINDEOT	// store PTR R0 on E0 ( or vice versa ) HA++, HA2++ ... types are not same
  repeat one of them
    C_UNIFY		// unify HA and HA2                     HA++, HA2++
  ...
    C_PROCEED	// unification complete

* Case 2: Can't unify

    C_FAIL		// unable to unify, triggers backtrack

* Case 3: Unifier subsumes unifiee

  use one of them
    C_BIND1		// store PTR EO on R0					HA++, HA2++   ... result type is not different from unifiee's type
    C_BIND1T	// store PTR EO on R0 and store STR		HA++, HA2++   ... result type is different from unifiee's type
  repeat one of them
    C_UNIFY		// unify HA and HA2                     HA++, HA2++
    C_SKIP1		// HA2 doesn't have correspondance		HA++
  ...
    C_PROCEED	// unification complete
    
* Case 4: Unifier subsumes unifiee

  use one of them
    C_BIND2		// store PTR RO on E0					HA++, HA2++   ... result type is not different from unifier's type
    C_BIND2T	// store PTR RO on E0 and store STR		HA++, HA2++   ... result type is different from unifier's type
  repeat one of them
    C_UNIFY		// unify HA and HA2                     HA++, HA2++
    C_SKIP2		// HA2 doesn't have correspondance		HA++
  ...
    C_PROCEED	// unification complete
    
* Case 5 : Unified type is a new type

    C_BINDNEW	// store PTR HP on EO and R0			HA++, HA2++, HP++
  repeat one of them
    C_FRESH		// no correspond feature is on unifier and unifiee   HP++
    C_COPY1		// store PTR HA on HP						HA++, HP++
    C_COPY2		// store PTR HA2 on HP						HA2++, HP++
    C_UNIFY_S	// unify HA and H2 and store pointer on HP  HA++, HA2++, HP++
  ...
    C_PROCEED	// unification complete
*/

} // namespace lilfes

