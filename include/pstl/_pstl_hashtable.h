// -*- C++ -*-
/*
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 2005
 * Takashi Tsunakawa and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Tsunakawa and Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __SGI_PSTL_INTERNAL_HASHTABLE_H
#define __SGI_PSTL_INTERNAL_HASHTABLE_H

#include "_pstl_algobase.h"
#include "_pstl_alloc.h"
#include "_pstl_construct.h"
#include "_pstl_tempbuf.h"
#include "_pstl_algo.h"
#include "_pstl_uninitialized.h"
#include "_pstl_function.h"
#include "_pstl_vector.h"
#include "_pstl_hash_fun.h"

__PSTL_BEGIN_NAMESPACE
    
/////////////////////////////////
//// hashtable node
template <class _Val, class _Arch>
class _Hashtable_node {
    typedef _Val value_type;
    typedef _Arch arch_type;
    typedef typename arch_type::difference_type difference_type;
    typedef typename arch_type::size_type size_type;
    typedef typename arch_type::bit_type bit_type;
    typedef typename arch_type::dev_type dev_type;
    typedef ptr<value_type, arch_type> pointer;
    typedef const_ptr<value_type, arch_type> const_pointer;
    typedef ref<value_type, arch_type> reference;
    typedef const_ref<value_type, arch_type> const_reference;
    typedef pheap<arch_type> pheap_type;
public:
    ptr<_Hashtable_node, arch_type> _M_next;
    value_type _M_val;
    pointer get_M_val_pointer(ptr<_Hashtable_node, arch_type> _M_this) {
	ptr_int _M_val_addr = reinterpret_cast<ptr_int>(&_M_val);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return pointer(_M_this.getAddress() + (_M_val_addr - this_addr), _M_next.get_pheap());
    }
    const_pointer get_M_val_pointer(const_ptr<_Hashtable_node, arch_type> _M_this) const {
	ptr_int _M_val_addr = reinterpret_cast<ptr_int>(&_M_val);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return const_pointer(_M_this.getAddress() + (_M_val_addr - this_addr), _M_next.get_pheap());
    }
    reference get_M_val(ptr<_Hashtable_node, arch_type> _M_this) {
	return *get_M_val_pointer(_M_this);
    }
    const_reference get_M_val(const_ptr<_Hashtable_node, arch_type> _M_this) const {
	return *get_M_val_pointer(_M_this);
    }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Val, class _Arch>
struct replace_pheap<_Hashtable_node<_Val, _Arch> > {
    void operator()(pheap_ptr __ph, _Hashtable_node<_Val, _Arch> &__n) {
	__replace_pheap(__ph, __n._M_next);
	__replace_pheap(__ph, __n._M_val);
    }
};

//////////////////////////////////////
//// hashtable iterator
template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_Val, _Arch) >
class hashtable;

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
class _Hashtable_iterator;

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
class _Hashtable_const_iterator;

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
class _Hashtable_iterator {
public:
    typedef forward_iterator_tag iterator_category;
    typedef _Val value_type;
    typedef _Alloc allocator_type;
    typedef _Arch arch_type;
    typedef typename arch_type::difference_type difference_type;
    typedef typename arch_type::size_type size_type;
    typedef typename arch_type::bit_type bit_type;
    typedef typename arch_type::dev_type dev_type;
    typedef ref<value_type, arch_type> reference;
    typedef const_ref<value_type, arch_type> const_reference;
    typedef ptr<value_type, arch_type> pointer;
    typedef const_ptr<value_type, arch_type> const_pointer;
    typedef pheap<arch_type> pheap_type;
    typedef hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> _Hashtable;    
    typedef _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> iterator;
    typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> const_iterator;
    typedef _Hashtable_node<_Val, arch_type> _Node;

    ptr<_Node, arch_type> _M_cur;
    ptr<_Hashtable, arch_type> _M_ht;
public:
    _Hashtable_iterator(ptr<_Node, arch_type> __n, ptr<_Hashtable, arch_type> __tab)
	: _M_cur(__n), _M_ht(__tab) {}
    _Hashtable_iterator(ptr<_Node, arch_type> __n, ptr<_Hashtable, arch_type> __tab, pheap_ptr __ph)
	: _M_cur(__n, __ph), _M_ht(__tab, __ph) {}
    _Hashtable_iterator() {}
    reference operator*()       { return (*_M_cur)->get_M_val(_M_cur); }
    const_reference operator*() const { return (*_M_cur)->get_M_val(_M_cur); }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
          pointer operator->()       { return (*_M_cur)->get_M_val_pointer(_M_cur); }
    const_pointer operator->() const { return (*_M_cur)->get_M_val_pointer(_M_cur); }
#endif // __SGI_PSTL_NO_ARROW_OPERATOR
          iterator &operator++();
    const_iterator &operator++() const;
    bool operator==(const iterator &__it) const { return _M_cur == __it._M_cur; }
    bool operator!=(const iterator &__it) const { return _M_cur != __it._M_cur; }
          pointer get_ptr()       { return (*_M_cur)->get_M_val_pointer(_M_cur); }
    const_pointer get_ptr() const { return (*_M_cur)->get_M_val_pointer(_M_cur); }
    operator pointer() { return get_ptr(); }
    operator const_pointer() const { return get_ptr(); }
    operator const_iterator() const { return const_iterator(*this); }

    ptr<_Node, arch_type> get_node() const { return _M_cur; }
};

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
struct replace_pheap<_Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> &__it) {
	__replace_pheap(__ph, __it._M_cur);
	__replace_pheap(__ph, __it._M_ht);
    }
};
    
template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
class _Hashtable_const_iterator {
public:
    typedef forward_iterator_tag iterator_category;
    typedef _Val value_type;
    typedef _Alloc allocator_type;
    typedef _Arch arch_type;
    typedef typename arch_type::difference_type difference_type;
    typedef typename arch_type::size_type size_type;
    typedef typename arch_type::bit_type bit_type;
    typedef typename arch_type::dev_type dev_type;
    typedef const_ref<value_type, arch_type> reference;
    typedef const_ptr<value_type, arch_type> pointer;
    typedef pheap<arch_type> pheap_type;
    typedef hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> _Hashtable;    
    typedef _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> iterator;
    typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> const_iterator;
    typedef _Hashtable_node<_Val, arch_type> _Node;

private:
    ptr<_Node, arch_type> _M_cur;
    ptr<_Hashtable, arch_type> _M_ht;
public:
    _Hashtable_const_iterator(ptr<_Node, arch_type> __n, ptr<_Hashtable, arch_type> __tab)
	: _M_cur(__n), _M_ht(__tab) {}
    _Hashtable_const_iterator(ptr<_Node, arch_type> __n, ptr<_Hashtable, arch_type> __tab, pheap_ptr __ph)
	: _M_cur(__n, __ph), _M_ht(__tab, __ph) {}
    _Hashtable_const_iterator(const iterator &__it) : _M_cur(__it._M_cur), _M_ht(__it._M_ht) {}
    _Hashtable_const_iterator() {}
    reference operator*() const { return (*_M_cur)->get_M_val(_M_cur); }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    pointer operator->() const { return (*_M_cur)->get_M_val_pointer(_M_cur); }
#endif // __SGI_PSTL_NO_ARROW_OPERATOR
    const_iterator &operator++();
    bool operator==(const const_iterator &__it) const { return _M_cur == __it._M_cur; }
    bool operator!=(const const_iterator &__it) const { return _M_cur != __it._M_cur; }
    pointer get_ptr() const { return (*_M_cur)->get_M_val_pointer(_M_cur); }
    operator pointer() const { return get_ptr(); }
    ptr<_Node, arch_type> get_node() const { return _M_cur; }

};

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
struct replace_pheap<_Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> &__it) {
	__replace_pheap(__ph, __it._M_cur);
	__replace_pheap(__ph, __it._M_ht);
    }
};



/////////////////////////////////
//// next size of hashtable

// Note: assumes long is at least 32 bits.
enum { __stl_num_primes = 28 };

static const unsigned long __stl_prime_list[__stl_num_primes] =
{
    53ul,         97ul,         193ul,       389ul,       769ul,
    1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
    1610612741ul, 3221225473ul, 4294967291ul
};

inline unsigned long __stl_next_prime(unsigned long __n)
{
    const unsigned long *__first = __stl_prime_list;
    const unsigned long *__last = __stl_prime_list + static_cast<int>(__stl_num_primes);
    const unsigned long *pos = __STD::lower_bound(__first, __last, __n);
    return pos == __last ? *(__last - 1) : *pos;
}

// Forward declaration of operator==.

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
class hashtable;

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
bool operator==(const hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All> &__ht1,
		const hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All> &__ht2);


// Hashtables handle allocators a bit differently than other containers
//  do.  If we're using standard-conforming allocators, then a hashtable
//  unconditionally has a member variable to hold its allocator, even if
//  it so happens that all instances of the allocator type are identical.
// This is because, for hashtables, this extra storage is negligible.  
//  Additionally, a base class wouldn't serve any other purposes; it 
//  wouldn't, for example, simplify the exception-handling code.

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
class hashtable {
public:
    typedef _Key key_type;
    typedef _Val value_type;
    typedef _HashFcn hasher;
    typedef _EqualKey key_equal;

    typedef _Arch arch_type;
    typedef typename arch_type::bit_type bit_type;
    typedef typename arch_type::dev_type dev_type;
    typedef typename arch_type::size_type size_type;
    typedef typename arch_type::difference_type difference_type;
    typedef ptr<value_type, arch_type> pointer;
    typedef const_ptr<value_type, arch_type> const_pointer;
    typedef ref<value_type, arch_type> reference;
    typedef const_ref<value_type, arch_type> const_reference;
    typedef pheap<arch_type> pheap_type;

    hasher hash_funct() const { return _M_hash; }
    key_equal key_eq() const { return _M_equals; }
    ptr<hashtable, arch_type> get_M_this() const { return _M_this; }

private:
    typedef _Hashtable_node<value_type, arch_type> _Node;
public:
    typedef _Node node_type;

#ifdef __PSTL_USE_STD_ALLOCATORS
public:
    typedef typename _Alloc_traits<_Val, arch_type, _Alloc>::allocator_type allocator_type;
    allocator_type get_allocator() const { return _M_node_allocator; }
private:
    typename _Alloc_traits<_Node, arch_type, _Alloc>::allocator_type _M_node_allocator;
    ptr<_Node, arch_type> _M_get_node() { return _M_node_allocator.allocate(1); }
    void _M_put_node(const ptr<_Node, arch_type> &__p) { _M_node_allocator.deallocate(__p, 1); }
# define __PSTL_HASH_ALLOC_INIT(__a) _M_node_allocator(__a),
#else // __PSTL_USE_STD_ALLOCATORS
public:
    typedef _Alloc allocator_type;
    ptr<allocator_type, arch_type> get_allocator() const { return allocator_type(); }
private:
    typedef simple_alloc<_Node, arch_type, allocator_type> _M_node_allocator_type;
    ptr<_Node, arch_type> _M_get_node() { return _M_node_allocator_type::allocate(1); }
    void _M_put_node(const ptr<_Node, arch_type> &__p) { _M_node_allocator_type::deallocate(__p, 1); }
# define __PSTL_HASH_ALLOC_INIT(__a)
#endif // __PSTL_USE_STD_ALLOCATORS

private:
    hasher                _M_hash    __PSTL_ALIGNED_64;
    key_equal             _M_equals  __PSTL_ALIGNED_64;
    _ExtractKey           _M_get_key __PSTL_ALIGNED_64;
    typedef pstl::vector<ptr<_Node, arch_type>, arch_type, allocator_type> buckets_type;
    buckets_type          _M_buckets __PSTL_ALIGNED_64;
    size_type             _M_num_elements;
    ptr<hashtable, arch_type> _M_this;

public:
    typedef _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc>
    iterator;
    typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc>
    const_iterator;

    friend class _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc>;
    friend class _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc>;

public:
    hashtable() {}
    hashtable(ptr<hashtable, arch_type> __this,
	      size_type __n,
	      const _HashFcn    &__hf,
	      const _EqualKey   &__eql,
	      const _ExtractKey &__ext,
	      const allocator_type &__a = allocator_type())
	: __PSTL_HASH_ALLOC_INIT(__a)
	  _M_hash(__hf),
	  _M_equals(__eql),
	  _M_get_key(__ext),
	  _M_buckets(__a),
	  _M_num_elements(0),
	  _M_this(__this)
	{
	    _M_initialize_buckets(__n);
	}

    hashtable(ptr<hashtable, arch_type> __this,
	      size_type __n,
	      const _HashFcn     &__hf,
	      const _EqualKey    &__eql,
	      const allocator_type &__a = allocator_type())
	: __PSTL_HASH_ALLOC_INIT(__a)
	  _M_hash(__hf),
	  _M_equals(__eql),
	  _M_get_key(_ExtractKey()),
	  _M_buckets(__a),
	  _M_num_elements(0),
	  _M_this(__this)
	{
	    _M_initialize_buckets(__n);
	}

    hashtable(const hashtable &__ht)
	: __PSTL_HASH_ALLOC_INIT(__ht.get_allocator())
	  _M_hash(__ht._M_hash),
	  _M_equals(__ht._M_equals),
	  _M_get_key(__ht._M_get_key),
	  _M_buckets(__ht.get_allocator()),
	  _M_num_elements(0),
	  _M_this(__ht.__this)
	{
	    _M_copy_from(__ht);
	}

#undef __PSTL_HASH_ALLOC_INIT
    
    hashtable &operator=(const hashtable &__ht)
	{
	    if (&__ht != this) {
		clear();
		_M_hash = __ht._M_hash;
		_M_equals = __ht._M_equals;
		_M_get_key = __ht._M_get_key;
		_M_this = __ht.__this;
		_M_copy_from(__ht);
	    }
	    return *this;
	}

    ~hashtable() { clear(); }

    //// pnew
    static ptr<hashtable, arch_type> pnew(const pheap_ptr __ph,
					  size_type __n,
					  const _HashFcn    &__hf,
					  const _EqualKey   &__eql,
					  const _ExtractKey &__ext,
					  const allocator_type &__a = allocator_type()) {
	ptr<hashtable, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hashtable)), __ph);
        ref<hashtable, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hashtable(__p, __n, __hf, __eql, __ext, __a);
	return __p;
    }
    static ptr<hashtable, arch_type> pnew(const pheap_ptr __ph,
					  size_type __n,
					  const _HashFcn    &__hf,
					  const _EqualKey   &__eql,
					  const allocator_type &__a = allocator_type()) {
	ptr<hashtable, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hashtable)), __ph);
        ref<hashtable, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hashtable(__p, __n, __hf, __eql, __a);
	return __p;
    }
    static ptr<hashtable, arch_type> pnew(const pheap_ptr __ph,
					  const hashtable &__ht) {
	ptr<hashtable, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hashtable)), __ph);
        ref<hashtable, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hashtable(__p, __ht);
	return __p;
    }

    size_type size() const { return _M_num_elements; }
    size_type max_size() const { return size_type(-1); }
    bool empty() const { return size() == 0; }

    void swap(hashtable &__ht)
	{
	    __STD::swap(_M_node_allocator, __ht._M_node_allocator);
	    __STD::swap(_M_hash, __ht._M_hash);
	    __STD::swap(_M_equals, __ht._M_equals);
	    __STD::swap(_M_get_key, __ht._M_get_key);
	    _M_buckets.swap(__ht._M_buckets);
	    __STD::swap(_M_num_elements, __ht._M_num_elements);
	    __STD::swap(_M_this, __ht._M_this);
	}
    iterator begin()
	{
	    for (size_type __n = 0; __n < _M_buckets.size(); ++__n) {
		if (_M_buckets[__n].get_obj()) {
		    return iterator(_M_buckets[__n].get_obj(), _M_this);
		}
	    }
	    return end();
	}

    iterator end() { return iterator(ptr<hashtable, arch_type>(0, _M_this.get_pheap()), _M_this); }

    const_iterator begin() const
	{
	    for (size_type __n = 0; __n < _M_buckets.size(); ++__n) {
		if (_M_buckets[__n].get_obj()) {
		    return const_iterator(_M_buckets[__n], _M_this);
		}
	    }
	    return end();
	}

    const_iterator end() const { return const_iterator(ptr<hashtable, arch_type>(0, _M_this.get_pheap()), _M_this); }

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _Vl, class _Ky, class _HF, class _Ex, class _Eq, class _Ar, class _Al>
    friend bool operator==(const hashtable<_Vl, _Ky, _HF, _Ex, _Eq, _Ar, _Al> &,
			   const hashtable<_Vl, _Ky, _HF, _Ex, _Eq, _Ar, _Al> &);
#else // __PSTL_MEMBER_TEMPLATES
    friend bool __STD_QUALIFIER
    operator== __PSTL_NULL_TMPL_ARGS (const hashtable &, const hashtable &);
#endif // __PSTL_MEMBER_TEMPLATES
    
public:

    size_type bucket_count() const { return _M_buckets.size(); }

    size_type max_bucket_count() const
	{ return __stl_prime_list[static_cast<int>(__stl_num_primes) - 1]; } 

    size_type elems_in_bucket(size_type __bucket) const
	{
	    size_type __result = 0;
	    for (ptr<_Node, arch_type> __cur = _M_buckets[__bucket]; __cur; __cur = (*__cur)->_M_next) {
		__result += 1;
	    }
	    return __result;
	}

    pstl::pair<iterator, bool, arch_type> insert_unique(const value_type &__obj)
	{
	    resize(_M_num_elements + 1);
	    return insert_unique_noresize(__obj);
	}

    iterator insert_equal(const value_type &__obj)
	{
	    resize(_M_num_elements + 1);
	    return insert_equal_noresize(__obj);
	}

    pstl::pair<iterator, bool, arch_type> insert_unique_noresize(const value_type &__obj);
    iterator insert_equal_noresize(const value_type &__obj);

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    void insert_unique(_InputIterator __f, _InputIterator __l)
	{
	    insert_unique(__f, __l, __ITERATOR_CATEGORY(__f));
	}
    
    template <class _InputIterator>
    void insert_equal(_InputIterator __f, _InputIterator __l)
	{
	    insert_equal(__f, __l, __ITERATOR_CATEGORY(__f));
	}
    
    template <class _InputIterator>
    void insert_unique(_InputIterator __f, _InputIterator __l,
		       input_iterator_tag)
	{
	    for ( ; __f != __l; ++__f) {
		insert_unique(*__f);
	    }
	}

    template <class _InputIterator>
    void insert_equal(_InputIterator __f, _InputIterator __l,
		      input_iterator_tag)
	{
	    for ( ; __f != __l; ++__f) {
		insert_equal(*__f);
	    }
	}

    template <class _ForwardIterator>
    void insert_unique(_ForwardIterator __f, _ForwardIterator __l,
		       forward_iterator_tag)
	{
	    size_type __n = 0;
	    distance(__f, __l, __n);
	    resize(_M_num_elements + __n);
	    for ( ; __n > 0; --__n, ++__f) {
		insert_unique_noresize(*__f);
	    }
	}

    template <class _ForwardIterator>
    void insert_equal(_ForwardIterator __f, _ForwardIterator __l,
		      forward_iterator_tag)
	{
	    size_type __n = 0;
	    distance(__f, __l, __n);
	    resize(_M_num_elements + __n);
	    for ( ; __n > 0; --__n, ++__f) {
		insert_equal_noresize(*__f);
	    }
	}

#else // __PSTL_MEMBER_TEMPLATES
    void insert_unique(const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l)
	{
	    size_type __n = __l - __f;
	    resize(_M_num_elements + __n);
	    for ( ; __n > 0; --__n, ++__f) {
		insert_unique_noresize(*__f);
	    }
	}

    void insert_equal(const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l)
	{
	    size_type __n = __l - __f;
	    resize(_M_num_elements + __n);
	    for ( ; __n > 0; --__n, ++__f) {
		insert_equal_noresize(*__f);
	    }
	}

    void insert_unique(const_iterator __f, const_iterator __l)
	{
	    size_type __n = 0;
	    distance(__f, __l, __n);
	    resize(_M_num_elements + __n);
	    for ( ; __n > 0; --__n, ++__f) {
		insert_unique_noresize(*__f);
	    }
	}

    void insert_equal(const_iterator __f, const_iterator __l)
	{
	    size_type __n = 0;
	    distance(__f, __l, __n);
	    resize(_M_num_elements + __n);
	    for ( ; __n > 0; --__n, ++__f)
		insert_equal_noresize(*__f);
	}
#endif // __PSTL_MEMBER_TEMPLATES
    
    reference find_or_insert(const value_type &__obj);

    iterator find(const key_type &__key) 
	{
	    size_type __n = _M_bkt_num_key(__key);
	    ptr<_Node, arch_type> __first = _M_buckets[__n].get_obj();
	    ref<_Node, arch_type> __first_ref;
	    if (__first) {
		__first_ref.reassign(*__first);
	    }
	    for ( ; __first && !_M_equals(_M_get_key(__first_ref->_M_val), __key);) {
		__first = __first_ref->_M_next;
		if (__first) {
		    __first_ref.reassign(*__first);
		}
	    }
	    return iterator(__first, _M_this);
	}

    template <class _KeyType>
    iterator find_light(const _KeyType &__key) 
	{
	    size_type __n = _M_bkt_num_key_light(__key);
	    ptr<_Node, arch_type> __first = _M_buckets[__n].get_obj();
	    ref<_Node, arch_type> __first_ref;
	    if (__first) {
		__first_ref.reassign(*__first);
	    }
	    for ( ; __first && !_M_equals(_M_get_key(__first_ref->_M_val), __key);) {
		__first = __first_ref->_M_next;
		if (__first) {
		    __first_ref.reassign(*__first);
		}
	    }
	    return iterator(__first, _M_this);
	}

    const_iterator find(const key_type &__key) const
	{
	    size_type __n = _M_bkt_num_key(__key);
	    ptr<_Node, arch_type> __first = _M_buckets[__n];
	    ref<_Node, arch_type> __first_ref;
	    if (__first) {
		__first_ref.reassign(*__first);
	    }
	    for ( ; __first && !_M_equals(_M_get_key(__first_ref->get_M_val(__first)), __key);) {
		__first = __first_ref->_M_next;
		if (__first) {
		    __first_ref.reassign(*__first);
		}
	    }
	    return const_iterator(__first, _M_this);
	}

    size_type count(const key_type &__key) const
	{
	    const size_type __n = _M_bkt_num_key(__key);
	    size_type __result = 0;

	    ptr<_Node, arch_type> __cur = _M_buckets[__n];
	    for (; __cur ;) {
		ref<_Node, arch_type> __cur_ref = *__cur;
		if (_M_equals(_M_get_key(__cur_ref->get_M_val(__cur)), __key)) {
		    ++__result;
		}
		__cur = __cur_ref->_M_next;
	    }
	    return __result;
	}

    pstl::pair<iterator, iterator, arch_type> 
    equal_range(const key_type &__key);
      
    pstl::pair<const_iterator, const_iterator, arch_type> 
    equal_range(const key_type &__key) const;

    size_type erase(const key_type &__key);
      
    void erase(const iterator &__it);
    void erase(iterator __first, iterator __last);

    void erase(const const_iterator &__it);
    void erase(const_iterator __first, const_iterator __last);

    void resize(size_type __num_elements_hint);
    void clear();

private:
    size_type _M_next_size(size_type __n) const
	{ return __stl_next_prime(__n); }

    void _M_initialize_buckets(size_type __n)
	{
	    const size_type __n_buckets = _M_next_size(__n);
	    _M_buckets.reserve(__n_buckets);
	    _M_buckets.insert(_M_buckets.end(), __n_buckets, ptr<_Node, arch_type>(0, _M_this.get_pheap()));
	    _M_num_elements = 0;
	}

    size_type _M_bkt_num_key(const key_type &__key) const
	{
	    return _M_bkt_num_key(__key, _M_buckets.size());
	}
    template <class _KeyType>
    size_type _M_bkt_num_key_light(const _KeyType &__key) const
	{
	    return _M_bkt_num_key_light(__key, _M_buckets.size());
	}

    size_type _M_bkt_num(const value_type &__obj) const
	{
	    return _M_bkt_num_key(_M_get_key(__obj));
	}

    size_type _M_bkt_num_key(const key_type &__key, size_t __n) const
	{
	    return _M_hash(__key) % __n;
	}
    template <class _KeyType>
    size_type _M_bkt_num_key_light(const _KeyType &__key, size_t __n) const
	{
	    return _M_hash(__key) % __n;
	}

    size_type _M_bkt_num(const value_type &__obj, size_t __n) const
	{
	    return _M_bkt_num_key(_M_get_key(__obj), __n);
	}

    ptr<_Node, arch_type> _M_new_node(const value_type &__obj)
	{
	    ptr<_Node, arch_type> __n = _M_get_node();
	    ref<_Node, arch_type> __n_ref = *__n;
	    __n_ref->_M_next = ptr<_Node, arch_type>(0, _M_this.get_pheap());
      
	    __PSTL_TRY {
		pstl_construct(__n_ref->get_M_val_pointer(__n), __obj);
		return __n;
	    }
	    __PSTL_UNWIND(_M_put_node(__n))
	}
  
    void _M_delete_node(ptr<_Node, arch_type> __n)
	{
	    pstl_destroy((*__n)->get_M_val_pointer(__n));
	    _M_put_node(__n);
	}

    void _M_erase_bucket(const size_type __n,
			 ptr<_Node, arch_type> __first, ptr<_Node, arch_type> __last);
    void _M_erase_bucket(const size_type __n, ptr<_Node, arch_type> __last);

    void _M_copy_from(const hashtable &__ht);

public:
    pheap_ptr get_pheap() const {
	return _M_this.get_pheap();
    }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc>
struct replace_pheap<hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> &__ht) {
#ifdef __PSTL_USE_STD_ALLOCATORS
	__replace_pheap(__ph, __ht._M_node_allocator);
#else // __PSTL_USE_STD_ALLOCATORS
#endif // __PSTL_USE_STD_ALLOCATORS
	__replace_pheap(__ph, __ht._M_buckets);
	__replace_pheap(__ph, __ht._M_this);
    }
};


template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
	  class _Arch, class _All>
_Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All> &
_Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::operator++()
{
    const_ptr<_Node, arch_type> __old = static_cast<const_ptr<_Node, arch_type> >(_M_cur);
    _M_cur = (*_M_cur)->_M_next;
    if (!_M_cur) {
	ref<_Hashtable, arch_type> _M_ht_ref = *_M_ht;
	size_type __bucket = _M_ht_ref->_M_bkt_num((*__old)->get_M_val(__old));
	while (!_M_cur && ++__bucket < _M_ht_ref->_M_buckets.size()) {
	    _M_cur = _M_ht_ref->_M_buckets[__bucket].get_obj();
	}
    }
    return *this;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
	  class _Arch, class _All>
_Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All> &
_Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::operator++() const
{
    const_ptr<_Node, arch_type> __old = _M_cur;
    _M_cur = (*_M_cur)->_M_next;
    if (!_M_cur) {
	ref<_Hashtable, arch_type> _M_ht_ref = *_M_ht;
	size_type __bucket = _M_ht_ref->_M_bkt_num((*__old)->get_M_val(__old));
	while (!_M_cur && ++__bucket < _M_ht_ref->_M_buckets.size())
	    _M_cur = _M_ht_ref->_M_buckets[__bucket];
    }
    return *this;
}

/*
template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _Arch, class _All>
inline _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>
_Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::operator++(int)
{
    iterator __tmp = _M_this;
    ++*_M_this;
    return __tmp;
}
*/

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _Arch, class _All>
_Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All> &
_Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::operator++()
{
    const_ptr<_Node, arch_type> __old = _M_cur;
    _M_cur = (*_M_cur)->_M_next;
    if (!_M_cur) {
	ref<_Hashtable, arch_type> _M_ht_ref = *_M_ht;
	size_type __bucket = _M_ht_ref->_M_bkt_num((*__old)->get_M_val(__old));
	while (!_M_cur && ++__bucket < _M_ht_ref->_M_buckets.size()) {
	    _M_cur = _M_ht_ref->_M_buckets[__bucket];
	}
    }
    return *this;
}

/*
template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _Arch, class _All>
inline _Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>
_Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::operator++(int)
{
    const_iterator __tmp = *_M_this;
    ++*_M_this;
    return __tmp;
}
*/



template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
	  class _Arch, class _All>
inline forward_iterator_tag
iterator_category(const _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All> &)
{
    return forward_iterator_tag();
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
	  class _Arch, class _All>
inline ptr<_Val, _Arch>
value_type(const _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All> &)
{
    return ptr<_Val, _Arch>(0);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
	  class _Arch, class _All>
inline typename hashtable<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::difference_type *
distance_type(const _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All> &)
{
    return reinterpret_cast<typename hashtable<_Val, _Key, _HF, _ExK, _EqK, _Arch, _All>::difference_type *>(0);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
bool operator==(const hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All> &__ht1,
		const hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All> &__ht2)
{
    typedef typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::_Node _Node;
    if (__ht1._M_buckets.size() != __ht2._M_buckets.size()) {
	return false;
    }
    for (int __n = 0; __n < __ht1._M_buckets.size(); ++__n) {
	ptr<_Node, _Arch> __cur1 = __ht1._M_buckets[__n];
	ptr<_Node, _Arch> __cur2 = __ht2._M_buckets[__n];
	ref<_Node, _Arch> __ref1, __ref2;
	if (__cur1) {
	    __ref1.reassign(*__cur1);
	}
	if (__cur2) {
	    __ref2.reassign(*__cur2);
	}
	for ( ; __cur1 && __cur2 && __ref1->get_M_val(__cur1) == __ref2->get_M_val(__cur2); ) {
	    __cur1 = __ref1->_M_next;
	    __cur2 = __ref2->_M_next;
	    if (__cur1) {
		__ref1.reassign(*__cur1);
	    }
	    if (__cur2) {
		__ref2.reassign(*__cur2);
	    }
	}
	if (__cur1 || __cur2) {
	    return false;
	}
    }
    return true;
}  

#ifdef __STL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
inline bool operator!=(const hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All> &__ht1,
		       const hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All> &__ht2) {
    return !(__ht1 == __ht2);
}
    
template <class _Val, class _Key, class _HF, class _Extract, class _EqKey, 
	  class _Arch, class _All>
inline void swap(hashtable<_Val, _Key, _HF, _Extract, _EqKey, _Arch, _All> &__ht1,
		 hashtable<_Val, _Key, _HF, _Extract, _EqKey, _Arch, _All> &__ht2) {
    __ht1.swap(__ht2);
}

#endif /* __STL_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
inline pstl::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::iterator, bool, _Arch> 
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::insert_unique_noresize(const value_type &__obj)
{
    const size_type __n = _M_bkt_num(__obj);
    ptr<_Node, arch_type> __first = _M_buckets[__n].get_obj();
    ptr<_Node, arch_type> __cur(__first, _M_this.get_pheap());
    ref<_Node, arch_type> __cur_ref;
    if (__cur) {
	__cur_ref.reassign(*__cur);
    }
    for ( ; __cur ; ) {
	if (_M_equals(_M_get_key(__cur_ref->_M_val), _M_get_key(__obj))) {
	    return pstl::pair<iterator, bool, arch_type>(iterator(__cur, _M_this), false);
	}
	__cur = __cur_ref->_M_next;
	if (__cur) {
	    __cur_ref.reassign(*__cur);
	}
    }
    
    ptr<_Node, arch_type> __tmp = _M_new_node(__obj);
    (*__tmp)->_M_next = __first;
    _M_buckets[__n] = __tmp;
    ++_M_num_elements;
    return pstl::pair<iterator, bool, arch_type>(iterator(__tmp, _M_this), true);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
inline typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::iterator 
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::insert_equal_noresize(const value_type &__obj)
{
    const size_type __n = _M_bkt_num(__obj);
    ptr<_Node, arch_type> __first = _M_buckets[__n];
    ptr<_Node, arch_type> __cur = __first;
    ref<_Node, arch_type> __cur_ref;
    if (__cur) {
	__cur_ref.reassign(*__cur);
    }
    
    for ( ; __cur ; ) {
	if (_M_equals(_M_get_key(__cur_ref->get_M_val(__cur)), _M_get_key(__obj))) {
	    ptr<_Node, arch_type> __tmp = _M_new_node(__obj);
	    (*__tmp)->_M_next = __cur_ref->_M_next;
	    __cur_ref->_M_next = __tmp;
	    ++_M_num_elements;
	    return iterator(__tmp, _M_this);
	}
	__cur = __cur->_M_next;
	if (__cur) {
	    __cur_ref.reassign(*__cur);
	}
    }

    ptr<_Node, arch_type> __tmp = _M_new_node(__obj);
    (*__tmp)->_M_next = __first;
    _M_buckets[__n] = __tmp;
    ++_M_num_elements;
    return iterator(__tmp, _M_this);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
inline typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::reference 
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::find_or_insert(const value_type &__obj)
{
    resize(_M_num_elements + 1);

    size_type __n = _M_bkt_num(__obj);
    ptr<_Node, arch_type> __first = _M_buckets[__n];
    ptr<_Node, arch_type> __cur = __first;
    ref<_Node, arch_type> __cur_ref;
    if (__cur) {
	__cur_ref.reassign(*__cur);
    }

    for ( ; __cur ; ) {
	if (_M_equals(_M_get_key(__cur_ref->get_M_val(__cur)), _M_get_key(__obj))) {
	    return __cur_ref->get_M_val(__cur);
	}
	__cur = __cur_ref->_M_next;
	if (__cur) {
	    __cur_ref.reassign(*__cur);
	}
    }

    ptr<_Node, arch_type> __tmp = _M_new_node(__obj);
    ref<_Node, arch_type> __tmp_ref = *__tmp;
    __tmp_ref->_M_next = __first;
    _M_buckets[__n] = __tmp;
    ++_M_num_elements;
    return __tmp_ref->get_M_val(__tmp);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
pstl::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::iterator,
	   typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::iterator,
	   typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::arch_type> 
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::equal_range(const key_type &__key)
{
    typedef pstl::pair<iterator, iterator, arch_type> _Pii;
    const size_type __n = _M_bkt_num_key(__key);
    ptr<_Node, arch_type> __first = _M_buckets[__n];
    ref<_Node, arch_type> __first_ref;
    if (__first) {
	__first_ref.reassign(*__first);
    }

    for ( ; __first ; ) {
	if (_M_equals(_M_get_key(__first_ref->get_M_val(__first)), __key)) {
	    ptr<_Node, arch_type> __cur = __first_ref->_M_next;
	    ref<_Node, arch_type> __cur_ref;
	    if (__cur) {
		__cur_ref.reassign(*__cur);
	    }
	    for ( ; __cur ; ) {
		if (!_M_equals(_M_get_key(__cur_ref->get_M_val(__cur)), __key)) {
		    return _Pii(iterator(__first, _M_this), iterator(__cur, _M_this));
		}
		__cur = __cur_ref->_M_next;
		if (__cur) {
		    __cur_ref.reassign(*__cur);
		}
	    }
	    for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m) {
		ptr<_Node, arch_type> __x = _M_buckets[__m];
		if (__x) {
		    return _Pii(iterator(__first, _M_this), iterator(__x, _M_this));
		}
	    }
	    return _Pii(iterator(__first, _M_this), end());
	}
	__first = __first_ref->_M_next;
	if (__first) {
	    __first_ref.reassign(*__first);
	}
    }
    return _Pii(end(), end());
}
    
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
pstl::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::const_iterator, 
	   typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::const_iterator,
	   typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::arch_type> 
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::equal_range(const key_type &__key) const
{
    typedef pstl::pair<const_iterator, const_iterator, arch_type> _Pii;
    const size_type __n = _M_bkt_num_key(__key);

    ptr<_Node, arch_type> __first = _M_buckets[__n];
    ref<_Node, arch_type> __first_ref;
    if (__first) {
	__first_ref.reassign(*__first);
    }

    for ( ; __first ; ) {
	if (_M_equals(_M_get_key(__first_ref->get_M_val(__first)), __key)) {
	    ptr<_Node, arch_type> __cur = __first_ref->_M_next;
	    ref<_Node, arch_type> __cur_ref;
	    if (__cur) {
		__cur_ref.reassign(*__cur);
	    }
	    for ( ; __cur ; ) {
		if (!_M_equals(_M_get_key(__cur_ref->get_M_val(__cur)), __key)) {
		    return _Pii(const_iterator(__first, _M_this), const_iterator(__cur, _M_this));
		}
		__cur = __cur_ref->_M_next;
		if (__cur) {
		    __cur_ref.reassign(*__cur);
		}
	    }
	    for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m) {
		ptr<_Node, arch_type> __x = _M_buckets[__m];
		if (__x) {
		    return _Pii(const_iterator(__first, _M_this), const_iterator(__x, _M_this));
		}
	    }
	    return _Pii(const_iterator(__first, _M_this), end());
	}
	__first = __first_ref->_M_next;
	if (__first) {
	    __first_ref.reassign(*__first);
	}
    }
    return _Pii(end(), end());
}
    
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::size_type 
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::erase(const key_type &__key)
{
    const size_type __n = _M_bkt_num_key(__key);
    ptr<_Node, arch_type> __first = _M_buckets[__n].get_obj();
    ref<_Node, arch_type> __first_ref;
    if (__first) {
	__first_ref.reassign(*__first);
    }
    size_type __erased = 0;

    if (__first) {
	ptr<_Node, arch_type> __cur = __first;
	ref<_Node, arch_type> __cur_ref = *__cur;
	ptr<_Node, arch_type> __next = __cur_ref->_M_next;
	ref<_Node, arch_type> __next_ref;
	if (__next) {
	    __next_ref.reassign(*__next);
	}
	while (__next) {
	    if (_M_equals(_M_get_key(__next_ref->_M_val), __key)) {
		__cur_ref->_M_next = __next_ref->_M_next;
		_M_delete_node(__next);
		__next = __cur_ref->_M_next;
		if (__next) {
		    __next_ref.reassign(*__next);
		}
		++__erased;
		--_M_num_elements;
	    }
	    else {
		__cur = __next;
		__cur_ref.reassign(*__cur);
		__next = __cur_ref->_M_next;
		if (__next) {
		    __next_ref.reassign(*__next);
		}
	    }
	}
	if (_M_equals(_M_get_key(__first_ref->_M_val), __key)) {
	    _M_buckets[__n] = __first_ref->_M_next;
	    _M_delete_node(__first);
	    ++__erased;
	    --_M_num_elements;
	}
    }
    return __erased;
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::erase(const iterator &__it)
{
    ptr<_Node, arch_type> __p = __it._M_cur;
    if (__p) {
        ref<_Node, arch_type> __r = *__p;
	const size_type __n = _M_bkt_num(__r->_M_val);
	ptr<_Node, arch_type> __cur = _M_buckets[__n].get_obj();
	ref<_Node, arch_type> __cur_ref;
	if (__cur) {
	    __cur_ref.reassign(*__cur);
	}
	if (__cur == __p) {
	    _M_buckets[__n] = __cur_ref->_M_next;
	    _M_delete_node(__cur);
	    --_M_num_elements;
	}
	else {
	    ptr<_Node, arch_type> __next = __cur_ref->_M_next;
	    while (__next) {
		if (__next == __p) {
		    __cur_ref->_M_next = (*__next)->_M_next;
		    _M_delete_node(__next);
		    --_M_num_elements;
		    break;
		}
		else {
		    __cur = __next;
		    __cur_ref.reassign(*__cur);
		    __next = __cur_ref->_M_next;
		}
	    }
	}
    }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::erase(iterator __first, iterator __last)
{
    if (__first._M_cur == __last._M_cur) {
	return;
    }
	 
    size_type __f_bucket = __first._M_cur ?
	_M_bkt_num((*__first._M_cur)->get_M_val(__first._M_cur)) : _M_buckets.size();
    size_type __l_bucket = __last._M_cur ?
	_M_bkt_num((*__last._M_cur)->get_M_val(__last._M_cur)) : _M_buckets.size();

    if (__f_bucket == __l_bucket)
	_M_erase_bucket(__f_bucket, __first._M_cur, __last._M_cur);
    else {
	_M_erase_bucket(__f_bucket, __first._M_cur, 0);
	for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n) {
	    _M_erase_bucket(__n, 0);
	}
	if (__l_bucket != _M_buckets.size()) {
	    _M_erase_bucket(__l_bucket, __last._M_cur);
	}
    }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
inline void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::erase(const_iterator __first, const_iterator __last)
{
    erase(iterator(ptr<_Node, arch_type>(__first._M_cur.getAddress(), __first._M_cur.get_pheap()),
		   ptr<hashtable, arch_type>(__first._M_ht.getAddress(), __first._M_ht.get_pheap())),
	  iterator(ptr<_Node, arch_type>(__last._M_cur.getAddress(), __last._M_cur.get_pheap()),
		   ptr<hashtable, arch_type>(__last._M_ht.getAddress(), __last._M_cur.get_pheap())));
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
inline void
hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::erase(const const_iterator &__it)
{
    erase(iterator(ptr<_Node, arch_type>(__it._M_cur.getAddress(), __it._M_cur.get_pheap()),
		   ptr<hashtable, arch_type>(__it._M_ht.getAddress(), __it._M_ht.get_pheap())));
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::resize(size_type __num_elements_hint)
{
    const size_type __old_n = _M_buckets.size();
    if (__num_elements_hint > __old_n) {
	const size_type __n = _M_next_size(__num_elements_hint);
	if (__n > __old_n) {
	    vector<ptr<_Node, arch_type>, arch_type, allocator_type> __tmp(__n, ptr<_Node, arch_type>(0), _M_buckets.get_allocator());
	    __PSTL_TRY {
		for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
		    ref<ptr<_Node, arch_type>, arch_type> __old_ref = _M_buckets[__bucket];
		    ptr<_Node, arch_type> __first = *__old_ref;
		    ref<_Node, arch_type> __first_ref;
		    if (__first) {
			__first_ref.reassign(*__first);
		    }
		    while (__first) {
			size_type __new_bucket = _M_bkt_num(__first_ref->_M_val, __n);
			ref<ptr<_Node, arch_type>, arch_type> __new_ref = __tmp[__new_bucket];
			*__old_ref = __first_ref->_M_next;
			__first_ref->_M_next = *__new_ref;
			*__new_ref = __first;
			__first = *__old_ref;
			if (__first) {
			    __first_ref.reassign(*__first);
			}
		    }
		}
		_M_buckets.swap(__tmp);
	    }
#               ifdef __PSTL_USE_EXCEPTIONS
	    catch(...) {
		for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
		    while (__tmp[__bucket].get_obj()) {
			ptr<_Node, arch_type> __next = (*__tmp[__bucket].get_obj())->_M_next;
			_M_delete_node(__tmp[__bucket].get_obj());
			__tmp[__bucket] =  __next;
		    }
		}
		throw;
	    }
#               endif /* __PSTL_USE_EXCEPTIONS */
	}
    }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::_M_erase_bucket(const size_type __n,
		  ptr<_Node, arch_type> __first, ptr<_Node, arch_type> __last)
{
    ptr<_Node, arch_type> __cur = _M_buckets[__n];
    if (__cur == __first) {
	_M_erase_bucket(__n, __last);
    }
    else {
	ref<_Node, arch_type> __cur_ref;
	if (__cur) {
	    __cur_ref.reassign(*__cur);
	}
	ptr<_Node, arch_type> __next = __cur_ref->_M_next;
	for ( ; __next != __first; ) {
	    __cur = __next;
	    if (__cur) {
		__cur_ref.reassign(*__cur);
	    }
	    __next = __cur_ref->_M_next;
	}
	while (__next != __last) {
	    if (__next) {
		__cur_ref->_M_next = (*__next)->_M_next;
	    }
	    _M_delete_node(__next);
	    __next = __cur_ref->_M_next;
	    --_M_num_elements;
	}
    }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::_M_erase_bucket(const size_type __n, ptr<_Node, arch_type> __last)
{
    ptr<_Node, arch_type> __cur = _M_buckets[__n];
    ref<_Node, arch_type> __cur_ref;
    if (__cur) {
	__cur_ref.reassign(*__cur);
    }
    while (__cur != __last) {
	ptr<_Node, arch_type> __next = __cur_ref->_M_next;
	_M_delete_node(__cur);
	__cur = __next;
	if (__cur) {
	    __cur_ref.reassign(*__cur);
	}
	_M_buckets[__n] = __cur;
	--_M_num_elements;
    }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>::clear()
{
    pheap_ptr __ph = _M_this.get_pheap();
    for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
	ptr<_Node, arch_type> __cur = _M_buckets[__i].get_obj();
	while (__cur) {
	    ptr<_Node, arch_type> __next = (*__cur)->_M_next;
	    _M_delete_node(__cur);
	    __cur = __next;
	}
	_M_buckets[__i] = ptr<_Node, arch_type>(0, __ph);
    }
    _M_num_elements = 0;
}
    
template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _Arch, class _All>
void hashtable<_Val, _Key, _HF, _Ex, _Eq, _Arch, _All>
::_M_copy_from(const hashtable &__ht)
{
    _M_buckets.clear();
    _M_buckets.reserve(__ht._M_buckets.size());
    _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), ptr<_Node, arch_type>(0));
    __PSTL_TRY {
	for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
	    const ptr<_Node, arch_type> __cur = __ht._M_buckets[__i];
	    const ref<_Node, arch_type> __cur_ref;
	    if (__cur) {
		__cur_ref.reassign(*__cur);
	    }
	    if (__cur) {
		ptr<_Node, arch_type> __copy = _M_new_node(__cur_ref->get_M_val(__cur));
		ref<_Node, arch_type> __copy_ref = *__copy;
		_M_buckets[__i] = __copy;

		ptr<_Node, arch_type> __next = __cur_ref->_M_next;
		for ( ; __next ; ) {
		    __copy_ref->_M_next = _M_new_node((*__next)->get_M_val(__next));
		    __copy = __copy_ref->_M_next;
		    __copy_ref.reassign(*__copy);
	    
		    __cur = __next;
		    __cur_ref.reassign(*__cur);
		    __next = __cur_ref->_M_next;
		}
	    }
	}
	_M_num_elements = __ht._M_num_elements;
    }
    __PSTL_UNWIND(clear());
}

__PSTL_END_NAMESPACE

#endif // __SGI_PSTL_INTERNAL_HASHTABLE_H

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/11/22 08:12:34  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.1.2.1  2005/11/21 13:05:18  tuna
 * new-db-5 にファイル名の変更／追加を反映させました。
 *
 * Revision 1.12.2.6.4.1.2.2  2005/09/22 11:08:53  tuna
 * hash_mapのerase,pnewと、listを修正しました。
 * map, multimapを追加しました。
 * テストプログラムpstl_test_hash_map.cc, pstl_test_map.cc, pstl_test_multimap.ccを追加しました。以前のテストプログラムはpstl_bench.ccに移しました。
 *
 * Revision 1.12.2.6.4.1.2.1  2005/08/30 09:29:41  ninomi
 * turbolinuxのg++3.2.2でコンパイルできるようにしました。
 *
 * Revision 1.12.2.6.4.1  2005/08/26 05:00:02  tuna
 * 新しい形式のreplace_pheapを使うバージョンにしました。
 *
 * Revision 1.12.2.6  2005/08/16 04:18:31  ninomi
 * PSTL_without_phに対応しました。でも、旧ldbmとのインターフェース部分が
 * できていないのでコンパイルできません。
 *
 * Revision 1.12.2.5  2005/08/15 10:49:49  ninomi
 * PSTL_without_phでコンパイルできるよう奮闘中
 *
 * Revision 1.12.2.4  2005/08/12 08:57:33  ninomi
 * replace_pheapをtemplate化して、あとからinstanciationできるようにしました。
 *
 * Revision 1.12.2.3  2005/08/09 09:31:48  tuna
 * *** empty log message ***
 *
 * Revision 1.12.2.1  2005/07/15 08:49:55  tuna
 * vector_construct を廃止しました。hashtableは修正中。
 *
 * Revision 1.12  2005/05/30 02:40:19  tuna
 * const に関する整理、castを新形式に統一、その他細かい修正をしました
 *
 * Revision 1.11  2005/05/23 01:18:55  ninomi
 * push_backまで実装。バグがでている状態。
 *
 * Revision 1.10  2005/05/18 10:29:21  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.9  2005/04/04 04:25:01  ninomi
 * a light check for hash key is introduced.  ldbm-type is still under construction.
 *
 * Revision 1.8  2005/04/01 10:09:21  ninomi
 * equalやhashのめそっどの引数のためのpheapを引数に加えました。
 *
 * Revision 1.7  2005/03/25 05:59:25  ninomi
 * hash functionの第一引数にpheapをいれるようにしました。
 *
 * Revision 1.6  2004/12/10 07:04:09  ninomi
 * template variableの_Bitと_Devの順番を逆にしました
 *
 * Revision 1.5  2004/12/10 06:13:44  ninomi
 * amd64で動くようにしたらia32でだめになったので、修正。
 *
 * Revision 1.4  2004/12/10 05:56:05  ninomi
 * amd64でも動くようにしました。
 *
 * Revision 1.3  2004/12/09 07:40:39  ninomi
 * pheapのページングアルゴリズムを改良しました。
 * hash_mapとhash_multimapに専用のreferenceを加えました。
 *
 * Revision 1.2  2004/12/06 05:05:24  ninomi
 * ようやくhash_mapも新しいpage mappingに対応。
 *
 * Revision 1.1  2004/11/12 08:15:07  ninomi
 * pstlをinclude/liblilfes/pstlからinclude/pstlにうつしました。
 *
 * Revision 1.1  2004/11/12 07:32:37  ninomi
 * ldbm_*をinclude/liblilfes/pstlに移動しました。
 *
 * Revision 1.2  2004/11/08 10:37:16  ninomi
 * persistent STLのptr, ref, vector, hash_mapまで完成。まだreferenceの
 * life time処理にバグがあるかも。とりあえずcommit。
 *
 * Revision 1.1  2004/10/04 08:51:29  ninomi
 * persistent allocatorとpersistent vectorをcommit, persistent hashはこれから
 *
 */


