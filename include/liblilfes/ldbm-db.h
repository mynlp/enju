/*
 * $Id: ldbm-db.h,v 1.10 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __ldbm_db_h
#define __ldbm_db_h

#include "config_liblilfes.h"
#include "../pstl/pstl_pheap.h"
#include "../pstl/pstl_pair.h"
#include "../pstl/pstl_vector.h"
#include "../pstl/pstl_list.h"
#include "../pstl/pstl_hash_map.h"
#include "ldbm-cell.h"
#include "ldbm-type.h"
#include "ldbm-core.h"
#include "ldbm-safe-list.h"
#include "ldbm-safe-hash-map.h"
#include "builtin.h"

#include <exception>
#include <utility>
#include <vector>

namespace ldbm {

    class bad_key_value_pair : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "invalid number of keys and its mapped valued\n";
        };
    };

        /*
    template <class _Alloc> class FSObject {
    public:
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename bit_type::size_type size_type;
        typedef size_type keyid_type;
    public:
        pstl::ptr<core<allocator_type>, bit_type> fsop;
        pstl::list<keyid_type, allocator_type> keyids;
        FSObject() {} // invalid
        FSObject(pstl::ptr<allocator_type, bit_type> allocp, pstl::ptr<core<allocator_type>, bit_type> __fsop)
                : fsop(__fsop), keyids(allocp) {}
        void construct(pstl::ptr<allocator_type, bit_type> allocp, pstl::ptr<core<allocator_type>, bit_type> __fsop) {
            new ((void*) this) FSObject(allocp, __fsop);
        }
        void destruct(pheap_type *ph) {
            keyids.destruct(ph);
        }
        static pstl::ptr<FSObject, bit_type> pnew(pheap_type *ph, pstl::ptr<allocator_type, bit_type> allocp,
                                                  pstl::ptr<core<allocator_type>, bit_type> __fsop) {
            pstl::ref<allocator_type, dev_type, bit_type> alloc(ph, allocp);
            pstl::ptr<FSObject, bit_type> p = (pstl::ptr<FSObject, bit_type>) alloc->allocate(ph, sizeof(FSObject));
            p(ph)->construct(allocp, __fsop);
            return p;
        }
        bool add_key_id(pheap_type *ph, keyid_type kid) {
            keyids.push_back(ph, kid);
        }
        bool delete_key_id(pheap_type *ph, keyid_type kid) {
            typename pstl::list<keyid_type, allocator_type>::iterator it = keyids.begin(ph), last = keyids.end(ph);
            for(; it != last ; ++it )
                if(*it == kid) { keyids.erase(ph, it); break; }
        }
    };

    template <class _Alloc> class KeyObject {
    public:
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename bit_type::size_type size_type;
        typedef size_type keyid_type;
        typedef size_type fsoid_type;
        typedef core<allocator_type> core_type;
        typedef typename core<allocator_type>::core_p core_p;
    public:
        pstl::ptr<core_type, bit_type> keyp;
        fsoid_type fsoid;
        core_p cp;
        KeyObject() {}
        KeyObject(pstl::ptr<core_type, bit_type> __keyp, fsoid_type __fsoid, core_p __cp)
                : keyp(__keyp), fsoid(__fsoid), cp(__cp) {}
        
        void construct(pstl::ptr<core_type, bit_type> __keyp, fsoid_type __fsoid, core_p __cp) {
            new ((void*) this) KeyObject(__keyp, __fsoid, __cp);
        }
        void destruct(pheap_type *) {
        }
        static pstl::ptr<KeyObject, bit_type> pnew(pheap_type *ph, pstl::ptr<allocator_type, bit_type> allocp,
						   pstl::ptr<core_type, bit_type> __keyp,
						   fsoid_type __fsoid, core_p __cp) {
            pstl::ref<allocator_type, dev_type, bit_type> alloc(ph, allocp);
            pstl::ptr<KeyObject, bit_type> p = (pstl::ptr<KeyObject, bit_type>) alloc->allocate(ph, sizeof(KeyObject));
            p(ph)->construct(__keyp, __fsoid, __cp);
            return p;
        }
    };

    template <class _Alloc> class db {
    public:
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename bit_type::size_type size_type;
        typedef typename bit_type::pointer pointer;
        typedef typename cell<bit_type>::core_p core_p;
        typedef core<allocator_type> core_type;

        typedef size_type keyid_type;
        typedef size_type fsoid_type;
        typedef KeyObject<allocator_type> keyobject_type;
        typedef FSObject<allocator_type> fsobject_type;
        typedef safe_hash_map<stl::ptr<core_type, bit_type>, keyobject_type, hash_core_ptr, equal_to_core_ptr, allocator_type> keydb_type;
        typedef safe_list<fsobject_type, allocator_type> fsodb_type;

    private:
        pstl::ptr<allocator_type, bit_type> allocp;
        keydb_type keydb;
        fsodb_type fsodb;

    public:
        db() {}
        db(pheap_type *ph, pstl::ptr<db, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp)
                : allocp(__allocp), keydb(ph, ___M_this, __allocp), fsodb(ph, __allocp) {}
        ~db() {}

        void construct(pheap_type *ph, pstl::ptr<db, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp) {
            new ((void*) this) db(ph, ___M_this, __allocp);
        }
        void destruct(pheap_type* ph) {
            keydb.destruct(ph);
            fsodb.destruct(ph);
        }
        
        
        void push_front(pheap_type *ph, pstl::ptr<core_type, bit_type> f) {
            fsodb.push_front(ph, fsobject_type(allocp, f));
        }
        void push_back(pheap_type *ph, pstl::ptr<core_type, bit_type> f) {
            fsodb.push_back(ph, fsobject_type(allocp, f));
        }
        void pop_front(pheap_type *ph) {
            erase(ph, fsodb.begin(ph));
            fsodb.pop_front(ph);
        }
        void pop_back(pheap_type *ph) {
            fsodb_type::iterator it = fsodb.end(ph); --it;
            erase(ph, it);
            fsodb.pop_back(ph);
        }
        fsodb_type::iterator insert(pheap_type *ph, fsodb_type::iterator it, pstl::ptr<core_type, bit_type> f) {
            return fsodb.insert(ph, it, f);
        }
        std::pair<keydb_type::iterator, bool> insert_key(pheap_type *ph,
                                                         pstl::ptr<core_type, bit_type> k, fsoid_type fid, core_p v) {
            std::pair<typename keydb_type::iterator, bool> kib =
                keydb.insert(ph, typename keydb_type::value_type(typename keydb_type::key_type(k),
                                                                 typename keydb_type::mapped_type(k, fid, v)));
            keyid_type kid = kib.first->get_id();
            if(!kib.second) { // crash
                    // delete keyid in fsobject
                fsodb_type::iterator fitold = fsodb.find(ph, kib.first->fsoid);
                fitold->delete_key_id(ph, kid);
                    
                    // rewrite the keydata
                kib.first->fsoid = fid;
                kib.first->cp = v;
            }                    
            fsodb_type::iterator fit = fsodb.find(ph, fsoid);
            fit->add_key_id(ph, kid);
            return kib;
        }
	        
        fsodb_type::iterator find(pheap_type *ph, fsoid_type fsoid) {
            return fsodb.find(ph, fsoid);
        }
        keydb_type::iterator find_key(pheap_type *ph, keyid_type keyid) {
            return keydb.find(ph, keyid);
        }
        std::pair<fsodb_type::iterator, keydb_type::iterator> find(pheap_type *ph, ptr<core_type, bit_type> k) {
            keydb_type::iterator kit = keydb.find(ph, k);
            if(kit == keydb.end(ph)) return std::pair<fsodb_type::iterator, keydb_type::iterator>(fsodb.end(ph), keydb.end(ph));
            ref<keyobject_type, dev_type, bit_type> key = *kit;
            return std::pair<fsodb_type::iterator, keydb_type::iterator>(fsodb.find(ph, key->fsoid), kit);
        }
        void erase(pheap_type *ph, fsodb_type::iterator it0) {
            ref<fsobject_type, dev_type, bit_type> fso = *it0;
            pstl::list<keyid_type, allocator_type>::iterator it = fso->keyids.begin(ph), last = fso->keyids.end(ph);
            for(; it != last ; ++it) {
                keyid_type kid = *it;
                erase(ph, keydb.find(ph, kid));
            }
            fsodb.erase(ph, it0);
        }
        void erase(pheap_type *ph, keydb_type::iterator it) {
            keydb.erase(ph, it);
        }
        void clear(pheap_type *ph) {
            fsodb.clear(ph);
            keydb.clear(ph);
        }
        
        size_type size(pheap_type *ph) { return fsodb.size(ph); }
        size_type size_key(pheap_type *ph) { return keydb.size(ph); }
        
        fsodb_type::iterator begin(pheap_type *ph) { return fsodb.begin(ph); }
        fsodb_type::iterator end(pheap_type *ph) { return fsodb.end(ph); }
        
        bool empty(pheap_type *ph) { return (size(ph) != 0); }
        bool empty_key(pheap_type *ph) { return (size_key(ph) != 0); }
    };

    template <class _Alloc> class ldb : public db<_Alloc> {
    public:
#if SIZEOF_INTP == 4
        typedef TypeMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b32> > typemapper_type;
        typedef FeatureMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b32> > featmapper_type;
        typedef typename pstl::b32::size_type lil_size_type;
        typedef pstl::allocator<pstl::pmem, pstl::b32> lil_allocator_type;
#elif SIZEOF_INTP == 8
        typedef TypeMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b64> > typemapper_type;
        typedef FeatureMapper<TypeManager<dev_type, bit_type>, TypeManager<pstl::pmem, pstl::b64> > featmapper_type;
        typedef typename pstl::b64::size_type lil_size_type;
        typedef pstl::allocator<pstl::pmem, pstl::b64> lil_allocator_type;
#endif
        typedef core<lil_allocator_type> lil_core_type;

        pstl::ptr<lil_allocator_type, bit_type> lilallocp;
    public:
        ldb() {}
        ldb(pheap_type *ph, pstl::ptr<ldb, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp)
                : db(ph, (pstl::ptr<db, bit_type>) ___M_this, __allocp) {
            lilallocp = lil_allocaotor_type::pnew(0);
        }
        ~ldb() {}

        void construct(pheap_type *ph, pstl::ptr<ldb, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp) {
            new ((void*) this) ldb(ph, ___M_this, __allocp);
        }
        void destruct(pheap_type* ph) {
            ::destruct(ph);
        }
        
    private:
        void __insert_keys(pheap_type *ph, fsodb_type::iterator fit,
                           std::vector<pstl::ptr<core_type, bit_type> >& kk, std::vector<core_p>& vv) {
            fsoid_type fid = fit.get_id();
            typename std::vector<pstl::ptr<core_type, bit_type> >::iterator it = kk.begin(), last = kk.end();
            typename std::vector<core_p>::iterator it2 = vv.begin();
            for(; it != last ; it++, it2++) {
                pstl::ptr<core_type, bit_type> k = *it;
                core_p v = *it2;
                insert_key(ph, k, fid, v);
            }
        }
        void __FSPMap2VecMap(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                             lilfes::FSP mm, std::vector<pstl::ptr<core_type, bit_type> >& kk, std::vector<lilfes::core_p>& vv) {
            while( mm.GetType()->IsSubType(lilfes::cons) ) {
                lilfes::FSP m0 = mm.Follow(lilfes::hd);
                lilfes::FSP k0 = m0.Follow(lilfes::f_arg[1]);
                lilfes::FSP v0 = m0.Follow(lilfes::f_arg[2]);
                
                pstl::ptr<core_type, bit_type> __k = core_type::pnew(ph, allocp);
                __k(ph)->write(ph, k0.GetMachine(), k0.GetAddress(), tm, fm);
                kk.push_back(__k);
                vv.push_back(v0.GetAddress());
                mm = mm.Follow(lilfes::tl);
            }
        }
    public:
        
        void push_front(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                        lilfes::FSP f, lilfes::FSP mm) {
            std::vector<pstl::ptr<core_type, bit_type> > kk;
            std::vector<lilfes::core_p> vv;
	    __FSPMap2VecMap(ph, tm, fm, mm, kk, vv);

            std::vector<core_p> ww;
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress(), tm, fm, vv, ww);
            push_front(ph, __v);
            fsodb_type::iterator it = fsodb.begin(ph);
            __insert_keys(ph, it, kk, ww);
        }
        void push_back(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
		       lilfes::FSP f, lilfes::FSP mm) {
            std::vector<pstl::ptr<core_type, bit_type> > kk;
            std::vector<lilfes::core_p> vv;
	    __FSPMap2VecMap(ph, tm, fm, mm, kk, vv);

            std::vector<core_p> ww;
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress(), tm, fm, vv, ww);
            push_back(ph, __v);
            fsodb_type::iterator it = fsodb.end(ph); --it;
            __insert_keys(ph, it, kk, ww);
        }
        void push_front(pheap_type *ph, lilfes::FSP f, lilfes::FSP kk) { push_front(ph, 0, 0, f, kk); }
        void push_back(pheap_type *ph, lilfes::FSP f, lilfes::FSP mm) { push_back(ph, 0, 0, f, mm); }
        
        void insert(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                    fsodb_type::iterator it0, lilfes::FSP f, lilfes::FSP mm) {
            
            std::vector<pstl::ptr<core_type, bit_type> > kk;
            std::vector<lilfes::core_p> vv;
	    __FSPMap2VecMap(ph, tm, fm, mm, kk, vv);

            std::vector<core_p> ww;
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress(), tm, fm, vv, ww);
            
            fsodb_type::iterator it = insert(ph, it0, __v);
            __insert_keys(ph, it, kk, ww);
        }

        bool find(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                  lilfes::FSP k, lilfes::FSP v, lilfes::FSP f, lilfes::FSP mm) {
            
            pstl::ptr<lil_core_type, bit_type> __k = lil_core_type::pnew(ph, lilallocp);
            std::pair<fsodb_type::iterator, keydb_type::iterator> ii = find(ph, __k);
            if(ii.first == fsodb.end(ph)) return false;

            pstl::ref<fsobject_type, dev_type, bit_type> fso = *(ii.first);
            pstl::ref<keyobject_type, dev_type, bit_type> key = *(ii.second);
            
                // get key vector
            std::vector<core_p> vv;
            pstl::list<keyid_type, allocator_type>::iterator it = fso->keyids.begin(), last = fso->keyids.end();
            for(; it != last ; ++it) {
                keydb_type::iterator it2 = keydb.find(ph, (*it));
                vv.push_back(it2->cp);
            }
            vv.push_back(key->cp);
            
                // copy feature object
            std::vector<lilfes::core_p> ww;
            pstl::ref<core_type, dev_type, bit_type> core = fso->fsop(ph);
            lilfes::FSP core_fsp = core->read(ph, mach, 0, tm, fm, vv, ww);
            core_p v_cp = ww.back(); ww.pop_back();
            
                // make key FSP
            FSP key_fsp(m);
            FSP key_fsp_root(m, key_fsp.GetAddress());
            std::vector<lilfes::core_p>::iterator it3 = ww.begin(), last3 = ww.end();
            for(; it3 != last3 ; it3++) {
                key_fsp.Follow(hd).Unify(FSP(mach, *it3));
                key_fsp = key_fsp.Follow(tl);
            }
            key_fsp.Coerce(nil);
            
            if(! v.Unify(lilfes::FSP(mach, v_cp))) return false;
            if(! f.Unify(core_fsp) ) return false;
            if(! mm.Unify(key_fsp_root) ) return false;
            
            return true;
        }            
        bool find(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                  fsodb_type::iterator it0, lilfes::FSP f, lilfes::FSP mm) {

            if(it0 == fsodb.end(ph)) return false;

            pstl::ref<fsobject_type, dev_type, bit_type> fso = *(it0);
            
                // get key vector
            std::vector<core_p> vv;
            pstl::list<keyid_type, allocator_type>::iterator it = fso->keyids.begin(), last = fso->keyids.end();
            for(; it != last ; ++it) {
                keydb_type::iterator it2 = keydb.find(ph, (*it));
                vv.push_back(it2->cp);
            }
            
                // copy feature object
            std::vector<lilfes::core_p> ww;
            pstl::ref<core_type, dev_type, bit_type> core = fso->fsop(ph);
            lilfes::FSP core_fsp = core->read(ph, mach, 0, tm, fm, vv, ww);
            
                // make key FSP
            FSP key_fsp(m);
            FSP key_fsp_root(m, key_fsp.GetAddress());
            std::vector<lilfes::core_p>::iterator it3 = ww.begin(), last3 = ww.end();
            for(; it3 != last3 ; it3++) {
                key_fsp.Follow(hd).Unify(FSP(mach, *it3));
                key_fsp = key_fsp.Follow(tl);
            }
            key_fsp.Coerce(nil);
            
            if(! f.Unify(core_fsp) ) return false;
            if(! mm.Unify(key_fsp_root) ) return false;
            
            return true;
        }            
        bool erase(pheap_type *ph, lilfes::FSP key) {
            pstl::ptr<lil_core_type, bit_type> __k = lil_core_type::pnew(ph, lilallocp);
            std::pair<fsodb_type::iterator, keydb_type::iterator> ii = find(ph, __k);
            if(ii.first == fsodb.end(ph)) return false;
            erase(ph, ii.second);
            return true;
        }
    };
        */
}

#endif // __ldbm_db_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2005/06/10 02:06:14  ninomi
 * ldbm-db.hからlist, hashの部分けしました。
 *
 * Revision 1.8  2005/06/09 06:58:12  ninomi
 * DbListとDbHashができました。
 *
 * Revision 1.7  2005/05/23 08:57:12  ninomi
 * push_back
 * FSO = [10&X, 20&Y, 30&Z] kvpair = [(1, X), (2, Y), (3, Z)]
 * に成功
 *
 * Revision 1.6  2005/05/23 03:21:53  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.5  2005/05/23 01:18:55  ninomi
 * push_backまで実装。バグがでている状態。
 *
 * Revision 1.4  2005/05/18 10:29:10  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.3  2005/05/13 10:12:07  ninomi
 * データベース作成中。
 *
 * Revision 1.2  2005/04/28 10:11:33  ninomi
 * db作成中です。
 *
 * Revision 1.1  2005/04/18 05:40:59  ninomi
 * coreからcellの部分を分離しました。dbを作りはじめました。
 *
 */
