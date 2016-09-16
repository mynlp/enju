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
#ifndef __SGI_PSTL_INTERNAL_HASH_MAP_H
#define __SGI_PSTL_INTERNAL_HASH_MAP_H

#include <iostream>
#include "pstl_concept_checks.h"

__PSTL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// Forward declaration of equality operator; needed for friend declaration.

template <class _Key, class _Tp, class _Arch,
	  class _HashFcn  __PSTL_DEPENDENT_DEFAULT_TMPL(hash<_Key>),
	  class _EqualKey __PSTL_DEPENDENT_DEFAULT_TMPL(equal_to<_Key>),
	  class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_Tp, _Arch) >
class hash_map;

template <class _Key, class _Tp, class _Arch, class _HashFn, class _EqKey, class _Alloc>
inline bool operator==(const hash_map<_Key, _Tp, _Arch, _HashFn, _EqKey, _Alloc> &,
		       const hash_map<_Key, _Tp, _Arch, _HashFn, _EqKey, _Alloc> &);
    
template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc>
class hash_map
{
    // requirements:

    __PSTL_CLASS_REQUIRES(_Key, _Assignable);
    __PSTL_CLASS_REQUIRES(_Tp, _Assignable);
    __PSTL_CLASS_UNARY_FUNCTION_CHECK(_HashFcn, size_t, _Key);
    __PSTL_CLASS_BINARY_FUNCTION_CHECK(_EqualKey, bool, _Key, _Key);

private:
    typedef hashtable<pstl::pair<const _Key, _Tp, _Arch>,
		      _Key, _HashFcn,
		      _Select1st<pstl::pair<const _Key, _Tp, _Arch> >,
		      _EqualKey, _Arch, _Alloc> _Ht;

public:
    typedef typename _Ht::key_type key_type;
    typedef _Tp data_type;
    typedef _Tp mapped_type;
    typedef typename _Ht::value_type value_type;
    typedef typename _Ht::hasher hasher;
    typedef typename _Ht::key_equal key_equal;
  
    typedef typename _Ht::size_type size_type;
    typedef typename _Ht::difference_type difference_type;
    typedef typename _Ht::pointer pointer;
    typedef typename _Ht::const_pointer const_pointer;
    typedef typename _Ht::reference hashtable_reference;
    typedef typename _Ht::const_reference const_hashtable_reference;

    typedef typename _Ht::iterator iterator;
    typedef typename _Ht::const_iterator const_iterator;

    typedef typename _Ht::allocator_type allocator_type;
    typedef typename _Ht::arch_type arch_type;
    typedef typename _Ht::bit_type bit_type;
    typedef typename _Ht::dev_type dev_type;
    typedef typename _Ht::pheap_type pheap_type;

    typedef ref<hash_map, arch_type> reference;
    typedef const_ref<hash_map, arch_type> const_reference;

private:
    _Ht _M_ht;

    pointer get_M_ht_pointer(ptr<hash_map, arch_type> _M_this) {
	ptr_int _M_ht_addr = reinterpret_cast<ptr_int>(&_M_ht);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return pointer(_M_this.getAddress() + (_M_ht_addr - this_addr), _M_this.get_pheap());
    }
    const_pointer get_M_ht_pointer(ptr<hash_map, arch_type> _M_this) const {
	ptr_int _M_ht_addr = reinterpret_cast<ptr_int>(&_M_ht);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return const_pointer(_M_this.getAddress() + (_M_ht_addr - this_addr), _M_this.get_pheap());
    }
    hashtable_reference get_M_ht(ptr<hash_map, arch_type> _M_this) {
	return *get_M_ht_pointer(_M_this);
    }
    const_hashtable_reference get_M_ht(ptr<hash_map, arch_type> _M_this) const {
	return *get_M_ht_pointer(_M_this);
    }

public:
    hasher hash_funct() const { return _M_ht.hash_funct(); }
    key_equal key_eq() const { return _M_ht.key_eq(); }
    allocator_type get_allocator() const { return _M_ht.get_allocator(); }

public:
    hash_map() {}
    hash_map(ptr<hash_map, arch_type> _M_this, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a) {}
    hash_map(ptr<hash_map, arch_type> _M_this, size_type __n,
	     allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a) {}
    hash_map(ptr<hash_map, arch_type> _M_this, size_type __n, const hasher &__hf,
	     allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a) {}
    hash_map(ptr<hash_map, arch_type> _M_this, size_type __n, const hasher &__hf, const key_equal &__eql,
	     allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a) {}
    ~hash_map() {}
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    hash_map(ptr<hash_map, arch_type> _M_this, _InputIterator __f, _InputIterator __l, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    template <class _InputIterator>
    hash_map(ptr<hash_map, arch_type> _M_this, _InputIterator __f, _InputIterator __l, size_type __n, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    template <class _InputIterator>
    hash_map(ptr<hash_map, arch_type> _M_this, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    template <class _InputIterator>
    hash_map(ptr<hash_map, arch_type> _M_this, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a)
	{ _M_ht.insert_unique(__f, __l); }

#else
    hash_map(ptr<hash_map, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_iterator __f, const_iterator __l, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_iterator __f, const_iterator __l, size_type __n, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_map(ptr<hash_map, arch_type> _M_this, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a)
	{ _M_ht.insert_unique(__f, __l); }
#endif /* __PSTL_MEMBER_TEMPLATES */
    
    /// pnew
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, size_type __n) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __n, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, size_type __n, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __n, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, size_type __n, const hasher &__hf) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __n, __hf, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, size_type __n, const hasher &__hf, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __n, __hf, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, size_type __n, const hasher &__hf, const key_equal &__eql) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __n, __hf, __eql, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __n, __hf, __eql, __a);
	return __p;
    }
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, allocator_type(__ph));
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __a);
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, allocator_type(__ph));
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __a);
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, allocator_type(__ph));
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __a);
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, const key_equal &__eql) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __eql, allocator_type(__ph));
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __eql, __a);
	return __p;
    }
#else
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, const key_equal &__eql) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __eql, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __eql, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __a);
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, const key_equal &__eql) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __eql, allocator_type(__ph));
	return __p;
    }
    static ptr<hash_map, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a) {
	ptr<hash_map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_map)), __ph);
        ref<hash_map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_map(__p, __f, __l, __n, __hf, __eql, __a);
	return __p;
    }
#endif /* __PSTL_MEMBER_TEMPLATES */
    
public:
    size_type size() const { return _M_ht.size(); }
    size_type max_size() const { return _M_ht.max_size(); }
    bool empty() const { return _M_ht.empty(); }
    void swap(hash_map &__hs) { _M_ht.swap(__hs._M_ht); }

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _K1, class _T1, class _HF, class _EqK, class _Ar, class _Al>
    friend bool operator==(const hash_map<_K1, _T1, _HF, _EqK, _Ar, _Al> &,
			   const hash_map<_K1, _T1, _HF, _EqK, _Ar, _Al> &);
#else /* __PSTL_MEMBER_TEMPLATES */
    friend bool __STD_QUALIFIER
    operator== __PSTL_NULL_TMPL_ARGS (const hash_map &, const hash_map &);
#endif /* __PSTL_MEMBER_TEMPLATES */
    
    iterator begin() { return _M_ht.begin(); }
    iterator end() { return _M_ht.end(); }
    const_iterator begin() const { return _M_ht.begin(); }
    const_iterator end() const { return _M_ht.end(); }

public:
    pstl::pair<iterator, bool, arch_type> insert(const value_type &__obj)
	{ return _M_ht.insert_unique(__obj); }
      
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    void insert(_InputIterator __f, _InputIterator __l)
	{ _M_ht.insert_unique(__f, __l); }
#else
    void insert(const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l)
	{ _M_ht.insert_unique(__f, __l); }
    void insert(const_iterator __f, const_iterator __l)
	{ _M_ht.insert_unique(__f, __l); }
#endif /* __PSTL_MEMBER_TEMPLATES */

    pstl::pair<iterator, bool, arch_type> insert_noresize(const value_type &__obj)
	{ return _M_ht.insert_unique_noresize(__obj); }

    iterator find(const key_type &__key) { return _M_ht.find(__key); }
    
    template <class _KeyType>
    iterator find_light(const _KeyType &__key) { return _M_ht.find_light(__key); }
    
    const_iterator find(const key_type &__key) const
	{ return _M_ht.find(__key); }

    ref<_Tp, arch_type> operator[](const key_type &__key) {
        ref<value_type, arch_type> r = _M_ht.find_or_insert(value_type(__key, _Tp()));
	return r->get_second(&r);
    }

    size_type count(const key_type &__key) const { return _M_ht.count(__key); }
  
    pstl::pair<iterator, iterator, arch_type> equal_range(pheap_ptr ph, const key_type &__key) {
        return _M_ht.equal_range(__key);
    }
    pstl::pair<const_iterator, const_iterator, arch_type>
    equal_range(const key_type &__key) const
	{ return _M_ht.equal_range(__key); }

    size_type erase(const key_type &__key) {return _M_ht.erase(__key); }
    void erase(iterator __it) { _M_ht.erase(__it); }
    void erase(iterator __f, iterator __l) { _M_ht.erase(__f, __l); }
    void clear() { _M_ht.clear(); }

    void resize(size_type __hint) { _M_ht.resize(__hint); }
    size_type bucket_count() const { return _M_ht.bucket_count(); }
    size_type max_bucket_count() const { return _M_ht.max_bucket_count(); }
    size_type elems_in_bucket(size_type __n) const
	{ return _M_ht.elems_in_bucket(__n); }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc>
struct replace_pheap<hash_map<_Key, _Tp, _Arch, _HashFcn, _EqualKey, _Alloc> > {
    void operator()(pheap_ptr __ph, hash_map<_Key, _Tp, _Arch, _HashFcn, _EqualKey, _Alloc> &__hm) {
	__replace_pheap(__ph, __hm._M_ht);
    }
};

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Arch, class _Alloc>
inline bool
operator==(const hash_map<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm1,
	   const hash_map<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm2)
{
    return __hm1._M_ht == __hm2._M_ht;
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Arch, class _Alloc>
inline bool
operator!=(const hash_map<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm1,
	   const hash_map<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm2)
{
    return !(__hm1 == __hm2);
}

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Arch, class _Alloc>
inline void 
swap(hash_map<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm1,
     hash_map<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm2)
{
    __hm1.swap(__hm2);
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

// Forward declaration of equality operator; needed for friend declaration.

template <class _Key, class _Tp, class _Arch, 
	  class _HashFcn  __PSTL_DEPENDENT_DEFAULT_TMPL(hash<_Key>),
	  class _EqualKey __PSTL_DEPENDENT_DEFAULT_TMPL(equal_to<_Key>),
	  class _Alloc =  __PSTL_DEFAULT_ALLOCATOR(_Tp, _Arch) >
class hash_multimap;

template <class _Key, class _Tp, class _HF, class _EqKey, class _Arch, class _Alloc>
inline bool 
operator==(const hash_multimap<_Key, _Tp, _HF, _EqKey, _Arch, _Alloc> &__hm1,
	   const hash_multimap<_Key, _Tp, _HF, _EqKey, _Arch, _Alloc> &__hm2);

template <class _Key, class _Tp, class _HashFcn, class _EqualKey, class _Arch, class _Alloc>
class hash_multimap
{
    // requirements:

    __PSTL_CLASS_REQUIRES(_Key, _Assignable);
    __PSTL_CLASS_REQUIRES(_Tp, _Assignable);
    __PSTL_CLASS_UNARY_FUNCTION_CHECK(_HashFcn, size_t, _Key);
    __PSTL_CLASS_BINARY_FUNCTION_CHECK(_EqualKey, bool, _Key, _Key);

private:
    typedef hashtable<pstl::pair<const _Key, _Tp, _Arch>, _Key, _HashFcn,
		      _Select1st<pstl::pair<const _Key, _Tp, _Arch> >,
		      _EqualKey, _Arch, _Alloc> _Ht;
    
public:
    typedef typename _Ht::key_type key_type;
    typedef _Tp data_type;
    typedef _Tp mapped_type;
    typedef typename _Ht::value_type value_type;
    typedef typename _Ht::hasher hasher;
    typedef typename _Ht::key_equal key_equal;

    typedef typename _Ht::size_type size_type;
    typedef typename _Ht::difference_type difference_type;
    typedef typename _Ht::pointer pointer;
    typedef typename _Ht::const_pointer const_pointer;
    typedef typename _Ht::reference hashtable_reference;
    typedef typename _Ht::const_reference const_hashtable_reference;

    typedef typename _Ht::iterator iterator;
    typedef typename _Ht::const_iterator const_iterator;

    typedef typename _Ht::allocator_type allocator_type;
    typedef typename _Ht::arch_type arch_type;
    typedef typename _Ht::bit_type bit_type;
    typedef typename _Ht::dev_type dev_type;
    typedef typename _Ht::pheap_type pheap_type;

    typedef ref<hash_multimap, arch_type> reference;
    typedef const_ref<hash_multimap, arch_type> const_reference;

private:
    _Ht _M_ht;

    pointer get_M_ht_pointer(ptr<hash_multimap, arch_type> _M_this) {
	ptr_int _M_ht_addr = reinterpret_cast<ptr_int>(&_M_ht);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return pointer(_M_this.getAddress() + (_M_ht_addr - this_addr), _M_this.get_pheap());
    }
    const_pointer get_M_ht_pointer(ptr<hash_multimap, arch_type> _M_this) const {
	ptr_int _M_ht_addr = reinterpret_cast<ptr_int>(&_M_ht);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return const_pointer(_M_this.getAddress() + (_M_ht_addr - this_addr), _M_this.get_pheap());
    }
    hashtable_reference get_M_ht(ptr<hash_multimap, arch_type> _M_this) {
	return *get_M_ht_pointer(_M_this);
    }
    const_hashtable_reference get_M_ht(ptr<hash_multimap, arch_type> _M_this) const {
	return *get_M_ht_pointer(_M_this);
    }

public:
    hasher hash_funct() const { return _M_ht.hash_funct(); }
    key_equal key_eq() const { return _M_ht.key_eq(); }
    allocator_type get_allocator() const { return _M_ht.get_allocator(); }

public:
    hash_multimap() {}
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a) {}
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, size_type __n,
	     allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a) {}
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, size_type __n, const hasher& __hf,
	     allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a) {}
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, size_type __n, const hasher& __hf, const key_equal& __eql,
	     allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a) {}
    ~hash_multimap() {}
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, _InputIterator __f, _InputIterator __l, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    template <class _InputIterator>
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, _InputIterator __f, _InputIterator __l, size_type __n, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    template <class _InputIterator>
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    template <class _InputIterator>
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a)
	{ _M_ht.insert_unique(__f, __l); }

#else
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_iterator __f, const_iterator __l, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), 100, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_iterator __f, const_iterator __l, size_type __n, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, hasher(), key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, key_equal(), __a)
	{ _M_ht.insert_unique(__f, __l); }
    hash_multimap(ptr<hash_multimap, arch_type> _M_this, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type())
	: _M_ht(get_M_ht_pointer(_M_this), __n, __hf, __eql, __a)
	{ _M_ht.insert_unique(__f, __l); }
#endif /* __PSTL_MEMBER_TEMPLATES */
    
    /// pnew
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, size_type __n, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __n, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, size_type __n, const hasher &__hf, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __n, __hf, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __n, __hf, __eql, __a);
	return __p;
    }
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __a);
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __a);
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __hf, __a);
	return __p;
    }
    template <class _InputIterator>
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, _InputIterator __f, _InputIterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __hf, __eql, __a);
	return __p;
    }
#else
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __hf, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __hf, __eql, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __hf, __a);
	return __p;
    }
    static ptr<hash_multimap, arch_type> pnew(pheap_ptr __ph, const_iterator __f, const_iterator __l, size_type __n, const hasher &__hf, const key_equal &__eql, allocator_type __a = allocator_type()) {
	ptr<hash_multimap, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(hash_multimap)), __ph);
        ref<hash_multimap, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) hash_multimap(__p, __f, __l, __n, __hf, __eql, __a);
	return __p;
    }
#endif /* __PSTL_MEMBER_TEMPLATES */

public:
    size_type size() const { return _M_ht.size(); }
    size_type max_size() const { return _M_ht.max_size(); }
    bool empty() const { return _M_ht.empty(); }
    void swap(hash_multimap &__hs) { _M_ht.swap(__hs._M_ht); }

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _K1, class _T1, class _HF, class _EqK, class _Ar, class _Al>
    friend bool operator==(const hash_multimap<_K1, _T1, _HF, _EqK, _Ar, _Al> &,
			   const hash_multimap<_K1, _T1, _HF, _EqK, _Ar, _Al> &);
#else /* __PSTL_MEMBER_TEMPLATES */
    friend bool __STD_QUALIFIER
    operator== __PSTL_NULL_TMPL_ARGS (const hash_multimap &, const hash_multimap &);
#endif /* __PSTL_MEMBER_TEMPLATES */    
    
    iterator begin() { return _M_ht.begin(); }
    iterator end() { return _M_ht.end(); }
    const_iterator begin() const { return _M_ht.begin(); }
    const_iterator end() const { return _M_ht.end(); }

public:
    iterator insert(const value_type &__obj) 
	{ return _M_ht.insert_equal(__obj); }
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    void insert(_InputIterator __f, _InputIterator __l)
	{ _M_ht.insert_equal(__f, __l); }
#else
    void insert(const_ptr<value_type, arch_type> __f, const_ptr<value_type, arch_type> __l)
	{ _M_ht.insert_equal(__f, __l); }
    void insert(const_iterator __f, const_iterator __l)
	{ _M_ht.insert_equal(__f, __l); }
#endif /* __PSTL_MEMBER_TEMPLATES */    
    
    iterator insert_noresize(const value_type &__obj)
	{ return _M_ht.insert_equal_noresize(__obj); }

    iterator find(const key_type &__key) { return _M_ht.find(__key); }

    template <class _KeyType>
    iterator find_light(const _KeyType &__key) { return _M_ht.find_light(__key); }
    
    const_iterator find(const key_type &__key) const 
	{ return _M_ht.find(__key); }

    size_type count(const key_type &__key) const { return _M_ht.count(__key); }
  
    pstl::pair<iterator, iterator, arch_type> equal_range(const key_type &__key) { return _M_ht.equal_range(__key); }
      
    pstl::pair<const_iterator, const_iterator, arch_type> equal_range(const key_type &__key) const
	{ return _M_ht.equal_range(__key); }

    size_type erase(const key_type &__key) {return _M_ht.erase(__key); }
    void erase(iterator __it) { _M_ht.erase(__it); }
    void erase(iterator __f, iterator __l) { _M_ht.erase(__f, __l); }
    void clear() { _M_ht.clear(); }

public:
    void resize(size_type __hint) { _M_ht.resize(__hint); }
    size_type bucket_count() const { return _M_ht.bucket_count(); }
    size_type max_bucket_count() const { return _M_ht.max_bucket_count(); }
    size_type elems_in_bucket(size_type __n) const { return _M_ht.elems_in_bucket(__n); }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc>
struct replace_pheap<hash_multimap<_Key, _Tp, _Arch, _HashFcn, _EqualKey, _Alloc> > {
    void operator()(pheap_ptr __ph, hash_multimap<_Key, _Tp, _Arch, _HashFcn, _EqualKey, _Alloc> &__hm) {
	__replace_pheap(__ph, __hm._M_ht);
    }
};

template <class _Key, class _Tp, class _HF, class _EqKey, class _Arch, class _Alloc>
inline bool 
operator==(const hash_multimap<_Key, _Tp, _HF, _EqKey, _Arch, _Alloc> &__hm1,
	   const hash_multimap<_Key, _Tp, _HF, _EqKey, _Arch, _Alloc> &__hm2)
{
    return __hm1._M_ht == __hm2._M_ht;
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Key, class _Tp, class _HF, class _EqKey, class _Arch, class _Alloc>
inline bool 
operator!=(const hash_multimap<_Key, _Tp, _HF, _EqKey, _Arch, _Alloc> &__hm1,
	   const hash_multimap<_Key, _Tp, _HF, _EqKey, _Arch, _Alloc> &__hm2) {
    return !(__hm1 == __hm2);
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Key, class _Tp, class _HashFcn, class _EqlKey, class _Arch, class _Alloc>
inline void 
swap(hash_multimap<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm1,
     hash_multimap<_Key, _Tp, _HashFcn, _EqlKey, _Arch, _Alloc> &__hm2)
{
    __hm1.swap(__hm2);
}


// Specialization of insert_iterator so that it will work for hash_map
// and hash_multimap.

#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION

template <class _Key, class _Tp, class _HashFn, class _EqKey, class _Arch, class _Alloc>
class insert_iterator<hash_map<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> > {
protected:
    typedef _Arch               arch_type;
    typedef hash_map<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> _Container;
    ptr<_Container, arch_type> container;
public:
    typedef _Container          container_type;
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    insert_iterator(_Container &__x) { (*container).get_obj() = __x; }
    insert_iterator(_Container &__x, typename _Container::iterator)
	{ (*container).get_obj() = __x; }
    insert_iterator<_Container> &
    operator=(const typename _Container::value_type &__value) { 
	(*container)->insert(__value);
	return *this;
    }
    insert_iterator<_Container> &operator*() { return *this; }
    insert_iterator<_Container> &operator++() { return *this; }
    //insert_iterator<_Container> &operator++(int) { return *this; }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Key, class _Tp, class _HashFn, class _EqKey, class _Arch, class _Alloc>
struct replace_pheap<insert_iterator<hash_map<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> > > {
    void operator()(pheap_ptr __ph, insert_iterator<hash_map<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> > &__it) {
	__replace_pheap(__ph, __it.container);
    }
};

template <class _Key, class _Tp, class _HashFn, class _EqKey, class _Arch, class _Alloc>
class insert_iterator<hash_multimap<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> > {
protected:
    typedef _Arch               arch_type;
    typedef hash_multimap<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> _Container;
    ptr<_Container, arch_type> container;
    typename _Container::iterator iter;
public:
    typedef _Container          container_type;
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;
    
    insert_iterator(_Container &__x) { (*container).get_obj() = __x; }
    insert_iterator(_Container &__x, typename _Container::iterator)
	{ (*container).get_obj() = __x; }
    insert_iterator<_Container> &
    operator=(const typename _Container::value_type &__value) { 
	(*container)->insert(__value);
	return *this;
    }
    insert_iterator<_Container> &operator*() { return *this; }
    insert_iterator<_Container> &operator++() { return *this; }
    //insert_iterator<_Container> &operator++(int) { return *this; }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Key, class _Tp, class _HashFn, class _EqKey, class _Arch, class _Alloc>
struct replace_pheap<insert_iterator<hash_multimap<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> > > {
    void operator()(pheap_ptr __ph, insert_iterator<hash_multimap<_Key, _Tp, _HashFn, _EqKey, _Arch, _Alloc> > &__it) {
	__replace_pheap(__ph, __it.container);
    }
};

#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__PSTL_END_NAMESPACE

#endif // __SGI_PSTL_BEGIN_NAMESPACE

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/11/22 08:12:34  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.1.2.1  2005/11/21 13:05:18  tuna
 * new-db-5 にファイル名の変更／追加を反映させました。
 *
 * Revision 1.11.2.5.4.1.2.2  2005/09/22 11:08:53  tuna
 * hash_mapのerase,pnewと、listを修正しました。
 * map, multimapを追加しました。
 * テストプログラムpstl_test_hash_map.cc, pstl_test_map.cc, pstl_test_multimap.ccを追加しました。以前のテストプログラムはpstl_bench.ccに移しました。
 *
 * Revision 1.11.2.5.4.1.2.1  2005/08/30 09:29:41  ninomi
 * turbolinuxのg++3.2.2でコンパイルできるようにしました。
 *
 * Revision 1.11.2.5.4.1  2005/08/26 05:00:02  tuna
 * 新しい形式のreplace_pheapを使うバージョンにしました。
 *
 * Revision 1.11.2.5  2005/08/18 08:12:17  ninomi
 * list, hash_map, vectorをpnewでつくるときや、constuctorを呼出すときは
 * list::allocator_type(ph)を渡さないといけないようにしました。
 *
 * Revision 1.11.2.4  2005/08/16 04:18:31  ninomi
 * PSTL_without_phに対応しました。でも、旧ldbmとのインターフェース部分が
 * できていないのでコンパイルできません。
 *
 * Revision 1.11.2.3  2005/08/15 10:49:49  ninomi
 * PSTL_without_phでコンパイルできるよう奮闘中
 *
 * Revision 1.11.2.2  2005/08/09 09:31:48  tuna
 * *** empty log message ***
 *
 * Revision 1.11  2005/05/30 02:40:19  tuna
 * const に関する整理、castを新形式に統一、その他細かい修正をしました
 *
 * Revision 1.10  2005/05/20 06:11:35  tuna
 * const に関係する追加 (const_ptr, const_ref) とその他の修正
 *
 * Revision 1.9  2005/04/22 06:54:34  tuna
 * list, pair, iterator_base are added.
 * alloc, hash_map, pheap, test and vector are a bit modified.
 *
 * Revision 1.8  2005/04/04 04:25:01  ninomi
 * a light check for hash key is introduced.  ldbm-type is still under construction.
 *
 * Revision 1.7  2005/04/01 10:09:21  ninomi
 * equalやhashのめそっどの引数のためのpheapを引数に加えました。
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
