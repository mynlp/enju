/*
 * $Id: lildbm-core.h,v 1.11 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-core.h
//  Core
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_core_h
#define __lildbm_core_h

#include "lildbm-cell.h"
#include "lildbm-type.h"

#include <cstring>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace lilfes {
    namespace lildbm {
        const int PAGE_NO = 0;
        const int PREV_PAGE_NO = 1;
        const int NEXT_PAGE_NO = 2;
        const int PAGE_POINTER=3;
        const int FIRST_CELL=4;

///////////////////
/// prototype
///
        template<class C> class DCore {};
        template<class C> class Page {};

/////////////////////////
/// class Page
///

/*
  template<> class Page<bdb> {
  protected:
        std::vector<bdb::dcell> page;

    public:
        Page(bdb::dcore_p p, bdb::dcore_p prev, bdb::dcore_p next) {
            page.push_back((bdb::dcell)p); // page_no
            page.push_back((bdb::dcell)prev); // prev_page_no
            page.push_back((bdb::dcell)next); // next_page_no
            page.push_back((bdb::dcell)FIRST_CELL); // page_pointer
        };
        ~Page() {};

        bdb::dcore_p getPrevPageNo() { return (bdb::dcore_p)page[PREV_PAGE_NO]; };
        bdb::dcore_p getNextPageNo() { return (bdb::dcore_p)page[NEXT_PAGE_NO]; };
        void setPrevPageNo(bdb::dcore_p pp) { page[PREV_PAGE_NO] = (bdb::dcell) pp; };
        void setNextPageNo(bdb::dcore_p pp) { page[NEXT_PAGE_NO] = (bdb::dcell) pp; };

        bdb::pcore_p getPagePointer() { return (bdb::pcore_p) page[PAGE_POINTER]; };
        bdb::dcore_p getPageNo() { return (bdb::dcore_p) page[PAGE_NO]; };

        bool checkPageLimit(bdb::pcore_p sz) {
            return page[PAGE_POINTER] + sz < ((bdb::pcore_p) bdb::page_size);
        }
        bool push(bdb::dcell c) {
            if(! (page[PAGE_POINTER] < ((bdb::pcore_p) bdb::page_size))) return false;
            if( page[PAGE_POINTER] < page.size() ) {
                page[page[PAGE_POINTER]++] = c;
            } else {
                page.push_back(c);
                page[PAGE_POINTER]++;
            }
            return true;
        };
        bool write(bdb::pcore_p cp, bdb::dcell c) {
            if(cp < page[PAGE_POINTER]) {
                page[cp] = c;
                return true;
            } else {
                return false;
            }
        };
        bdb::dcell read(bdb::pcore_p cp) {
            if(cp < page[PAGE_POINTER]) {
                return page[cp];
            } else {
                return bdb::invalid_dcell;
            }
        };
        void initialize() {
            page[PAGE_POINTER] = FIRST_CELL;
        }
    };
*/

/////////////////////////
/// class DCore (= データベース上でのヒープ)
///
        template<> class DCore<mem> {
        protected:
            mem::dcore_p core_pointer;
            std::vector<mem::dcell> heap;
        
        public:
            DCore() : core_pointer(0) {};
            ~DCore() {};
            mem::dcore_p getCorePointer() {
                return core_pointer;
            };
            void setCorePointer(mem::dcore_p cp) {
                core_pointer = cp;
            };
            void reserve(size_t x) {
                heap.reserve(x);
            }
            size_t size() {
                return heap.size();
            }
            size_t capacity() {
                return heap.capacity();
            }
            bool clear() {
                heap.clear();
                core_pointer = 0;
                return true;
            };
            bool push(mem::dcell c) { // return page overflow flag
                heap.push_back(c);
                core_pointer++;
                return true;
            };
	    bool resize(mem::dcore_p cp) {
		if( cp < core_pointer) return true;
		heap.resize(cp);
		core_pointer = cp;
		return true;
	    }
            bool write(mem::dcore_p cp, mem::dcell c) {
                if(! (cp < core_pointer)) return false;
                heap[cp] = c;
                return true;
            };
            mem::dcell read(mem::dcore_p cp) {
                if(! (cp < core_pointer)) return mem::invalid_dcell;
                return heap[cp];
            };
            mem::dcore_p nextAddress(mem::dcore_p cp) {
                return cp + 1;
            };
	
            mem::dcore_p DDeref(mem::dcore_p dcp) {
                mem::dcell c = read(dcp);
                while(mem::IsDPTR(c)) {
                    dcp = mem::dc2PTR(c);
                    c = read(dcp);
                }
                return dcp;
            };
	
            void print() {
                for(mem::dcore_p dcp = 0 ; dcp < core_pointer ; dcp++) {
                    std::cout << "[" << dcp << "] ";
                    mem::dcell c = read(dcp);
                    switch(mem::DTag(c)) {
                        case DT_VAR:
                        {
                            std::cout << "(VAR) " << type::Serial(mem::dc2VAR(c).v)->GetName() << std::endl;
                            break;
                        }
                        case DT_INT:
                        {
                            std::cout << "(INT) " << mem::dc2INT(c) << std::endl;
                            break;
                        }
                        case DT_FLO:
                        {
                            std::cout << "(FLO) " << mem::dc2FLO(c) << std::endl;
                            break;
                        }
                        case DT_STG:
                        {
                            std::cout << "(STG) " << mem::dc2STG(c) << std::endl;
                            break;
                        }
                        case DT_STR:
                        {
                            std::cout << "(STR) " << type::Serial(mem::dc2STR(c).v)->GetName() << std::endl;
                            break;
                        }
                        case DT_PTR: case DT_PTR1: case DT_PTR2: case DT_PTR3:
                        {
                            std::cout << "(PTR) " << c << std::endl;
                            break;
                        }
                        case DT_CHR:
                        {
                            std::string str;
                            mem::dcell c2 = c;
                            for(int j = 0 ; j < mem::charlen; j++) {
                                c2 = c2 << 8;
                                char ch = mem::dc2CHR0(c2);
                                if(ch)
                                    str += ch;
                                else
                                    str += "\\0";
                            }
                            std::cout << "(CHR) " << std::hex << c << std::dec << "(" << str.c_str() << ")" << std::endl;
                            break;
                        }
                        default:
                        {
                            std::cout << "(\?\?\?) " << c << std::endl;
                        }
                    }
                }
            };

                // class method
            static void Serialize(machine* mach, core_p addr, std::vector<mem::dcell>& vec, mem::lcp2dcp_map& mapping) {

                addr = mach->Deref(addr);
                mem::lcp2dcp_map::iterator it = mapping.find(addr);
                if(it != mapping.end() ) { // already traversed
                    return;
                } else {
                    cell c = mach->ReadHeap(addr);
                    switch(Tag(c)) {
                        case T_VAR: case T_INT: case T_FLO:
                        {
                            mem::dcore_p mapped_addr = vec.size();
                            mapping.insert(mem::lcp2dcp_map::value_type(mem::lcp2dcp_map::key_type(addr),
                                                                        mem::lcp2dcp_map::mapped_type(mapped_addr)));
                            vec.push_back(mem::cell2dcell(c));
                            return;
                        }
                        case T_STG:
                        {
                            const char *str = c2STG(c);
                            int len = std::strlen(str);

                            mem::dcore_p mapped_addr = vec.size();
                            mapping.insert(mem::lcp2dcp_map::value_type(mem::lcp2dcp_map::key_type(addr),
                                                                        mem::lcp2dcp_map::mapped_type(mapped_addr)));
                            vec.push_back(mem::STG2dc(len));
		    
                            for(bool cont = true; cont ; ) {
                                mem::dcell ic = 0;
                                for(int j = 0 ; j < mem::charlen ; j++) {
                                    ic = ic << 8;
                                    if( *str == (char)NULL) cont = false; else  {
                                        unsigned char ustr = *(str++);
                                        ic |= (mem::dcell) ustr;
                                    }
                                }
                                vec.push_back(mem::CHR2dc(ic));
                            }
                            return;
                        }
                        case T_STR:
                        {
                            const type* t = c2STR(c);
                            int nf = t->GetNFeatures();
                            mem::dcore_p mapped_addr = vec.size();
                            mapping.insert(mem::lcp2dcp_map::value_type(mem::lcp2dcp_map::key_type(addr),
                                                                        mem::lcp2dcp_map::mapped_type(mapped_addr)));
                            vec.push_back(mem::cell2dcell(c));
                            for(int i = 0; i < nf ; ++i)
                                vec.push_back(mem::PTR2dc(0));
		
                            mapped_addr++;
                            for(int i = 0; i < nf ; ++i) {
                                Serialize(mach, addr+i+1, vec, mapping);
                                vec[mapped_addr] = mem::PTR2dc(mapping.find(mach->Deref(addr+i+1))->second);
                                mapped_addr++;
                            }
                            return;
                        }
                        case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3: case T_CHR:
                            ABORT("error in DCore<mem>::Serialize<mem>");

                        default:
                            ABORT("error in DCore<mem>::Serialize<mem>");
                    }
                }
            };
            static void Serialize(machine* mach, core_p addr, std::vector<mem::dcell>& vec) {
                mem::lcp2dcp_map mapping;
                Serialize(mach, addr, vec, mapping);
            };

            void readFS(machine *mach, mem::dcore_p addr, mem::dcp2lcp_map& mark) {
                addr = DDeref(addr);
                mem::dcp2lcp_map::iterator it = mark.find(addr);
                if(it != mark.end()) { // already traversed
                    return;
                } else {
                    mem::dcell c = read(addr);
                    switch(mem::DTag(c)) {
                        case DT_VAR: case DT_INT: case DT_FLO:
                        {
                            core_p mapped_addr = mach->GetHP();
                            mark.insert(mem::dcp2lcp_map::value_type(mem::dcp2lcp_map::key_type(addr),
                                                                     mem::dcp2lcp_map::mapped_type(mapped_addr)));
                            mach->CheckHeapLimit(mapped_addr);
                            mach->PutHeap(mem::dcell2cell(c));
                            return;
                        }
                        case DT_STG:
                        {
                            std::string str;
                            int len = mem::dc2STG(c);

                            core_p mapped_addr = mach->GetHP();
                            mark.insert(mem::dcp2lcp_map::value_type(mem::dcp2lcp_map::key_type(addr),
                                                                     mem::dcp2lcp_map::mapped_type(mapped_addr)));
                            mach->CheckHeapLimit(mapped_addr);

                            addr++;
                            int i = 0;
                            while(i < len) {
                                c = read(addr++);
                                for(int j = 0 ; j < mem::charlen; j++, i++) {
                                    c = c << 8;
                                    char ch = mem::dc2CHR0(c);
                                    if( i < len )
                                        str += ch;
                                }
                            }
			
                            mach->PutHeap(STG2c(mach, str.c_str()));
                            return;
                        }
                        case DT_STR:
                        {
                            tserial ts = mem::dc2STR(c).v;
                            const type *t = type::Serial(ts);
                        
                            int nf = t->GetNFeatures();
                            core_p mapped_addr = mach->GetHP();
                            mark.insert(mem::dcp2lcp_map::value_type(mem::dcp2lcp_map::key_type(addr),
                                                                     mem::dcp2lcp_map::mapped_type(mapped_addr)));
                            mach->CheckHeapLimit(mapped_addr);
                            mach->PutHeap(STR2c(ts));
                            for(int i = 0; i < nf ; ++i)
                                mach->PutHeap(PTR2c(0)); // dummy
                        
                            for(int i = 0; i < nf ; ++i)
                                readFS(mach, addr+i+1, mark);

                            for(int i = 0; i < nf ; ++i)
                                mach->WriteHeap(mapped_addr+i+1, PTR2c(mark.find(DDeref(addr+i+1))->second));
                            return;
                        
                        }
                        case DT_PTR: case DT_PTR1: case DT_PTR2: case DT_PTR3: case DT_CHR:
                            ABORT("error in LdbmManagerForLiLFeS::IHeap2Heap");
                        
                        default:
                            ABORT("error in findEntry");
                    }
                }
            };
            void readFS(machine* mach, mem::dcore_p addr) {
                mem::dcp2lcp_map mark;
                readFS(mach, addr, mark);
            };

            void writeFS(machine *mach, core_p addr, mem::lcp2dcp_map& mark) {
                addr = mach->Deref(addr);
                mem::lcp2dcp_map::iterator it = mark.find(addr);
                if(it != mark.end() ) { // already traversed
                    return;
                } else {
                    cell c = mach->ReadHeap(addr);
                    switch(Tag(c)) {
                        case T_VAR: case T_INT: case T_FLO:
                        {
                            mem::dcore_p mapped_addr = getCorePointer();
                            mark.insert(mem::lcp2dcp_map::value_type(mem::lcp2dcp_map::key_type(addr),
                                                                     mem::lcp2dcp_map::mapped_type(mapped_addr)));
                            push(mem::cell2dcell(c));
                            return;
                        }
                        case T_STG:
                        {
                            const char *str = c2STG(c);
                            int len = strlen(str);

                            mem::dcore_p mapped_addr = getCorePointer();
                            mark.insert(mem::lcp2dcp_map::value_type(mem::lcp2dcp_map::key_type(addr),
                                                                     mem::lcp2dcp_map::mapped_type(mapped_addr)));
                            push(mem::STG2dc(len));
		    
                            for(bool cont = true; cont ; ) {
                                mem::dcell ic = 0;
                                for(int j = 0 ; j < mem::charlen ; j++) {
                                    ic = ic << 8;
                                    if( *str == (char)NULL) cont = false; else  {
                                        unsigned char ustr = *(str++);
                                        ic |= (mem::dcell) ustr;
                                    }
                                }
                                push(mem::CHR2dc(ic));
                            }
                            return;
                        }
                        case T_STR:
                        {
                            const type* t = c2STR(c);
                            int nf = t->GetNFeatures();
                            mem::dcore_p mapped_addr = getCorePointer();
                            mark.insert(mem::lcp2dcp_map::value_type(mem::lcp2dcp_map::key_type(addr),
                                                                     mem::lcp2dcp_map::mapped_type(mapped_addr)));
                            push(mem::cell2dcell(c));
                            for(int i = 0; i < nf ; ++i)
                                push(mem::PTR2dc(0));
		
                            mapped_addr++;
                            for(int i = 0; i < nf ; ++i) {
                                writeFS(mach, addr+i+1, mark);
                                write(mapped_addr, mem::PTR2dc(mark.find(mach->Deref(addr+i+1))->second));
                                mapped_addr++;
                            }
                            return;
                        }
                        case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3: case T_CHR:
                            ABORT("error in DCore<mem>::copy");
                        
                        default:
                            ABORT("error in DCore<mem>::copy");
                    }
                }
            };
            void writeFS(machine *mach, core_p addr) {
                mem::lcp2dcp_map mark;
                writeFS(mach, addr, mark);
            };
        };
    
#ifdef WITH_BDBM

        template<> class DCore<bdb> {
        private:
            DbTypeMapper<bdb, mem>* type_mapper;
            DbFeatureMapper<bdb, mem>* feat_mapper;

            Db* core_class_dbp;
            Db* core_dbp;
            bdb::dcore_p core_pointer;
            typedef std::vector<bdb::dcell> Page;

            bdb::page_p cache_size;
            bdb::page_p cache_pointer;
            typedef _HASHMAP<bdb::page_p, Page*, hash<bdb::page_p>, equal_to<bdb::page_p> > PageCache;
            PageCache cache;

            bool savePage(bdb::page_p pp) {
                PageCache::iterator it = cache.find(pp);
                if( it == cache.end() ) return false;
                Page* page = it->second;
                Dbt key((void*) &pp, sizeof(bdb::page_p));
                Dbt val((void*) &(*(page->begin())), sizeof(bdb::dcell) * bdb::page_size);
	    
                if( core_dbp->put(NULL, &key, &val, 0) != 0 ) {
                    std::cout << "fail to save" << std::endl;
                }
                return true;
                    //	    return (core_dbp->put(NULL, &key, &val, 0) == 0);
            };

            bool loadPage(bdb::page_p pp, Page* page) {
                Dbt key((void*) &pp, sizeof(bdb::page_p));
                Dbt val;
                if( core_dbp->get(NULL, &key, &val, 0) != 0) return false;
                int sz = sizeof(bdb::dcell) * bdb::page_size;

                if(((int) val.get_size()) != sz) {
                    std::cerr << "inside error in lildbm-core loadPage" << std::endl;
                    return false;
                }
                bdb::dcell* dt = (bdb::dcell*) val.get_data();
                for(int i = 0 ; i < bdb::page_size ; i++)
                    (*page)[i] = dt[i];
                return true;
            };
            Page* getPage(bdb::page_p pp) {
                PageCache::iterator it = cache.find(pp);
                if( it != cache.end() ) // hit
                    return it->second;

                    // fail to hit
                Page* new_page;

                if( cache_pointer < cache_size ) {
                    new_page = new Page(bdb::page_size);
                } else { // save the first page in the cache and remove it
                    PageCache::iterator it2 = cache.begin();
                    if(! savePage(it2->first) ) return (Page*) 0;
                    new_page = it2->second;
                    cache.erase(it2);
                    cache_pointer--;
                }
                    // load the specified page to the cache
                loadPage(pp, new_page);
                cache.insert(PageCache::value_type(PageCache::key_type(pp),
                                                   PageCache::mapped_type(new_page)));
                cache_pointer++;
                return new_page;
            };

        public:
            bool syncCache() {
                bool ret = true;
                PageCache::iterator it = cache.begin(), last = cache.end();
                for(; it != last ; it++) {
                    bdb::page_p pp = it->first;
                    ret &= savePage(pp);
                }
                return ret;
            };

            bool syncCorePointer() {
                Dbt key((void*)"core_pointer", strlen("core_pointer") + 1);
                Dbt val(&core_pointer, sizeof(bdb::dcore_p));
                return (core_class_dbp->put(NULL, &key, &val, 0) == 0);
            };
            bool sync() {
                return (syncCache() && syncCorePointer());
            };

            DCore() {};
            ~DCore() {};
            bool close() {
                std::cout << "dcore close start" << std::endl;
                if(! sync() ) return false;
                std::cout << "dcore close sync: ok" << std::endl;
                PageCache::iterator it = cache.begin(), last = cache.end();
                for(; it != last ; it++)
                    delete it->second;
                std::cout << "dcore page deletion: ok" << std::endl;
                return true;
            };
            bool open(Db* cc, Db* c, uint64 cs, DbTypeMapper<bdb, mem>* tm, DbFeatureMapper<bdb, mem>* fm) {
                core_class_dbp = cc;
                core_dbp = c;

                cache_size = cs / (sizeof(bdb::dcell) * bdb::page_size);
                if( cache_size <= 0 ) {
                    std::cerr << "too small cache size: " << cache_size << ", specified size: " << cs << std::endl;
                    return false;
                }

                cache_pointer = 0;
                type_mapper = tm;
                feat_mapper = fm;

                    // resume core_pointer
                Dbt key((void*)"core_pointer", strlen("core_pointer") + 1);
                Dbt val(&core_pointer, sizeof(bdb::dcore_p));
                val.set_flags(DB_DBT_USERMEM);
                val.set_ulen(sizeof(bdb::dcore_p));
    
                if( core_class_dbp->get(NULL, &key, &val, 0) != 0) {
                        // create new core_pointer
                    core_pointer = 0;
                    Dbt val(&core_pointer, sizeof(bdb::dcore_p));
                    core_class_dbp->put(NULL, &key, &val, 0);
                }
                std::cout << "core_pointer: " << core_pointer << std::endl;

                return true;
            };
            bool clear() {

                    // set core pointer to 0
                core_pointer = 0;
                if(! syncCorePointer() ) return false;

                    // clear cache
                cache_pointer = 0;
                PageCache::iterator it = cache.begin(), last = cache.end();
                for(; it != last ; it++)
                    delete it->second;
                cache.clear();

                    // clear heap
                u_int32_t count_p;
                if( core_dbp->truncate(NULL, &count_p, 0) != 0 ) return false;

                return true;
            };

            bdb::dcore_p getCorePointer() { return core_pointer; };
            bool push(bdb::dcell dc) {
                bdb::page_p pp = bdb::getPage(core_pointer);
                bdb::pcore_p cp = bdb::getOffset(core_pointer);
                    //	    std::cout << "core push pp: " << pp << " cp: " << cp << " dc: " << dc << std::endl;
                Page* page = getPage(pp);
                if(!page) return false;
                (*page)[cp] = dc;
                core_pointer++;
                return true;
            };
            bool write(bdb::dcore_p dcp, bdb::dcell dc) {
                if( dcp >= core_pointer) return false;
                bdb::page_p pp = bdb::getPage(dcp);
                bdb::pcore_p cp = bdb::getOffset(dcp);
                Page* page = getPage(pp);
                if(!page) return false;
                (*page)[cp] = dc;
                return true;
            };
            bdb::dcell read(bdb::dcore_p dcp) {
                if( dcp >= core_pointer) return false;
                bdb::page_p pp = bdb::getPage(dcp);
                bdb::pcore_p cp = bdb::getOffset(dcp);
                Page* page = getPage(pp);
                if(!page) return false;
                return (*page)[cp];
            };
            bdb::dcore_p nextAddress(bdb::dcore_p dcp) {
                return dcp + 1;
            };
            bdb::dcore_p DDeref(bdb::dcore_p dcp) {
                bdb::dcell c = read(dcp);
                while(bdb::IsDPTR(c)) {
                    dcp = bdb::dc2PTR(c);
                    c = read(dcp);
                }
                return dcp;
            };

            static void markStructureSharing(machine* mach, core_p cp, core_p_mark& mark, core_p_mark &ss) {
                std::vector<core_p> queue;
                queue.push_back(cp);

                while(! queue.empty() ) {
                    core_p cp = mach->Deref(queue.back());
                    queue.pop_back();

                    core_p_mark::iterator it = mark.find(cp);
        
                    if( it != mark.end() ) { // structure sharing
                        ss.insert(core_p_mark::value_type(core_p_mark::key_type(cp),
                                                          core_p_mark::mapped_type(0)));
                    } else {
                        mark.insert(core_p_mark::value_type(core_p_mark::key_type(cp),
                                                            core_p_mark::mapped_type(0)));
                        cell c = mach->ReadHeap(cp);
                        switch(Tag(c)) {
                            case T_VAR: case T_INT:  case T_FLO: case T_STG:
                                break;
                    
                            case T_STR:
                            {
                                const type* t = c2STR(c);
                                int nf = t->GetNFeatures();
                                for(int i = 0; i < nf ; ++i)
                                    queue.push_back(cp + i + 1);
                                break;
                            }
                    
                            case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3: case T_CHR:
                                ABORT("DCore<bdb>::markStructureSharing");
                    
                            default:
                                ABORT("DCore<bdb>::markStructurSharing");
                        }
                    }
                }
            };
	
            static bool markDefault(machine* mach, core_p cp,
                                    core_p_mark &mark, core_p_mark &ss, core_p_mark &nfhash,
                                    const type* default_type) {

                cp = mach->Deref(cp);
    
                if( mark.find(cp) != mark.end() ) { // this has been serialized -- structure sharing
                    return true;
                }
    
                mark.insert(core_p_mark::value_type(core_p_mark::key_type(cp),
                                                    core_p_mark::mapped_type(0)));
                cell c = mach->ReadHeap(cp);

                switch(Tag(c)) {
                    case T_VAR:
                    {
                        const type *t = c2VAR(c);
                        if( t == default_type  && ss.find(cp) == ss.end() ) {
                                //		std::cout << "T_VAR: default" << std::endl;
                            return false;
                        } else {
                            return true;
                        }
                    }

                    case T_INT:  case T_FLO: case T_STG:
                        return true;
                    
                    case T_STR:
                    {
                        const type* t = c2STR(c);
                        int nf = t->GetNFeatures();
                        int nf2 = 0;
                
                        for(int i = 0; i < nf ; ++i) {
                            if( markDefault(mach, cp + i + 1,  mark, ss, nfhash, t->GetAppType(i)) )
                                nf2++; 
                        }
                        nfhash.insert(core_p_mark::value_type(core_p_mark::key_type(cp),
                                                              core_p_mark::mapped_type(nf2)));
                        if( nf2 == 0 && t == default_type && ss.find(cp) == ss.end() ) {
                                //                std::cout << "T_STR: default" << std::endl;
                            return false;
                        } else {
                            return true;
                        }
                    }
        
                    case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3: case T_CHR:
                        ABORT("DCore<bdb>::markDefault");
                    
                    default:
                        ABORT("DCore<bdb>::markDefault");
                }
            };

        public:
            static bool Serialize(machine* mach, DbTypeMapper<bdb, mem>* type_mapper, DbFeatureMapper<bdb, mem>* feat_mapper, core_p addr, std::vector<bdb::dcell>& vec, bdb::lcp2dcp_map& mapping, core_p_mark& ss, core_p_mark &nfhash, const type* default_type) {

                addr = mach->Deref(addr);
                bdb::lcp2dcp_map::iterator it = mapping.find(addr);
                if(it != mapping.end() ) { // already traversed
                    return true;
                } else {
                    cell c = mach->ReadHeap(addr);
                    switch(Tag(c)) {
                        case T_VAR:
                        {
                            const type* t = c2VAR(c);
                            if( t == default_type && ss.find(addr) == ss.end() ) {
                                return false;
                            } else {
                                bdb::dcore_p mapped_addr = vec.size();
                                mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                            bdb::lcp2dcp_map::mapped_type(mapped_addr)));

                                serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
                                serial<dtype<bdb> > bdb_sn = type_mapper->CoerceB2A(mem_sn).second;

                                vec.push_back(bdb::VAR2dc(bdb_sn));
                                return true;
                            }
                        }
                        case T_INT: case T_FLO:
                        {
                            bdb::dcore_p mapped_addr = vec.size();
                            mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                        bdb::lcp2dcp_map::mapped_type(mapped_addr)));
                            vec.push_back(bdb::cell2dcell(c));
                            return true;
                        }
                        case T_STG:
                        {
                            char *str = c2STG(c);
                            int len = strlen(str);

                            bdb::dcore_p mapped_addr = vec.size();
                            mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                        bdb::lcp2dcp_map::mapped_type(mapped_addr)));
                            vec.push_back(bdb::STG2dc(len));
		    
                            for(bool cont = true; cont ; ) {
                                bdb::dcell ic = 0;
                                for(int j = 0 ; j < bdb::dcharlen ; j++) {
                                    ic = ic << 8;
                                    if( *str == (char)NULL) cont = false; else {
                                        unsigned char ustr = *(str++);
                                        ic |= (bdb::dcell) ustr;
                                    }
                                }
                                vec.push_back(bdb::CHR2dc(ic));
                            }
                            return true;
                        }
                        case T_STR:
                        {
                            const type* t = c2STR(c);
                            int nf = t->GetNFeatures();
                            int nf2 = nfhash.find(addr)->second;

                            if( nf2 != 0 ) { // STR

                                bdb::dcore_p mapped_addr = vec.size();
                                mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                            bdb::lcp2dcp_map::mapped_type(mapped_addr)));

                                serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
                                serial<dtype<bdb> > bdb_sn = type_mapper->CoerceB2A(mem_sn).second;
                                vec.push_back(bdb::STR2dc(bdb_sn));
                                vec.push_back(bdb::NFT2dc(nf2));
			
                                mapped_addr++; mapped_addr++;

                                for(int i = 0; i < nf2 ; ++i) {
                                    vec.push_back(bdb::FTR2dc(serial<dtype<bdb> >(0)));
                                    vec.push_back(bdb::PTR2dc(0));
                                }

                                std::vector<serial<dtype<mem> > > featvec;
                                std::vector<int> featorder;
                                for(int i = 0 ; i < nf ; ++i) {
                                    featvec.push_back(t->Feature(i)->GetRepType()->GetSerialNo());
                                }
                                feat_mapper->CoerceB2A(mem_sn, featvec, featorder);

                                for(int i = 0; i < nf ; ++i) {
                                    int mapped_i = featorder[i];
                                    ASSERT( mapped_i != -1 );
                                    if(Serialize(mach, type_mapper, feat_mapper, addr+i+1, vec,
                                                 mapping, ss, nfhash, t->GetAppType(i)) ) { // not default
                                        serial<dtype<mem> > feat_sn =
                                            (serial<dtype<mem> >) t->Feature(i)->GetRepType()->GetSerialNo();
                                        vec[mapped_addr+mapped_i*2] =
                                            bdb::FTR2dc(type_mapper->CoerceB2A(feat_sn).second);
                                        vec[mapped_addr+mapped_i*2+1] =
                                            bdb::PTR2dc(mapping.find(mach->Deref(addr+i+1))->second);
                                    }
                                }
                                return true;
                            } else { // VAR or default
                                if( t == default_type && ss.find(addr) == ss.end() ) { // default
                                    return false;
                                } else { //VAR
                                    serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
                                    serial<dtype<bdb> > bdb_sn = type_mapper->CoerceB2A(mem_sn).second;
                                    vec.push_back(bdb::VAR2dc(bdb_sn));
                                    return true;
                                }
                            }
                        }
                        case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3: case T_CHR:
                            ABORT("error in Serialize<bdb>");

                        default:
                            ABORT("error in Serialize<bdb>");
                    }
                }
            };
            static void Serialize(machine* mach, DbTypeMapper<bdb, mem>* type_mapper, DbFeatureMapper<bdb, mem>* feat_mapper, core_p addr, std::vector<bdb::dcell>& vec) {
                core_p_mark mark;
                core_p_mark ss;
                core_p_mark nfhash;

                markStructureSharing(mach, addr, mark, ss);
                mark.clear();
                markDefault(mach, addr, mark, ss, nfhash, (const type*) NULL);
                mark.clear();

                bdb::lcp2dcp_map mapping;
                Serialize(mach, type_mapper, feat_mapper, addr, vec,
                          mapping, ss, nfhash, (const type*) NULL);
            };
            void readFS(machine *mach, bdb::dcore_p addr, bdb::dcp2lcp_map& mark) {
                addr = DDeref(addr);
                bdb::dcp2lcp_map::iterator it = mark.find(addr);
                if(it != mark.end()) { // already traversed
                    return;
                } else {
                    bdb::dcell c = read(addr);
                    switch(bdb::DTag(c)) {
                        case DT_VAR:
                        {
                            core_p mapped_addr = mach->GetHP();
                            mark.insert(bdb::dcp2lcp_map::value_type(bdb::dcp2lcp_map::key_type(addr),
                                                                     bdb::dcp2lcp_map::mapped_type(mapped_addr)));
                            serial<dtype<bdb> > bdb_sn = bdb::dc2VAR(c);
                            std::pair<bool, serial<dtype<mem> > > x = type_mapper->A2B(bdb_sn);
                            mach->CheckHeapLimit(mapped_addr);
                            if(x.first) {
                                mach->PutHeap(VAR2c(type::Serial(x.second.v)));
                                return;
                            } else {
                                RUNWARN("a type is not defined -- returned feature structure is truncated");
                                mach->PutHeap(VAR2c(bot));
                                return;
                            }
                        }
                        case DT_INT: case DT_FLO:
                        {
                            core_p mapped_addr = mach->GetHP();
                            mark.insert(bdb::dcp2lcp_map::value_type(bdb::dcp2lcp_map::key_type(addr),
                                                                     bdb::dcp2lcp_map::mapped_type(mapped_addr)));
                            mach->CheckHeapLimit(mapped_addr);
                            mach->PutHeap(bdb::dcell2cell(c));
                            return;
                        }
                        case DT_STG:
                        {
                            std::string str;
                            int len = bdb::dc2STG(c);

                            core_p mapped_addr = mach->GetHP();
                            mark.insert(bdb::dcp2lcp_map::value_type(bdb::dcp2lcp_map::key_type(addr),
                                                                     bdb::dcp2lcp_map::mapped_type(mapped_addr)));
                            mach->CheckHeapLimit(mapped_addr);

                            addr++;
                            int i = 0;
                            while(i < len) {
                                c = read(addr++);
                                for(int j = 0 ; j < bdb::dcharlen; j++, i++) {
                                    c = c << 8;
                                    char ch = bdb::dc2CHR0(c);
                                    if( i < len )
                                        str += ch;
                                }
                            }
			
                            mach->PutHeap(STG2c(mach, str.c_str()));
                            return;
                        }
                        case DT_STR:
                        {
                            core_p mapped_addr = mach->GetHP();
                            mark.insert(bdb::dcp2lcp_map::value_type(bdb::dcp2lcp_map::key_type(addr),
                                                                     bdb::dcp2lcp_map::mapped_type(mapped_addr)));
                            mach->CheckHeapLimit(mapped_addr);

                            serial<dtype<bdb> > bdb_sn = bdb::dc2VAR(c);
                            std::pair<bool, serial<dtype<mem> > > x = type_mapper->A2B(bdb_sn);
                            if(!x.first) {
                                RUNWARN("a type is not defined -- returned feature structure is truncated");
                                mach->PutHeap(VAR2c(bot));
                                return;
                            }

                            const type *t = type::Serial(x.second.v);
                            mach->PutHeap(STR2c(t));
                            int nf = t->GetNFeatures();
                            int dnf = bdb::dc2NFT(read(addr+1));

                                //			std::vector<serial<dtype<bdb> > > featvec;
                                //			for(int i = 0 ; i < dnf ; i++) {
                                //			    serial<dtype<bdb> > feat_sn = bdb::dc2FTR(read(addr+2+2*i));
                                //			    featvec.push_back(feat_sn);
                                //			}
                                //
                                //			std::vector<int> fovec; 
                                //			if(! feat_mapper->CoerceA2B(bdb_sn, featvec, fovec) ) {
                                //			    RUNERR("internal error in Serialize");
                                //			    return;
                                //			}
			
                            for(int i = 0; i < nf ; ++i)
                                mach->PutHeap(VAR2c(bot));
                        
                            for(int i = 0; i < dnf ; ++i) {
                                serial<dtype<bdb> > feat_sn = bdb::dc2FTR(read(addr + 2 + 2*i));
                                std::pair<bool, serial<dtype<mem> > > x = type_mapper->A2B(feat_sn);
                                if(!x.first) {
                                    RUNWARN("a feature is not defined -- returned feature structure is truncated");
                                    continue;
                                }
                                int mapped_i = t->GetIndex(feature::Search((tserial) x.second.v));
                                if( mapped_i == -1 ) {
                                    RUNWARN("a feature is not defined -- returned feature structure is truncated");
                                    continue;
                                }
                                readFS(mach, addr+2+2*i+1, mark);
                                mach->WriteHeap(mapped_addr+mapped_i+1,
                                                PTR2c(mark.find(DDeref(addr+2+2*i+1))->second));
                            }
                            return;
                        }
                        case DT_PTR: case DT_PTR1: case DT_PTR2: case DT_PTR3: case DT_CHR:
                            ABORT("error in DCore<bdb>::readFS");
                        
                        default:
                            ABORT("error in DCore<bdb>::readFS");
                    }
                }
            };
            void readFS(machine *mach, bdb::dcore_p addr) {
                bdb::dcp2lcp_map mark;
                readFS(mach, addr, mark);
            };

            bool writeFS(machine *mach, core_p addr, bdb::lcp2dcp_map& mapping, core_p_mark& ss, core_p_mark &nfhash, const type* default_type) {
                addr = mach->Deref(addr);
                bdb::lcp2dcp_map::iterator it = mapping.find(addr);
                if(it != mapping.end() ) { // already traversed
                    return true;
                } else {
                    cell c = mach->ReadHeap(addr);
                    switch(Tag(c)) {
                        case T_VAR:
                        {
                            const type* t = c2VAR(c);
                            if( t == default_type && ss.find(addr) == ss.end() ) {
                                return false;
                            } else {
                                bdb::dcore_p mapped_addr = getCorePointer();
                                mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                            bdb::lcp2dcp_map::mapped_type(mapped_addr)));

                                serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
                                serial<dtype<bdb> > bdb_sn = type_mapper->CoerceB2A(mem_sn).second;

                                push(bdb::VAR2dc(bdb_sn));
                                return true;
                            }
                        }
                        case T_INT: case T_FLO:
                        {
                            bdb::dcore_p mapped_addr = getCorePointer();
                            mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                        bdb::lcp2dcp_map::mapped_type(mapped_addr)));
                            push(bdb::cell2dcell(c));
                            return true;
                        }
                        case T_STG:
                        {
                            char *str = c2STG(c);
                            int len = strlen(str);

                            bdb::dcore_p mapped_addr = getCorePointer();
                            mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                        bdb::lcp2dcp_map::mapped_type(mapped_addr)));
                            push(bdb::STG2dc(len));
		    
                            for(bool cont = true; cont ; ) {
                                bdb::dcell ic = 0;
                                for(int j = 0 ; j < bdb::dcharlen ; j++) {
                                    ic = ic << 8;
                                    if( *str == (char)NULL) cont = false; else  {
                                        unsigned char ustr = *(str++);
                                        ic |= (bdb::dcell) ustr;
                                    }
                                }
                                push(bdb::CHR2dc(ic));
                            }
                            return true;
                        }
                        case T_STR:
                        {
                            const type* t = c2STR(c);
                            int nf = t->GetNFeatures();
                            int nf2 = nfhash.find(addr)->second;

                            if( nf2 != 0 ) { // STR

                                bdb::dcore_p mapped_addr = getCorePointer();
                                mapping.insert(bdb::lcp2dcp_map::value_type(bdb::lcp2dcp_map::key_type(addr),
                                                                            bdb::lcp2dcp_map::mapped_type(mapped_addr)));

                                serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
                                serial<dtype<bdb> > bdb_sn = type_mapper->CoerceB2A(mem_sn).second;
                                push(bdb::STR2dc(bdb_sn));
                                push(bdb::NFT2dc(nf2));
			
                                mapped_addr++; mapped_addr++;

                                for(int i = 0; i < nf2 ; ++i) {
                                    push(bdb::FTR2dc(serial<dtype<bdb> >(0)));
                                    push(bdb::PTR2dc(0));
                                }

                                std::vector<serial<dtype<mem> > > featvec;
                                std::vector<int> featorder;
                                for(int i = 0 ; i < nf ; ++i) {
                                    featvec.push_back(t->Feature(i)->GetRepType()->GetSerialNo());
                                }
                                feat_mapper->CoerceB2A(mem_sn, featvec, featorder);
			    
                                for(int i = 0; i < nf ; ++i) {
                                    int mapped_i = featorder[i];
                                    ASSERT( mapped_i != -1 );
                                    if(writeFS(mach, addr+i+1, mapping, ss, nfhash, t->GetAppType(i)) ) { // not default
				    
                                        serial<dtype<mem> > feat_sn =
                                            (serial<dtype<mem> >) t->Feature(i)->GetRepType()->GetSerialNo();
                                        write(mapped_addr+mapped_i*2,
                                              bdb::FTR2dc(type_mapper->CoerceB2A(feat_sn).second));
                                        write(mapped_addr+mapped_i*2+1,
                                              bdb::PTR2dc(mapping.find(mach->Deref(addr+i+1))->second));
                                    }
                                }
                                return true;
                            } else { // VAR or default
                                if( t == default_type && ss.find(addr) == ss.end() ) { // default
                                    return false;
                                } else { //VAR
                                    serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
                                    serial<dtype<bdb> > bdb_sn = type_mapper->CoerceB2A(mem_sn).second;
                                    push(bdb::VAR2dc(bdb_sn));
                                    return true;
                                }
                            }
                        }
                        case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3: case T_CHR:
                            ABORT("error in DCore<bdb>::writeFS");

                        default:
                            ABORT("error in DCore<bdb>::writeFS");
                    }
                }
            };

            void print() {
                for(bdb::dcore_p dcp = 0 ; dcp < core_pointer ; dcp++) {
                    std::cout << "[" << dcp << "] ";
                    bdb::dcell c = read(dcp);
                    switch(bdb::DTag(c)) {
                        case DT_VAR:
                        {
                            std::cout << "(VAR) " << bdb::dc2VAR(c).v << std::endl;
                            break;
                        }
                        case DT_INT:
                        {
                            std::cout << "(INT) " << bdb::dc2INT(c) << std::endl;
                            break;
                        }
                        case DT_FLO:
                        {
                            std::cout << "(FLO) " << bdb::dc2FLO(c) << std::endl;
                            break;
                        }
                        case DT_STG:
                        {
                            std::cout << "(STG) " << bdb::dc2STG(c) << std::endl;
                            break;
                        }
                        case DT_STR:
                        {
                            std::cout << "(STR) " << bdb::dc2STR(c).v << std::endl;
                            break;
                        }
                        case DT_PTR: case DT_PTR1: case DT_PTR2: case DT_PTR3:
                        {
                            std::cout << "(PTR) " << c << std::endl;
                            break;
                        }
                        case DT_CHR:
                        {
                            std::string str;
                            bdb::dcell c2 = c;
                            for(int j = 0 ; j < bdb::dcharlen; j++) {
                                c2 = c2 << 8;
                                char ch = bdb::dc2CHR0(c2);
                                if(ch)
                                    str += ch;
                                else
                                    str += "\\0";
                            }
                            std::cout << "(CHR) " << std::hex << c << std::dec << "(" << str.c_str() << ")" << std::endl;
                            break;
                        }
                        case DT_NFT:
                        {
                            std::cout << "(NFT) " << bdb::dc2NFT(c) << std::endl;
                            break;
                        }
                        case DT_FTR:
                        {
                            std::cout << "(FTR) " << bdb::dc2FTR(c).v << std::endl;
                            break;
                        }
                        default:
                        {
                            std::cout << "(\?\?\?) " << c << std::endl;
                        }
                    }
                }
            };

        };

#else // WITH_BDBM

        template<> class DCore<bdb> {
        public:
            DCore() {};
            ~DCore() {};
            bdb::dcore_p getCorePointer() { return bdb::DCoreP(0, 0); };
            bool push(bdb::dcell) { return false; };
            bool write(bdb::dcore_p, bdb::dcell) { return false; };
            bdb::dcell read(bdb::dcore_p) { return (bdb::dcell) 0; };
            bdb::dcore_p nextAddress(bdb::dcore_p) { return 0; };

            void readFS(machine *, bdb::dcore_p, bdb::dcp2lcp_map&) { return; };
            void readFS(machine *, bdb::dcore_p) { return; };
            bool writeFS(machine *, core_p, bdb::lcp2dcp_map&, core_p_mark&, core_p_mark &, const type*) { return false; };
            void print() { return; };
            bdb::dcore_p DDeref(bdb::dcore_p) { return (bdb::dcore_p) 0; };
        
            bool syncCache() { return false; };
            bool syncCorePointer() { return false; };
            bool sync() { return false; };
            bool close() { return false; };
            bool clear() { return false; };
            static void markStructureSharing(machine*, core_p, core_p_mark&, core_p_mark &) { return; };
            static bool markDefault(machine*, core_p, core_p_mark &, core_p_mark &, core_p_mark &, const type*) { return false; };
            static bool Serialize(machine*, DbTypeMapper<bdb, mem>*, DbFeatureMapper<bdb, mem>*, core_p, std::vector<bdb::dcell>&, bdb::lcp2dcp_map&, core_p_mark&, core_p_mark &, const type*) { return false; };
            static void Serialize(machine*, DbTypeMapper<bdb, mem>*, DbFeatureMapper<bdb, mem>*, core_p, std::vector<bdb::dcell>&) { return; };
        };
#endif // WIDH_BDBM

/////////////////////////
/// class DCP (DCorePointer)
///

        template<class C> class DCP {
        protected:
            DCore<C>* core;
            typename C::dcore_p addr;
        public:
            DCP(DCore<C>* c, typename C::dcore_p p) : core(c), addr(p) {};
            ~DCP() {};
            void inc() { core->NextAddress(addr); };
            typename C::dcore_p getAddress() {return addr; };
        };
    } // end of namespace lildbm
} // end of namespace lilfes

#endif // lildbm_core_h
// end of lildbm-core.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2005/05/18 10:29:10  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.9  2004/08/24 10:20:15  ninomi
 * DBにstringとして日本語(charで負の数)をいれられないばぐをとりました。
 *
 * Revision 1.8  2004/06/01 15:24:55  ninomi
 * db_reserve, db_size, db_capacity
 * db_reserve_k, db_size_k, db_capacity_k
 * db_reserve_m, db_size_m, db_capacity_mを加えました。
 *
 * Revision 1.7  2004/05/27 06:58:58  ninomi
 * db_loadのresizeバージョンです。
 *
 * Revision 1.6  2004/05/22 22:43:59  ninomi
 * lildbmのmem版のcoreへの書き込みの高速化＆メモリ節約化
 * lildbmのmem版のcoreへのsaveのアドレス消去
 *
 * Revision 1.5  2004/05/07 15:47:05  ninomi
 * FSPのserialize, unserializeの際にstringの中身をvector<cell>に書き込むようにした
 *
 * Revision 1.4  2004/04/23 08:28:39  yusuke
 *
 * std:: をつけました。
 *
 * Revision 1.3  2004/04/20 10:22:35  ninomi
 * save, load機能追加中
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
