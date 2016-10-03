/* $Id: ftype.cpp,v 1.7 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1996-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: ftype.cpp,v 1.7 2011-05-02 10:38:23 matuzaki Exp $";

#include "ftype.h"
#include "structur.h"
#include "builtin.h"
#include "codelist.h"
#include "parse.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <search.h>

namespace lilfes {

using std::copy;
using std::cout;
using std::endl;
using std::make_pair;
using std::memcpy;
using std::pair;
using std::replace;
using std::string;
using std::vector;


#ifdef TYPE_TABLE_HASH

const coercode coercode_fail = {
	0, TS_INVALID, onlyFail, onlyFail, NULL, onlyFail };
_HASHMAP<int, coercode *> coerceHash;
#endif


bool type::IsExtendable() const
{
	const type *e = module::CoreModule()->Search("extendable");
	return GetModule()->GetExtendableMode() || (e!=NULL && attr!=NULL && attr->GetType()->IsSubType(e));
}


string Quote(const string &name, bool AllowCapital)
{
//	printf("Quote: < %s\n", name);
	char first = name[0];
	if( (islower( first )) || (AllowCapital && isupper( first )) || (signed char)first < 0 )
	{
		const char *p = name.c_str();
		while( *p != '\0' )
		{
			if( (signed char)*p < 0 && *(p+1) != '\0' )
			{
				p+=2;
			}
			else if( isalnum(*p) || *p == '$' || *p == '_' )
			{
				p++;
			}
			else
			{
				break;
			}
		}
		if( *p == '\0' )
		{
			return name;
		}
	}
	
	string ret = "'";
	for( string::const_iterator it =name.begin();
		 it != name.end(); ++it)
	{
		if (*it == '\'')
		{
			ret += '\'';
		}
		ret += *it;
	}
	ret += '\'';

	return ret;
}


////////////////////////////////////////
// Global variables

// bot
//    Bottom type. It should be the root type of all types.
type *bot;

static void init() 
{ 
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif

	bot = new type("bot", module::CoreModule()); 

	type *feat = new type("feature", module::CoreModule());
	feat->SetAsChildOf(bot);
	feat->Fix();
	t_feature = feat;

	type *type_attribute = new type("type_attr", module::CoreModule());
	type_attribute->SetAsChildOf(bot);
	type_attribute->Fix();

//	type *special = new type("special_type"); special->SetAsChildOf(type_attribute); special->Fix();
	type *final = new type("final", module::CoreModule());
	final->SetAsChildOf(type_attribute);
	final->Fix();

	type *extendable = new type("extendable", module::CoreModule());
	extendable->SetAsChildOf(type_attribute);
	extendable->Fix();

	type *type_constrained = new type("type_constrained", module::CoreModule()); 
	type_constrained->SetAsChildOf(type_attribute); 
	type_constrained->AddFeature(new feature("constr\\", module::CoreModule())); 
	type_constrained->AddFeature(new feature("pred\\", module::CoreModule())); 
	type_constrained->Fix();

	type *type_finalconst = new type("type_final_constrained", module::CoreModule());
	type_finalconst->SetAsChildOf(final);
	type_finalconst->SetAsChildOf(type_constrained);
	type_finalconst->Fix();

	type *type_extconst = new type("type_extendable_constrained", module::CoreModule());
	type_extconst->SetAsChildOf(extendable);
	type_extconst->SetAsChildOf(type_constrained);
	type_extconst->Fix();
}

#ifdef XCONSTR
code *finalize_constraint;

static void init2() 
{ 
	codelist l;
	l.AddCode(C_RESTORE_REG);
	l.AddCode(C_RETURN);
	l.EndCode();
	finalize_constraint = l;
}
#endif

static void term()
{
#ifdef XCONSTR
	delete[] finalize_constraint;
#endif
	type::terminate();
	feature::terminate();
}


static InitFunction IF(init, 10);
#ifdef XCONSTR
static InitFunction IF2(init2, 120);
#endif
static TermFunction TF(term, 10);

void type::terminate()
{
	uint i;
	for( _HASHMAP<int, coercode *>::const_iterator it = coerceHash.begin();
		 it != coerceHash.end(); ++it)
	{
		coercode *p = it->second;
		if( p )
		{
			if( p->coercion != p->coercion_var ) {
				DeleteCode(p->coercion);
			}
			DeleteCode(p->coercion_var);
			DeleteCode(p->constraint);
			DeleteCode(p->unification);
			delete p;
		}
	}

	for( i=0; i<GetSerialCount(); i++ )
	{
		delete (type *)(serialtable[i]);
	}
}

void feature::terminate()
{
	for( uint i=0; i<GetSerialCount(); i++ )
	{
		delete (feature *)(serialtable[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// class feature
//

// Static data member

vector<const feature *> feature::serialtable;
_HASHMAP<tserial, feature *> feature::hash;

fserial feature::builtin_limit = 0;
// Constructor

feature::feature(const string &in, module *mod, int ip, bool iexported)
{ 
	priority = ip; 
	basetype = S_INVALID;

	const string &l = in; // l = l + FEATURE_POSTFIX;
	const type *t = mod->Search(l);
	if( t == NULL )
	{
		type *tt = new type(l, mod, iexported);
		tt->SetAsChildOf(t_feature);
		tt->Fix();
		procedure::NewTypeNotify();
		t = tt;
	}
	else
	{
		if( ! t->IsSubType(t_feature) )
		{
			ABORT("Overlapping feature reptype");
		}
	}
	reptype = t;
	name = t->GetSimpleName();
	propername = t->GetProperName();
	ASSERT(propername.length() >= 2);
	propername.replace(propername.length() - 2, 2, "'");

	pair<_HASHMAP<tserial, feature *>::iterator, bool> ret = hash.insert(make_pair(this->GetKey(), this));
	if( !ret.second ) /// 既に同じキーを持つものが存在していた場合
	{
		ABORT("Duplicating feature reptype");
	}

	serialno = (fserial)serialtable.size();

	if( (serialno & ((1 << FEATTABLE_GROW_BITS)-1)) == 0 )
	{
		P2("feature::feature - ExpandTypeTable");
		int newsize = serialno + (1 << FEATTABLE_GROW_BITS);
		for( int i=0; i<type::GetSerialCount(); i++ )
		{
			featcode *newfeatureCode = new featcode[newsize];
			memcpy( newfeatureCode, featureCode[i], serialno*sizeof(featcode) );
			delete[] featureCode[i];
			featureCode[i] = newfeatureCode;
		}
	}

	for( int i=0; i<type::GetSerialCount(); i++ )
	{
		featureCode[i][serialno].result = S_INVALID;
		featureCode[i][serialno].index = (ushort)(-1);
		featureCode[i][serialno].coercion = NULL;
	}

	serialtable.push_back(this);
#ifdef FEATURETABLE
	type::ExpandFeatureTable(GetSerialCount());
#endif
}

feature::feature(const type *t, int ip)
{ 
	priority = ip; 
	basetype = S_INVALID;

	reptype = t;
	name = t->GetSimpleName();		
	propername = t->GetProperName();	
	if( name[name.length()-1] == '\\' )
	{
		name.replace(name.length()-1, 1, "");
		ASSERT(propername.length() >= 2);
		propername.replace(propername.length()-2, 2, "'");
	}
		
	pair<_HASHMAP<tserial, feature *>::iterator, bool> ret = hash.insert(make_pair(this->GetKey(), this));
	if( !ret.second )
	{
		ABORT("Duplicating feature reptype");
	}

	serialno = (fserial)serialtable.size();

	if( (serialno & ((1 << FEATTABLE_GROW_BITS)-1)) == 0 )
	{
		P2("feature::feature - ExpandTypeTable");
		int newsize = serialno + (1 << FEATTABLE_GROW_BITS);
		for( int i=0; i<type::GetSerialCount(); i++ )
		{
			featcode *newfeatureCode = new featcode[newsize];
			memcpy( newfeatureCode, featureCode[i], serialno*sizeof(featcode) );
			delete[] featureCode[i];
			featureCode[i] = newfeatureCode;
		}
	}

	for( int i=0; i<type::GetSerialCount(); i++ )
	{
		featureCode[i][serialno].result = S_INVALID;
		featureCode[i][serialno].index = (ushort)(-1);
		featureCode[i][serialno].coercion = NULL;
	}

	serialtable.push_back(this);
#ifdef FEATURETABLE
	type::ExpandFeatureTable(GetSerialCount());
#endif
}


// Destructor

feature::~feature() 
{ 
//	hash.Delete(this);
//	delete[] name; 
}

void feature::Delete()
{
	if( !GetRepType()->IsDeleted() )
	{
		return;
	}
	hash.erase(this->GetKey());
	return;
}

const string &feature::GetPrintName() const 
{ 
	if( module::UserModule()->Search(reptype->GetSimpleName()) == reptype )
	{
		return name;
	}
	else
	{
		return propername;
	}
}


//////////////////////////////////////////////////////////////////////////////
//
// class type
//

// Static data member

vector<const type *> type::serialtable;

#ifdef FEATUREATABLE
int type::feattablesize = (1<<FEATTABLE_GROW_BITS);
#endif

tserial type::builtin_limit = 0;

// Constructor

type::type(const string &in, module *imod, bool iexported)
{
	int i;
	
	name = in; // new char[strlen(in)+1];	strcpy(name, in);
	mod = imod;
	exported = iexported;
	dynamic = multifile = false;
	propername = Quote(mod->GetName()) + ":" + Quote(name);
	nf = nc = np = 0;
	feats = NULL;
	apptypes = NULL;
	children = NULL;
	parents = NULL;
	attr = NULL;
#ifdef XCONSTR
	constraint = NULL;
	constrained = false;
	feat_constrained = false;
	fixed = false;
#endif
	deleted = false;
	appcounter = 0;
#ifdef FEATURETABLE
	featindex = new short[feattablesize];
	for( i=0; i<feattablesize; i++ )
		featindex[i] = -1;
#endif

	mod->Add(this);

	serialno = (tserial)serialtable.size();

	if( (serialno & ((1 << TYPETABLE_GROW_BITS)-1)) == 0 )
	{
		int i;
		P2("type::type - ExpandTypeTable");
		int newsize = serialno + (1 << TYPETABLE_GROW_BITS);

#ifndef TYPE_TABLE_HASH
		coercode **newcoerceCode = new coercode*[newsize];
		for( i=0; i<newsize; i++ )
		{
			newcoerceCode[i] = new coercode[newsize];
			if( i < serialno )
			{
				memcpy( newcoerceCode[i], coerceCode[i], serialno*sizeof(coercode) );
				delete[] coerceCode[i];
			}
		}
		delete[] coerceCode;
		coerceCode = newcoerceCode;
#endif

		featcode **newfeatureCode = new featcode*[newsize];
		memcpy( newfeatureCode, featureCode,  serialno*sizeof(featcode *) );
		for( i=serialno; i<newsize; i++ )
		{
			newfeatureCode[i] = NULL;
		}
		delete[] featureCode;
		featureCode = newfeatureCode;
	}


#ifndef TYPE_TABLE_HASH
	for( i=0; i<serialno; i++ )
	{
		coerceData(serialno,i).result_s = S_INVALID;
		coerceData(serialno,i).coercion = onlyFail;
		coerceData(serialno,i).coercion_var = onlyFail;
		coerceData(serialno,i).unification = onlyFail;
		coerceData(serialno,i).constraint = NULL;
		coerceData(i,serialno).result_s = S_INVALID;
		coerceData(i,serialno).coercion = onlyFail;
		coerceData(i,serialno).coercion_var = onlyFail;
		coerceData(i,serialno).unification = onlyFail;
		coerceData(i,serialno).constraint = NULL;
	}
#endif
	coercode &c = coerceDataAdd(serialno,serialno);
	c.result_s = serialno;
	c.coercion = coerceNone;
	c.coercion_var = coerceNone;
	c.constraint = NULL;

	int fsize = (feature::GetSerialCount() + (1 << FEATTABLE_GROW_BITS)) & ~((1 << FEATTABLE_GROW_BITS)-1);
	featureCode[serialno] = new featcode[fsize];
	for( i=0; i<fsize; i++ )
	{
		featureCode[serialno][i].result = S_INVALID;
		featureCode[serialno][i].index = (ushort)(-1);
		featureCode[serialno][i].coercion = NULL;
	}

	serialtable.push_back(this);
}

// Destructor

type::~type()
{
//	delete[] name;
	delete[] feats;
	delete[] apptypes;
	delete[] children;
	delete[] parents;
#ifdef FEATURETABLE
	delete[] featindex:
#endif
}

#ifdef FEATURETABLE
void type::ExpandFeatureTable(int newnf)
{
	int newfeattablesize = (newnf + (1<<FEATTABLE_GROW_BITS)) & ~((1 << FEATTABLE_GROW_BITS)-1);
	if( newfeattablesize > feattablesize )
	{
		P2("type::ExpandFeatureTable");
		for( int i=0; i<GetSerialCount(); i++ )
		{
			type *t = Serial(i);
			short *newfeatindex = new short[newfeattablesize];
			memcpy( newfeatindex, t->featindex, sizeof(short)*feattablesize );
			for( int j=feattablesize; j<newfeattablesize; j++ )
			{
				newfeatindex[j] = -1;
			}
			delete t->featindex;
			t->featindex = newfeatindex;
		}
		feattablesize = newfeattablesize;
	}
}
#endif


const string &type::GetPrintName() const 
{ 
	if( module::UserModule()->Search(name) == this )
	{
		return name;
	}
	else
	{
		return propername; 
	}
}

const string type::GetSimpleProperName() const
{
	return Quote(name);
}

////////////////////////////////////////
//  type::SetAsChildOf(parent)
//  Sets the type as the child of "parent".

bool type::SetAsChildOf(type *parent)
{
	P4("type::SetAsChildOf");

	if( fixed || deleted )
	{
		RUNERR("Internal error: Cannot add child");
		return false;
	}
	if( parent->IsDeleted() )
	{
		RUNERR("Cannot add child for a deleted type");
		return false;
	}

	if( GetModule() != parent->GetModule() && ! parent->IsExtendable() )
	{
		RUNWARN("Type " << parent->GetName() << " is not allowed to be derived from outside of the module " << parent->GetModule()->GetName());
	}
//	cout << "Before SetAsChildOf (" << GetName() << ", " << parent->GetName() << ")" << endl;
//	for(int x=0; x<nf; x++ )
//		cout << x << ": " << feats[x]->GetName() << endl;

	// register as a child 

	if( (parent->nc & (TYPE_GROW_STEP-1)) == 0 )
	{		// Grow the array
//		cout << GetName() << " <- " << parent->GetName() << ": " << parent->nc << " -> " << parent->nc+TYPE_GROW_STEP << endl;
		const type **newchildren = new const type* [parent->nc+TYPE_GROW_STEP];
		if( parent->children != NULL ) {
			memcpy(newchildren, parent->children, sizeof(type*)*parent->nc);
			delete[] parent->children;
		}
		parent->children = newchildren;
	}
	else
	{
//		cout << GetName() << " <- " << parent->GetName() << ": " << parent->nc << endl;
	}
	(parent->children)[parent->nc] = this;
	parent->nc++;

	if( (np & (TYPE_GROW_STEP-1)) == 0 )
	{		// Grow the array
		const type **newparents = new const type* [np+TYPE_GROW_STEP];
		if( parent->children != NULL ) {
			memcpy(newparents, parents, sizeof(type*)*np);
			delete[] parents;
		}
		parents = newparents;
	}
	parents[np++] = parent;

	// merge feature list
	
	int kekkasize = (nf + parent->nf + FEAT_GROW_STEP-1) & -FEAT_GROW_STEP;
	const feature **kekkafeats = new const feature*[kekkasize];
	const type **kekkaapps = new const type*[kekkasize];
	
	int thisp = 0, parentp = 0, kekkap = 0;
	while( thisp < nf && parentp < parent->nf )
	{
		if( feats[thisp] == parent->feats[parentp] )
		{
			const type *unitype = apptypes[thisp]->TypeUnify(parent->apptypes[parentp]);
			if( unitype == NULL )
			{
				RUNERR("Type '" << GetName() << "' cannot subsume '" << parent->GetName() << "': "
					  << "Appropriate type of the feature '" << feats[thisp]->GetName() << "' is incompatible, '" 
					  << apptypes[thisp]->GetName() << "' and '" << parent->apptypes[thisp]->GetName() << "'");
				return false;
			}
			kekkafeats[kekkap] = feats[thisp];
			kekkaapps[kekkap] = unitype;
			apptypes[thisp]->DecAppCounter();
			kekkaapps[kekkap]->IncAppCounter();
			kekkap++;
			thisp++; parentp++;
		}
		else if( compfeature( feats[thisp], parent->feats[parentp] ) < 0 )
		{
			kekkafeats[kekkap] = feats[thisp];
			kekkaapps[kekkap] = apptypes[thisp];
			kekkap++; thisp++;
		}
		else
		{
			kekkafeats[kekkap] = parent->feats[parentp];
			kekkaapps[kekkap] = parent->apptypes[parentp];
			kekkaapps[kekkap]->IncAppCounter();
			kekkap++; parentp++;
		}
	}
	memcpy( &kekkafeats[kekkap], &feats[thisp],           sizeof(const feature *)*(nf-thisp));
	memcpy( &kekkafeats[kekkap], &parent->feats[parentp], sizeof(const feature *)*(parent->nf-parentp));
	memcpy( &kekkaapps[kekkap], &apptypes[thisp],           sizeof(const type *)*(nf-thisp));
	memcpy( &kekkaapps[kekkap], &parent->apptypes[parentp], sizeof(const type *)*(parent->nf-parentp));
	kekkap += (nf+parent->nf) - (thisp+parentp);
	
	delete [] feats;
	delete [] apptypes;
	int fsize = (kekkap + FEAT_GROW_STEP-1) & -FEAT_GROW_STEP;
	if( fsize == kekkasize )
	{
		feats = kekkafeats;		// Need not resize.
		apptypes = kekkaapps;		// Need not resize.
	}
	else
	{
		feats = new const feature*[fsize];
		memcpy( feats, kekkafeats, sizeof(const feature *)*kekkap );
		delete [] kekkafeats;
		apptypes = new const type*[fsize];
		memcpy( apptypes, kekkaapps, sizeof(const type *)*kekkap );
		delete [] kekkaapps;
	}
	nf = kekkap;

//	cout << "After SetAsChildOf (" << GetName() << ", " << parent->GetName() << ")" << endl;
//	for(int x=0; x<nf; x++ )
//		cout << x << ": " << feats[x]->GetName() << endl;

#ifdef XCONSTR
	if( parent->IsConstrained() )
	{
		constrained = true;
	}
	if( parent->IsFeatureConstrained() )
	{
		feat_constrained = true;
	}
#endif

	return true;
}

////////////////////////////////////////
//  type::AddFeature(f)
//  Sets the type to have the feature f and returns the index of feature.

int type::AddFeature(feature *f, const type *app)
{
	P4("type::AddFeature");

	if( fixed || deleted )
	{
		RUNERR("Internal error: Cannot add feature");
		return -1;
	}
	if( app->IsDeleted() || f->GetRepType()->IsDeleted() )
	{
		RUNERR("Cannot add deleted feature or apptype");
		return false;
	}

//	cout << "Before AddFeature (" << GetName() << ", " << f->GetName() << ")" << endl;
//	for(int x=0; x<nf; x++ )
//		cout << x << ": " << feats[x]->GetName() << endl;

	if( f->GetBaseTypeS() == S_INVALID ) // First add
	{
		f->SetBaseType(this);
	}
	else
	{
		int i;
		for( i=0; i<GetNParents(); i++ )
		{
			if( f->GetBaseType()->IsSuperType(Parent(i)) )
			{				
				break;
			}
		}
		if( i == GetNParents() )	// no parent type found
		{
			RUNERR("Type '" << GetName() << "' cannot have the feature '" << f->GetName() << "': " 
				   << "it must be a subtype of the feature's base class '" << f->GetBaseType()->GetName() << "'.");
			return -1;
		}
	}

#ifdef XCONSTR
	if( app->IsConstrained() || app->IsFeatureConstrained() )
	{
		feat_constrained = true;
	}
#endif

	int ret = GetIndex(f);	// if f is already in the type, do nothing

	if( ret == -1 )
	{
		if( (nf & (FEAT_GROW_STEP-1)) == 0 )
		{		// Grow the array
//			cout << "Expansion in AddFeature " << nf << " -> " << endl;
			const feature **newfeats = new const feature * [nf+FEAT_GROW_STEP];
			const type **newapps = new const type * [nf+FEAT_GROW_STEP];
			if( feats != NULL ) {
				memcpy(newfeats, feats, sizeof(const feature *)*nf);
				delete[] feats;
				memcpy(newapps, apptypes, sizeof(const type *)*nf);
				delete[] apptypes;
			}
			feats = newfeats;
			apptypes = newapps;
		}
		
		// insert feature to the list (sorted...)
		int i;
		for( i=nf; i>0; i-- )
		{
//	cout << "During AddFeature " << i << "(" << GetName() << ", " << f->GetName() << ")" << endl;
//	for(int x=0; x<nf; x++ )
//		cout << x << ": " << feats[x] << "    " << feats[x]->GetName() << endl;

			if( compfeature( feats[i-1], f ) > 0 )
			{
				feats[i] = feats[i-1];
				apptypes[i] = apptypes[i-1];
				
#ifdef FEATURETABLE
				featindex[feats[i]->GetSerialNo()] = i;
#endif
			}
			else
			{
				break;
			}
		}
//	cout << "At AddFeature " << i << "(" << GetName() << ", " << f->GetName() << ")" << endl;
//	for(int x=0; x<nf; x++ )
//		cout << x << ": " << feats[x] << "    " << feats[x]->GetName() << endl;
		feats[i] = f;
		apptypes[i] = app;
		app->IncAppCounter();

#ifdef FEATURETABLE
		featindex[f->GetSerialNo()] = i;
#endif
		nf++;

//		cout << "After AddFeature (insert) (" << GetName() << ", " << f->GetName() << ")" << endl;
//		for(int x=0; x<nf; x++ )
//			cout << x << ": " << feats[x] << "    " << feats[x]->GetName() << endl;

		return i;
	}
	else 
	{
		const type *unitype = apptypes[ret]->TypeUnify(app);
		if( unitype == NULL )
		{
			RUNERR("For the type '" << GetName() << "', the feature '" << f->GetName() 
			       << "' cannot be appropriate for the type '" << app->GetName() << "': "
			       << "the feature is already restricted for the type '" << apptypes[ret]->GetName() << "'.");
			return -1;
		}
		if( app != bot && unitype != app )
		{
			RUNWARN("For the type '" << GetName() << "', the appropriate tyoe of the feature '" << f->GetName() 
			       << "' is specified as '" << app->GetName() << "', "
			       << "but it is actually '" << unitype->GetName() << "' to solve the constraints.");
		}
		apptypes[ret]->DecAppCounter();
		apptypes[ret] = unitype;
		apptypes[ret]->IncAppCounter();
		
//		cout << "After AddFeature (replace) (" << GetName() << ", " << f->GetName() << ")" << endl;
//		for(int x=0; x<nf; x++ )
//			cout << x << ": " << feats[x]->GetName() << endl;

		return ret;
	}
}

void type::SetAttribute(const FSP &iniattr)
{
	if( deleted )
	{
		return;
	}

	FSP copy = iniattr.Copy();
	machine *m = copy.GetMachine();
	int valuable = 0;
	int modify = 0;

	if( copy.GetType()->IsSubType(module::CoreModule()->Search("type_constrained")) && 
		copy.Follow(feature::Search(module::CoreModule()->Search("pred\\"))).GetType() == bot )
	{
		FSP constr_attr = copy.Follow(feature::Search(module::CoreModule()->Search("constr\\")));
		if( constr_attr.Coerce(this) == false )
		{
			RUNERR("Type attribute is inconsistent to the type itself; type "<< GetName());
		}
		else
		{
			ShareList sl; sl.AddMark(constr_attr); sl.MakeShare();
			
			for( int i=0; i<constr_attr.GetType()->GetNFeatures(); i++ )
			{
				const feature *f = constr_attr.GetType()->Feature(i);
				FSP constr_attr_f = constr_attr.Follow(f);
				if( constr_attr_f.IsLeaf() && ! constr_attr_f.IsInteger() && 
				  ! constr_attr_f.IsString() && ! constr_attr_f.IsFloat() )
				{
					const type *newtype = apptypes[GetIndex(f)]->TypeUnify(constr_attr_f.GetType());

					if( apptypes[GetIndex(f)] != newtype )
					{
						apptypes[GetIndex(f)]->DecAppCounter();
						apptypes[GetIndex(f)] = newtype;
						apptypes[GetIndex(f)]->IncAppCounter();
						if( newtype->IsConstrained() || newtype->IsFeatureConstrained() )
						{
							feat_constrained = true;
						}
						modify++;
					}
					if( sl.SearchShare(constr_attr_f) != -1 )
					{
						valuable++;
					}
				}
				else
				{
					valuable++;
				}
			}
			if( valuable == 0 && modify == 0 )
			{
				return;
			}
			if( valuable == 0 )
			{
				attr = NULL;
			}
		}
	}
	else
	{
		valuable = 1;
	}

	if( valuable )
	{	
		int x = m->GetCurrentSlot();
		m->GoToHeap(0);
	
		attr = new FSP(iniattr.Copy());	// copieddata は HEAP0 に入る
	
		m->GoToHeap(x);
	}

	P1("Compiling types");

	const type *constr = module::CoreModule()->Search("type_constrained");
	bool self_constr = (constr != NULL && attr != NULL && attr->GetType()->IsSubType(constr));
	bool feat_constr = IsFeatureConstrained();
	if( self_constr ) // || feat_constr )
	{
		ShareList sl;
		if( attr )
		{
			sl.AddMark(*attr);
		}
		sl.MakeShare();
		
		ptree *pt = GenPTreeOfConstraint(sl, (attr ? *attr : FSP()), this, self_constr, feat_constr);
		if( pt != NULL ) 
		{
			constrained = true;
			
			parseinfo *info = new parseinfo(attr->GetMachine());
			pt->CollectInfo(0, info);
			codelist *l = pt->Compile(0, info);
			l->EndCode();
			constraint = *l;
#ifdef DEBUG
			if( DEB )
			{
				cout << "Constraint: " << GetName() << endl;
				l->DebugOutput();
			}
#endif
			delete l;
			delete info;
		}
		modify++;
	}

	if( modify && fixed )
	{	// Refix
//			cout << "Refix\n";
		for( tserial t1=0; t1<type::GetSerialCount(); t1++ )
		{
			if( coerceData(t1, serialno).result_s == serialno ) 
			{
				for( tserial t2=t1; t2<type::GetSerialCount(); t2++ )
				{
					if( (coerceData(t1,t2).result_s) == serialno )
					{
						CompileType(t1, t2);
						if( t1 != t2 )
						{
							CompileType(t2, t1);
						}
					}
				}
			}
		}
	}
	else
	{
//			cout << "No refix\n";
	}
}

FSPointer type::GetAttribute(machine &m) const
{ 
	return attr == NULL ? FSP(m,VAR2c(module::CoreModule()->Search("type_attr"))) : (*attr).Copy(); 
}

//bool type::IsSpecial() { return attr && *attr.GetType().IsSubType(type::Search("special_type")); }
bool type::IsFinal() const { return attr && attr->GetType()->IsSubType(module::CoreModule()->Search("final")); }

bool type::Fix(bool displayError)
{
	P1("Compiling types");
	if( deleted )
	{
		return false;
	}
	
#ifdef XCONSTR
	fixed = true;
#endif

#define MGCS(t1,t2) (coerceData((t1),(t2)).result_s)
#define IsSuperType(t1,t2) (MGCS(t1,t2) == (t2))
#define IsSubType(t1,t2) (MGCS(t1,t2) == (t1))
#define tau serialno
#define TOP TS_INVALID
	tserial pi;
	int i,j;

	CompileType(this, this);

	for( i=0; i<nf; i++ )
	{
		featureCode[serialno][feats[i]->GetSerialNo()].index = (ushort)(i+1);
		featureCode[serialno][feats[i]->GetSerialNo()].result = (tserial)serialno;
		featureCode[serialno][feats[i]->GetSerialNo()].coercion = NULL;
	}

	for( i=0; i<np; i++ )
	{
		pi = parents[i]->GetSerialNo();
		if( IsSubType(pi, tau) )
		{
			if( displayError )
			{
				RUNERR("Type '"
					   << GetName() 
					   << "' and "
					   "'" 
					   << parents[i]->GetName() 
					   << "' makes a loop in the type hierarchy" );
			}
			 return false;
		}
		else
		{
			for( tserial t1=0; t1<GetSerialCount(); t1++ )
			{
				if( IsSuperType(t1, pi) )
				{
					for( tserial t2=0; t2<GetSerialCount(); t2++ )
					{
						if( IsSuperType(t2, tau) ) // && t2 != tau )
						{
							if( MGCS(t1, t2) == TOP
							 || (IsSubType(MGCS(t1, t2), tau) && MGCS(t1,t2) != tau) ) // Not necessary but...
							{
//								MGCS(t1, t2) = MGCS(t2, t1) = tau;
								coerceDataAdd(t1,t2).result_s = tau;
								coerceDataAdd(t2,t1).result_s = tau;
								CompileType(t1, t2);
								CompileType(t2, t1);
							}
							else if( IsSuperType(MGCS(t1, t2), tau) 
							 || IsSuperType(MGCS(t1, t2), pi) )
							{
								/* do nothing */ ;
							}
							else
							{
								/* check whether remain parents resolves this bad situation */
								for( j=i+1; j<np; j++ )
								{
									if( IsSuperType(MGCS(t1,t2), parents[j]->GetSerialNo()) )
									{
										break;
									}
								}
								if( j >= np )
								{
									if( displayError ) {
										RUNERR( "Type '" << type::Serial(t1)->GetName() << "' and "
												"'" << type::Serial(t2)->GetName() << "' have two MGSATs, "
												"'" << type::Serial(MGCS(t1,t2))->GetName() << "' and "
												"'" << GetName() << "'");
									}
									return false;
								}
							}
						}
					}
				}
			}
		}
	}
#undef tau
#undef TOP
#undef IsSuperType
#undef IsSubType
	return true;
}


bool type::Delete()
{
	if( deleted )
	{
		return true;	// already deleted
	}
	if( IsBuiltIn() )
	{
		return false;	// built-in type cannot be deleted
	}
	if( nc != 0 )
	{
		return false;	// type with child cannot be deleted
	}
	if( IsSubType(t_feature) && !feature::Search(this)->GetBaseType()->IsDeleted() )
	{
		return false;	// feature with base type cannot be deleted
	}
	if( appcounter )
	{
		return false;	// already used somewhere as an apptype
	}

	deleted = true;

	// Delete features
	for( int i=0; i<nf; i++ )
	{
		if( feats[i]->GetBaseType() == this )
		{
			type *t = (type *)(feats[i]->GetRepType());
			bool r = t->Delete();	// feature reptype deletion
			if( r == false )
			{
				return false;
			}
			feature *f = (feature *)(feats[i]);
			f->Delete();				// feature deletion
		}
		apptypes[i]->DecAppCounter();
	}

	// Delete types
	for( int i=0; i<np; i++ )
	{
		int j;
		for( j=0; j<parents[i]->nc; j++ )
		{
			if( parents[i]->children[j] == this )
			{
				break;
			}
		}
		if( j >= parents[i]->nc )
		{
			return false;	// Cannot find appropriate child
		}
		for( ; j < parents[i]->nc - 1; j++ )
		{
			parents[i]->children[j] = parents[i]->children[j+1];
		}
		type *t = (type *)(parents[i]);
		t->nc --;
	}

	// Delete from unify table
	for( tserial t1=0; t1<type::GetSerialCount(); t1++ )
	{
		if( coerceData(t1, serialno).result_s == serialno && t1 != serialno) 
		{
			for( tserial t2=t1; t2<type::GetSerialCount(); t2++ )
			{
				if( (coerceData(t1,t2).result_s) == serialno && t2 != serialno)
				{
					coerceDataDelete(t1,t2);
					if( t1 != t2 )
					{
						coerceDataDelete(t2,t1);
					}
				}
			}
		}
	}
	for( tserial t1=0; t1<type::GetSerialCount(); t1++ )
	{
		coerceDataDelete(t1, serialno);
		if( t1 != serialno )
		{
			coerceDataDelete(serialno, t1);
		}
	}

	// Delete from hash
	mod->Delete(this);

	return true;
}


////////////////////////////////////////
// featurecmp - for lfind() in type::GetIndex

static int COMPAREFUNC featurecmp(const void *a, const void *b) 
{
	return *(const feature **)a < *(const feature **)b ? -1 : *(const feature **)a > *(const feature **)b ? 1 : 0; 
}
//static int featurecmp(const void *a, const void *b) { return *(const feature **)a-*(const feature **)b; }

////////////////////////////////////////
// type::GetIndex(f)
//    Searchs index of feature f in the type ( index origin is 0 ).
//    when the feature is not in the type, return -1.

int type::GetIndex(const feature *f) const
{
	P4("type::GetIndex");
	ASSERT(f != NULL);
	
#ifdef FEATURETABLE
	return featindex[f->GetSerialNo()];
#else
// kairyouable function
	size_t nnf = nf;
	const feature **ret = (const feature **)lfind(&f, feats, &nnf, sizeof(f), featurecmp);
	if( ret == NULL )
	{
		return -1;
	}
	return ret - feats;
#endif
}

////////////////////////////////////////
//  struct typep_depth - used in type::TypeUnify()

struct typep_depth
{
	const type *t;
	int depth;
};

////////////////////////////////////////
//  struct typep_depth - for lfind() in type::TypeUnify()
/*
static int typep_cmp(const void *a, const void *b) {
    return ((typep_depth *)a)->t < ((typep_depth *)b)->t ? -1 : ((typep_depth *)a)->t > ((typep_depth *)b)->t ? 1 : 0; }
*/
#define MAXTYPES 2048

////////////////////////////////////////
//  type::AcceptableType
//

const type *type::AcceptableType(const feature *f) const
{
	P4("type::AcceptableType");
	
	int index = GetIndex(f);
	if( index != -1 )
	{
		return this;		// this can accept f
	}

#if 1
	tserial s = coerceData(GetSerialNo(),f->GetBaseTypeS()).result_s;
	if( s == TS_INVALID )
	{
		return NULL;
	}
	else
	{
		return type::Serial(s);
	}
#else
	typep_depth *thisc = new typep_depth [MAXTYPES]; 
	size_t thisr=0, thisw=1; // children of this

	thisc[0].t = this; thisc[0].depth = 0;

	typep_depth find = { NULL, INT_MAX };
	bool error = false;
	
	int curdepth = 0;
	while( thisr < thisw && find.t == NULL)
	{
		while( thisr < thisw && thisc[thisr].depth == curdepth )
		{
//			cerr << "thisr = " << thisr << " (" << thisc[thisr].t->GetName() << ")" << endl;
			if( thisc[thisr].t->GetIndex(f) != -1 )
			{
				error = (curdepth == find.depth && find.t != thisc[thisr].t);
				if ( error ) 
				{
					cout << find.t->GetName();
				}
				find.t = thisc[thisr].t;
				find.depth = curdepth;
			}
			for( int i=0; i<thisc[thisr].t->nc; i++ )
			{
				ASSERT(thisw < MAXTYPES);
				thisc[thisw].t = thisc[thisr].t->children[i];
				thisc[thisw].depth = curdepth+1;
//				cerr << "thisw = " << thisw << " (" << thisc[thisw].t->GetName() << ")" << endl;
				thisw++;
			}
			thisr++;
		}
		curdepth++;
	}
	if( error )
	{
		cout << find.t->GetName();
		throw xBadType();
	}
	delete[] thisc;
	if( find.t == NULL )
	{
		return NULL;
	}
	return find.t;
#endif
}


////////////////////////////////////////
//  type::TypeUnify
//
//  Calculate the unified type between the type (this) and another type (that).
//  When no appropriate type is found, return NULL.
//  When more than one appropriate types are found, throw BadType().

//#define DEBOUT

// TODO: REMOVE SAME TYPE ON thisc AND thatc

const type * type::TypeUnify(const type *that) const
{
	P3("type::TypeUnify");
	// kairyoable function

#if 1
	tserial s = coerceData(GetSerialNo(),that->GetSerialNo()).result_s;
	if( s == TS_INVALID )
	{
		return NULL;
	}
	else
	{
		return type::Serial(s);
	}
#else

//  This function holds two list of descendants, for this and that.
//  Growing the lists alternatively, the function 
//  searches for any type that is on both lists.

	typep_depth *thisc = new typep_depth [MAXTYPES]; 
	size_t thisr=0, thisw=1; // children of this
	typep_depth *thatc = new typep_depth [MAXTYPES]; 
	size_t thatr=0, thatw=1; // children of that
	
	int curdepth = 0;
	
	thisc[0].t = this; thisc[0].depth = 0;
	thatc[0].t = that; thatc[0].depth = 0;
#ifdef DEBOUT
	cout << "TypeUnify(" << this->GetName() << "," << that->GetName() << ")." << endl;

	cout << "thisc[0] = " << thisc[0].t->GetName() << endl;
	cout << "thatc[0] = " << thatc[0].t->GetName() << endl;
#endif
	
	typep_depth find = { NULL, INT_MAX };
	bool error = false;

	while((thisr < thisw || thatr < thatw) && find.t == NULL)
	{
		while( thisr < thisw && thisc[thisr].depth == curdepth )
		{
			typep_depth *f = (typep_depth *)lfind(&thisc[thisr], thatc, &thatw, sizeof(typep_depth), typep_cmp);
			if( f != NULL )
			{
#ifdef DEBOUT
				cout << "thisc[" << thisr << "] (" << thisc[thisr].t->GetName() 
				     << ") == thatc[" << f-thatc << "] (" << f->t->GetName() << ")" << endl;
#endif
				error = (f->depth+curdepth == find.depth && find.t != f->t);
				if ( error )
				{
					cout << find.t->GetName();
				}
				find.t = f->t; find.depth = curdepth + f->depth;
			}
			for( int i=0; i<thisc[thisr].t->nc; i++ )
			{
				ASSERT(thisw < MAXTYPES);
				thisc[thisw].t = thisc[thisr].t->children[i];
				thisc[thisw].depth = curdepth+1;
#ifdef DEBOUT
				cout << "thisc[" << thisw << "] = " << thisc[thisw].t->GetName() << endl;
#endif
				thisw++;
			}
			thisr++;
		}
		while( thatr < thatw && thatc[thatr].depth == curdepth )
		{
			typep_depth *f = (typep_depth *)lfind(&thatc[thatr], thisc, &thisw, sizeof(typep_depth), typep_cmp);
			if( f != NULL )
			{
#ifdef DEBOUT
				cout << "thatc[" << thatr << "] (" << thatc[thatr].t->GetName() 
				     << ") == thisc[" << f-thisc << "] (" << f->t->GetName() << ")" << endl;
#endif
				error = (f->depth+curdepth == find.depth && find.t != f->t);
				if ( error )
				{
					cout << find.t->GetName();
				}
				find.t = f->t; find.depth = curdepth + f->depth;
			}
			for( int i=0; i<thatc[thatr].t->nc; i++ )
			{
				ASSERT(thatw < MAXTYPES);
				thatc[thatw].t = thatc[thatr].t->children[i];
				thatc[thatw].depth = curdepth+1;
#ifdef DEBOUT
				cout << "thatc[" << thatw << "] = " << thatc[thatw].t->GetName() << endl;
#endif
				thatw++;
			}
			thatr++;
		}
		curdepth++;
	}
	if( error )
	{
		cout << find.t->GetName();
		throw xBadType();
	}
	delete[] thisc;
	delete[] thatc;
	if( find.t == NULL )
	{
		return NULL;
	}
	return find.t;
#endif
}

//  CompileTypes() is located at compile.cpp.


} // namespace lilfes

