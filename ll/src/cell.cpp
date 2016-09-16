/* $Id: cell.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $ 
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: cell.cpp,v 1.8 2011-05-02 10:38:23 matuzaki Exp $";

#include "cell.h"
#include "machine.h"
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>


namespace lilfes {

using std::cout;
using std::dec;
using std::hex;
using std::make_pair;
using std::ostream;
using std::string;
using std::vector;



//////////////////////////////////////////////////////////////////////////////
//
// class cell
//

ostream& operator<<(ostream& os, cell c)
{
//	ASSERT( c != NULL );
  switch( Tag(c) )
    {
    case T_STR:
      os << "STR " << c2STR(c)->GetName(); break;
    case T_VAR:
      os << "VAR " << c2VAR(c)->GetName(); break;
    case T_PTR:
    case T_PTR1:
    case T_PTR2:
    case T_PTR3:
      if( c2PTR(c) == CORE_P_INVALID )
        os << "PTR (INVALID)"; 
      else
        os << "PTR " << hex << GetSlot(c2PTR(c)) << "-" << (cp2int(GetOffset(c2PTR(c)))) << dec; 
      break;
    case T_INT:
      os << "INT " << c2INT(c); break;
    case T_FLO:
      os << "FLO " << c2FLO(c); break;
    case T_CHR:
      os << "CHR '" << (char) c2CHR(c) << "' (0x" << hex << c2CHR(c) << dec << ")"; break;
    case T_STG:
      os << "STG \"" << c2STG(c) << "\""; break;
    default:
      os << "Unknown Tag"; break;
    }
  return os;
}

vector<const strsymbol *> strsymbol::serialtable;
_HASHMAP<string, strsymbol *> strsymbol::hash;

strsymbol::strsymbol(machine *mach, string ininame) 
{ 
	name = ininame;
	hash.insert(make_pair(ininame, this));
	used = permanent = false;
	serialno = serialtable.size();
#ifdef USE_STRING_GC
	if( (serialno&65535)==0 && serialno > 0)
		gc(mach);
#endif
	serialtable.push_back(this);
}

strsymbol::~strsymbol() 
{ 
	serialtable[serialno] = NULL;
	hash.erase(this->GetKey());
}

void strsymbol::gc(machine *mach)
{
	P1("String GC");

	//cout << "[String GC at count=" << serialtable.size() << flush;
	
	int serialcount = serialtable.size();
	for( int i=0; i < serialcount; i++ )
	{
		if( serialtable[i] != NULL )
			serialtable[i]->used = serialtable[i]->permanent;
	}

	for( core_p i=int2core_p(0); i < mach->GetRP(); i++ )
		if( IsSTG(mach->ReadLocal(i)) )
			serialtable[c2STGS(mach->ReadLocal(i))]->used = true;

	for( core_p i=int2core_p(0); i < mach->GetTP(); i++ )
		if( IsSTG(mach->ReadTrail(i)) )
			serialtable[c2STGS(mach->ReadTrail(i))]->used = true;

	for( int h=0; h < NHEAP; h++ )
	{
		core_p limit;
		if( h == NHEAP - 1 )
			limit = mach->GetSP();
		else if( h == GetSlot(mach->GetHP()) )
			limit = mach->GetHP();
		else
			limit = mach->ReadHeap(MakeCoreP(h, int2core_p(0)));

		//cout << "(heap" << h << "=" << limit - MakeCoreP(h, 0) << ")" << flush;
		for( core_p i=MakeCoreP(h, 0); i<limit; i++ )
			if( IsSTG(mach->ReadHeap(i)) )
				serialtable[c2STGS(mach->ReadHeap(i))]->used = true;
	}

	int cnt = 0, siz = 0;
	for( int i=0; i < serialcount; i++ )
	{
		if( serialtable[i] != NULL )
			if( serialtable[i]->used == false )
			{
				cnt++; siz += serialtable[i]->name.length() + 8;
				delete serialtable[i];
			}
//			else
//				cout << "Remain: \"" << serialtable[i]->name << "\" (permanent = " << (serialtable[i]->permanent ? "true" : "false") << ")" << std::endl;
	}
	
	//cout << " " << cnt << "strings (" << siz << " bytes) collected. Remained strings = " << hash.size() << "]" << std::endl;

}

} // namespace lilfes
