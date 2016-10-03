/* $Id: structur.cpp,v 1.14 2011-05-02 10:38:24 matuzaki Exp $ 
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: structur.cpp,v 1.14 2011-05-02 10:38:24 matuzaki Exp $";

//#define DOASSERT

#include "structur.h"
#include "builtin.h"
#include "parse.h"

#include <cstring>
#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace lilfes {

using std::cout;
using std::dec;
using std::endl;
using std::ends;
using std::hex;
using std::ios;
using std::make_pair;
using std::map;
using std::ostringstream;
using std::pair;
using std::string;
using std::strlen;
using std::vector;


int ShareList::AddrRefPair::cmp(const void *a, const void *b)
{
	return ((const AddrRefPair *)a)->addr == ((const AddrRefPair *)b)->addr;
}

void FSP::VAR2STR( void )
{
	addr = mach->Deref( addr );
	cell c = mach->ReadHeap( addr );
	if ( ! IsVAR( c ) )
	{
		return;
	}
	mach->WriteHeap( addr, PTR2c( mach->GetHP() ) );
	addr = mach->GetHP();
	tserial ts = c2VARS( c );
	const type* t = type::Serial( ts );
	int nfeats = t->GetNFeatures();
	mach->CheckHeapLimit( addr + nfeats + 1 );
	mach->PutHeap( STR2c( ts ) );
	for ( int i = 0; i < nfeats; i++ )
	{
		mach->PutHeap( VAR2c( t->GetAppTypeS(i) ) );
	}
	return;
}

const type *FSP::GetType() const
{ 
	cell c = ReadCell();
	switch(Tag(c))
	{
		case T_STR:
		case T_VAR:
			return c2STRorVAR(mach->ReadHeap(mach->Deref(addr))); 
		case T_INT:
			return t_int;
		case T_FLO:
			return t_flo;
		case T_CHR:
			return t_chr;
		case T_STG:
			return t_stg;
		default:
			ABORT("UNKNOWN TAG TYPE");
			return bot;
	}
}


bool FSP::Coerce(const type *t)
{
	ASSERT( this != NULL );
	if( addr == CORE_P_INVALID || t == NULL )
		return false;
	ASSERT( mach != NULL );

#ifdef DEBUG
	if( DEB )
	{
		cout << "CoerceType " << hex << cp2int(addr) << dec << ", " << t->GetName() << endl;
	}
#endif

	return mach->CoerceType(addr, t);
}

bool FSP::Unify(FSP fs2)
{
	FSP *that = &fs2;
	ASSERT( this != NULL && that != NULL );
	ASSERT( this->mach == that->mach );
	ASSERT( mach != NULL );

	if( this->addr == CORE_P_INVALID || that->addr == CORE_P_INVALID )
	{
		return false;
	}

	return mach->Unify(this->addr, that->addr);
}

FSP FSP::GetTop()
{
	cell c = ReadCell();
	switch(Tag(c))
	{
		case T_STR:
		case T_VAR:
			return FSP(mach,VAR2c(c2STRSorVARS(mach->ReadHeap(mach->Deref(addr))))); 
		case T_INT:
		case T_FLO:
		case T_CHR:
		case T_STG:
			return FSP(mach,c);
		default:
			ABORT("UNKNOWN TAG TYPE");
			return FSP(mach,VAR2c(bot));
	}
}

/*
void FSP::__Serialize(vector<cell> &vec) {
  core_p cp = mach->GetHP();
  
  FSP new_fsp = Copy();
  core_p new_root, new_addr;
  new_root = new_addr = new_fsp.GetAddress();
  bool cont = true;
  
  while(cont) {
    cell c = mach->ReadHeap(new_addr++);
    switch(Tag(c)) {
    case T_PTR3:
      if (c == PTR2c(CORE_P_INVALID)) {
	cont = false;
      } else {
	vec.push_back(PTR2c(c2PTR(c) - new_root));
      }
      break;
      
    case T_PTR:    case T_PTR1:    case T_PTR2:
      vec.push_back(PTR2c(c2PTR(c) - new_root));
      break;
      
    case T_VAR: case T_STR: case T_INT: case T_FLO: case T_CHR:
      vec.push_back(c);
      break;

    case T_STG:
      strsymbol::Serial(c2STGS(c))->SetPermanent();
      vec.push_back(c);
      break;
            
    default:
      ABORT("FSP::Serialize");
    }
  }
  mach->SetHP(cp);
  
}

void FSP::__FSP(int, int, vector<cell> &vec) {

  vector<cell>::iterator c = vec.begin(), last = vec.end();
  for(; c != last; c++) {
    mach->CheckHeapLimit(mach->GetHP() + 1);
    switch(Tag(*c)) {
    case T_PTR:    case T_PTR1:    case T_PTR2:	case T_PTR3:
      mach->PutHeap(PTR2c(c2PTR(*c) + addr));
      break;
      
    case T_VAR: case T_STR: case T_INT: case T_FLO: case T_CHR: case T_STG:
      mach->PutHeap(*c);
      break;
            
    default:
      ABORT("FSP::FSP(machine*, vector<cell> &)");
    }
  }
}
*/

#ifdef CELL_64BIT
const int charlen = 7;
const int char_shift = 56;
#else
const int charlen = 3;
const int char_shift = 24;
#endif

static const int serialize_size = 32768;

void FSP::__Serialize(vector<cell>& vec) {
    
    cell c;
    core_p a = GetAddress();
    core_p base0 = mach->GetHP();
    size_t base1 = vec.size();

    while( IsPTR(c = mach->ReadHeap(a)))
	{
        a = c2PTR(c);
	}
    
    switch(Tag(c))
    {
        case T_PTR:
        case T_PTR1:
        case T_PTR2:
        case T_PTR3:
            ABORT("internal heap error");
            
        case T_VAR:
        case T_INT:
        case T_FLO:
        case T_CHR:
            vec.push_back(c);
            break;
            
        case T_STG:
        {
            const char *str = c2STG(c);
            int len = (int) strlen(str);
            vec.push_back(int2cell( ((cell_int) len & NTAG_MASK) | ((cell_int)T_STG << TAG_SHIFT)));
            int k = 0;
            for(; k < len ; )
			{
                cell_int ic = 0;
                for(int j = 0 ; j < charlen ; j++, k++)
				{
                    ic = ic << 8;
                    if( k < len )
					{
                        ic |= (cell_int) (unsigned char) *(str++);
					}
                }
                vec.push_back(int2cell( (ic & NTAG_MASK) | ((cell_int)T_CHR  << TAG_SHIFT)));
            }
            break;
        }
        case T_STR:
            mach->SetTrailPoint();

            {  // Perform actual copy
                vector< pair<core_p, size_t> > array;
                array.reserve(serialize_size);

                array.push_back( make_pair(a,base1) );
                int slot = GetSlot(base0);
                vec.resize(base1+1);

                while( ! array.empty() )
                {
                    core_p ptr0 = array.back().first;
                    size_t ptr1 = array.back().second; 
                    array.pop_back();
					
                    core_p addr = ptr0;

                    cell c = mach->ReadHeap(addr);
				
                    if( IsPTR(c) )
                    {
                        vec[ptr1] = PTR2c((core_p) base1 + (c2PTR(c) - base0));
                    }
                    else
                    {
                        unsigned nf = c2STR(c)->GetNFeatures();

                        size_t hp1 = vec.size();
                        mach->WriteHeap(ptr0, PTR2c(base0 + (core_p)(hp1 - base1)));
                        vec[ptr1] = PTR2c((core_p) hp1);

                        vec.resize(hp1 + nf + 1);
                        vec[hp1] = c;
                        
                        for( unsigned i=nf; i>0; i-- )
                        {
                            core_p a = addr + i;
                            while( IsPTR(c = mach->ReadHeap(a)) )
                            {
                                a = c2PTR(c);
                                if( (GetSlot(a) == slot) && a >= base0 )
                                {
                                    vec[hp1+i] = PTR2c((core_p) base1 + (a - base0));
                                    goto hogehoge;
                                }
                            }

                            if( IsSTR(c) )
                            {
                                array.push_back( make_pair(a, hp1 + i) );
                            }
                            else if( IsSTG(c) )
                            {
                                mach->WriteHeap(a, PTR2c(base0 + (core_p)(vec.size() - base1)));
                                vec[hp1+i] = PTR2c((core_p) vec.size());
                                
                                const char *str = c2STG(c);
                                int len = (int) strlen(str);
                                vec.push_back(int2cell( ((cell_int) len & NTAG_MASK) | ((cell_int)T_STG << TAG_SHIFT)));
                                int k = 0;
                                for(; k < len ; )
								{
                                    cell_int ic = 0;
                                    for(int j = 0 ; j < charlen ; j++, k++)
									{
                                        ic = ic << 8;
                                        if( k < len )
										{
                                            ic |= (cell_int)  (unsigned char) *(str++);
										}
                                    }
                                    vec.push_back(int2cell( (ic & NTAG_MASK) | ((cell_int)T_CHR  << TAG_SHIFT)));
                                }
                            }
                            else
                            {
                                mach->WriteHeap(a, PTR2c(base0 + (core_p)(hp1 + i - base1)));
                                vec[hp1+i] = c;
                            }
                          hogehoge: ;
                        }
                    }
                }

                mach->TrailBack();
            }
            break;
        default:
            ABORT("Unknown Tag!!!!");
            break;
    }
    return;
}

void printvec(const vector<cell>& vec) {
    for(int i = 0; i < ((int) vec.size()) ; i++)
	{
        cell c = vec[i];
        switch(Tag(c))
		{
            case T_VAR:
            {
                cout << "[" << i << "]" << "(VAR) " << c2VAR(c)->GetName() << endl;
                break;
            }
            case T_INT:
            {
                cout << "[" << i << "]" << "(INT) " << c2INT(c) << endl;
                break;
            }
            case T_FLO:
            {
                cout << "[" << i << "]" << "(FLO) " << c2FLO(c) << endl;
                break;
            }
            case T_STG:
            {
                cout << "[" << i << "]" << "(STG) " << (cell2int(c) << 4 >> 4) << endl;
                break;
            }
            case T_STR:
            {
                cout << "[" << i << "]" << "(STR) " << c2STR(c)->GetName() << endl;
                break;
            }
            case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3:
            {
                cout << "[" << i << "]" << "(PTR) " << (uint32) c << endl;
                break;
            }
            case T_CHR:
            {
                string str;
                cell_int ic = cell2int(c);
                for(int j = 0 ; j < charlen; j++)
				{
                    ic = ic << 8;
                    char ch = (char) (ic >> char_shift);
                    if(ch)
					{
                        str += ch;
					}
                    else
					{
                        str += "\\0";
					}
                }
                cout << "[" << i << "]" << "(CHR) " << hex << (uint32) c << dec << "(" << str.c_str() << ")" << endl;
                break;
            }
            default:
            {
                cout << "[" << i << "]" << "(\?\?\?) " << c << endl;
            }
        }
    }
}

vector<char> FSP::buffer(10000);

void FSP::__FSP(int s, int e, const vector<cell>&vec) {
    vector<cell>::const_iterator it = vec.begin() + s, last = vec.begin() + e;
    for(; it != last ; it++) {
        cell c = (*it);
        switch(Tag(c)) {
            case T_PTR:    case T_PTR1:    case T_PTR2:	case T_PTR3:
            {
                core_p cp = c2PTR(c);
                ASSERT(cp >= s && cp < e);
                mach->CheckHeapLimit(mach->GetHP()+1);
                mach->PutHeap(PTR2c(cp - s + addr));
                break;
            }
            case T_VAR: case T_STR: case T_INT: case T_FLO:
            {
                mach->CheckHeapLimit(mach->GetHP()+1);
                mach->PutHeap(c);
                break;
            }
            case T_STG:
            {
                int len = cell2int(c) << 4 >> 4;
                core_p base = mach->GetHP();
                mach->CheckHeapLimit(base+1);
                mach->PutHeap(PTR2c( CORE_P_INVALID));

                if(buffer.size() < (size_t)(len + 1))
				{
                    buffer.resize(len + 1);
                }
                
                int k = 0;
                while(k < len) {
                    it++;
                    cell_int ic = cell2int(*it);
                    mach->CheckHeapLimit(mach->GetHP()+1);
                    mach->PutHeap(PTR2c( CORE_P_INVALID));
                    for(int j = 0 ; j < charlen; j++, k++)
					{
                        ic = ic << 8;
                        char ch = (char) (ic >> char_shift);
                        if( k < len )
						{
                            buffer[k] = ch;
						}
                    }
                }
                buffer[len] = '\0';
                mach->WriteHeap(base, STG2c(mach, &*buffer.begin()));
                break;
            }
            case T_CHR:
            {
                printvec(vec);
                ABORT("error FSP::__FSP -- T_CHR");
            }
            default:
                ABORT("error FSP::__FSP -- default");
        }
    }
}

/*
DEF_BUILTIN(test_serialize, 2, m)
{
	GETARGS2(m);
	vector<cell> vec;

	arg1.Serialize(vec);
	arg2.Unify(FSP(m, vec));
}

_HASHMAP<vector<cell>, vector<cell>, hash_vc, eqvc> hm;
_HASHMAP<vector<cell>*, vector<cell>*, hash_vcp, eqvcp> hmp;

DEF_BUILTIN(test_add, 2, m)
{
	GETARGS2(m);
	vector<cell> vec1;
	vector<cell>* vec1p = new vector<cell>;
	vector<cell> vec2;
	vector<cell>* vec2p = new vector<cell>;

	arg1.Serialize(vec1);
	arg1.Serialize(*vec1p);
	arg2.Serialize(vec2);
	arg2.Serialize(*vec2p);
	hm[vec1] = vec2;
	hmp[vec1p] = vec2p;
}

DEF_BUILTIN(test_find, 3, m)
{
	GETARGS3(m);
	vector<cell> vec1;
	vector<cell>* vec1p = new vector<cell>;
	arg1.Serialize(vec1);
	arg1.Serialize(*vec1p);
	
	vector<cell> vec2 = hm[vec1];
	vector<cell>* vec2p = hmp[vec1p];
	arg2.Unify(FSP(m, vec2));
	arg3.Unify(FSP(m, *vec2p));
}
*/

static const int copy_size = 32768;

FSP FSP::Copy() const
{
#ifdef PROF_COPY
	P1("FeatureStructure::Copy");
#else
	P2("FeatureStructure::Copy");
#endif
	cell c;
	core_p a = GetAddress();
	core_p ret;

	while( IsPTR(c = mach->ReadHeap(a)) 
//			&& cp2int(c2PTR(c)) >= HEAP_BASE
			 )
	{
		a = c2PTR(c);
	}
	switch(Tag(c))
	{
		case T_PTR:
		case T_PTR1:
		case T_PTR2:
		case T_PTR3:
			ret = c2PTR(c);
			break;
		case T_VAR:
		case T_INT:
		case T_FLO:
		case T_CHR:
		case T_STG:
			ret = mach->GetHP();
			mach->CheckHeapLimit(ret+2);
			mach->PutHeap(c);
			mach->PutHeap(PTR2c(CORE_P_INVALID));
			break;
		case T_STR:
#ifdef DEBUG
//			mach->Dump();
#endif
			mach->SetTrailPoint();

			{  // Perform actual copy
				vector< pair<core_p, core_p> > array;
				array.reserve(copy_size);

				ret = mach->GetHP();
				array.push_back( make_pair(a,ret) );

				int slot = GetSlot(ret);

				while( ! array.empty() )
				{
					core_p ptr0 = array.back().first;
					core_p ptr1 = array.back().second; 
					array.pop_back();
					
					ASSERT(   ( ptr1 >= ret && (GetSlot(ptr1) == GetSlot(mach->GetHP())) ) );
					ASSERT( ! ( ptr0 >= ret && (GetSlot(ptr0) == GetSlot(mach->GetHP())) ) );

					core_p addr = ptr0;

					cell c = mach->ReadHeap(addr);
				
					if( IsPTR(c) )
					{
						ASSERT( (c2PTR(c) >= ret && (GetSlot(c2PTR(c)) == GetSlot(mach->GetHP())) ) );
						mach->WriteHeapDirect(ptr1, c);
			//			ret = c2PTR(c);
					}
					else
					{
						ASSERT(IsSTR(c));
						unsigned nf = c2STR(c)->GetNFeatures();

						core_p hp = mach->GetHP();
						mach->WriteHeapDirect(ptr1, PTR2c(hp));
						mach->WriteHeap(ptr0, PTR2c(hp));

						mach->CheckHeapLimit(hp+nf+1);
						mach->SetHP(hp + nf + 1);

						mach->WriteHeapDirect(hp, c);
						for( unsigned i=nf; i>0; i-- )
						{
							core_p a = addr + i;
							ASSERT( !(a >= ret && (GetSlot(a) == slot) ) );
							while( IsPTR(c = mach->ReadHeap(a)) )
							{
								a = c2PTR(c);
								if( (GetSlot(a) == slot) && a >= ret )
								{
									mach->WriteHeapDirect(hp+i, c);
									goto hogehoge;
								}
							}

							if( IsSTR(c) )
							{
								array.push_back( make_pair(a, hp+i) );
							}
							else
							{
								mach->WriteHeap(a, PTR2c(hp+i));
								mach->WriteHeapDirect(hp+i, c);
							}
							hogehoge: ;
						}
					}
				}

				core_p newHP = mach->GetHP();
				mach->TrailBack();
				mach->SetHP(newHP);	// because TrailBack() restores HP...
				mach->CheckHeapLimit(newHP+1);
				mach->PutHeap(PTR2c(CORE_P_INVALID));
			}
#ifdef DEBUG
//			Dump();
#endif
			break;
		default:
			ABORT("Unknown Tag!!!!");
			break;
	}

	return FSP(mach, ret);
}

FSP FSP::SharedNodeList( map< core_p, unsigned int >& shared_node ) const
{
#ifdef PROF_COPY
	P1("FeatureStructure::SharedNodeList");
#else
	P2("FeatureStructure::SharedNodeList");
#endif
	cell c;
	core_p a = GetAddress();
	core_p ret;

	while( IsPTR(c = mach->ReadHeap(a)) 
//			&& cp2int(c2PTR(c)) >= HEAP_BASE
			 )
	{
		a = c2PTR(c);
	}
	switch(Tag(c))
	{
		case T_PTR:
		case T_PTR1:
		case T_PTR2:
		case T_PTR3:
			ret = c2PTR(c);
			break;
		case T_VAR:
		case T_INT:
		case T_FLO:
		case T_CHR:
		case T_STG:
			ret = mach->GetHP();
			mach->CheckHeapLimit(ret+2);
			mach->PutHeap(c);
			mach->PutHeap(PTR2c(CORE_P_INVALID));
			break;
		case T_STR:
#ifdef DEBUG
//			mach->Dump();
#endif
			mach->SetTrailPoint();

			{  // Perform actual copy
				vector< pair<core_p, core_p> > array;
				array.reserve(copy_size);

				ret = mach->GetHP();
				array.push_back( make_pair(a,ret) );

				int slot = GetSlot(ret);

				while( ! array.empty() )
				{
					core_p ptr0 = array.back().first;
					core_p ptr1 = array.back().second; 
					array.pop_back();
					
					ASSERT(   ( ptr1 >= ret && (GetSlot(ptr1) == GetSlot(mach->GetHP())) ) );
					ASSERT( ! ( ptr0 >= ret && (GetSlot(ptr0) == GetSlot(mach->GetHP())) ) );

					core_p addr = ptr0;

					cell c = mach->ReadHeap(addr);
				
					if( IsPTR(c) )
					{
						ASSERT( (c2PTR(c) >= ret && (GetSlot(c2PTR(c)) == GetSlot(mach->GetHP())) ) );
						mach->WriteHeapDirect(ptr1, c);
			//			ret = c2PTR(c);
					}
					else
					{
						ASSERT(IsSTR(c));
						unsigned nf = c2STR(c)->GetNFeatures();

						core_p hp = mach->GetHP();
						mach->WriteHeapDirect(ptr1, PTR2c(hp));
						mach->WriteHeap(ptr0, PTR2c(hp));

						mach->CheckHeapLimit(hp+nf+1);
						mach->SetHP(hp + nf + 1);

						mach->WriteHeapDirect(hp, c);
						for( unsigned i=nf; i>0; i-- )
						{
							core_p a = addr + i;
							ASSERT( !(a >= ret && (GetSlot(a) == slot) ) );
							while( IsPTR(c = mach->ReadHeap(a)) )
							{
								a = c2PTR(c);
								if( (GetSlot(a) == slot) && a >= ret )
								{
                                                                  // Shared node
                                                                  ++shared_node[ a ];
                                                                  mach->WriteHeapDirect(hp+i, c);
                                                                  goto hogehoge;
								}
							}

							if( IsSTR(c) )
							{
								array.push_back( make_pair(a, hp+i) );
							}
							else
							{
								mach->WriteHeap(a, PTR2c(hp+i));
								mach->WriteHeapDirect(hp+i, c);
							}
							hogehoge: ;
						}
					}
				}

				core_p newHP = mach->GetHP();
				mach->TrailBack();
				mach->SetHP(newHP);	// because TrailBack() restores HP...
				mach->CheckHeapLimit(newHP+1);
				mach->PutHeap(PTR2c(CORE_P_INVALID));
			}
#ifdef DEBUG
//			Dump();
#endif
			break;
		default:
			ABORT("Unknown Tag!!!!");
			break;
	}

	return FSP(mach, ret);
}

static bool Canonicalize( machine* mach, core_p addr, tserial ts, map< core_p, unsigned int >& shared_node ) {
	addr = mach->Deref( addr );
	//std::cerr << addr << ", " << mach->ReadHeap( addr ) << endl;
	bool is_default = true;
	map< core_p, unsigned int >::iterator shared_it = shared_node.find( addr );
	if ( shared_it != shared_node.end() )
	{
		//std::cerr << "Shared! " << addr << ", " << mach->ReadHeap( addr ) << endl;
		is_default = false;
		if ( shared_it->second == 0 )
		{
			return is_default;
		}
		//std::cerr << "Continue canonicalize" << endl;
		shared_it->second = 0;
	}
	cell c = mach->ReadHeap( addr );
	switch ( Tag( c ) ) {
	case T_INT:
	case T_FLO:
	case T_CHR:
	case T_STG:
		return false;
	case T_VAR:
	case T_FVAR:
		return c2VARS( c ) == ts && is_default;
	case T_STR:
	case T_FSTR: {
		const type* t = c2STR( c );
		int nfeatures = t->GetNFeatures();
		bool is_child_default = true;
		for ( int i = 0; i < nfeatures; ++i )
		{
			is_child_default &= Canonicalize( mach, addr + i + 1, t->GetAppTypeS( i ), shared_node );
		}
		if ( is_child_default )
		{
			c = VAR2c( t );
			mach->WriteHeapDirect( addr, c );
		}
		is_default &= is_child_default;
		is_default &= t->GetSerialNo() == ts;
		return is_default;
	}
	default:
		ABORT( "FSP::CanonicalCopy" );
		break;
	}
}

FSP FSP::CanonicalCopy() const {
	map< core_p, unsigned int > shared_node;
	FSP fs = this->SharedNodeList( shared_node );
	core_p addr = fs.GetAddress();
	(void)Canonicalize( mach, addr, bot->GetSerialNo(), shared_node );
	return fs;
}


//////////////////////////////////////////////////////////////////////////////
//
// PrettyPrint functions
//

// for lfind/lsearch of core_p *

//static int COMPAREFUNC core_pcmp(const void *a, const void *b) 
//{ return *(core_p *)a - *(core_p *)b; }

int ShareList::SearchShare(FSP fs)
{
	FSP f = fs.Deref();
	for( uint i=0; i<share.size(); i++ )
	{
		if( share[i].addr == f.GetAddress() )
		{
			return i;
		}
	}
	return -1;
}

void ShareList::MakeShare()
{
	for( uint i=0; i<mark.size(); i++ )
	{
		if( mark[i].ref >= 2 )	// If there is a structure sharing...
		{
			share.push_back(AddrRefPair());
			AddrRefPair &ns = share.back();
			
			ns.addr = mark[i].addr;
			ns.ref  = 0;
		}
	}
}

void ShareList::MergeRef(ShareList &sl)
{
	ASSERT( share.size() == sl.share.size() );
	for( uint i=0; i<sl.share.size(); i++ )
	{
		if( sl.share[i].ref )
		{
			ASSERT( share[i].addr = sl.share[i].addr );
			share[i].ref += sl.share[i].ref;
		}
	}
}

bool ShareList::AddMarkWithoutRecursion(FSP fs)
{
#ifndef FAST
	if( mach == NULL )
	{
		mach = fs.GetMachine();
	}
	else if( mach != fs.GetMachine() )
	{
		ABORT("Abstract machine mixed!");
	}
#endif

	FSP addr = fs.Deref();

	int ndata = mark.size();

	for( int i=0; i<ndata; i++ )
	{
		if( mark[i].addr == addr.GetAddress() )
		{
			mark[i].ref++;
			return true;
		}
	}

	mark.push_back(AddrRefPair());
	AddrRefPair &ns = mark.back();
	
	ns.addr = addr.GetAddress();
	ns.ref  = 1;

	return false;
}

int ShareList::GetRefCount(FSP fs)
{
#ifndef FAST
	if( mach == NULL )
	{
		mach = fs.GetMachine();
	}
	else if( mach != fs.GetMachine() )
	{
		ABORT("Abstract machine mixed!");
	}
#endif

	FSP addr = fs.Deref();

	int ndata = mark.size();

	int min = 0, max = ndata;

	while( max - min > 2 )
	{
		int mid = (max + min) >> 1;
		if( mark[mid].addr < addr.GetAddress() )
		{
			max = mid;
		}
		else
		{
			min = mid;
		}
	}
	if( mark[min].addr == addr.GetAddress() )
	{
		return mark[min].ref;
	}
	else
	{
		return 0;
	}
}

void ShareList::AddMark(FSP fs)
{
#ifndef FAST
	if( mach == NULL )
	{
		mach = fs.GetMachine();
	}
	else if( mach != fs.GetMachine() )
	{
		ABORT("Abstract machine mixed!");
	}
#endif

	FSP addr = fs.Deref();

	int ndata = mark.size();

	for( int i=0; i<ndata; i++ )
	{
		if( mark[i].addr == addr.GetAddress() )
		{
			mark[i].ref++;
			return;
		}
	}

	mark.push_back(AddrRefPair());
	AddrRefPair &ns = mark.back();
	
	ns.addr = addr.GetAddress();
	ns.ref  = 1;

	cell c = addr.ReadCell();
	if( IsSTR(c) )
	{
		const type *t = c2STR(c);
		for( int i=0; i<t->GetNFeatures(); i++ )
		{
			AddMark( addr.FollowNth(i) );
		}
	}
#ifdef PACKING
	if ( IsDSJ(c) )
	{
		for ( int i = 0; i < addr.GetNDisjuncts(); i++ )
		{
            AddMark( addr.FollowNthDisjunct(i) );
		}
	}
	else if ( IsPFS(c) )
	{
		AddMark( addr.FollowDFS() );
		for ( int i = 0; i < addr.GetNDependencies(); i++ )
		{
            AddMark( addr.FollowNthDependency( i ) );
		}
	}
	else if ( IsDEP(c) )
	{
		for ( int i = 0; i < addr.GetNDisjNodes(); i++ )
		{
            AddMark( addr.FollowNthDisjNode( i ) );
            AddMark( addr.FollowNthDisj( i ) );
		}
	}
#endif // PACKING
}

outform FSP::DisplayAVM() {
	ShareList sl;
	sl.AddMark(*this);
	sl.MakeShare();
	return DisplayAVM(sl);
}

// Added by mitsuisi (1999.11.30)
outform FSP::DisplayAVMMinimum()
{
	ShareList sl;
	sl.AddMark(*this);
	sl.MakeShare();
	outform o(DisplayAVM(sl, 1, bot));
	if (o.IsEmpty())
	{
		return outform(bot->GetPrintName());
	}
	else
	{
		return o;
	}
}

outform FSP::DisplayLiLFeS() {
	ShareList sl;
	sl.AddMark(*this);
	sl.MakeShare();

	outform o(DisplayLiLFeS(sl));
	if (o.IsEmpty())
	{
		return outform(bot->GetName());
	}
	else
	{
		return o;
	}
}
// Added by mitsuisi (1999.11.30)
/*
string FSP::DisplayLiLFeSwithoutSpaces() { ShareList sl; sl.AddMark(*this); sl.MakeShare();
                               string o(DisplayLiLFeSwithoutSpaces(sl));
                               if (o == "") {
                                   return bot->GetName();
                               } else {
                                   return o;
                               }}
*/

outform FSP::DisplayAVM(ShareList &sl, bool minimum, const type *deftype)
{
	FSP ad = *this;
	
	string sh( "" );
	cell c = ad.ReadCell();

	int number = sl.SearchShare(ad);
	if( number != -1 )
	{
		sh = "[$" + int2str(number+1) + "] ";
		if (minimum)
		{
		    int sc = sl.GetShareCount(number);

		    if( sc == 1 && (IsVAR(c) && c2VAR(c) == bot) )
			{
				return outform( sh + bot->GetName() );
		    }
		    if( sc >= 2 || (IsVAR(c) && c2VAR(c) == deftype) )
			{
				return outform( sh + "..." );	// after the first appearance,
					// "..." is written instead of rewrite the structure again
			}
		}
		else if( sl.GetShareCount(number) >= 2 )
		{
			return outform( sh + "..." );	// after the first appearance,
			// "..." is written instead of rewrite the structure again
		}
	}
	else if (minimum && IsVAR(c) && c2VAR(c) == deftype)
	{
	    return outform();
	}

	tag_t t = Tag(c);
	switch( t )
	{
		case T_VAR:
			if( c2VAR(c) == nil )
			{
				return outform( sh + "< >" );
			}
			else
			{
				return outform( sh + c2VAR(c)->GetPrintName() );
			}
		case T_STR:
			{
				const type *t = c2STR(c);
				if( t == cons )
				{
					outform oshx;
					int find = -1;
					while( c == STR2c(cons) && find == -1 )
					{
						outform o = ad.FollowNth(0).DisplayAVM(sl, minimum);
						oshx.AddAtRight(o);
						ad = ad.FollowNth(1);
						c = ad.ReadCell();

						find = sl.SearchShare(ad);
						if( c == STR2c(cons) && find == -1 )
						{
							oshx.AddAtRight(outform(", "));
						}
					}

					outform osh( sh + "< ");
					osh.AddAtRight(oshx);

					if( c == VAR2c(nil) && find == -1 )
					{
						osh.AddAtRight(outform(" >"));
					}
					else
					{
						outform o = ad.DisplayAVM(sl, minimum, t_list);
						o.AddAtRight(outform(" >"));
						outform x(" | ");
						x.AddAtRight(o);
						osh.AddAtRight(x);
					}
					return osh;
				}
				else
				{
					outform osh( sh );
					outform o( t->GetPrintName() );
					
//					cout << o << "feats: " << t->GetNFeatures() <<endl;
					
					for( int i=0; i<t->GetNFeatures(); i++ )
					{
//						cout << "---1" << endl;
						const feature *f = t->Feature(i);
						
//						cout << t->GetPrintName() << " " << i << endl;
//						cout << f->GetPrintName() << endl;
						
						outform os(f->GetPrintName());
//						cout << os << endl;
						outform ox = FollowNth(i).DisplayAVM(sl, minimum, ad.GetType()->GetAppType(i));
//						cout << ox << endl;
						if (!ox.IsEmpty())
						{
						    os.AddAtRight(outform(":"));
//							cout << os << endl;
						    os.AddAtRight(ox);
//							cout << os << endl;
						    o.AddAtBottom(os);
//							cout << o << "---2" << endl;
						}
					}
//					cout << o << "---3" << endl;
					o.SurroundByBracket("|~", "| ", "|_", "|~", "~|", " |", "_|", "_|");
//					cout << o << "---4" << endl;
					osh.AddAtRight(o);
//					cout << o << "---5" << endl;
					return osh;
				}
			}
		case T_INT:
			{
				ostringstream oss;
				oss << c2INT(c);
				return outform( sh + oss.str().c_str() );
			}
		case T_FLO:
			{
				ostringstream oss;
				oss.setf(ios::showpoint);
				oss << c2FLO(c);
				return outform( sh + oss.str().c_str() );
			}
		case T_CHR:
			{
				ostringstream oss;
				oss << "'" << (uchar)c2CHR(c) << "'";
				return outform( sh + oss.str().c_str() );
			}
		case T_STG:
			{
				ostringstream oss;
				oss << "\"" << c2STG(c) << "\"";
				outform of = outform( sh + oss.str().c_str() );
				return of;
			}
		default:
			ABORT("programming");
	}
	return outform();
}

outform FSP::DisplayLiLFeS(ShareList &sl, const type *deftype)
{
	FSP ad = *this;
	
	string sh( "" ), sh2( "" );
	cell c = ad.ReadCell();

	int number = sl.SearchShare(ad);
	if( number != -1 )
	{
		sh = "$" + int2str(number+1);
		if( sl.GetShareCount(number) >= 2 || (IsVAR(c) && (void *) c2VAR(c) == (void *) deftype) )
		{
			return outform( sh );
		}
		sh = "(" + sh + "& ";
		sh2 = ")";
	}
	else if (IsVAR(c) && (void *) c2VAR(c) == (void *) deftype)
	{
		return outform();	// こうするとうまくいくが、
							// sh の立場がなくなる。
							// 下の方の IsEmpty との絡みの問題。
	}

	tag_t t = Tag(c);
	switch( t )
	{
		case T_VAR:
			if( c2VAR(c) == nil )
			{
				return outform( sh + "[ ]" + sh2 );
			}
			else
			{
				string s = c2VAR(c)->GetProperName();
				return outform( sh + s + sh2 );
			}
		case T_STR:
			{
				const type *t = c2STR(c);
				if( t == cons )
				{
					outform osh;
					if( number != -1 )
					{
						osh = outform(sh);
					}
					int find = -1;
					const char *s = "[ ";
					while( c == STR2c(cons) && find == -1 )
					{
						outform o(s);
						outform oe(ad.FollowNth(0).DisplayLiLFeS(sl));
						if ( oe.IsEmpty() )
						{
						    o.AddAtRightTop(bot->GetPrintName());
						}
						else
						{
						    o.AddAtRightTop(oe);
						}
						s = ", ";

						ad = ad.FollowNth(1);
						c = ad.ReadCell();

						find = sl.SearchShare(ad);
						osh.AddAtBottom(o);
					}

					if( c == VAR2c(nil) && find == -1 )
					{
						;
					}
					else
					{
						outform o = ad.DisplayLiLFeS(sl);
						outform x("| ");
						x.AddAtRightTop(o);
						osh.AddAtBottom(x);
					}
					osh.SurroundByBracket("", "", "", "", " ", " ", "]", "]");
					if( number != -1 )
					{
						osh.AddAtRightBottom(outform(sh2));
					}
					return osh;
				}
				else if( t->IsSubType(t_pred[1]) )
				{
//					cout << "Hoge" << endl;
					// Calculate the best representational type
					const type *tp = t;
					const type *oldtp;
					do {
//						cout << tp->GetName() << endl;
						oldtp = tp;
						for( int i=0; i<tp->GetNParents(); i++ )
						{
							const type *ta = tp->Parent(i);
							if( ta != (const type *)t_pred[ta->GetNFeatures()] && ta->GetNChildren() == 1 && ta->IsSubType(pred) )
							{
								tp = ta;
								break;
							}
						}
					} while( oldtp != tp );
					
					outform oa;
					for( int i=0; i<t->GetNFeatures(); i++ )
					{
						const feature *f = t->Feature(i);
//						cout << f->GetName() << " " << f_arg[i+1]->GetName() << endl;
						if( f != f_arg[i+1] )
						{
							goto NORMAL_OUT;
						}
						outform oa1 = FollowNth(i).DisplayLiLFeS(sl, GetType()->GetAppType(i));
						if( oa1.IsEmpty() )
						{
							oa1 = outform("_");
						}
						oa1.SurroundByBracket(i==0?"(":",", " ", " ", i==0?"(":",", "", "", "", "");

						oa.AddAtBottom(oa1);
					}
					oa.SurroundByBracket( "", "", "", "", " ", " ", ")", ")" );
					outform osh;
					if( number != -1 )
					{
						osh = outform(sh);
					}

					osh.AddAtRight(outform( tp->GetProperName() ));
					osh.AddAtRightTop(oa);
					return osh;
				}
				else
				{
			NORMAL_OUT:
					outform osh;
					if( number != -1 )
					{
						osh = outform(sh);
					}

					osh.AddAtRight(outform( t->GetProperName() ));
						
					for( int i=0; i<t->GetNFeatures(); i++ )
					{
						const feature *f = t->Feature(i);
						
						outform os( "&" + string(f->GetProperName()) + "\\");
//						FSP fi = FollowNth(i);
						outform ox = FollowNth(i).DisplayLiLFeS(sl, GetType()->GetAppType(i));
						if( !ox.IsEmpty() )
						{
							os.AddAtRightTop(ox);
							osh.AddAtBottom(os);
						}
					}

					osh.SurroundByBracket("(", " ", " ", "(", " ", " ", ")", ")");
					if( number != -1 )
					{
						osh.AddAtRightBottom(outform(sh2));
					}
					return osh;
				}
			}
		case T_INT:
			{
				ostringstream oss;
				oss << c2INT(c);
				return outform( sh + oss.str().c_str() + sh2 );
			}
		case T_FLO:
			{
				ostringstream oss;
				oss.setf(ios::showpoint);
				oss << c2FLO(c);
				return outform( sh + oss.str().c_str() + sh2 );
			}
		case T_CHR:
			{
				ostringstream oss;
				oss << "'" << (uchar)c2CHR(c) << "'";
				return outform( sh + oss.str().c_str() + sh2);
			}
		case T_STG:
			{
				const char *p = c2STG(c);
				char *q = new char[strlen(p)*2+1];
				strcpy(q, p);
				char *r, *s = q;

				while((r=strstr(q,"\"")) != NULL) 
				{ 
					memmove(r+1,r,strlen(r)+1); 
					q=r+2; 
				} 

				outform of = outform( sh + "\"" + s + "\"" + sh2 );
				delete s;
				return of;
			}
		default:
			ABORT("programming");
	}
	return outform();
}

// Added by mitsuisi (1999.11.30)
/*
string FSP::DisplayLiLFeSwithoutSpaces(ShareList &sl, const type *deftype)
{
	FSP ad = *this;
	
	string sh( "" ), sh2( "" );
	cell c = ad.ReadCell();

	int number = sl.SearchShare(ad);
	if( number != -1 )
	{
		sh = "$" + int2str(number+1);
		if( sl.GetShareCount(number) >= 2 || (IsVAR(c) && (void *) c2VAR(c) == (void *) deftype) )
			return sh;
		sh = "(" + sh + "&";
		sh2 = ")";
	} else if (IsVAR(c) && (void *) c2VAR(c) == (void *) deftype) {
		return "";
	}

	tag_t t = Tag(c);
	switch( t )
	{
		case T_VAR:
			if( c2VAR(c) == nil )
				return sh + "[]" + sh2;
			else
			{
				string s = c2VAR(c)->GetProperName();
				return sh + s + sh2;
			}
		case T_STR:
			{
				const type *t = c2STR(c);
				if( t == cons )
				{
				        string osh;
					if( number != -1 )
						osh = sh;
					int find = -1;
					char *s = "[";
					while( c == STR2c(cons) && find == -1 )
					{
						string o = s;
						string oe = ad.FollowNth(0).DisplayLiLFeSwithoutSpaces(sl);
						if ( oe == "" ) {
						    o += bot->GetPrintName();
						} else {
						    o += oe;
						}
						s = ",";

						ad = ad.FollowNth(1);
						c = ad.ReadCell();

						find = sl.SearchShare(ad);
						osh += o;
					}

					if( c == VAR2c(nil) && find == -1 )
						;
					else {
					        string o = ad.DisplayLiLFeSwithoutSpaces(sl);
						string x = "|";
						x += o;
						osh += x;
					}
					osh += "]";
					if( number != -1 )
						osh += sh2;
					return osh;
				}
				else if( t->IsSubType(t_pred[1]) )
				{
//					cout << "Hoge" << endl;
					// Calculate the best representational type
					const type *tp = t;
					const type *oldtp;
					do {
//						cout << tp->GetName() << endl;
						oldtp = tp;
						for( int i=0; i<tp->GetNParents(); i++ )
						{
							const type *ta = tp->Parent(i);
							if( ta != (const type *)t_pred[ta->GetNFeatures()] && ta->GetNChildren() == 1 && ta->IsSubType(pred) )
							{
								tp = ta;
								break;
							}
						}
					} while( oldtp != tp );
					
					string oa;
					for( int i=0; i<t->GetNFeatures(); i++ )
					{
						const feature *f = t->Feature(i);
//						cout << f->GetName() << " " << f_arg[i+1]->GetName() << endl;
						if( f != f_arg[i+1] )
							goto NORMAL_OUT;
						string oa1 = FollowNth(i).DisplayLiLFeSwithoutSpaces(sl, GetType()->GetAppType(i));
						if ( oa1 == "" )
						    oa1 = "_";
						oa1 = (i==0?"(":",") + oa1;

						oa += oa1;
					}
					oa += ")";
					string osh;
					if( number != -1 )
						osh = sh;

					osh += tp->GetProperName();
					osh += oa;
					return osh;
				}
				else
				{
			NORMAL_OUT:
					string osh;
					if( number != -1 )
						osh = sh;

					osh += t->GetProperName();
						
					for( int i=0; i<t->GetNFeatures(); i++ )
					{
						const feature *f = t->Feature(i);
						
						string os = "&" + string(f->GetProperName()) + "\\";
//						FSP fi = FollowNth(i);
						string ox = FollowNth(i).DisplayLiLFeSwithoutSpaces(sl, GetType()->GetAppType(i));
						if( ox != "" )
						{
							os += ox;
							osh += os;
						}
					}

					osh = "(" + osh + ")";
					if( number != -1 )
						osh += sh2;
					return osh;
				}
			}
		case T_INT:
			{
				char buf[64];
				ostringstream oss(buf, sizeof(buf));
				oss << c2INT(c) << ends;
				return sh + oss.str() + sh2;
			}
		case T_FLO:
			{
				char buf[64];
				ostringstream oss(buf, sizeof(buf));
				oss.setf(ios::showpoint);
				oss << c2FLO(c) << ends;
				return sh + oss.str() + sh2;
			}
		case T_CHR:
			{
				char buf[64];
				ostringstream oss(buf, sizeof(buf));
				oss << "'" << (uchar)c2CHR(c) << "'" << ends;
				return sh + oss.str() + sh2;
			}
		case T_STG:
			{
				ostringstream oss;
				char *p = c2STG(c);
				char *q = new char[strlen(p)*2+1];
				strcpy(q, p);
				char *r, *s = q;

				while((r=strstr(q,"\"")) != NULL) 
				{ 
					memmove(r+1,r,strlen(r)+1); 
					q=r+2; 
				} 

				string of = sh + "\"" + s + "\"" + sh2;
				delete s;
				return of;
			}
		default:
			ABORT("programming");
	}
	return "";
}
*/

//////////////////////////////////////////////////////////////////////

ptree *GenPTreeOfConstraint(ShareList &sl, FSP fs, const type *t, bool self, bool)
{
	if( self )
	{
		FSP fspc = fs.Follow(feature::Search(
			module::CoreModule()->Search("constr\\")));
		FSP fspp = fs.Follow(feature::Search(
			module::CoreModule()->Search("pred\\")));

		ptree *desc = GenPTreeOfDescription(sl, fspc);
		if( desc==NULL ) 
		{
			return NULL;
		}
//		if( feat )
//			desc = new pconj(new pfeatconstr(t), desc);
		pprocdef *h = new pprocdef(t, new pdefarg(new pempty, desc));
		ptree *c    = (fspp.GetType() == bot ? new pempty : GenPTreeOfGoal      (sl, fspp) );
		if( c==NULL )
		{
			return NULL;
		}
		return new pconstraint(h, c);
	}
	else
	{
		return new pempty;
//		return new pconstraint(new pprocdef(t, new pfeatconstr(t)), new pempty);
	}
}


ptree *GenPTreeOfClause(ShareList &sl, FSP fs)
{
	const type *t = fs.GetType();

	if( t->IsSubType(pred) )
	{ // fact 
		pprocdef *h = GenPTreeOfDefinition(sl, fs);
		if( h == NULL )
		{
			return NULL;
		}
		return new pclause( h, new pempty );
	}
	else if( t->IsSubType(t_means) )
	{ /* clause */
		if( t->GetIndex(f_chead) != -1 )
		{ // head present; clause declaration
			pprocdef *h = GenPTreeOfDefinition(sl, fs.Follow(f_chead));
			ptree *c = GenPTreeOfGoal      (sl, fs.Follow(f_cbody));
			if( h == NULL || c == NULL )
			{
				return NULL;
			}
			return new pclause(h, c);
		}
		else
		{ // no head present
			pprocdef *h = GenPTreeOfDefinition(sl, FSP(fs.GetMachine()));
			ptree *c = GenPTreeOfGoal(sl, fs.Follow(f_cbody));
			if( h == NULL || c == NULL )
			{
				return NULL;
			}
			return new pclause(h, c);
		}
	}
	else
	{
		INERR(fs.GetMachine(), "Input is neither type declaration nor clause definition");
		return NULL;
	}
}

ptree *GenPTreeOfDescription(ShareList &sl, FSP fs, const type *basetype)
{
	machine *mach = fs.GetMachine();
	core_p addr = mach->Deref(fs.GetAddress());
	ptree *var = NULL;

	int cnt = sl.SearchShare(fs);
	if( cnt != -1 )
	{
		ostringstream oss;
		oss << "$" << (cnt+1);
		var = new pvariable(oss.str().c_str());

		if( sl.GetShareCount(cnt) != 1 )
		{
			return var;
		}
	}

	ptree *tree;
	cell c = mach->ReadHeap(addr);
	switch( Tag(c) )
	{
	case T_VAR:
		if( c2VAR(c) != basetype )
		{
			tree = new ptype(c2VAR(c));
		}
		else
		{
			tree = NULL;
		}
		break;

	case T_STR:
		{
			const type *t = c2STR(c);
			int nf = t->GetNFeatures();
		
			if( t != basetype )
			{
				tree = new ptype(t);
			}
			else
			{
				tree = NULL;
			}
			for( int i=0; i<nf; i++ )
			{
				ptree *q = GenPTreeOfDescription(sl, FSP(mach, addr+i+1), t->GetAppType(i) );
				if( q==NULL )
				{
					return NULL;
				}
				
				if( strcmp(q->GetClassName(), pempty::className) == 0 )
				{
					delete q;
				}
				else
				{
					q = new pfeature(t->Feature(i), q);
					if( tree == NULL )
					{
						tree = q;
					}
					else
					{
						tree = new pconj(tree, q);
					}
				}
			}
			break;
		}
	case T_INT:
			tree = new pint(c2INT(c));
			break;
	case T_FLO:
			tree = new pfloat(c2FLO(c));
			break;
	case T_CHR:
			tree = new pchar(c2CHR(c));
			break;
	case T_STG:
			tree = new pstring(c2STG(c));
			break;
	default:
			ABORT("Unknown Tag");
	}

	if( var != NULL )
	{
		if( tree != NULL )
		{
			return new pconj(var, tree);
		}
		else
		{
			return var;
		}
	}
	else
	{
		if( tree != NULL )
		{
			return tree;
		}
		else
		{
			return new pempty;
		}
	}
}

pprocdef *GenPTreeOfDefinition(ShareList &sl, FSP fs)
{
	machine *mach = fs.GetMachine();
	core_p addr = mach->Deref(fs.GetAddress());
	cell c = mach->ReadHeap(addr);
	const type *t = c2STRorVAR(c);
	ptree *parg = new pempty;

	if( fs.IsLeaf() && t != bot )
	{
		type *t0;
		type *t1 = (type *)(fs.GetType());
		if( ! t1->IsSubType(pred) )
		{
			INERR(fs.GetMachine(), t1->GetName() << " is not a predicate");
			return NULL;
		}
		else
		{
			t = t1->TypeUnify(t_pred[0]);
			if( t != NULL )
			{
				if( t->IsSubType(t_pred[1]) )
				{
					for( int i=0; i<t->GetNParents(); i++ )
					{
						if( t->Parent(i)->GetModule() != module::CoreModule() && t->Parent(i)->IsSubType(pred) )
						{
							INERR(fs.GetMachine(), "Predicate " << t1->GetName() << " cannot be used without arguments");
							return NULL;
						}
					}
				}
			}
			else
			{
				type *t2 = t_pred[0];
				ostringstream oss;
				oss << t1->GetSimpleName() << "/0";
				if( t1->GetModule()->Search(oss.str().c_str()) )
				{
					INERR(fs.GetMachine(), "Functor " << t1->GetProperName() << " cannot be used;  Existing type " << (t1->GetModule()->Search(oss.str().c_str()))->GetProperName() << " prevents internal type generation");
					return NULL;
				} else {
					t0 = new type(oss.str().c_str(),t1->GetModule(),t1->IsExported()); t0->SetAsChildOf(t1); t0->SetAsChildOf(t2);
					t0->Fix();
					t = t0;
					procedure::NewTypeNotify();
				}
			}
		}
	}
	for( int i=1; i<MAXARGS; i++ )
	{
		if( t->IsSubType(t_pred[i]) )
		{
			ptree *p = GenPTreeOfDescription(sl, fs.Follow(f_arg[i]));
			if( p==NULL )
			{
				delete parg;
				return NULL;
			}
			parg = new pdefarg(parg, p);
		}
	}
	return new pprocdef(t, parg);
}

ptree *GenPTreeOfCall(ShareList &sl, FSP fs)
{
	machine *mach = fs.GetMachine();
	core_p addr = mach->Deref(fs.GetAddress());
	cell c = mach->ReadHeap(addr);
	const type *t = c2STRorVAR(c);

	if( t == t_cut )
	{
		return new pcut();
	}

	if( t->IsSubType(t_equal) )
	{
		ptree *p1 = GenPTreeOfDescription(sl, fs.Follow(f_arg[1]));
		ptree *p2 = GenPTreeOfDescription(sl, fs.Follow(f_arg[2]));
		if( p1==NULL || p2==NULL )
		{
			return NULL;
		}
		return new pequal(p1, p2);
	}

	ptree *parg = new pempty;
	if( ! t->IsSubType(pred) )
	{
		if( t == t_means )
		{
			INERR(fs.GetMachine(), " :- is not a goal (possibly a period is missing at the end of the previous clause)");
		}
		else
		{
			INERR(fs.GetMachine(), t->GetName() << " is not a predicate");
		}
		return NULL;
	}
	if( fs.IsLeaf() )
	{
		type *t0;
		type *t1 = (type *)(fs.GetType());
		t = t1->TypeUnify(t_pred[0]);
		if( t != NULL )
		{
			if( t->IsSubType(t_pred[1]) )
			{
				for( int i=0; i<t->GetNParents(); i++ )
				{
					if( t->Parent(i)->GetModule() != module::CoreModule() && t->Parent(i)->IsSubType(pred) )
					{
						INERR(fs.GetMachine(), "Predicate " << t1->GetName() << " cannot be used without arguments");
						return NULL;
					}
				}
			}
		}
		else
		{
			type *t2 = t_pred[0];
			ostringstream oss;
			oss << t1->GetSimpleName() << "/0";
			if( t1->GetModule()->Search(oss.str().c_str()) )
			{
				INERR(fs.GetMachine(), "Functor " << t1->GetProperName() << " cannot be used;  Existing type " << (t1->GetModule()->Search(oss.str().c_str()))->GetProperName() << " prevents internal type generation");
				return NULL;
			} else {
				t0 = new type(oss.str().c_str(),t1->GetModule(),t1->IsExported()); t0->SetAsChildOf(t1); t0->SetAsChildOf(t2);
				t0->Fix();
				t = t0;
				procedure::NewTypeNotify();
			}
		}
	}

	for( int i=1; i<MAXARGS; i++ )
	{
		if( t->IsSubType(t_pred[i]) )
		{
			ptree *p = GenPTreeOfDescription(sl, fs.Follow(f_arg[i]));
			if( p==NULL )
			{
				return NULL;
			}
			parg = new pcallarg(parg, p);
		}
	}

	return new pcall(t, parg);
}

ptree *GenPTreeOfGoal(ShareList &sl, FSP fs, const char *label /* = NULL */ )
{	
	machine *mach = fs.GetMachine();
	core_p addr = mach->Deref(fs.GetAddress());
	cell c = mach->ReadHeap(addr);
	const type *t = c2STRorVAR(c);

	if( t == t_is )
	{
		ptree *p1 = GenPTreeOfDescription(sl, FSP(mach, addr+1));
		ptree *p = GenPTreeOfExpression(sl, p1, FSP(mach, addr+2));
		if( p1==NULL || p==NULL )
		{
			return NULL;
		}
		return p;
	}
	if( t == t_comma )
	{
		ptree *p1 = GenPTreeOfGoal(sl, FSP(mach, addr+1));
		ptree *p2 = GenPTreeOfGoal(sl, FSP(mach, addr+2));
		if( p1==NULL || p2==NULL )
		{
			return NULL;
		}
		return new pgoalconj( p1, p2 );
	}
	else if( t == t_semicolon )
	{
		FSP disj1(mach, addr+1);
		FSP disj2(mach, addr+2);
		core_p addr2 = disj1.Deref().GetAddress();

		ostringstream oss;
		oss << "_ IF_THEN_ELSE" << cell2int(PTR2c(addr));
                string label_str( label ? label : oss.str() );

		if( disj1.GetType() == t_ifthen )
		{
			ShareList slcopy(sl);
			ptree *p1 = GenPTreeOfGoal(sl, FSP(mach, addr2+1));
			ptree *p2 = GenPTreeOfGoal(sl, FSP(mach, addr2+2), label_str.c_str());
			ptree *p3 = GenPTreeOfGoal(slcopy, disj2, label_str.c_str());
			if( p1==NULL || p2==NULL || p3==NULL )
			{
				return NULL;
			}
			sl.MergeRef(slcopy);

			ptree *tree = 
				new pgoaldisj( 
						new pgoalconj(
							p1,
							new pgoalconj(
								new pcut( label_str.c_str() ),
								p2 )),
						p3 );
			return label ? tree : new pcutregion(label_str.c_str(), tree);
		}
		else
		{
			ShareList slcopy(sl);
			ptree *p1 = GenPTreeOfGoal(sl, disj1);
			ptree *p2 = GenPTreeOfGoal(slcopy, disj2, label_str.c_str());
			if( p1==NULL || p2==NULL )
			{
				return NULL;
			}
			sl.MergeRef(slcopy);
			return 
			  label ? (ptree*)( new pgoaldisj( p1, p2 ) )
					: (ptree*)( new pcutregion( label_str.c_str(), new pgoaldisj( p1, p2 ) ) );
		}
	}
	else if( t == t_ifthen )
	{
		ptree *p1 = GenPTreeOfGoal(sl, FSP(mach, addr+1));
		ptree *p2 = GenPTreeOfGoal(sl, FSP(mach, addr+2));
		if( p1==NULL || p2==NULL )
		{
			return NULL;
		}

		ostringstream oss;
		oss << "_ IF_THEN" << cell2int(PTR2c(addr));
		string label_str( label ? label : oss.str() );

		ptree *tree = 
			new pgoalconj( p1,  
				new pgoalconj(
					new pcut( label_str.c_str() ), 
					p2) );

		return label ? tree : new pcutregion( label_str.c_str(), tree );
	}
	else if( t == t_not )
	{
		ptree *p1 = GenPTreeOfGoal(sl, FSP(mach, addr+1));
		ptree *p2 = GenPTreeOfGoal(sl, FSP(mach, VAR2c(module::CoreModule()->Search("fail"))));
		if( p1==NULL || p2==NULL )
		{
			return NULL;
		}

		ostringstream oss;
		oss << "_ NOT" << cell2int(PTR2c(addr));
		string label_str( label ? label : oss.str() );

		ptree *tree = 
			new pgoaldisj( 
				new pgoalconj(
					p1,
					new pgoalconj(
						new pcut( label_str.c_str() ),
						p2 )), 
				new pempty);

		return label ? tree : new pcutregion( label_str.c_str(), tree );
	}
	else
	{
		return GenPTreeOfCall(sl, fs);
	}
}

ptree *GenPTreeOfExpression(ShareList &sl, ptree *&result, FSP fsx)
{
	static int valcnt = 0;	// for generating valnames

	ptree *px = NULL;

	if( fsx.IsInteger() )
	{
		px = new pint(fsx.ReadInteger());
	}
	else if( fsx.IsFloat() )
	{
		px = new pfloat(fsx.ReadFloat());
	}
	else if( fsx.IsString() )
	{
		px = new pstring(fsx.ReadString());
	}
	else if( fsx.GetType() == bot || fsx.GetType() == t_int || fsx.GetType() == t_flo || fsx.GetType() == t_stg )
	{
		// It may be variable
		px = GenPTreeOfDescription(sl, fsx);
		if( px == NULL ) return NULL;
	}

	if( px )
	{
		if( result )
		{
			return new pequal(result, px);
		}
		else
		{
			result = px;
			return new pempty;
		}
	}

	ptree *var;

	if( result != NULL )
	{
		var = result;
	}
	else
	{
		ostringstream oss;
		oss << "$$" << (valcnt++);
		result = new pvariable(oss.str().c_str());
		var = new pvariable(oss.str().c_str());
	}

	const type *t = fsx.GetType();
	const type *tr = NULL;
	ptree *parg = new pempty;
	ptree *p1 = NULL;
	ptree *p2 = NULL;

	if( t->IsSubType(t_plus) )
	{
		if( t == module::CoreModule()->Search("+/1") )
		{
			return GenPTreeOfExpression(sl, result, fsx.Follow(f_arg[1]));
		}
		tr = module::CoreModule()->Search("+/3");
	}
	else if( t->IsSubType(t_minus) )
	{
		tr = module::CoreModule()->Search("-/3");
		if( t == module::CoreModule()->Search("-/1") )
		{
			ptree *pa = GenPTreeOfExpression(sl, p1, fsx.Follow(f_arg[1]));
			if( pa == NULL )
			{
				return NULL;
			}
			FSP fsc(fsx.GetMachine(), (mint)0);
			return new pgoalconj(pa, new pcalcrev(p1, tr, fsc, result));
		}
	}
	if( tr )
	{
		if( fsx.Follow(f_arg[2]).IsInteger() )
		{
			ptree *pa = GenPTreeOfExpression(sl, p1, fsx.Follow(f_arg[1]));
			if( pa == NULL )
			{
				return NULL;
			}
			return new pgoalconj(pa, new pcalc(p1, tr, fsx.Follow(f_arg[2]), result));
		}
		else if( fsx.Follow(f_arg[1]).IsInteger() )
		{
			ptree *pa = GenPTreeOfExpression(sl, p1, fsx.Follow(f_arg[2]));
			if( pa == NULL )
			{
				return NULL;
			}
			return new pgoalconj(pa, new pcalcrev(p1, tr, fsx.Follow(f_arg[1]), result));
		}
	}
	else if( t->IsSubType(t_aster) )
	{
		tr = module::CoreModule()->Search("*/3");
	}
	else if( t->IsSubType(t_slash) )
	{
		tr = module::CoreModule()->Search("//3");
	}
	else if( t->IsSubType(t_intdiv) )
	{
		tr = module::CoreModule()->Search("///3");
	}
	else if( t->IsSubType(t_modulo) )
	{
		tr = module::CoreModule()->Search("mod/3");
	}
	else if( t->IsSubType(t_bitand) )
	{
		tr = module::CoreModule()->Search("/\\/3");
	}
	else if( t->IsSubType(t_bitor) )
	{
		tr = module::CoreModule()->Search("\\//3");
	}
	else if( t->IsSubType(t_bitxor) )
	{
		tr = module::CoreModule()->Search("#/3");
	}
	else if( t->IsSubType(t_bitnot) )
	{
		tr = module::CoreModule()->Search("\\/2");
	}

//	cout << t->GetPrintName() << ", " << tr->GetPrintName() << endl;
	if( tr == NULL )
	{
		INERR(fsx.GetMachine(), "Invalid expression");
		return NULL;
	}

	ptree *pa = GenPTreeOfExpression(sl, p1, fsx.Follow(f_arg[1]));
	if( pa == NULL )
	{
		return NULL;
	}
	parg = new pcallarg(parg, p1);
	if( t->GetNFeatures() >= 2 )
	{
		ptree *pb = GenPTreeOfExpression(sl, p2, fsx.Follow(f_arg[2]));
		if( pb == NULL )
		{
			return NULL;
		}
		parg = new pcallarg(parg, p2);
		pa = new pgoalconj(pa, pb);
	}
	parg = new pcallarg(parg, var);

	return new pgoalconj(pa, new pcall(tr, parg));
}

ptree *GenPTreeOfImmediateGoal(ShareList &, FSP fs)
{
	return new pinterpret(fs);
}

int FSP::GetArity() const
{
	const type *t = GetType();
	int i;
	if( ! t->IsSubType(pred) )
	{
		ABORT("Internal error: cannot take arity of non-pred");
	}
	
	for( i=1; i<MAXARGS; i++ )
	{
		if( ! t->IsSubType(t_pred[i]) )
		{
			break;
		}
	}

	if( i >= MAXARGS )
	{
		ABORT("Arglist contains unknown type??");
	}
	
	return i-1;
}

bool FSP::b_equiv(FSP arg2)
{
#ifdef PROF_COPY
	P1("FeatureStructure::b_equiv");
#else
	P2("FeatureStructure::b_equiv");
#endif

#ifdef FORCE_NORMAL_COPY
	vector<AddrPair> pairinfo;
	return c_equiv_(*GetMachine(), pairinfo, GetAddress(), arg2.GetAddress());
#else
	machine &m = *GetMachine();
	FSP arg1 = *this;
	core_p addr1 = arg1.GetAddress();
	core_p addr2 = arg2.GetAddress();
	if (addr1 == addr2)
	{
		return true;
	}

	uint a1 = core_p2int(addr1);
	uint a2 = core_p2int(addr2);
	if (a1 < a2)
	{
		core_p tmp;
		tmp = addr1;
		addr1 = addr2;
		addr2 = tmp;
	}
	uint offset = addr1 - addr2;
    
	while(1)
	{
		cell c1 = m.ReadHeap(addr1++);
		cell c2 = m.ReadHeap(addr2++);
		tag_t tag1 = Tag(c1);
//		tag_t tag2 = Tag(c2);
		switch(tag1)
		{
		case T_PTR3:
			if (c1 == PTR2c(CORE_P_INVALID))
			{
				return true;
			}
		case T_PTR:
		case T_PTR1:
		case T_PTR2:
			if (!IsPTR(c2))
			{
				return false;
			}
			if (c2PTR(c1) != c2PTR(c2) + offset)
			{
				return false;
			}
			continue;
		default:
			if (c1 != c2)
			{
				return false;
			}
			continue;
		}
	}
#endif
}

} // namespace lilfes
