/*
 * $Id: ldbm-list.h,v 1.2 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __ldbm_list_h
#define __ldbm_list_h

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
#include <vector>

namespace ldbm {
//////////////////////////////////////
/////
///// Safe List
/////
        
    template <class _Tp, class _Alloc>
    class safe_list_node {
    public:
        typedef _Tp value_type;
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef typename allocator_type::size_type size_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        
        size_type _M_id;
        value_type _M_data;
        pstl::ptr<safe_list_node, bit_type> _M_prev;
        pstl::ptr<safe_list_node, bit_type> _M_next;
        
    public:
        safe_list_node() {}
        ~safe_list_node() {}
        
        pstl::ptr<value_type, bit_type> get_M_data_pointer(pstl::ptr<safe_list_node, bit_type> _M_this) {
            pstl::ptr_int _M_data_addr = (pstl::ptr_int) (&_M_data);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ptr<value_type, bit_type>(_M_this.getAddress() + (_M_data_addr - this_addr));
        }
        pstl::ref<value_type, dev_type, bit_type> get_M_data(pheap_type *ph, pstl::ptr<safe_list_node, bit_type> _M_this) {
            pstl::ptr_int _M_data_addr = (pstl::ptr_int) (&_M_data);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ref<value_type, dev_type, bit_type>(ph, _M_this.getAddress() + (_M_data_addr - this_addr));
        }

    };

    template <class _Val, class _Alloc>
    class safe_list_iterator {
    public:
        typedef _Val value_type;
        typedef _Alloc allocator_type;
        typedef typename allocator_type::difference_type difference_type;
        typedef typename allocator_type::size_type size_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef safe_list_node<value_type, allocator_type> node_type;
        typedef pstl::ptr<value_type, bit_type> pointer;
        typedef pstl::ref<value_type, dev_type, bit_type> reference;
        typedef safe_list_iterator iterator;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
    private:
        pheap_type *_M_pheap;
        pstl::ptr<node_type, bit_type> _M_cur;
    public:
        safe_list_iterator(pheap_type *ph, pstl::ptr<node_type, bit_type> __n) : _M_pheap(ph), _M_cur(__n) {}
        safe_list_iterator() {}
        iterator operator++() { _M_cur = _M_cur(_M_pheap)->_M_next; }
        iterator operator--() { _M_cur = _M_cur(_M_pheap)->_M_prev; }
        reference operator*() { return _M_cur(_M_pheap)->get_M_data(_M_pheap, _M_cur); }
        reference operator->() { return _M_cur(_M_pheap)->get_M_data(_M_pheap, _M_cur); }
        bool operator==(iterator& __it) { return _M_cur == __it._M_cur; }
        bool operator!=(iterator& __it) { return _M_cur != __it._M_cur; }
        pointer get_ptr() { return _M_cur(_M_pheap)->get_M_data_pointer(_M_cur); }
        operator pointer() { return _M_cur(_M_pheap)->get_M_data_pointer(_M_cur); }
        pstl::ptr<node_type, bit_type> get_node() { return _M_cur; }
        size_type get_id() { return _M_cur(_M_pheap)->_M_id; }
        pheap_type* get_pheap() { return _M_pheap; }
    };

    template <class _Tp, class _Alloc>
    class safe_list {
    public:
        typedef _Tp value_type;
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef typename allocator_type::size_type size_type;
        typedef safe_list_node<value_type, allocator_type> node_type;
        typedef safe_list_iterator<value_type, allocator_type> iterator;
        typedef pstl::ref<value_type, dev_type, bit_type> value_reference;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;

    private:
        pstl::vector<pstl::ptr<node_type, bit_type>, allocator_type> _M_id_assoc;
        pstl::ptr<node_type, bit_type> _M_base;
        pstl::ptr<allocator_type, bit_type> _M_node_allocator;
        size_type _M_size;

    public:
        safe_list() {} // invalid
        safe_list(pheap_type *ph, pstl::ptr<allocator_type, bit_type> alloc) {
            _M_id_assoc.construct(alloc);
            _M_node_allocator = alloc;
            _M_base = _M_get_node(ph);
            pstl::ref<node_type, dev_type, bit_type> base = _M_base(ph);
            base->_M_prev = _M_base;
            base->_M_next = _M_base;
            _M_size = 0;
        }
        ~safe_list() {}

	size_type size(pheap_type*) { return _M_size; }
	bool empty(pheap_type*) { return (_M_size == 0); }
	size_type get_sn_count(pheap_type*) { return _M_id_assoc.size(); }

        void construct() {
            new ((void*) this) safe_list();
        }
        void construct(pheap_type *ph, pstl::ptr<allocator_type, bit_type> alloc) {
            new ((void*) this) safe_list(ph, alloc);
        }
        void destruct(pheap_type *ph) {
            typename pstl::vector<pstl::ptr<node_type, bit_type>, allocator_type>::iterator it = _M_id_assoc.begin(ph), last = _M_id_assoc.end(ph);
            for(; it != last; ++it) {
                pstl::ptr<node_type, bit_type> x = (*it)->get_obj();
                if(!x)
                    _M_put_node(ph, x);
            }
        }
        void clear(pheap_type *ph) {
            typename pstl::vector<pstl::ptr<node_type, bit_type>, allocator_type>::iterator it = _M_id_assoc.begin(ph), last = _M_id_assoc.end(ph);
            for(; it != last; ++it) {
                pstl::ptr<node_type, bit_type> x = (*it)->get_obj();
                if(!x)
                    _M_put_node(ph, x);
            }
            _M_id_assoc.clear(ph);
            pstl::ref<node_type, dev_type, bit_type> base = _M_base(ph);
            base->_M_prev = _M_base;
            base->_M_next = _M_base;
            _M_size = 0;
        }

        pstl::ptr<node_type, bit_type> _M_get_node(pheap_type *ph) { _M_size++; return _M_node_allocator(ph)->allocate(ph, sizeof(node_type)); }
        void _M_put_node(pheap_type *ph, pstl::ptr<node_type, bit_type> __p) { _M_size--; _M_node_allocator(ph)->deallocate(ph, (pstl::ptr<void, bit_type>)__p); }

        iterator begin(pheap_type *ph) {
            return iterator(ph, _M_base(ph)->next);
        }
        iterator end(pheap_type *ph) {
            return iterator(ph, _M_base);
        }

        void push_front(pheap_type *ph, const _Tp& __x) {
	    pstl::ptr<node_type, bit_type> __newnode =_M_get_node(ph);
            pstl::ref<node_type, dev_type, bit_type> newnode = __newnode(ph);
            pstl::ref<node_type, dev_type, bit_type> base = _M_base(ph);
            pstl::ptr<node_type, bit_type> __first = base->_M_next;
            pstl::ref<node_type, dev_type, bit_type> first = __first(ph);

            size_type id = _M_id_assoc.size();
            _M_id_assoc.push_back(ph, __newnode);

            newnode->_M_id = id;
            newnode->_M_data = __x;
            newnode->_M_prev = _M_base;
            newnode->_M_next = __first;
            base->_M_next = __newnode;
            first->_M_prev = __newnode;
        }

        void push_back(pheap_type *ph, const _Tp& __x) {
            pstl::ptr<node_type, bit_type> __newnode =_M_get_node(ph);
            pstl::ref<node_type, dev_type, bit_type> newnode = __newnode(ph);
            pstl::ref<node_type, dev_type, bit_type> base = _M_base(ph);
            pstl::ptr<node_type, bit_type> __last = base->_M_prev;
            pstl::ref<node_type, dev_type, bit_type> last = __last(ph);

            size_type id = _M_id_assoc.size();
            _M_id_assoc.push_back(ph, __newnode);

            newnode->_M_id = id;
            newnode->_M_data = __x;
            newnode->_M_prev = __last;
            newnode->_M_next = _M_base;
            last->_M_next = __newnode;
            base->_M_prev = __newnode;

        }
        void pop_front(pheap_type *ph) {
            pstl::ref<node_type, dev_type, bit_type> base = _M_base(ph);
            pstl::ptr<node_type, bit_type> __first = base->_M_next;
            pstl::ref<node_type, dev_type, bit_type> first = __first(ph);
	    pstl::ptr<node_type, bit_type> __second = first->_M_next;
            pstl::ref<node_type, dev_type, bit_type> second = __second(ph);
      
            _M_id_assoc.find(ph, first->_M_id) = 0;
      
            base->_M_next = __second;
            second->_M_prev = _M_base;
      
            _M_put_node(ph, __first);
        }
        void pop_back(pheap_type *ph) {
            pstl::ref<node_type, dev_type, bit_type> base = _M_base(ph);
            pstl::ptr<node_type, bit_type> __last = base->_M_prev;
            pstl::ref<node_type, dev_type, bit_type> last = __last(ph);
            pstl::ptr<node_type, bit_type> __second_last = last->_M_prev;
            pstl::ref<node_type, dev_type, bit_type> second_last = __second_last(ph);
      
            _M_id_assoc.find(ph, last->_M_id) = 0;
      
            base->_M_prev = __second_last;
            second_last->_M_next = _M_base;
      
            _M_put_node(ph, __last);
        }
        iterator insert(pheap_type *ph, iterator __i, const _Tp& __x) {
            pstl::ptr<node_type, bit_type> __cur = __i.get_node();
            pstl::ref<node_type, dev_type, bit_type> cur = __cur(ph);
            pstl::ptr<node_type, bit_type> __prev = cur->_M_prev;
            pstl::ref<node_type, dev_type, bit_type> prev = __prev(ph);

            pstl::ptr<node_type, bit_type> __newnode = _M_get_node(ph);
            pstl::ref<node_type, dev_type, bit_type> newnode = __newnode(ph);
            
            size_type id = _M_id_assoc.size();
            _M_id_assoc.push_back(ph, __newnode);

            newnode->_M_id = id;
            newnode->_M_data = __x;
            newnode->_M_prev = __prev;
            newnode->_M_next = __cur;
            prev->_M_next = __newnode;
            cur->_M_prev = __newnode;
            return iterator(ph, newnode);
        }
        void erase(pheap_type *ph, iterator __i) {
            pstl::ptr<node_type, bit_type> __cur = __i.get_node();
            pstl::ref<node_type, dev_type, bit_type> cur = __cur(ph);
            pstl::ptr<node_type, bit_type> __prev = cur->_M_prev;
            pstl::ptr<node_type, bit_type> __next = cur->_M_next;

            _M_id_assoc.find(ph, cur->_M_id) = 0;
            _M_put_node(ph, __cur);

            __prev(ph)->_M_next = __next;
            __next(ph)->_M_prev = __prev;
        }
        iterator find(pheap_type *ph, size_type __i) {
            if(__i >= _M_id_assoc.size()) return end(ph);
            pstl::ptr<node_type, bit_type> __cur = _M_id_assoc.find(ph, __i);
            if(!__cur) return end(ph);
            return iterator(ph, __cur);
        }
    };

//////////////////////////////////////
/////
///// DB List
/////

    template <class _Alloc> class DbList {
    public:
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename bit_type::size_type size_type;
        typedef typename bit_type::pointer pointer;
        typedef typename cell<bit_type>::core_p core_p;
        typedef core<allocator_type> core_type;

        typedef size_type fsoid_type;
        typedef safe_list<pstl::ptr<core_type, bit_type>, allocator_type> fsodb_type;

    protected:
        pstl::ptr<allocator_type, bit_type> allocp;
        fsodb_type fsodb;

    public:
        DbList() {}
        DbList(pheap_type *ph, pstl::ptr<allocator_type, bit_type> __allocp)
                : allocp(__allocp), fsodb(ph, __allocp) {}
        ~DbList() {}

        void construct(pheap_type *ph, pstl::ptr<allocator_type, bit_type> __allocp) {
            new ((void*) this) DbList(ph, __allocp);
        }
        void destruct(pheap_type* ph) {
            fsodb.destruct(ph);
        }
        
        void push_front(pheap_type *ph, pstl::ptr<core_type, bit_type> f) {
            fsodb.push_front(ph, f);
        }
        void push_back(pheap_type *ph, pstl::ptr<core_type, bit_type> f) {
            fsodb.push_back(ph, f);
        }
        void pop_front(pheap_type *ph) {
            fsodb.pop_front(ph);
        }
        void pop_back(pheap_type *ph) {
            fsodb.pop_back(ph);
        }
        typename fsodb_type::iterator insert(pheap_type *ph, typename fsodb_type::iterator it, pstl::ptr<core_type, bit_type> f) {
            return fsodb.insert(ph, it, f);
        }
        typename fsodb_type::iterator find(pheap_type *ph, fsoid_type fsoid) {
            return fsodb.find(ph, fsoid);
        }
        void erase(pheap_type *ph, typename fsodb_type::iterator it0) {
            fsodb.erase(ph, it0);
        }
        void clear(pheap_type *ph) {
            fsodb.clear(ph);
        }
        
        size_type size(pheap_type *ph) { return fsodb.size(ph); }
        
        typename fsodb_type::iterator begin(pheap_type *ph) { return fsodb.begin(ph); }
        typename fsodb_type::iterator end(pheap_type *ph) { return fsodb.end(ph); }
        
        bool empty(pheap_type *ph) { return fsodb.empty(ph); }
    };
        
//////////////////////////////////////
/////
///// DB List for FSP
/////
    
    template <class _Alloc> class DbListFsp : public DbList<_Alloc> {
    public:
        typedef _Alloc allocator_type;
        typedef typename allocator_type::bit_type bit_type;
        typedef typename allocator_type::dev_type dev_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename bit_type::size_type size_type;
        typedef typename bit_type::pointer pointer;
        typedef typename cell<bit_type>::core_p core_p;
        typedef core<allocator_type> core_type;
        typedef size_type fsoid_type;
        typedef safe_list<pstl::ptr<core_type, bit_type>, allocator_type> fsodb_type;
        
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
        DbListFsp() {}
        DbListFsp(pheap_type *ph, pstl::ptr<allocator_type, bit_type> __allocp)
                : DbList<_Alloc>(ph, __allocp) {
            lilallocp = static_cast<pstl::ptr<lil_allocator_type, bit_type> >(lil_allocator_type::pnew(0));
        }
        ~DbListFsp() {}

        void construct(pheap_type *ph, pstl::ptr<allocator_type, bit_type> __allocp) {
            new ((void*) this) DbListFsp(ph, __allocp);
        }
        void destruct(pheap_type* ph) {
            DbList<_Alloc>::destruct(ph);
        }
        
    public:
        
        void push_front(pheap_type *ph, typemapper_type *tm, featmapper_type *fm, lilfes::FSP f) {
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, DbList<_Alloc>::allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress(), tm, fm);
            push_front(ph, __v);
        }
        void push_back(pheap_type *ph, typemapper_type *tm, featmapper_type *fm, lilfes::FSP f) {
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, DbList<_Alloc>::allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress(), tm, fm);
            DbList<_Alloc>::push_back(ph, __v);
        }
        void push_front(pheap_type *ph, lilfes::FSP f) { push_front(ph, 0, 0, f); }
        void push_back(pheap_type *ph, lilfes::FSP f) { push_back(ph, 0, 0, f); }

        void pop_front(pheap_type *ph) {
            erase(ph, DbList<_Alloc>::fsodb.begin(ph));
        }
        void pop_back(pheap_type *ph) {
            typename fsodb_type::iterator it = DbList<_Alloc>::fsodb.end(ph); --it;
            erase(ph, it);
        }
        void insert(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                    typename fsodb_type::iterator it0, lilfes::FSP f) {
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, DbList<_Alloc>::allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress(), tm, fm);
            insert(ph, it0, __v);
        }

        void insert(pheap_type *ph, typename fsodb_type::iterator it0, lilfes::FSP f) {
	    pstl::ptr<core_type, bit_type> __v = core_type::pnew(ph, DbList<_Alloc>::allocp);
	    __v(ph)->write(ph, f.GetMachine(), f.GetAddress());
            insert(ph, it0, __v);
        }

        bool find(pheap_type *ph, typemapper_type *tm, featmapper_type *fm,
                  typename fsodb_type::iterator it0, lilfes::FSP f) {

            if(it0 == DbList<_Alloc>::fsodb.end(ph)) return false;
            pstl::ptr<core_type, bit_type> corep = (*it0).get_obj();
            lilfes::FSP core_fsp = corep(ph)->read(ph, f.GetMachine(), 0, tm, fm);
            return f.Unify(core_fsp);
        }
        bool find(pheap_type *ph, typename fsodb_type::iterator it0, lilfes::FSP f) {

            if(it0 == DbList<_Alloc>::fsodb.end(ph)) return false;
            pstl::ptr<core_type, bit_type> corep = (*it0).get_obj();
            lilfes::FSP core_fsp = corep(ph)->read(ph, f.GetMachine(), 0);
            return f.Unify(core_fsp);
        }
        void erase(pheap_type *ph, typename fsodb_type::iterator it0) {
            if(it0 == DbList<_Alloc>::fsodb.end(ph)) return;
            pstl::ptr<core_type, bit_type> corep = (*it0).get_obj();
            pstl::pdelete(ph, corep);
            DbList<_Alloc>::erase(ph, it0);
        }
    };
    
}

#endif // __ldbm_list_h

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
