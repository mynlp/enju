/*
 * $Id: ldbm-core.h,v 1.16 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __ldbm_core_h
#define __ldbm_core_h

#include "../pstl/pstl_pheap.h"
#include "../pstl/pstl_vector.h"
#include "../pstl/pstl_hash_map.h"
#include "ldbm-cell.h"
#include "ldbm-type.h"

#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

namespace ldbm {
    
    struct hash_core_ptr;
    struct equal_to_core_ptr;
    
    template <class _Alloc> class core {
    public:
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename bit_type::size_type size_type;
        typedef typename bit_type::pointer pointer;
        typedef typename cell<bit_type>::core_p core_p;
#if SIZEOF_INTP == 4
        typedef TypeMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b32> > typemapper_type;
        typedef FeatureMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b32> > featmapper_type;
        typedef typename pstl::b32::size_type lil_size_type;
#elif SIZEOF_INTP == 8
        typedef TypeMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b64> > typemapper_type;
        typedef FeatureMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b64> > featmapper_type;
        typedef typename pstl::b64::size_type lil_size_type;
#endif
        typedef pstl::vector<cell<bit_type>, allocator_type> heap_type;

    private:        
        heap_type heap;
        static const size_t serialize_size = 32768;
        
    public:
        core() {}
        core(pstl::ptr<allocator_type, bit_type> allocp) : heap(allocp) {}
        ~core() {}
        void construct() {
            new ((void*) this) core();
        }
        void construct(pstl::ptr<allocator_type, bit_type> allocp) {
            new ((void*) this) core(allocp);
        }
        void destruct(pheap_type *ph) {
            heap(ph)->destruct(ph);
        }
        static pstl::ptr<core, bit_type> pnew(pheap_type *ph, pstl::ptr<allocator_type, bit_type> allocp) {
            pstl::allocator_reference<dev_type, bit_type> alloc(ph, allocp);
            pstl::ptr<core, bit_type> p = (pstl::ptr<core, bit_type>) alloc.allocate(sizeof(core));
            p(ph)->construct(allocp);
            return p;
        }
        
        size_type size() const { return heap.size(); }
        size_type max_size() const { return heap.max_size(); }
        size_type capacity() const { return heap.capacity(); }
        bool empty() const { return heap.empty(); }
        void reserve(size_type __n) { heap.reserve(__n); }
        void resize(pheap_type *ph, size_type __new_size) { heap.resize(ph, __new_size); }
        void clear(pheap_type *ph) { heap.clear(ph); }

    private:
        void __write(pheap_type *ph, lilfes::machine *mach, lilfes::core_p addr0) {
            
            lilfes::cell c;
            lilfes::core_p base0 = mach->GetHP();
            core_p base1 = (core_p) heap.size();
            core_p addr1;

            while( lilfes::IsPTR(c = mach->ReadHeap(addr0)))
                addr0 = c2PTR(c);

            switch(lilfes::Tag(c)) {
                case lilfes::T_PTR:
                case lilfes::T_PTR1:
                case lilfes::T_PTR2:
                case lilfes::T_PTR3:
                    std::cerr << "internal heap error" << std::endl; exit(0);
                    
                case T_VAR:
                case T_INT:
                case T_FLO:
                {
                    addr1 = (core_p) heap.size();
                    mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (addr1 - base1)));
                    heap.push_back(ph, cell2cell<bit_type>(c));
                    break;
                }
                case T_CHR:
                    std::cerr << "T_CHR in LiLFeS" << std::endl; exit(0);
                case T_STG:
                {
                    addr1 = (core_p) heap.size();
                    mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (addr1 - base1)));
                    char *str = lilfes::c2STG(c);
                    typename cell<bit_type>::size_type len = (typename cell<bit_type>::size_type) strlen(str);
                    heap.push_back(ph, STG2c<bit_type>(len));
                    
                    typename cell<bit_type>::size_type k = 0;
                    for(; k < len ; ) {
                        cell<bit_type> ic(0);
                        for(int j = 0 ; j < cell<bit_type>::charlen ; j++, k++) {
                            ic.word = ic.word << 8;
                            if( k < len ) {
                                unsigned char ustr = *(str++);
                                ic.word |= (typename cell<bit_type>::word_type) ustr;
                            }
                        }
                        heap.push_back(ph, CHR2c<bit_type>(ic.word));
                    }
                    break;
                }
                case T_STR:
                {
                    
                    static std::vector< std::pair<lilfes::core_p, typename core<allocator_type>::core_p> > array(serialize_size);
                    array.clear();

                    addr1 = (core_p) heap.size();
                    array.push_back( std::make_pair(addr0,addr1) );
                    int slot = lilfes::GetSlot(base0);
                    heap.resize(ph, addr1+1);

                    while( ! array.empty() ) {
                        addr0 = array.back().first;
                        addr1 = array.back().second; 
                        array.pop_back();
					
                        c = mach->ReadHeap(addr0);
				
                        if( lilfes::IsPTR(c) ) {
                            heap.find(ph, addr1) = PTR2c<bit_type>(base1 + ((lilfes::c2PTR(c) - base0)));
                        } else {
                            unsigned nf = lilfes::c2STR(c)->GetNFeatures();

                            typename core<allocator_type>::core_p hp1 = heap.size();
                            mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (hp1 - base1)));
                            heap.find(ph, addr1) = PTR2c<bit_type>(hp1);

                            heap.resize(ph, hp1 + nf + 1);
                            heap.find(ph, hp1) = cell2cell<bit_type>(c);
                            
                            for( unsigned i=nf; i>0; i-- ) {
                                lilfes::core_p a = addr0 + i;
                                while( lilfes::IsPTR(c = mach->ReadHeap(a)) ) {
                                    a = lilfes::c2PTR(c);
                                    if( (lilfes::GetSlot(a) == slot) && a >= base0 ) {
                                        heap.find(ph, hp1 + i) = PTR2c<bit_type>(base1 + (a - base0));
                                        goto hogehoge;
                                    }
                                }
				    
                                if( lilfes::IsSTR(c) ) {
                                    array.push_back( std::make_pair(a, hp1 + i) );
                                } else if( lilfes::IsSTG(c) ) {
                                    addr1 = heap.size();
                                    mach->WriteHeap(a, lilfes::PTR2c(base0 + (addr1 - base1)));
                                    heap.find(ph, hp1+i) = PTR2c<bit_type>(addr1);
					
                                    char *str = lilfes::c2STG(c);
                                    typename cell<bit_type>::size_type len = (typename cell<bit_type>::size_type) strlen(str);
                                    heap.push_back(ph, STG2c<bit_type>(len));
                                    typename cell<bit_type>::size_type k = 0;
                                    for(; k < len ; ) {
                                        cell<bit_type> ic(0);
                                        for(int j = 0 ; j < cell<bit_type>::charlen ; j++, k++) {
                                            ic.word = ic.word << 8;
                                            if( k < len ) {
                                                unsigned char ustr = *(str++);
                                                ic.word |= (typename cell<bit_type>::word_type) ustr;
                                            }
                                        }
                                        heap.push_back(ph, CHR2c<bit_type>(ic.word));
                                    }
                                } else {
                                    mach->WriteHeap(a, lilfes::PTR2c(base0 + (hp1 + i - base1)));
                                    heap.find(ph, hp1+i) = cell2cell<bit_type>(c);
                                }
                              hogehoge: ;
                            }
                        }
                    }
		    break;
                }
                default:
                    std::cerr << "unknown tag" << std::endl; exit(0);
            }
        }
        void __write(pheap_type *ph, lilfes::machine *mach, lilfes::core_p addr0, typemapper_type *typemap, featmapper_type *featmap) {
//            std::cout << "__write addr0: " << addr0 << std::endl;
            if(!typemap) {
                __write(ph, mach, addr0);
            } else {
            
                lilfes::cell c;
                lilfes::core_p base0 = mach->GetHP();
                core_p base1 = (core_p) heap.size();
                core_p addr1;

                while( lilfes::IsPTR(c = mach->ReadHeap(addr0)))
                    addr0 = c2PTR(c);

                switch(lilfes::Tag(c)) {
                    case lilfes::T_PTR:
                    case lilfes::T_PTR1:
                    case lilfes::T_PTR2:
                    case lilfes::T_PTR3:
                        std::cerr << "internal heap error" << std::endl; exit(0);
                    
                    case T_VAR:
                    {
//                        std::cout << "__write T_VAR: " << std::endl;
                        addr1 = (core_p) heap.size();
                        mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (addr1 - base1)));
                        heap.push_back(ph, VAR2c<bit_type>(typemap->CoerceB2A(lilfes::c2VARS(c))));
                        break;
                    }
                    case T_INT:
                    case T_FLO:
                    {
//                        std::cout << "__write T_INTorFLO: " << std::endl;
                        addr1 = (core_p) heap.size();
                        mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (addr1 - base1)));
                        heap.push_back(ph, cell2cell<bit_type>(c));
                        break;
                    }
                    case T_CHR:
                        std::cerr << "T_CHR in LiLFeS" << std::endl; exit(0);
                    case T_STG:
                    {
//                        std::cout << "__write T_STG: " << std::endl;
                        addr1 = (core_p) heap.size();
                        mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (addr1 - base1)));
                        char *str = lilfes::c2STG(c);
                        typename cell<bit_type>::size_type len = (typename cell<bit_type>::size_type) strlen(str);
                        heap.push_back(ph, STG2c<bit_type>(len));
                    
                        typename cell<bit_type>::size_type k = 0;
                        for(; k < len ; ) {
                            cell<bit_type> ic(0);
                            for(int j = 0 ; j < cell<bit_type>::charlen ; j++, k++) {
                                ic.word = ic.word << 8;
                                if( k < len ) {
                                    unsigned char ustr = *(str++);
                                    ic.word |= (typename cell<bit_type>::word_type) ustr;
                                }
                            }
                            heap.push_back(ph, CHR2c<bit_type>(ic.word));
                        }
                        break;
                    }
                    case T_STR:
                    {
//                        std::cout << "__write T_STR: " << std::endl;
                        static std::vector< std::pair<lilfes::core_p, typename core<allocator_type>::core_p> > array(serialize_size);
                        array.clear();

                        addr1 = (core_p) heap.size();
                        array.push_back( std::make_pair(addr0,addr1) );
                        int slot = lilfes::GetSlot(base0);
                        heap.resize(ph, addr1+1);

                        while( ! array.empty() ) {
                            addr0 = array.back().first;
                            addr1 = array.back().second; 
                            array.pop_back();
					
                            c = mach->ReadHeap(addr0);
				
                            if( lilfes::IsPTR(c) ) {
//                                std::cout << "__write IsPtr: c2PTR" << lilfes::c2PTR(c) << " base0: " << base0 << std::endl;
                                heap.find(ph, addr1) = PTR2c<bit_type>(base1 + ((lilfes::c2PTR(c) - base0)));
                            } else {
                                const lilfes::type* t = c2STR(c);
                                unsigned nf = t->GetNFeatures();

                                typename core<allocator_type>::core_p hp1 = heap.size();
                                mach->WriteHeap(addr0, lilfes::PTR2c(base0 + (hp1 - base1)));
//                                std::cout << "__write writeheap0 addr0: " << addr0 << " base0+hp1-base1:" << (base0 + (hp1 - base1)) << std::endl;
                                
                                heap.find(ph, addr1) = PTR2c<bit_type>(hp1);

                                heap.resize(ph, hp1 +  2 * nf + 2);
                                heap.find(ph, hp1++) = STR2c<bit_type>(typemap->CoerceB2A(lilfes::c2STRS(c)));
                                heap.find(ph, hp1++) = NFT2c<bit_type>((typename cell<bit_type>::size_type) nf);

                                std::vector<lil_size_type> featvec;
                                std::vector<int> featorder;
                            
                                for(unsigned i = 0 ; i < nf ; ++i)
                                    featvec.push_back((lil_size_type) t->Feature(i)->GetRepType()->GetSerialNo());
                            
                                featmap->CoerceB2A((lil_size_type) t->GetSerialNo(), featvec, featorder);
                                
                                for(unsigned i = 0 ; i < nf ; ++i) {
                                    int mapped_i = featorder[i];
                                    lilfes::core_p a = addr0 + i + 1;
                                    while( lilfes::IsPTR(c = mach->ReadHeap(a)) ) {
                                        a = lilfes::c2PTR(c);
                                        if( (lilfes::GetSlot(a) == slot) && a >= base0 ) {
                                            heap.find(ph, hp1 + 2*mapped_i) = PTR2c<bit_type>(base1 + (a - base0));
                                            goto hogehoge;
                                        }
                                    }
				    
                                    if( lilfes::IsSTR(c) ) {
//                        std::cout << "__write IsSTR: " << std::endl;
                                        array.push_back( std::make_pair(a, hp1 + 2*mapped_i) );
                                    } else if( lilfes::IsSTG(c) ) {
//                        std::cout << "__write IsSTG: " << std::endl;
                                        addr1 = heap.size();
                                        mach->WriteHeap(a, lilfes::PTR2c(base0 + (addr1 - base1)));
                                        heap.find(ph, hp1+2*mapped_i) = PTR2c<bit_type>(addr1);
					
                                        char *str = lilfes::c2STG(c);
                                        typename cell<bit_type>::size_type len = (typename cell<bit_type>::size_type) strlen(str);
                                        heap.push_back(ph, STG2c<bit_type>(len));
                                        typename cell<bit_type>::size_type k = 0;
                                        for(; k < len ; ) {
                                            cell<bit_type> ic(0);
                                            for(int j = 0 ; j < cell<bit_type>::charlen ; j++, k++) {
                                                ic.word = ic.word << 8;
                                                if( k < len ) {
                                                    unsigned char ustr = *(str++);
                                                    ic.word |= (typename cell<bit_type>::word_type) ustr;
                                                }
                                            }
                                            heap.push_back(ph, CHR2c<bit_type>(ic.word));
                                        }
                                    } else if( lilfes::IsVAR(c) ) {
//                        std::cout << "__write IsVAR: " << std::endl;
                                        mach->WriteHeap(a, lilfes::PTR2c(base0 + (hp1 + 2*mapped_i - base1)));
                                        heap.find(ph, hp1 + 2*mapped_i) = VAR2c<bit_type>(typemap->CoerceB2A(lilfes::c2VARS(c)));
                                    } else {
//                        std::cout << "__write else: " << std::endl;
                                        mach->WriteHeap(a, lilfes::PTR2c(base0 + (hp1 + 2*mapped_i - base1)));
//                                        std::cout << "__write else a:" << a << " base0+hp1+2*mapped_i-base1: " << (base0 + (hp1 + 2*mapped_i - base1)) << std::endl;
                                        heap.find(ph, hp1 + 2*mapped_i) = cell2cell<bit_type>(c);
                                    }
                                  hogehoge: ;
                                }
                            }
                        }
                        break;
                    }
                    default:
                        std::cerr << "unknown tag" << std::endl; exit(0);
                }
            }
        }
        
        lilfes::FSP __read(pheap_type *ph, lilfes::machine *mach, core_p root) {
            static std::vector<char> buffer(1024);
            
            lilfes::core_p base = mach->GetHP();
            typename heap_type::iterator it = heap.begin(ph), last = heap.end(ph);
            for(; it != last ; it++) {
                cell<bit_type> c = (*it);
                switch(Tag(c)) {
                    case T_PTR:    case T_PTR1:    case T_PTR2:	case T_PTR3:
                    {
                        core_p cp = c2PTR(c);
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        mach->PutHeap(lilfes::PTR2c(cp + base));
                        break;
                    }
                    case T_VAR: case T_STR: case T_INT: case T_FLO:
                    {
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        mach->PutHeap(cell2cell(c));
                        break;
                    }
                    case T_STG:
                    {
                        typename cell<bit_type>::size_type len = c2STG(c);
                        core_p stgbase = mach->GetHP();
                        mach->CheckHeapLimit(stgbase+1);
                        mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID));

                        if(buffer.size() < (size_t)(len + 1)) {
                            buffer.resize(len + 1);
                        }
                
                        typename cell<bit_type>::size_type k = 0;
                        while(k < len) {
                            it++;
                            cell<bit_type> ic = *it;
                            mach->CheckHeapLimit(mach->GetHP()+1);
                            mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID));
                            for(int j = 0 ; j < cell<bit_type>::charlen; j++, k++) {
                                ic.word = ic.word << 8;
                                char ch = (char) (ic.word >> cell<bit_type>::char_shift);
                                if( k < len )
                                    buffer[k] = ch;
                            }
                        }
                        buffer[len] = '\0';
                        mach->WriteHeap(stgbase, lilfes::STG2c(mach, &*buffer.begin()));
                        break;
                    }
                    case T_CHR:
                        std::cerr << "error FSP::__FSP -- T_CHR" << std::endl; exit(0);
                    default:
                        std::cerr << "error FSP::__FSP -- default" << std::endl; exit(0);
                }
            }
            return lilfes::FSP(mach, base + (lilfes::core_p) root);
        }
        
        lilfes::FSP __read(pheap_type *ph, lilfes::machine *mach, core_p root, typemapper_type *typemap, featmapper_type *featmap) {

            if(!typemap) return __read(ph, mach, root);
            
            static std::vector<char> buffer(1024);
            
            lilfes::core_p base = mach->GetHP();
            int i = 0;
            typename heap_type::iterator it = heap.begin(ph), last = heap.end(ph);
            for(; it != last ;) {
                cell<bit_type> c = *(it++); i++;
                switch(Tag(c)) {
                    case T_PTR:    case T_PTR1:    case T_PTR2:	case T_PTR3:
                    {
                        core_p cp = c2PTR(c);
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        mach->PutHeap(lilfes::PTR2c(((lilfes::core_p) cp) + base));
                        break;
                    }
                    case T_VAR:
                    {
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        mach->PutHeap(lilfes::VAR2c((lilfes::tserial) typemap->CoerceA2B(c2VAR(c))));
                        break;
                    }
                    case T_STR:
                    {
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        typename cell<bit_type>::size_type t0 = typemap->CoerceA2B(c2STR(c));
                        const lilfes::type *t1 = lilfes::type::Serial((lilfes::tserial) t0);
                        int nf1 = t1->GetNFeatures();
                        mach->PutHeap(lilfes::STR2c(t1));
                        
                        typename cell<bit_type>::size_type nf0 = c2NFT(*(it++)); i++;
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID ));

                        lilfes::core_p ftrbase = mach->GetHP();
                        mach->CheckHeapLimit(ftrbase + 2* nf0);
                        std::vector<lil_size_type> featvec;
                        std::vector<int> featorder;
                        for(int j = 0 ; j < nf0 ; j++) {
                            featvec.push_back((typename cell<bit_type>::size_type) c2FTR(heap[i+2*j]));
                            mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID));
                            mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID));
                        }
                        featmap->CoerceA2B((lil_size_type) t1->GetSerialNo(), featvec, featorder);
                        for(int j = 0 ; j < nf0 ; j++) {
                            typename cell<bit_type>::fserial f = c2FTR(it++); i++;
                            cell<bit_type> c2 = *(it++); i++;
                            switch(Tag(c2)) {
                                case T_PTR:    case T_PTR1:    case T_PTR2:	case T_PTR3:
                                {
                                    mach->WriteHeap(ftrbase + featorder[j], lilfes::PTR2c(((lilfes::core_p) c2PTR(c2)) + base));
                                    break;
                                }
                                case T_VAR:
                                {
                                    mach->WriteHeap(ftrbase + featorder[j],
                                                    lilfes::VAR2c((lilfes::tserial) typemap->CoerceA2B(c2VAR(c2))));
                                    break;
                                }
                                case T_STR:
                                    std::cerr << "T_STR in T_STR" << std::endl; exit(0);
                                case T_INT: case T_FLO:
                                {
                                    mach->WriteHeap(ftrbase + featorder[j], cell2cell(c2));
                                    break;
                                }
                                case T_STG:
                                    std::cerr << "T_STG in T_STR" << std::endl; exit(0);
                                case T_CHR:
                                    std::cerr << "T_CHR in T_STR" << std::endl; exit(0);
                                default:
                                    std::cerr << "unknown tag" << std::endl; exit(0);
                            }
                        }
                        break;
                    }
                    case T_INT: case T_FLO:
                    {
                        mach->CheckHeapLimit(mach->GetHP()+1);
                        mach->PutHeap(cell2cell(c));
                        break;
                    }
                    case T_STG:
                    {
                        typename cell<bit_type>::size_type len = c2STG(c);
                        core_p stgbase = mach->GetHP();
                        mach->CheckHeapLimit(stgbase+1);
                        mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID));

                        if(buffer.size() < (size_t)(len + 1)) {
                            buffer.resize(len + 1);
                        }
                
                        typename cell<bit_type>::size_type k = 0;
                        while(k < len) {
                            ++it; ++i;
                            cell<bit_type> ic = *it;
                            mach->CheckHeapLimit(mach->GetHP()+1);
                            mach->PutHeap(lilfes::PTR2c( lilfes::CORE_P_INVALID));
                            for(int j = 0 ; j < cell<bit_type>::charlen; j++, k++) {
                                ic.word = ic.word << 8;
                                char ch = (char) (ic.word >> cell<bit_type>::char_shift);
                                if( k < len )
                                    buffer[k] = ch;
                            }
                        }
                        buffer[len] = '\0';
                        mach->WriteHeap(stgbase, lilfes::STG2c(mach, &*buffer.begin()));
                        break;
                    }
                    case T_CHR:
                        std::cerr << "error FSP::__FSP -- T_CHR" << std::endl; exit(0);
                    default:
                        std::cerr << "error FSP::__FSP -- default" << std::endl; exit(0);
                }
            }
            return lilfes::FSP(mach, base + (lilfes::core_p) root);
        }
   public:
        void write(pheap_type *ph, lilfes::machine *mach, lilfes::core_p addr0) {
            write(ph, mach, addr0, 0, 0);
        }
        void write(pheap_type *ph, lilfes::machine *mach, lilfes::core_p addr0,
                   std::vector<lilfes::core_p>& aa0, std::vector<core_p>& aa1) { // pp is the core_ps corresponding to aa
            write(ph, mach, addr0, 0, 0, aa0, aa1);
        }
        void write(pheap_type *ph, lilfes::machine *mach, lilfes::core_p addr0,
                   typemapper_type *typemap, featmapper_type *featmap) {
            mach->SetTrailPoint();
            __write(ph, mach, addr0, typemap, featmap);
            mach->TrailBack();
        }
        void write(pheap_type *ph, lilfes::machine *mach, lilfes::core_p addr0,
                   typemapper_type *typemap, featmapper_type *featmap,
                   std::vector<lilfes::core_p>& aa0, std::vector<core_p>& aa1) {
            mach->SetTrailPoint();
            lilfes::core_p base0 = mach->GetHP();
            core_p base1 = (core_p) heap.size();
            int slot = lilfes::GetSlot(base0);
            
            __write(ph, mach, addr0, typemap, featmap);
                
            typename std::vector<lilfes::core_p>::iterator it = aa0.begin(), last = aa0.end();
            for(; it != last ; it++) {
                lilfes::core_p a0 = *it;
                lilfes::cell c;
                while( lilfes::IsPTR(c = mach->ReadHeap(a0))) {
                    a0 = lilfes::c2PTR(c);
                    if( (lilfes::GetSlot(a0) == slot) && a0 >= base0 ) {
                        aa1.push_back(base1 + (a0 - base0));
                        break;
                    }
                }
            }
            mach->TrailBack();
        }
        
        lilfes::FSP read(pheap_type *ph, lilfes::machine *mach, core_p root) {
            return read(ph, mach, root, 0, 0);
        }
        lilfes::FSP read(pheap_type *ph, lilfes::machine *mach, core_p root,
                         std::vector<core_p>& aa0, std::vector<lilfes::core_p>& aa1) {
            return read(ph, mach, root, 0, 0, aa0, aa1);
        }
        lilfes::FSP read(pheap_type *ph, lilfes::machine *mach, core_p root, typemapper_type *typemap, featmapper_type *featmap) {
            return __read(ph, mach, root, typemap, featmap);
        }
        lilfes::FSP read(pheap_type *ph, lilfes::machine *mach, core_p root, typemapper_type *typemap, featmapper_type *featmap,
                         std::vector<core_p>& aa0, std::vector<lilfes::core_p>& aa1) {
            lilfes::core_p base1 = mach->GetHP();
            lilfes::FSP ret = __read(ph, mach, root, typemap, featmap);
            typename std::vector<core_p>::iterator it = aa0.begin(), last = aa0.end();
            for(; it != last ; ++it) {
                aa1.push_back(base1 + static_cast<lilfes::core_p>(*it));
            }
            return ret;
        }
                    

         friend struct hash_core_ptr;
        friend struct equal_to_core_ptr;
    };
        /// hash and equal
    struct hash_core_ptr {
        template <class _Dev, class _Bit>
        typename _Bit::size_type operator()(pstl::pheap<_Dev, _Bit> *ph, pstl::ptr<core<pstl::allocator<_Dev, _Bit> >, _Bit> cp) const {
            typename _Bit::size_type __h = 0;
            pstl::ref<core<pstl::allocator<_Dev, _Bit> >, _Dev, _Bit> c = cp(ph);
            typename core<pstl::allocator<_Dev, _Bit> >::heap_type::iterator it = c->heap.begin(ph), last = c->heap.end(ph);
            for( ; it != last ; ++it)
                __h = 5 * __h + it->word;
            return __h;
        }
        
    };
    struct equal_to_core_ptr {
        template <class _Dev, class _Bit>
        bool operator()(pstl::pheap<_Dev, _Bit> *ph, pstl::ptr<core<pstl::allocator<_Dev, _Bit> >, _Bit> cp1,
                        pstl::ptr<core<pstl::allocator<_Dev, _Bit> >, _Bit> cp2) {
            pstl::ref<core<pstl::allocator<_Dev, _Bit> >, _Dev, _Bit> c1 = cp1(ph);
            pstl::ref<core<pstl::allocator<_Dev, _Bit> >, _Dev, _Bit> c2 = cp2(ph);
            
            if(c1->size() != c2->size() ) return false;

            typename core<pstl::allocator<_Dev, _Bit> >::heap_type::iterator it = c1->heap.begin(ph), last = c1->heap.end(ph);
            typename core<pstl::allocator<_Dev, _Bit> >::heap_type::iterator it2 = c2->heap.begin(ph);
            for( ; it != last ; ++it, ++it2)
                if(*it != *it2) return false;
            return true;
            
        }
        template <class _Dev1, class _Bit1, class _Dev2, class _Bit2>
        bool operator()(pstl::pheap<_Dev1, _Bit1> *ph1, pstl::ptr<core<pstl::allocator<_Dev1, _Bit1> >, _Bit1> cp1,
                        pstl::pheap<_Dev2, _Bit2> *ph2, pstl::ptr<core<pstl::allocator<_Dev2, _Bit2> >, _Bit2> cp2) {
            pstl::ref<core<pstl::allocator<_Dev1, _Bit1> >, _Dev1, _Bit1> c1 = cp1(ph1);
            pstl::ref<core<pstl::allocator<_Dev2, _Bit2> >, _Dev2, _Bit2> c2 = cp2(ph2);
            if(c1.size() != c2.size() ) return false;

            typename core<pstl::allocator<_Dev1, _Bit1> >::heap_type::iterator it = c1->heap.begin(ph1), last = c1->heap.end(ph2);
            typename core<pstl::allocator<_Dev2, _Bit2> >::heap_type::iterator it2 = c2->heap.begin(ph2);
            for( ; it != last ; ++it, ++it2)
                if(*it != *it2) return false;
            return true;
        }
    };
}


#endif // __ldbm_core_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.15  2005/06/09 06:58:12  ninomi
 * DbListとDbHashができました。
 *
 * Revision 1.14  2005/05/23 08:57:12  ninomi
 * push_back
 * FSO = [10&X, 20&Y, 30&Z] kvpair = [(1, X), (2, Y), (3, Z)]
 * に成功
 *
 * Revision 1.13  2005/05/23 07:02:40  ninomi
 * ばぐとり中です。
 *
 * Revision 1.12  2005/05/23 03:21:53  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.11  2005/05/23 01:18:55  ninomi
 * push_backまで実装。バグがでている状態。
 *
 * Revision 1.10  2005/05/13 10:12:07  ninomi
 * データベース作成中。
 *
 * Revision 1.9  2005/04/28 10:11:32  ninomi
 * db作成中です。
 *
 * Revision 1.8  2005/04/18 05:40:59  ninomi
 * coreからcellの部分を分離しました。dbを作りはじめました。
 *
 * Revision 1.7  2005/04/12 11:02:21  ninomi
 * coreのread, writeを追加しました。
 *
 * Revision 1.6  2005/04/08 08:39:30  ninomi
 * coreに対するserialize, unserializeを書換え中
 *
 * Revision 1.5  2005/03/25 06:50:59  ninomi
 * type managerを追加しました。
 *
 */
