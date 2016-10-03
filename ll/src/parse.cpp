/* $Id: parse.cpp,v 1.13 2011-05-02 10:38:24 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "parse.h"
#include "proc.h"
#include "code.h"
#include "builtin.h"
#include "in.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>


namespace lilfes {

using std::copy;
using std::cout;
using std::endl;
using std::ends;
using std::make_pair;
using std::ostringstream;
using std::setw;
using std::string;



static const char rcsid[] = "$Id: parse.cpp,v 1.13 2011-05-02 10:38:24 matuzaki Exp $";


const char * const clause_cut_label = "_ CUT _";

// For debugging
ptree *parsetop = NULL;

const char *ptree		::className = "ptree";
const char *pempty		::className = "pempty";
const char *pcut		::className = "pcut";
const char *pfeature	::className = "pfeature";
const char *ptype		::className = "ptype";
const char *pvariable	::className = "pvariable";
const char *pconj		::className = "pconj";
const char *pdisj		::className = "pdisj";
const char *pequal		::className = "pequal";
const char *pcallarg	::className = "pcallarg";
const char *pcall		::className = "pcall";
const char *pgoalconj	::className = "pgoalconj";
const char *pgoaldisj	::className = "pgoaldisj";
const char *pgoaldesc	::className = "pgoaldesc";
const char *pdefarg		::className = "pdefarg";
const char *pprocdef	::className = "pprocdef";
const char *pclause		::className = "pclause";
const char *pquery		::className = "pquery";
const char *pcutregion	::className = "pcutregion";
const char *pinterpret	::className = "pinterpret";
const char *pcalc		::className = "pcalc";
const char *pcalcrev	::className = "pcalcrev";
const char *pint		::className = "pint";
const char *pfloat		::className = "pfloat";
const char *pchar		::className = "pchar";
const char *pstring		::className = "pstring";
#ifdef XCONSTR
const char *pconstraint	::className = "pcoinstraint";
const char *pfeatconstr	::className = "pfeatconstr";
#endif

//////////////////////////////////////////////////////////////////////////////
// pcut

void pcut::CollectInfo(int, parseinfo *info) 
{ 
	PInfoMap::const_iterator it = info->hashcut.find(label);
	if( it == info->hashcut.end() )
	{
		it = info->hashcut.insert(make_pair(label, new pinfo_var(label, -1))).first;
	}
	it->second->NewUse(info->nnewchoice);

	if( it->second->FirstUse() +2 <= it->second->LastUse() )
	{
		PInfoMap::const_iterator it = info->hash.find(label);
		if( it == info->hash.end() )
		{
			it = info->hash.insert(make_pair(label, new pinfo_var(label, 0 /* toriaezu */))).first;
		}
		it->second->NewUse(info->ncall);
	}
}

codelist *pcut::Compile(int, parseinfo *info)
{
	codelist *l = new codelist;

	PInfoMap::const_iterator it = info->hashcut.find(label);
	ASSERT(it != info->hashcut.end());
	pinfo_var *pc = it->second;

	if( pc->FirstUse() == pc->LastUse() )
	{
		REC(*l);
		/* No code necessary */
	} 
	else if( pc->FirstUse() + 1 == pc->LastUse() )
	{
		REC(*l);
		l->AddCode(C_SHALCUT);
	}
	else
	{
		PInfoMap::const_iterator it = info->hash.find(label);
		ASSERT( it != info->hash.end() );
		pinfo_var *pv = it->second;

		REC(*l);
		l->AddCode(C_DEEPCUT, (short)pv->VarNo());
	}
	return l;
}

outform pcut::OutputTree()
{
	outform o("<cut>: ");
	o.AddAtRight(label);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pfeature

void pfeature::CollectInfo(int flag, parseinfo *info) 
{ 
	desc->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pfeature::Compile(int flag, parseinfo *info)
{
	codelist *l; 
	VarState prevha = info->ha_state;
	info->ha_state = PV_SECOND;

	if( flag & PF_FIRSTARG )
	{
		if( info->toparg == NULL )
			info->toparg = bot->AcceptableType(feat);
		else
			info->toparg = info->toparg->AcceptableType(feat);
	}

#ifndef AMAVL_MODE
	if( prevha == PV_SECOND && ( desc->GetClassName() == ptype::className 
							  || desc->GetClassName() == pvariable::className ) )
	{
		info->feat = feat;
		l = desc->Compile(PF_PUSH_COMBINED | (flag & PF_INHERITMASK), info);
	}
	else 
#endif
	{
		l = new codelist;
		codelist *lx = desc->Compile(PF_LASTDESC | (flag & PF_INHERITMASK), info);
#ifndef AMAVL_MODE
		if( flag & PF_LASTDESC )
		{
			REC(*l);
			l->AddCode( prevha == PV_FIRST    ? C_FOLLOW1ST :
						prevha == PV_PROBABLE ? C_FOLLOWPRO :
							                    C_FOLLOW ,
					 feat);
			l->AddCodelist(lx);
		}
		else
#endif
		{
			REC(*l);
			l->AddCode( prevha == PV_FIRST    ? C_PUSH1ST :
						prevha == PV_PROBABLE ? C_PUSHPRO :
							                    C_PUSH ,
					 feat);
			l->AddCodelist(lx);
			l->AddCode(C_POP);
		}
		delete lx;
	}
	return l;
}

outform pfeature::OutputTree()
{
	outform o("<feature>: ");
	o.AddAtRight(outform(feat->GetName()));
	outform p("+-");
	p.AddAtRightTop(desc->OutputTree());
	o.AddAtBottom(p);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// ptype

void ptype::CollectInfo(int, parseinfo *info) 
{
	if( newtype->IsConstrained() || newtype->IsFeatureConstrained() )
			info->neednewchoice+=100;
	else
			info->neednewchoice++;
}

codelist *ptype::Compile(int flag, parseinfo *info)
{
	codelist *l = new codelist;

	if( flag & PF_FIRSTARG )
	{
		if( info->toparg == NULL )
			info->toparg = newtype;
		else
			info->toparg = info->toparg->TypeUnify(newtype);
	}

	REC(*l);
	if( flag & PF_PUSH_COMBINED )
	{
		ASSERT(info->ha_state == PV_SECOND);
		if( newtype->IsConstrained() || newtype->IsFeatureConstrained() )
		{
			l->AddCode( C_FEATADD_C, info->feat );
		}
		else
			l->AddCode( C_FEATADD, info->feat );
		l->AddCode( newtype );
	}
	else if( ! (flag & PF_INCONSTR) )
	{
		if( newtype->IsConstrained() || newtype->IsFeatureConstrained() )
		{
			l->AddCode( info->ha_state == PV_FIRST    ? C_ADDNEW1ST_C :
						info->ha_state == PV_PROBABLE ? C_ADDNEWPRO_C :
							                            C_ADDNEW_C );
		}
		else
			l->AddCode( info->ha_state == PV_FIRST    ? C_ADDNEW1ST :
						info->ha_state == PV_PROBABLE ? C_ADDNEWPRO :
							                            C_ADDNEW );
		l->AddCode( newtype );
	}
	if( newtype->IsConstrained() || newtype->IsFeatureConstrained() )
			info->neednewchoice+=100;
	else
			info->neednewchoice++;
	info->ha_state = PV_SECOND;
	return l;
}

outform ptype::OutputTree()
{
	outform o("<type>: ");
	o.AddAtRight(outform(newtype->GetName()));
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pvariable

void pvariable::CollectInfo(int, parseinfo *info) 
{
	PInfoMap::const_iterator it = info->hash.find(varname);
	if( it == info->hash.end() )
	{
		info->hash.insert(make_pair(varname, new pinfo_var(varname, info->ncall)));
		info->neednewchoice++;
	}
	else
	{
		it->second->NewUse(info->ncall);
	}
}

codelist *pvariable::Compile(int flag, parseinfo *info)
{
	PInfoMap::const_iterator it = info->hash.find(varname);
	codelist *l = new codelist;
	code c;
	
	ASSERT( it != info->hash.end() );
	pinfo_var *pv = it->second;

	REC(*l);

	if( flag & PF_PUSH_COMBINED )
	{
		ASSERT(info->ha_state == PV_SECOND);
		c = pv->GetState() == PV_FIRST    ? C_FEATLINK :
			pv->GetState() == PV_PROBABLE ? C_FEATTEST :
				                            C_FEATUNIFY;
		l->AddCode(c, info->feat);
	}
	else
	{
		if( pv->GetState() == PV_FIRST )
		{
			c = info->ha_state == PV_FIRST    ? C_LINK1ST :
				info->ha_state == PV_PROBABLE ? C_LINKVAR :
					                            C_LINK2ND ;
		}
		else if( pv->GetState() == PV_SECOND )
		{
			c = info->ha_state == PV_FIRST    ? C_UNIFY1ST :
				info->ha_state == PV_PROBABLE ? C_UNIFYVAR :
				                                C_UNIFY2ND ;
			info->neednewchoice++;
		}
/*		else if( pv->GetState() == PV_PROBABLE )
		{
			c = info->ha_state == PV_FIRST    ? C_TEST1ST :
				info->ha_state == PV_PROBABLE ? C_TESTVAR :
				                                C_TEST2ND ;
		} */
		else
			ABORT("Unknown var state");
		l->AddCode(c);
	}

	l->AddCode(pv->VarNo());
	info->ha_state = PV_SECOND;
	pv->SetState(PV_SECOND);

	return l;
}

outform pvariable::OutputTree()
{
	outform o("<variable>: ");
	o.AddAtRight(outform(varname));
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pconj

void pconj::CollectInfo(int flag, parseinfo *info) 
{ 
	desc1->CollectInfo((flag & PF_INHERITMASK), info); 
	desc2->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pconj::Compile(int flag, parseinfo *info)
{
	codelist *l1 = desc1->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG | PF_INCONSTR)), info);
	codelist *l2 = desc2->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG | PF_LASTDESC | PF_INCONSTR)), info);
	
	l1->AddCodelist(l2);
	delete l2;
	return l1;
}

outform pconj::OutputTree()
{
	outform o("<conj>: ");
	outform p("+-");
	p.AddAtRightTop(desc1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(desc2->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pequal

void pequal::CollectInfo(int flag, parseinfo *info) 
{ 
	info->neednewchoice = 0;
	desc1->CollectInfo((flag & PF_INHERITMASK), info); 
	int n1 = info->neednewchoice;
	info->neednewchoice = false;
	desc2->CollectInfo((flag & PF_INHERITMASK), info); 
	int n2 = info->neednewchoice;
#if 0
	if( flag & PF_NEED_EQUAL_CLRTEMP )
	{
		info->calls.push_back(pinfo_call());
		info->calls[info->ncall++].callargs = 0;
	}
#endif
	if( n1 + n2 >= 100 || (n1 && n2) )
		info->nnewchoice+=2;
}

codelist *pequal::Compile(int flag, parseinfo *info)
{
	info->ha_state = PV_FIRST;
	info->neednewchoice = false;
	codelist *l1 = desc1->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG)), info);
	int n1 = info->neednewchoice;

	info->neednewchoice = false;
#ifdef FORCE_DYNAMIC_UNIFY
	VarState vs = info->ha_state;
	info->ha_state = PV_FIRST;
	codelist *l2 = desc2->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG)), info);
#else
	codelist *l2 = desc2->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG | PF_LASTDESC)), info);
#endif
	int n2 = info->neednewchoice;

	
#ifdef FORCE_DYNAMIC_UNIFY
	REC(*l1);
	l1->AddCode(vs == PV_FIRST ? C_LINK1ST : C_LINK2ND, (ushort)0);
	l1->AddCodelist(l2);
	REC(*l1);
	l1->AddCode(info->ha_state == PV_FIRST ? C_UNIFY1ST : C_UNIFY2ND, (ushort)0);
#else
	l1->AddCodelist(l2);
#endif
	delete l2;

	if( n1 + n2 >= 100 || (n1 && n2) )
	{
		info->nnewchoice+=2;
		REC(*l1);
		l1->AddCode(C_COMMIT);
	}

	return l1;
}

outform pequal::OutputTree()
{
	outform o("<equal>: ");
	outform p("+-");
	p.AddAtRightTop(desc1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(desc2->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}


//////////////////////////////////////////////////////////////////////////////
// pdisj

void pdisj::CollectInfo(int flag, parseinfo *info) 
{ 
	desc1->CollectInfo((flag & PF_INHERITMASK), info); 
	desc2->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pdisj::Compile(int flag, parseinfo *info)
{
	parseinfo p1 = *info;
	if( (flag & PF_INDISJ) == 0 )
	{
		// the first disjunction: LASTTRY setting is necessary
		ostringstream oss;
		oss << "_DISJ" << p1.ntemp++ << ends;
                oss.str().copy(p1.disjlabel, sizeof(p1.disjlabel));
	}
	parseinfo p2 = p1;
	
	codelist *l1 = desc1->Compile(PF_INDISJ | (flag & (PF_INHERITMASK | PF_LASTDESC | PF_FIRSTARG)), &p1);
	p2.ntemp = p1.ntemp;
	codelist *l2 = desc2->Compile(PF_INDISJ | (flag & (PF_INHERITMASK | PF_LASTDESC | PF_FIRSTARG)), &p2);
	info->ntemp = p2.ntemp;

	for( PInfoMap::const_iterator it1 = p1.hash.begin();
		 it1 != p1.hash.end(); ++it1 )
	{
		pinfo_var *pv1 = it1->second;
		PInfoMap::const_iterator it2 = p2.hash.find(it1->first);
		ASSERT(it2 != p2.hash.end());
		pinfo_var *pv2 = it2->second;

		*output_stream << pv1->GetName() << " " << pv1->GetState() << "," << pv2->GetState() << endl;
		if( pv2->GetState() == pv1->GetState() )
		{
			PInfoMap::const_iterator it = info->hash.find(pv1->GetName());
			if( it == info->hash.end() )
			{
				it = info->hash.insert(make_pair(pv1->GetName(), new pinfo_var(*pv1))).first;
			}
			it->second->SetState(pv1->GetState());
			continue;
		}
		PInfoMap::const_iterator it_pv = info->hash.find(pv1->GetName());
		if( it_pv == info->hash.end() )
		{
			it_pv = info->hash.insert(make_pair(pv1->GetName(), new pinfo_var(*pv1))).first;
		}
		pinfo_var *pv = it_pv->second;
		if( pv1->GetState() == PV_FIRST )
		{
			REC(*l1);
			l1->AddCode(C_LINK1ST, pv1->VarNo());
		}
		if( pv2->GetState() == PV_FIRST )
		{
			REC(*l2);
			l2->AddCode(C_LINK1ST, pv2->VarNo());
		}
		pv->SetState(PV_SECOND);
	}

	if( p1.ha_state == p2.ha_state )
		info->ha_state = p1.ha_state;
	else
	{
		info->ha_state = PV_SECOND;
	}
	
	codelist *l;
	
	if( (flag & PF_INDISJ) != 0 )
	{
		// now in disjunction: no TRY setting is necessary
		l = l1;
	}
	else
	{
		// desc 1 is not disjunction: TRY setting is necessary
		l = new codelist;
		ostringstream oss;
		oss << "_NTRY" << (info->ntemp++);
		REC(*l);
		l->AddCode(C_TRY, new LabelRefCode(oss.str().c_str()));
		l->AddCodelist(l1);
		REC(*l);
		l->AddCode(C_JUMP, new LabelRefCode(p1.disjlabel));
		l->SetLabelDefCode(new LabelDefCode(oss.str().c_str()));
	}
	
	if( (flag & PF_INDISJ) == 0 )
	{
		// the top disjunction: LASTTRY setting is necessary
		REC(*l);
		l->AddCode(C_LASTTRY);
		l->AddCodelist(l2);
		l->SetLabelDefCode(new LabelDefCode(p1.disjlabel));
	}
	else
	{
		// not the top disjunction: RETRY setting is necessary
		ostringstream oss;
		oss << "_NTRY" << (info->ntemp++);
		REC(*l);
		l->AddCode(C_RETRY, new LabelRefCode(oss.str().c_str()));
		l->AddCodelist(l2);
		REC(*l);
		l->AddCode(C_JUMP, new LabelRefCode(p1.disjlabel));
		l->SetLabelDefCode(new LabelDefCode(oss.str().c_str()));
	}
	
	// Merge the two variable tables, p1.hash and p2.hash .
	// For each variable appears on one of (or both) two tables,
	//   * If both table includes the variable and both table holds the state PV_SECOND,
	//     the result table include the variable and its state is PV_SECOND.
	//   * Otherwise, the result table include the variable and its state is PV_PROBABLE.

	return l;
}

outform pdisj::OutputTree()
{
	outform o("<disj>: ");
	outform p("+-");
	p.AddAtRightTop(desc1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(desc2->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pcallarg

void pcallarg::CollectInfo(int flag, parseinfo *info) 
{ 
	rest->CollectInfo((flag & PF_INHERITMASK), info); 
	desc->CollectInfo((flag & PF_INHERITMASK), info); 
	info->ncallarg ++;
}

codelist *pcallarg::Compile(int flag, parseinfo *info)
{
	codelist *l, *r;
	
	info->ha_state = PV_FIRST;
	r = rest->Compile(flag & PF_INHERITMASK, info);

	info->ha_state = PV_FIRST;
	l = desc->Compile(flag & PF_INHERITMASK, info);

	REC(*l);
	code c;
	c = info->ha_state == PV_FIRST    ? C_LINK1ST :
		info->ha_state == PV_PROBABLE ? C_LINKVAR :
			                            C_LINK2ND ;
	l->AddCode(c, info->ncallarg);
	info->ncallarg ++;

	r->AddCodelist(l);
	delete l;
	
	return r;
}

outform pcallarg::OutputTree()
{
	outform o("<callarg>");
	outform p("+-");
	p.AddAtRightTop(rest->OutputTree());
	outform q("+-");
	q.AddAtRightTop(desc->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pcall

void pcall::CollectInfo(int flag, parseinfo *info) 
{ 
	info->ncallarg = 0;
	arg->CollectInfo((flag & PF_INHERITMASK), info); 

	info->calls.push_back(pinfo_call());
	info->calls.back().callargs = 0;
	if( info->calls[info->ncall].callargs < info->ncallarg  )
		info->calls[info->ncall].callargs = info->ncallarg;
	info->ncall++;

	if (reptype != module::CoreModule()->Search("true") &&
            (reptype->GetModule() != module::BuiltinModule() ||
             reptype == module::BuiltinModule()->Search("call")))   // call/1 requires backtracking (2004.10.14)
        {
		info->nnewchoice+=2;
	}

	if( (flag & PF_LASTCALL) == 0 )
		info->nonlastcallexist = true;
}

codelist *pcall::Compile(int flag, parseinfo *info) 
{
	codelist *l;
	
	info->ncallarg = 0;
	info->neednewchoice = 0;

	l = arg->Compile(flag & PF_INHERITMASK, info);
	procedure *p = procedure::New(reptype, info->ncallarg);

	int n1 = info->neednewchoice;
	if( n1>= 100 )
		l->AddCode(C_COMMIT);

	if( flag & PF_LASTCALL )
	{
		REC(*l);
		if( flag & PF_NEED_DEALLOC ) {
			l->AddCode(C_DEALLOC);
		}
		l->AddCode(C_EXECUTE, p);
//		l->SetLabelDefCode(new LabelDefCodeMarker(info->ncallarg));
		info->lastcall = true;
	}
	else
	{
		REC(*l);
		if (reptype != module::CoreModule()->Search("true")) {
		    l->AddCode(C_CALL, p);
		}
//		l->SetLabelDefCode(new LabelDefCodeMarker(info->ncallarg));
		l->AddCode(C_CLRTEMP, (uint16)info->calls[info->ncall+1].ntemp);
	}
	info->ncall++;
	if (reptype != module::CoreModule()->Search("true") &&
            (reptype->GetModule() != module::BuiltinModule() ||
             reptype == module::BuiltinModule()->Search("call")))   // call/1 requires backtracking (2004.10.14)
	{
		info->nnewchoice+=2;
	}

	return l;
}

outform pcall::OutputTree()
{
	outform o("<call>: ");
	o.AddAtRight(outform(reptype->GetProperName()));
	outform p("+-");
	p.AddAtRightTop(arg->OutputTree());
	o.AddAtBottom(p);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pgoaldesc

void pgoaldesc::CollectInfo(int flag, parseinfo *info) 
{ 
	desc->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pgoaldesc::Compile(int flag, parseinfo *info)
{
	codelist *l, *d;
	
	l = new codelist;
	
	REC(*l);
//	l->AddCode(C_START);
	info->ha_state = PV_FIRST;
	d = desc->Compile(PF_LASTDESC | (flag & PF_INHERITMASK), info);
	l->AddCodelist(d);
	delete d;

	return l;
}

outform pgoaldesc::OutputTree()
{
	outform o("<goaldesc>");
	outform p("+-");
	p.AddAtRightTop(desc->OutputTree());
	o.AddAtBottom(p);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pgoalconj

void pgoalconj::CollectInfo(int flag, parseinfo *info) 
{ 
	goal1->CollectInfo((flag & PF_INHERITMASK), info); 
	goal2->CollectInfo((flag & (PF_INHERITMASK | PF_LASTCALL)), info); 
}

codelist *pgoalconj::Compile(int flag, parseinfo *info) 
{
	codelist *l1, *l2;
	
	l1 = goal1->Compile(flag & PF_INHERITMASK, info);
	l2 = goal2->Compile(flag & (PF_INHERITMASK | PF_LASTCALL), info);
	l1->AddCodelist(l2);
	delete l2;

	return l1;
}

outform pgoalconj::OutputTree()
{
	outform o("<goalconj> ");
	outform p("+-");
	p.AddAtRightTop(goal1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(goal2->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pgoaldisj

void pgoaldisj::CollectInfo(int flag, parseinfo *info) 
{ 
	int norg = info->ncall;
	if( ! (flag & PF_INDISJ ) )
		info->nnewchoice++;
	int corg = info->nnewchoice;


	goal1->CollectInfo(/*PF_INDISJ |*/ (flag & (PF_INHERITMASK | PF_LASTCALL))| PF_NEED_EQUAL_CLRTEMP, info); 
	int nn = info->ncall;
	info->ncall = norg;
	int cc = info->nnewchoice;
	info->nnewchoice = corg;

	goal2->CollectInfo(PF_INDISJ | (flag & (PF_INHERITMASK | PF_LASTCALL))| PF_NEED_EQUAL_CLRTEMP, info); 
	info->ncall = info->ncall < nn ? nn : info->ncall;
	info->nnewchoice = (info->nnewchoice < cc ? cc : info->nnewchoice) + 2;
}

codelist *pgoaldisj::Compile(int flag, parseinfo *info) 
{
	if( (flag & PF_INDISJ) == 0 )
	{
		// the first disjunction: make label for DISJ_END
		ostringstream oss;
		oss << "_DEND" << info->ntemp++ << ends;
                oss.str().copy(info->disjlabel, sizeof(info->disjlabel));
		info->nnewchoice++;
	}
	parseinfo p1 = *info;
	parseinfo p2 = p1;
	
	// Because info, p1, and p2 are sharing same entries of VARS,
	// Save their status here

	_HASHMAP<string, VarState> temp;

	for( PInfoMap::const_iterator it = info->hash.begin();
		 it != info->hash.end(); ++it)
	{
		 temp.insert(make_pair(it->first, it->second->GetState()));
	}

	codelist *l1 = goal1->Compile(/*PF_INDISJ |*/ (flag & (PF_INHERITMASK | PF_LASTCALL))| PF_NEED_EQUAL_CLRTEMP, &p1);
	p2.ntemp = p1.ntemp;

	for( _HASHMAP<string, VarState>::iterator it = temp.begin();
		 it != temp.end(); ++it )
	{
		pinfo_var* pp = p2.hash[it->first];
		VarState v = pp->GetState();
		pp->SetState(it->second);
		it->second = v;
	}

	codelist *l2 = goal2->Compile(PF_INDISJ | (flag & (PF_INHERITMASK | PF_LASTCALL))| PF_NEED_EQUAL_CLRTEMP, &p2);
	info->ntemp = p2.ntemp;

	// Merge the variable table
	if( ( flag & PF_LASTCALL ) == 0 )
	{
		for( _HASHMAP<string, VarState>::const_iterator it = temp.begin();
			 it != temp.end(); ++it)
		{
			pinfo_var *pv1 = p1.hash[it->first];
			VarState pv1state = it->second;
			pinfo_var *pv = info->hash[pv1->GetName()];
			pinfo_var *pv2 = p2.hash[pv->GetName()];

#ifdef DEBUG
			if( DEB )
			{
				cout << pv->GetName() << ": " << pv->GetState() << " " << pv1state << " " << pv2->GetState() << endl;
			}
#endif
			if( pv2->GetState() == pv1state )
			{
				pv->SetState(pv1state);
			}
			else
			{
				if( pv1state != PV_SECOND )
				{
					REC(*l1);
					l1->AddCode(C_LINK1ST, pv1->VarNo());
				}
				if( pv2->GetState() != PV_SECOND )
				{
					REC(*l2);
					l2->AddCode(C_LINK1ST, pv2->VarNo());
				}
				pv->SetState(PV_SECOND);
			}
		}
	}

	info->ncall = p1.ncall < p2.ncall ? p2.ncall : p1.ncall;
	info->nnewchoice = (p1.nnewchoice < p2.nnewchoice ? p2.nnewchoice : p1.nnewchoice) + 2;
	
	codelist *l;
	l = new codelist;
	
	if( (flag & PF_INDISJ) == 0 )
	{
		// the top disjunction: TRY setting is necessary
		ostringstream oss;
		oss << "_DISJ" << info->ntemp++;

		REC(*l);
		l->AddCode(C_TRY, new LabelRefCode(oss.str().c_str()));
		l->AddCodelist(l1);
		delete l1;
		REC(*l);
		l->AddCode(C_JUMP, new LabelRefCode(info->disjlabel));
		l->SetLabelDefCode(new LabelDefCode(oss.str().c_str()));
	}
	else
	{
		// not the top disjunction: RETRY setting is necessary
		ostringstream oss;
		oss << "_DISJ" << info->ntemp++;

		REC(*l);
		l->AddCode(C_RETRY, new LabelRefCode(oss.str().c_str()));
		l->AddCodelist(l1);
		delete l1;
		REC(*l);
		l->AddCode(C_JUMP, new LabelRefCode(info->disjlabel));
		l->SetLabelDefCode(new LabelDefCode(oss.str().c_str()));
	}

	if( goal2->GetClassName() == pgoaldisj::className || goal2->GetClassName() == pcutregion::className )
	{
		// goal 2 is disjunction: no LASTTRY setting is necessary
		l->AddCodelist(l2);
	}
	else
	{
		// goal 2 is not disjunction: LASTTRY setting is necessary
		REC(*l);
		l->AddCode(C_LASTTRY);
		l->AddCodelist(l2);
		l->SetLabelDefCode(new LabelDefCode(info->disjlabel));
		l->AddCode(C_CLRTEMP, (uint16)info->calls[info->ncall].ntemp);
	}

	delete l2;
	return l;
}

outform pgoaldisj::OutputTree()
{
	outform o("<goaldisj> ");
	outform p("+-");
	p.AddAtRightTop(goal1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(goal2->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pdefarg

void pdefarg::CollectInfo(int flag, parseinfo *info) 
{ 
	rest->CollectInfo((flag & PF_INHERITMASK), info); 
	desc->CollectInfo((flag & PF_INHERITMASK), info); 
	info->ndefarg++;
}

codelist *pdefarg::Compile(int flag, parseinfo *info) 
{
	codelist *l, *r;

	info->ha_state = PV_FIRST;
	l = rest->Compile(flag & (PF_INHERITMASK | PF_FIRSTARG | PF_INCONSTR),info);

	if( rest->GetClassName() != pempty::className )
		flag &= ~PF_FIRSTARG;

	REC(*l);
#if defined( FORCE_DYNAMIC_UNIFY) 
	info->ha_state = PV_FIRST;
	r = desc->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG | PF_INCONSTR)),info);
	l->AddCodelist(r);
	if( info->ha_state != PV_FIRST )
	{
		REC(*l);
		l->AddCode(C_UNIFY2ND, info->ncallarg);
	}
	delete r;
#else
	if( desc->GetClassName() != pempty::className )
	{
		l->AddCode(C_UNIFY1ST, info->ncallarg);
		info->ha_state = PV_SECOND;
		r = desc->Compile(PF_LASTDESC | (flag & (PF_INHERITMASK | PF_FIRSTARG | PF_INCONSTR)),info);
		l->AddCodelist(r);
		delete r;
	}
#endif
	info->ncallarg ++;

	return l;
}

outform pdefarg::OutputTree()
{
	outform o("<defarg> ");
	outform p("+-");
	p.AddAtRightTop(rest->OutputTree());
	outform q("+-");
	q.AddAtRightTop(desc->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pprocdef

void pprocdef::CollectInfo(int flag, parseinfo *info) 
{ 
	info->ndefarg = 0;
	defarg->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pprocdef::Compile(int flag, parseinfo *info)
{
	info->ncallarg = 0;
	return defarg->Compile(PF_FIRSTARG | (flag & (PF_INHERITMASK | PF_INCONSTR)), info);
}

outform pprocdef::OutputTree()
{
	outform o("<procdef>: ");
	o.AddAtRight(outform(reptype->GetProperName()));
	outform p("+-");
	p.AddAtRightTop(defarg->OutputTree());
	o.AddAtBottom(p);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pclause

void pclause::CollectInfo(int flag, parseinfo *info) 
{ 
	info->neednewchoice = 0;
	procdef->CollectInfo((flag & PF_INHERITMASK), info); 
	if( info->neednewchoice )
		info->nnewchoice+=2;
	goals->CollectInfo(PF_LASTCALL | (flag & PF_INHERITMASK), info); 
	
	ResolveVariable(info);
}

codelist *pclause::Compile(int flag, parseinfo *info)
{
	codelist *l, *p, *g;
	
	info->ncall = 0;
	info->nnewchoice = 0;
	info->neednewchoice = 0;
	
	l = new codelist;

//	cout << "maxnperm: " << info->maxnperm << endl;
	if( info->maxnperm != 0 || info->nonlastcallexist )
	{
		flag |= PF_NEED_DEALLOC;
		REC(*l);
		l->AddCode(C_ALLOC);
		l->AddCode((uint16)info->maxnperm);
		l->AddCode((uint16)info->calls[0].ntemp);
	}
	else if( info->calls[0].ntemp != 0 )
	{
		REC(*l);
		l->AddCode(C_CLRTEMP);
		l->AddCode((uint16)info->calls[0].ntemp);
	}
	p = procdef->Compile(flag & (PF_INHERITMASK | PF_INCONSTR), info);

	int n1 = info->neednewchoice;

	g = goals->Compile(PF_LASTCALL | (flag & (PF_INHERITMASK)), info);


	l->AddCodelist(p);
	delete p;

	PInfoMap::const_iterator it;
	if( (it = info->hashcut.find(clause_cut_label)) != info->hashcut.end() )
	{
		pinfo_var *pc = it->second;
		if( pc->FirstUse() +2 <= pc->LastUse() )
		{
			pinfo_var *pv = info->hash[clause_cut_label];
			REC(*l);
			l->AddCode(C_GETLEVEL, (short)pv->VarNo());
		}
	}
	if( n1 )
		l->AddCode(C_COMMIT);

	l->AddCodelist(g);
	delete g;
	
	if( info->lastcall == false )
	{
		// No LASTCALL optimization; set RETURN 
		REC(*l);
		if( flag & PF_NEED_DEALLOC ) {
			l->AddCode(C_DEALLOC);
		}
		l->AddCode(C_RETURN);
	}

	return l;
}

outform pclause::OutputTree()
{
	outform o("<clause>");
	outform p("+-");
	p.AddAtRightTop(procdef->OutputTree());
	outform q("+-");
	q.AddAtRightTop(goals->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

void pclause::ResolveVariable(parseinfo *info)
{
	P2("pclause::ResolveVariable");
	int i;

	info->calls.push_back(pinfo_call());
	pinfo_call *p = &(info->calls.back());
	p->callargs = 0;

	int *varno = new int[info->hash.size()];
	for( uint x=0; x<info->hash.size(); x++ )
		varno[x] = info->ncall+1;	// The number is big enough ...

	int maxnperm = 0;
	for( i=info->ncall; i>=0; i-- )
	{
		int ntemp = info->calls[i].callargs;
		if( ntemp == 0 )
			ntemp = 1;
		if( i == 0 )
			if( ntemp < info->ndefarg )
				ntemp = info->ndefarg;
		for( PInfoMap::const_iterator it = info->hash.begin();
			 it != info->hash.end(); ++it )
		{
			pinfo_var *v = it->second;
			if( v->LastUse() == i )
			{
				if( v->FirstUse() == i )
				{
					// temporary variable; set a positive value
					v->SetVarNo(ntemp++);
				}
				else
				{
					// permanent variable; set a negative value
					int k=0;
					while( !( varno[k] > v->LastUse() ) )
					{
						k++;
						ASSERT(k < (int)info->hash.size());
					}
					v->SetVarNo( -(k+1) );
					varno[k] = 0; // = v->firstuse; // IF VAR_SHOKIKA IS ENABLED...
					if( maxnperm < k+1 )
						maxnperm = k+1;
				}
			}
		}
			
		info->calls[i].nperm = maxnperm;
		info->calls[i].ntemp = ntemp;
	}
	delete[] varno;

	info->maxntemp = info->calls[0].ntemp;
	info->maxnperm = maxnperm;
#ifdef DEBUG
	if( DEB )
	{
		cout << " ************* " << endl;
		cout << "Predicate call statistics" << endl;
		for( i=0; i<=info->ncall; i++ )
		{
			cout << setw(3) << i
			     << " : callargs =" << setw(3) << info->calls[i].callargs 
			     << " : nperm =" << setw(3) << info->calls[i].nperm 
			     << " : ntemp =" << setw(3) << info->calls[i].ntemp 
			     << endl;
		}
	#if 1
		cout << " ************* " << endl;
		parseinfo *info2 = new parseinfo(*info);
		for( i=0; i<=info2->ncall; i++ )
		{
			cout << setw(3) << i
			     << " : callargs =" << setw(3) << info2->calls[i].callargs 
			     << " : nperm =" << setw(3) << info2->calls[i].nperm 
			     << " : ntemp =" << setw(3) << info2->calls[i].ntemp 
			     << endl;
		}
		delete info2;
	#endif
		cout << " ************* " << endl;
		cout << "Variable statistics" << endl;
		for( PInfoMap::const_iterator it = info->hash.begin();
			 it != info->hash.end(); ++it )
		{
			pinfo_var *v = it->second;
			if( v != NULL )
			{
				cout << setw(10) << v->GetName() << " :" 
				     << setw(4) << v->FirstUse() << "/" 
				     << setw(4) << v->LastUse() << " -> " 
				     << setw(4) << v->VarNo() 
				     << " (" << v->GetState() << ")"
				     << endl;
			}
		}
		cout << " ************* " << endl;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
// pquery

void pquery::CollectInfo(int flag, parseinfo *info) 
{ 
	goals->CollectInfo((flag & PF_INHERITMASK), info); 
	
	ResolveVariable(info);
}

codelist *pquery::Compile(int flag, parseinfo *info)
{
	codelist *l, *g;

	info->ncall = 0;
	
	l = new codelist;
	
	if( goals->GetClassName() != pempty::className )
	{
		flag |= PF_NEED_DEALLOC;
		REC(*l);
		l->AddCode((uchar)C_ALLOC);
		l->AddCode((uint16)info->maxnperm);
		l->AddCode((uint16)info->calls[0].ntemp);
	}
	else if( info->calls[0].ntemp != 0 )
	{
		REC(*l);
		l->AddCode(C_CLRTEMP);
		l->AddCode((uint16)info->calls[0].ntemp);
	}
	g = goals->Compile((flag & PF_INHERITMASK), info);
	l->AddCodelist(g);
	delete g;
	
	REC(*l);
	l->AddCode(C_SUCCEED);

	return l;
}

outform pquery::OutputTree()
{
	outform o("<query>");
	outform p("+-");
	p.AddAtRightTop(goals->OutputTree());
	o.AddAtBottom(p);
	return o;
}

void pquery::ResolveVariable(parseinfo *info)
{
	P2("pquery::ResolveVariable");
	int i;
	uint u;

	info->calls.push_back(pinfo_call());
	pinfo_call *p = &(info->calls.back());
	p->callargs = 0;

	int *varno = new int[info->hash.size()];
	for( u=0; u<info->hash.size(); u++ )
		varno[u] = info->ncall+1;	// The number is big enough ...

	int maxnperm = 0;
	for( i=info->ncall; i>=0; i-- )
	{
		int ntemp = info->calls[i].callargs;
		if( ntemp == 0 )
			ntemp = 1;
		for( PInfoMap::const_iterator it = info->hash.begin();
			 it != info->hash.end(); ++it )
		{
			pinfo_var *v = it->second;
			if( v->LastUse() == i )
			{
				if( *(v->GetName()) == '_' )	// if it is anonymous...
				{
					// temporary variable; set a positive value
					v->SetVarNo(ntemp++);
				}
				else
				{
					// permanent variable; set a negative value
					int k=0;
					while( !( varno[k] > v->LastUse() ) )
					{
						k++;
						ASSERT(k < (int)info->hash.GetNData());
					}
					v->SetVarNo( -(k+1) );
					varno[k] = 0; // Because all variable should be saved
					if( maxnperm < k+1 )
					{
						maxnperm = k+1;
					}
				}
			}
		}
			
		info->calls[i].ntemp = ntemp;
	}
	delete[] varno;

	for( i=0; i<=info->ncall; i++ )
		info->calls[i].nperm = maxnperm;

	varlist = new _varlist[maxnperm];
	int x = 0;
	for( PInfoMap::const_iterator it = info->hash.begin();
		 it != info->hash.end(); ++it )
	{
		pinfo_var *v = it->second;
		if( v->VarNo() < 0 )
		{
			varlist[x].name = v->GetName();
			varlist[x].varno = v->VarNo();
			x++;
		}
	}
	ASSERT( x == maxnperm );
	nvars = x;
	info->maxntemp = info->calls[0].ntemp;
	info->maxnperm = maxnperm;

#ifdef DEBUG
	if( DEB )
	{
		cout << " ************* " << endl;
		cout << "Predicate call statistics" << endl;
		for( i=0; i<=info->ncall; i++ )
		{
			cout << setw(3) << i
			     << " : callargs =" << setw(3) << info->calls[i].callargs 
			     << " : nperm =" << setw(3) << info->calls[i].nperm 
			     << " : ntemp =" << setw(3) << info->calls[i].ntemp 
			     << endl;
		}
		cout << " ************* " << endl;
		cout << "Variable statistics" << endl;
		for( PInfoMap::const_iterator it = info->hash.begin();
			 it != info->hash.end(); ++it )
		{
			pinfo_var *v = it->second;
			cout << setw(10) << v->GetName() << " :" 
					  << setw(4) << v->FirstUse() << "/" 
					  << setw(4) << v->LastUse() << " -> " 
					  << setw(4) << v->VarNo() 
					  << " (" << v->GetState() << ")"
					  << endl;
		}
		cout << " ************* " << endl;
	}
#endif
}
/*
bool pquery::DisplayResult(machine &m)
{
	m.DisplayStatistics();
	if( nvars == 0 )
	{
		cout << "yes" << endl;
		return false;
	}
	else
	{
		core_p *ml = new core_p[STRUCTURE_MAXSIZE];
		int *mlcnt = new int[STRUCTURE_MAXSIZE];
		size_t mlsize = 0;
		core_p *share;
		int    *sharecnt;
		size_t nshare;
		int i;

		for( i=0; i<nvars; i++ )
		{
			m.DetectSharedStructure(c2PTR(m.ReadLocal(varlist[i].varno)), ml, mlcnt, mlsize);
		}

		m.MakeShareArray(ml, mlcnt, mlsize,  share, sharecnt, nshare);
		delete[] ml;
		delete[] mlcnt;
	
		
		for( i=0; i<nvars; i++ )
		{
			outform of(varlist[i].name);
			of.AddAtRight(outform(": "));
			of.AddAtRight(m.FormatStructure(c2PTR(m.ReadLocal(varlist[i].varno)), share, sharecnt, nshare));
			cout << of;
		}

		delete[] share;
		delete[] sharecnt;

		for(;;) {
			int c = getchar();
			if( c == EOF )
				return true;
			if( c == '\n' )
				return false;
			if( c == ';' )
			{
				while( getchar() != '\n' )
					;
				return true;
			}
			cout << "';' for more choices, otherwise press ENTER --> ";
		}
	}
}
*/
//////////////////////////////////////////////////////////////////////////////
// pcutregion

void pcutregion::CollectInfo(int flag, parseinfo *info) 
{
	PInfoMap::const_iterator it = info->hashcut.find(label);
	if( it == info->hashcut.end() )
	{
		info->hashcut.insert(make_pair(label, new pinfo_var(label, info->nnewchoice - (flag & PF_INDISJ ? 1 : 0))));
	}
	else
	{
		it->second->NewUse(info->nnewchoice);
	}
	
	goals->CollectInfo((flag & (PF_INHERITMASK | PF_LASTCALL | PF_INDISJ)), info);
}

codelist *pcutregion::Compile(int flag, parseinfo *info)
{
	codelist *l = new codelist;

	PInfoMap::const_iterator it_pc = info->hashcut.find(label);
	if( it_pc != info->hashcut.end() && it_pc->second->FirstUse() +2 <= it_pc->second->LastUse() )
	{
		PInfoMap::const_iterator it_pv = info->hash.find(label);
		ASSERT(it_pv != info->hash.end());
		REC(*l);
		l->AddCode(C_GETCP, (short)it_pv->second->VarNo());
	}
	
	codelist *l2 = goals->Compile((flag & (PF_INHERITMASK | PF_LASTCALL | PF_INDISJ)), info);

	if( !(flag & PF_INDISJ) || goals->GetClassName() == pgoaldisj::className || goals->GetClassName() == pcutregion::className )
	{
		// goal is disjunction: no LASTTRY setting is necessary
		l->AddCodelist(l2);
	}
	else
	{
		// goal 2 is not disjunction: LASTTRY setting is necessary
		REC(*l);
		l->AddCode(C_LASTTRY);
		l->AddCodelist(l2);
		l->SetLabelDefCode(new LabelDefCode(info->disjlabel));
		l->AddCode(C_CLRTEMP, (uint16)info->calls[info->ncall].ntemp);
	}

	delete l2;
	return l;
}

outform pcutregion::OutputTree()
{
	outform o("<cutregion>: ");
	o.AddAtRight(label);
	outform p("+-");
	p.AddAtRightTop(goals->OutputTree());
	o.AddAtBottom(p);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pinterpret

void pinterpret::CollectInfo(int /*flag*/, parseinfo */*info*/) 
{ 
	/* do nothing */
}

codelist *pinterpret::Compile(int /*flag*/, parseinfo * /*info*/)
{
	codelist *cl = new codelist;
	REC(*cl);
	codelist *g = fs.GetMachine()->Interpret(fs);
	cl->AddCodelist(*g);
	delete g;
	return cl;
}

outform pinterpret::OutputTree()
{
	outform o("<interpret>: ");
	outform p("+-");
	p.AddAtRightTop(fs.DisplayAVM());
	o.AddAtBottom(p);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pcalc

void pcalc::CollectInfo(int flag, parseinfo *info) 
{ 
	p1->CollectInfo((flag & PF_INHERITMASK), info); 
	result->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pcalc::Compile(int flag, parseinfo *info)
{
	info->ha_state = PV_FIRST;
	codelist *l1 = p1->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG)), info);

	mint i = p2.ReadInteger();
	if( op->IsSubType(t_plus) )
	{
		REC(*l1);
		l1->AddCode(C_PLUS);
		l1->AddCode(i);
	}
	else if( op->IsSubType(t_minus) )
	{
		REC(*l1);
		l1->AddCode(C_PLUS);
		l1->AddCode(-i);
	}

	codelist *lresult = result->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG | PF_LASTDESC)), info);
	l1->AddCodelist(lresult);
	delete lresult;
	return l1;
}

outform pcalc::OutputTree()
{
	outform o("<calc>: ");
	outform p("+-");
	p.AddAtRightTop(p1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(result->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pcalcrev

void pcalcrev::CollectInfo(int flag, parseinfo *info) 
{ 
	p1->CollectInfo((flag & PF_INHERITMASK), info); 
	result->CollectInfo((flag & PF_INHERITMASK), info); 
}

codelist *pcalcrev::Compile(int flag, parseinfo *info)
{
	info->ha_state = PV_FIRST;
	codelist *l1 = p1->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG)), info);

	mint i = p2.ReadInteger();
	if( op->IsSubType(t_plus) )
	{
		REC(*l1);
		l1->AddCode(C_PLUS);
		l1->AddCode(i);
	}
	else if( op->IsSubType(t_minus) )
	{
		REC(*l1);
		l1->AddCode(C_MINUS);
		l1->AddCode(i);
	}
	
	codelist *lresult = result->Compile((flag & (PF_INHERITMASK | PF_FIRSTARG | PF_LASTDESC)), info);
	l1->AddCodelist(lresult);
	delete lresult;
	return l1;
}

outform pcalcrev::OutputTree()
{
	outform o("<calcrev>: ");
	outform p("+-");
	p.AddAtRightTop(p1->OutputTree());
	outform q("+-");
	q.AddAtRightTop(result->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}

//////////////////////////////////////////////////////////////////////////////
// pint

codelist *pint::Compile(int flag, parseinfo *info)
{
	codelist *l = new codelist;

	if( flag & PF_FIRSTARG )
	{
		if( info->toparg == NULL )
			info->toparg = t_int;
		else
			info->toparg = info->toparg->TypeUnify(t_int);

		if( info->is_toparg == false )
		{
			info->is_toparg = true;
			info->toparg_int = val;
		}
	}


	REC(*l);
//	if( flag & PF_PUSH_COMBINED )
//	{
//		ASSERT(info->ha_state == PV_SECOND);
//		l->AddCode( C_FEATADD, info->feat );
//	}
//	else
		l->AddCode( info->ha_state == PV_FIRST    ? C_SETINT1ST :
					info->ha_state == PV_PROBABLE ? C_SETINTPRO :
						                            C_SETINT );
	l->AddCode( (mint)val );
	info->ha_state = PV_SECOND;
	return l;
}

outform pint::OutputTree()
{
	ostringstream oss;
	oss << "<int>: " << val;
	return outform(oss.str().c_str());
}

//////////////////////////////////////////////////////////////////////////////
// pfloat

codelist *pfloat::Compile(int flag, parseinfo *info)
{
	codelist *l = new codelist;

	if( flag & PF_FIRSTARG )
	{
		if( info->toparg == NULL )
			info->toparg = t_flo;
		else
			info->toparg = info->toparg->TypeUnify(t_flo);
	}

	REC(*l);
//	if( flag & PF_PUSH_COMBINED )
//	{
//		ASSERT(info->ha_state == PV_SECOND);
//		l->AddCode( C_FEATADD, info->feat );
//	}
//	else
		l->AddCode( info->ha_state == PV_FIRST    ? C_SETFLO1ST :
					info->ha_state == PV_PROBABLE ? C_SETFLOPRO :
						                            C_SETFLO );
	l->AddCode( (mfloat)val );
	info->ha_state = PV_SECOND;
	return l;
}

outform pfloat::OutputTree()
{
	ostringstream oss;
	oss << "<float>: " << val;
	return outform(oss.str().c_str());
}

//////////////////////////////////////////////////////////////////////////////
// pchar

codelist *pchar::Compile(int flag, parseinfo *info)
{
	codelist *l = new codelist;

	if( flag & PF_FIRSTARG )
	{
		if( info->toparg == NULL )
			info->toparg = t_chr;
		else
			info->toparg = info->toparg->TypeUnify(t_chr);
	}

	REC(*l);
//	if( flag & PF_PUSH_COMBINED )
//	{
//		ASSERT(info->ha_state == PV_SECOND);
//		l->AddCode( C_FEATADD, info->feat );
//	}
//	else
		l->AddCode( info->ha_state == PV_FIRST    ? C_SETCHR1ST :
					info->ha_state == PV_PROBABLE ? C_SETCHRPRO :
						                            C_SETCHR );
	l->AddCode( val );
	info->ha_state = PV_SECOND;
	return l;
}

outform pchar::OutputTree()
{
	ostringstream oss;
	oss << "<char>: " << val;
	return outform(oss.str().c_str());
}


//////////////////////////////////////////////////////////////////////////////
// pstring

codelist *pstring::Compile(int flag, parseinfo *info)
{
	codelist *l = new codelist;

	if( flag & PF_FIRSTARG )
	{
		if( info->toparg == NULL )
			info->toparg = t_stg;
		else
			info->toparg = info->toparg->TypeUnify(t_stg);

		if( info->is_toparg == false )
		{
			info->is_toparg = true;
			info->toparg_stg = strsymbol::New(info->mach, val)->GetSerialNo();
		}
	}

	REC(*l);
//	if( flag & PF_PUSH_COMBINED )
//	{
//		ASSERT(info->ha_state == PV_SECOND);
//		l->AddCode( C_FEATADD, info->feat );
//	}
//	else
		l->AddCode( info->ha_state == PV_FIRST    ? C_SETSTG1ST :
					info->ha_state == PV_PROBABLE ? C_SETSTGPRO :
						                            C_SETSTG );
	l->AddCode( (sserial)strsymbol::New(info->mach, val)->GetSerialNo() );
	strsymbol::New(info->mach, val)->SetPermanent();
	info->ha_state = PV_SECOND;
	return l;
}

outform pstring::OutputTree()
{
	ostringstream oss;
	oss << "<string>: " << val;
	outform of = outform(oss.str().c_str());
	return of;
}

#ifdef XCONSTR
//////////////////////////////////////////////////////////////////////////////
// pconstraint

codelist *pconstraint::Compile(int flag, parseinfo *info)
{
	codelist *l;
	codelist *l1 = pclause::Compile(flag | PF_INCONSTR, info);

	l = new codelist;
	REC(*l);
	l->AddCode(C_CLRTEMP, (ushort) 1);
	l->AddCode(C_UNIFY1ST, (ushort) -1);
	l->AddCode(C_LINK2ND, (ushort) 0);
	l->AddCode(C_DEALLOC);
	l->AddCodelist(l1);
	delete l1;
	
	return l;
}

outform pconstraint::OutputTree()
{
	outform o("<constraint>");
	outform p("+-");
	p.AddAtRightTop(procdef->OutputTree());
	outform q("+-");
	q.AddAtRightTop(goals->OutputTree());
	o.AddAtBottom(p);
	o.AddAtBottom(q);
	return o;
}


//////////////////////////////////////////////////////////////////////////////
// pfeatconstr

codelist *pfeatconstr::Compile(int, parseinfo *)
{
	codelist *l = new codelist;

	REC(*l);

	for( int i=0; i<constrtype->GetNFeatures(); i++ )
	{
		if( constrtype->GetAppType(i)->IsConstrained() )
		{
			l->AddCode( C_FEATCONSTR );
			l->AddCode( constrtype->Feature(i) );
			l->AddCode( constrtype->GetAppType(i) );
		}
	}
	return l;
}

outform pfeatconstr::OutputTree()
{
	outform o("<type>: ");
	o.AddAtRight(outform(constrtype->GetName()));
	return o;
}

#endif // XCONSTR


/*
HA が 
  初期化されてない     - 0
  初期化されているかも - 3
  初期化されている     - 6

VAR が
  初期化されてない     - 0
  初期化されているかも - 1
  初期化されている     - 2

  0 : LINK1ST
  1 : TEST1ST
  2 : UNIFY1ST
  
  3 : LINKVAR
  4 : TESTVAR
  5 : UNIFYVAR
  
  6 : LINK2ND
  7 : TEST2ND
  8 : UNIFY2ND
  -----------------
  0 : ADDNEW1ST    PUSH1ST  FOLLOW1ST
  1 : ADDNEWPRO    PUSHPRO  FOLLOWPRO
  2 : ADDNEW       PUSH     FOLLOW
*/


} // namespace lilfes
