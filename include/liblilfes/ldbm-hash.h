/*
 * $Id: ldbm-hash.h,v 1.2 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __ldbm_hash_h
#define __ldbm_hash_h

#include "config_liblilfes.h"
#include "../pstl/pstl_pheap.h"
#include "../pstl/pstl_pair.h"
#include "../pstl/pstl_vector.h"
#include "../pstl/pstl_list.h"
#include "../pstl/pstl_hash_map.h"
#include "ldbm-cell.h"
#include "ldbm-type.h"
#include "ldbm-core.h"
#include "builtin.h"

#include <exception>
#include <utility>
#include <vector>

namespace ldbm {
//////////////////////////////////////
/////
///// Safe Hash
/////

    template <class _Tp, class _Alloc>
    class safe_hash_map_node {
    public:
        typedef _Tp value_type;
        typedef _Alloc allocator_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::size_type id_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        
        id_type _M_id;
        value_type _M_data;
    public:
        safe_hash_map_node() {}
        safe_hash_map_node(id_type __x, value_type __y) : _M_id(__x), _M_data(__y) {}
        safe_hash_map_node(const safe_hash_map_node& __x) : _M_id(__x._M_id), _M_data(__x._M_data) {}
        ~safe_hash_map_node() {}

        id_type get_M_id() { return _M_id; }
        pstl::ptr<value_type, bit_type> get_M_data_pointer(pstl::ptr<safe_hash_map_node, bit_type> _M_this) {
            pstl::ptr_int _M_data_addr = (pstl::ptr_int) (&_M_data);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ptr<value_type, bit_type>(_M_this.getAddress() + (_M_data_addr - this_addr));
        }
        pstl::ref<value_type, dev_type, bit_type> get_M_data(pheap_type *ph, pstl::ptr<safe_hash_map_node, bit_type> _M_this) {
            pstl::ptr_int _M_data_addr = (pstl::ptr_int) (&_M_data);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ref<value_type, dev_type, bit_type>(ph, _M_this.getAddress() + (_M_data_addr - this_addr));
        }
    };

    template <class _Key, class _Tp, class _HashFcn, class _EqualKey, class _Alloc>
    class safe_hash_map_iterator {
    public:
        typedef _Key key_type;
        typedef _Tp value_type;
        typedef _HashFcn hasher;
        typedef _EqualKey key_equal;
        typedef _Alloc allocator_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::size_type id_type;
        typedef typename allocator_type::size_type size_type;
        typedef safe_hash_map_node<value_type, allocator_type> node_type;
        typedef pstl::hash_map<key_type, node_type, hasher, key_equal, allocator_type> hash_map_type;
        typedef safe_hash_map_iterator iterator;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;

        typename hash_map_type::iterator _M_it;
    public:
        safe_hash_map_iterator(typename hash_map_type::iterator it) : _M_it(it) {}
        safe_hash_map_iterator() {}
        iterator operator++() { ++_M_it; }
        iterator operator--() { --_M_it; }
        pstl::ref<value_type, dev_type, bit_type> operator*() {
            pstl::ptr<node_type, bit_type> p = _M_it.get_ptr();
            return p(_M_it.get_pheap())->get_M_data(p);
        }
        pstl::ref<value_type, dev_type, bit_type> operator->() {
            pstl::ptr<node_type, bit_type> p = _M_it.get_ptr();
            return p(_M_it.get_pheap())->get_M_data(p);
        }
        bool operator==(iterator& __it) { return _M_it == __it.get_it(); }
        bool operator!=(iterator& __it) { return _M_it != __it.get_it(); }
        pstl::ptr<value_type, bit_type> get_ptr() {
            pstl::ptr<node_type, bit_type> p = _M_it.get_ptr();
            return p(_M_it.get_pheap())->get_M_data_pointer(p);
        }
        operator pstl::ptr<value_type, bit_type>() {
            pstl::ptr<node_type, bit_type> p = _M_it.get_ptr();
            return p(_M_it.get_pheap())->get_M_data_pointer(p);
        }
        typename hash_map_type::iterator get_it() { return _M_it; }
        size_type get_id() {
            pstl::ptr<node_type, bit_type> p = _M_it.get_ptr();
            return p(_M_it.get_pheap())->get_M_id();
        }
        pheap_type* get_pheap() { return _M_it.get_pheap(); }
    };
    
    template <class _Key, class _Tp, class _HashFcn, class _EqualKey, class _Alloc>
    class safe_hash_map {
    public:
        typedef _Key key_type;
        typedef _Tp mapped_type;
        typedef std::pair<key_type, mapped_type> value_type;
        typedef _HashFcn hasher;
        typedef _EqualKey key_equal;
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef typename allocator_type::size_type size_type;
        typedef typename allocator_type::size_type id_type;
        typedef safe_hash_map_node<value_type, allocator_type> node_type;
        
        typedef pstl::hash_map<key_type, node_type, hasher, key_equal, allocator_type> hash_map_type;
        typedef pstl::_Hashtable_node<node_type, allocator_type> hash_map_node_type;
        typedef safe_hash_map_iterator<key_type, mapped_type, hasher, key_equal, allocator_type> iterator;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;

    private:
        pstl::ptr<safe_hash_map, bit_type> _M_this;
        pstl::ptr<allocator_type, bit_type> _M_node_allocator;
        pstl::vector<pstl::ptr<hash_map_node_type, bit_type>, allocator_type> _M_id_assoc;
        hash_map_type _M_hm;
        
        pstl::ptr<hash_map_type, bit_type> get_M_hm_pointer(pstl::ptr<safe_hash_map, bit_type> _M_this) {
            pstl::ptr_int _M_hm_addr = (pstl::ptr_int) (&_M_hm);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ptr<hash_map_type, bit_type>(_M_this.getAddress() + (_M_hm_addr - this_addr));
        }
        pstl::ref<hash_map_type, dev_type, bit_type> get_M_hm(pheap_type *ph, pstl::ptr<safe_hash_map, bit_type> _M_this) {
            pstl::ptr_int _M_hm_addr = (pstl::ptr_int) (&_M_hm);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ref<hash_map_type, dev_type, bit_type>(ph, _M_this.getAddress() + (_M_hm_addr - this_addr));
        }

    public:
        safe_hash_map() {} // invalid
        safe_hash_map(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __a)
                : _M_this(___M_this), _M_node_allocator(__a), _M_id_assoc(__a),
                  _M_hm(ph, get_M_hm_pointer(_M_this), __a) {}
        
        explicit safe_hash_map(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> ___M_this, size_type __n,
                               pstl::ptr<allocator_type, bit_type> __a)
                : _M_this(___M_this), _M_node_allocator(__a), _M_id_assoc(__a),
                  _M_hm(ph, get_M_hm_pointer(_M_this), __n, hasher(), key_equal(), __a) {}
        
        safe_hash_map(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> ___M_this, size_type __n, const hasher& __hf,
                      pstl::ptr<allocator_type, bit_type> __a)
                : _M_this(___M_this), _M_node_allocator(__a), _M_id_assoc(__a),
                  _M_hm(ph, get_M_hm_pointer(_M_this), __n, __hf, key_equal(), __a) {}

        safe_hash_map(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> ___M_this, size_type __n, const hasher& __hf,
                      const key_equal& __eql, pstl::ptr<allocator_type, bit_type> __a)
                : _M_this(___M_this), _M_node_allocator(__a), _M_id_assoc(__a),
                  _M_hm(ph, get_M_hm_pointer(_M_this), __n, __hf, __eql, __a) {}

        ~safe_hash_map() {}
        
	size_type size(pheap_type*) { return _M_hm.size(); }
	size_type get_sn_count(pheap_type*) { return _M_id_assoc.size(); }
        bool empty(pheap_type*) { return (_M_hm.size() == 0); }

        void construct() {
            new ((void*) this) safe_hash_map();
        }
        void construct(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> _M_this, pstl::ptr<allocator_type, bit_type> __a) {
            new ((void*) this) safe_hash_map(ph, _M_this, __a);
        }
        void construct(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> _M_this, size_type __n,
                       pstl::ptr<allocator_type, bit_type> __a) {
            new ((void*) this) safe_hash_map(ph, _M_this, __n, __a);
        }
        void construct(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> _M_this, size_type __n, const hasher& __hf,
                       pstl::ptr<allocator_type, bit_type> __a) {
            new ((void*) this) safe_hash_map(ph, _M_this, __n, __hf, __a);
        }
        void construct(pheap_type* ph, pstl::ptr<safe_hash_map, bit_type> _M_this, size_type __n, const hasher& __hf,
                      const key_equal& __eql, pstl::ptr<allocator_type, bit_type> __a) {
            new ((void*) this) safe_hash_map(ph, _M_this, __n, __hf, __eql, __a);
        }
        void destruct(pheap_type *ph) {
            _M_id_assoc.destruct(ph);
            _M_hm.destruct(ph);
        }
        
        pstl::ptr<node_type, bit_type> _M_get_node(pheap_type *ph) { return _M_node_allocator(ph)->allocate(ph, sizeof(node_type)); }
        void _M_put_node(pheap_type *ph, pstl::ptr<node_type, bit_type> __p) { _M_node_allocator(ph)->deallocate(ph, (pstl::ptr<void, bit_type>)__p); }
        

        void clear(pheap_type *ph) {
            _M_id_assoc.clear(ph);
            _M_hm.clear(ph);
        }
        iterator begin(pheap_type *ph) {
            return iterator(ph, _M_hm.begin(ph));
        }
        iterator end(pheap_type *ph) {
            return iterator(ph, _M_hm.end(ph));
        }
        std::pair<iterator, bool> insert(pheap_type *ph, const _Tp& __x) {
            id_type newid = _M_id_assoc.size();
            std::pair<typename hash_map_type::iterator, bool> ib =
                _M_hm.insert(ph, hash_map_type::value_type(hash_map_type::key_type(__x.first),
                                                           hash_map_type::mapped_type(newid, __x.second)));
            if(ib.second) { // inserted
                _M_id_assoc.push_back(ph, ib.first.get_node());
            }
            return std::pair<iterator, bool>(iterator(ph, ib.first), ib.second);
        }
        void erase(pheap_type *ph, iterator __i) {
            id_type id = __i.get_id();
            _M_id_assoc.find(ph, id) = 0;
            _M_hm.erase(ph, __i.get_it());
        }
        void erase(pheap_type *ph, const key_type& __key) {
            iterator it = find(ph, __key);
            if(it == end(ph)) return;
            erase(ph, it);
        }
        template <class _DevK, class _BitK, class _KeyType>
        void erase(pheap_type *ph, pstl::pheap<_DevK, _BitK> *kph, const _KeyType& __key) {
            iterator it = find(ph, kph, __key);
            if(it == end(ph)) return;
            erase(ph, it);
        }
        iterator find(pheap_type *ph, size_type __i) {
            if(__i >= _M_id_assoc.size()) return end(ph);
            pstl::ptr<hash_map_node_type, bit_type> __cur = _M_id_assoc.find(ph, __i);
            if(!__cur) return end(ph);
            typename hash_map_type::iterator it = hash_map_type::iterator(ph, __cur, get_M_hm_pointer(_M_this));
            return iterator(ph, it);
        }
        iterator find(pheap_type *ph, const key_type& __key) {
            typename hash_map_type::iterator it = _M_hm.find(ph, __key);
            return iterator(ph, it);
        }
        template <class _DevK, class _BitK, class _KeyType>
        iterator find(pheap_type *ph, pstl::pheap<_DevK, _BitK> *kph, const _KeyType& __key) {
            typename hash_map_type::iterator it = _M_hm.find(ph, kph, __key);
            return iterator(ph, it);
        }
    };

//////////////////////////////////////
/////
///// DB Hash
/////
    
    template <class _Alloc> class DbHash {
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
        typedef safe_hash_map<pstl::ptr<core_type, bit_type>, pstl::ptr<core_type, bit_type>,
                              hash_core_ptr, equal_to_core_ptr, allocator_type> hashdb_type;

    private:
        pstl::ptr<allocator_type, bit_type> allocp;
        hashdb_type hashdb;

    public:
        DbHash() {}
        DbHash(pheap_type *ph, pstl::ptr<DbHash, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp)
                : allocp(__allocp), hashdb(ph, ___M_this, __allocp) {}
        ~DbHash() {}

        void construct(pheap_type *ph, pstl::ptr<DbHash, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp) {
            new ((void*) this) DbHash(ph, ___M_this, __allocp);
        }
        void destruct(pheap_type* ph) {
            hashdb.destruct(ph);
        }
        
        std::pair<typename hashdb_type::iterator, bool>
        insert(pheap_type *ph, pstl::ptr<core_type, bit_type> k, pstl::ptr<core_type, bit_type> v) {
            hashdb.insert(ph, typename hashdb_type::value_type(typename hashdb_type::key_type(k),
                                                               typename hashdb_type::mapped_type(v)));
        }
        typename hashdb_type::iterator find(pheap_type *ph, pstl::ptr<core_type, bit_type> k) {
            return hashdb.find(ph, k);
        }
        void erase(pheap_type *ph, typename hashdb_type::iterator it0) {
            hashdb.erase(ph, it0);
        }
        void clear(pheap_type *ph) {
            hashdb.clear();
        }
        
        size_type size(pheap_type *ph) { return hashdb.size(ph); }
        
        typename hashdb_type::iterator begin(pheap_type *ph) { return hashdb.begin(ph); }
        typename hashdb_type::iterator end(pheap_type *ph) { return hashdb.end(ph); }
        
        bool empty(pheap_type *ph) { return hashdb.empty(); }
    };

//////////////////////////////////////
/////
///// DB Hash for FSP
/////
    
    template <class _Alloc> class DbHashFsp : public DbHash<_Alloc> {
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
        typedef safe_hash_map<pstl::ptr<core_type, bit_type>, pstl::ptr<core_type, bit_type>,
                              hash_core_ptr, equal_to_core_ptr, allocator_type> hashdb_type;

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
        DbHashFsp() {}
        DbHashFsp(pheap_type *ph, pstl::ptr<DbHashFsp, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp)
                : DbHashFsp(ph, static_cast<pstl::ptr<DbHashFsp, bit_type> >(___M_this), __allocp) {
            lilallocp = lil_allocator_type::pnew(0);
        }
        ~DbHashFsp() {}

        void construct(pheap_type *ph, pstl::ptr<DbHashFsp, bit_type> ___M_this, pstl::ptr<allocator_type, bit_type> __allocp) {
            new ((void*) this) DbHashFsp(ph, ___M_this, __allocp);
        }
        void destruct(pheap_type* ph) {
            DbHash<_Alloc>::destruct(ph);
        }
        
    public:

        std::pair<typename hashdb_type::iterator, bool>        
        insert(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
               lilfes::FSP k, lilfes::FSP v) {
	    pstl::ptr<core_type, bit_type> __k = core_type::pnew(ph, DbHash<_Alloc>::allocp);
	    __k(ph)->write(ph, k.GetMachine(), k.GetAddress(), tm, fm);
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, DbHash<_Alloc>::allocp);
	    __v(ph)->write(ph, v.GetMachine(), v.GetAddress(), tm, fm);

            std::pair<typename hashdb_type::iterator, bool> r = DbHash<_Alloc>::hashdb.insert(ph, k, v);
            if(!r.second) {
                pstl::pdelete(ph, __k);
                pstl::pdelete(ph, __v);
            }
            return r;
        }
        std::pair<typename hashdb_type::iterator, bool>        
        insert(pheap_type *ph, lilfes::FSP k, lilfes::FSP v) {
            return insert(ph, 0, 0, k, v);
        }
        template <class _DevK, class _BitK>
        typename hashdb_type::iterator find(pheap_type *ph, pstl::pheap<_DevK, _BitK> *lilph,
                                            typemapper_type *tm, featmapper_type *fm, lilfes::FSP k) {
            pstl::ptr<lil_core_type, bit_type> __k = lil_core_type::pnew(ph, lilallocp);
            __k(ph)->write(lilph, k.GetMachine(), k.GetAddress(), tm, fm);
            typename hashdb_type::iterator r = DbHash<_Alloc>::hashdb.find(ph, lilph, __k);
            pstl::pdelete(ph, __k);
            return r;
        }
        template <class _DevK, class _BitK>
        typename hashdb_type::iterator find(pheap_type *ph, pstl::pheap<_DevK, _BitK> *lilph, lilfes::FSP k) {
            return find(ph, lilph, 0, 0, k);
        }
        template <class _DevK, class _BitK>
        bool erase(pheap_type *ph, pstl::pheap<_DevK, _BitK> *lilph,
                   typemapper_type *tm, featmapper_type *fm, lilfes::FSP k) {
            pstl::ptr<lil_core_type, bit_type> __k = lil_core_type::pnew(ph, lilallocp);
            __k(ph)->write(lilph, k.GetMachine(), k.GetAddress(), tm, fm);
            typename hashdb_type::iterator r = DbHash<_Alloc>::hashdb.erase(ph, lilph, __k);
            pstl::pdelete(ph, __k);
            return r;
        }
        template <class _DevK, class _BitK>
        bool erase(pheap_type *ph, pstl::pheap<_DevK, _BitK> *lilph, lilfes::FSP k) {
            return erase(ph, lilph, 0,0,k);
        }
    };
    
}

#endif // __ldbm_hash_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/06/10 02:05:21  ninomi
 * ldbm-db.hをldbm-list.hとldbm-hash.hにわけました。ldbm-safe-list.hはldbm-list.hにうつしました。ldbm-safe-hash-map.hはldbm-hash.hにうつしました。
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
