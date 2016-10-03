/*
 *
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
 * Copyright (c) 1996,1997
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
 * Copyright (c) 2005
 * Takashi Tsunakawa, Kenta Oouchida and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Tsunakawa, Kenta Oouchida and
 * Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

/* NOTE: This is an internal header file, included by other PSTL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_PSTL_INTERNAL_MAP_H
#define __SGI_PSTL_INTERNAL_MAP_H

#include "pstl_concept_checks.h"

__PSTL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// Forward declarations of operators == and <, needed for friend declarations.
template <class _Key, class _Tp, class _Arch, 
          class _Compare __PSTL_DEPENDENT_DEFAULT_TMPL(less<_Key>),
          class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_Tp, _Arch) >
class map;

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator==(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                       const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y);

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator<(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                      const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y);

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
class map {
public:

// requirements:

    __PSTL_CLASS_REQUIRES(_Tp, _Assignable);
    __PSTL_CLASS_BINARY_FUNCTION_CHECK(_Compare, bool, _Key, _Key);

// typedefs:

    typedef _Key                  key_type;
    typedef _Tp                   data_type;
    typedef _Tp                   mapped_type;
    typedef pstl::pair<const _Key, _Tp, _Arch> value_type;
    typedef _Compare              key_compare;

    typedef _Arch arch_type;
    typedef typename arch_type::dev_type dev_type;
    typedef typename arch_type::bit_type bit_type;
    typedef pheap<arch_type> pheap_type;
    
    class value_compare
	: public binary_function<value_type, value_type, bool> {
	friend class map<_Key, _Tp, _Arch, _Compare, _Alloc>;
    protected :
	_Compare comp __PSTL_ALIGNED_64;
	value_compare(_Compare __c) : comp(__c) {}
    public:
	bool operator()(const value_type &__x, const value_type &__y) const {
	    return comp(__x.first, __y.first);
	}
    };

private:
    typedef _Rb_tree<key_type, value_type, arch_type, 
		     _Select1st<value_type>, key_compare, _Alloc> _Rep_type;
    _Rep_type _M_t;  // red-black tree representing map
public:
    typedef typename _Rep_type::pointer pointer;
    typedef typename _Rep_type::const_pointer const_pointer;
    typedef typename _Rep_type::reference value_reference;
    typedef typename _Rep_type::const_reference const_value_reference;
    typedef typename _Rep_type::iterator iterator;
    typedef typename _Rep_type::const_iterator const_iterator;
    typedef typename _Rep_type::reverse_iterator reverse_iterator;
    typedef typename _Rep_type::const_reverse_iterator const_reverse_iterator;
    typedef typename _Rep_type::size_type size_type;
    typedef typename _Rep_type::difference_type difference_type;
    typedef typename _Rep_type::allocator_type allocator_type;

    typedef ref<map, arch_type> reference;
    typedef const_ref<map, arch_type> const_reference;
    
    // allocation/deallocation

    map() : _M_t(_Compare(), allocator_type()) {}
    explicit map(const _Compare &__comp,
		 const allocator_type &__a = allocator_type())
	: _M_t(__comp, __a) {}

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    map(_InputIterator __first, _InputIterator __last)
	: _M_t(_Compare(), allocator_type())
	{ _M_t.insert_unique(__first, __last); }

    template <class _InputIterator>
    map(_InputIterator __first, _InputIterator __last, const _Compare &__comp,
	const allocator_type &__a = allocator_type())
	: _M_t(__comp, __a) { _M_t.insert_unique(__first, __last); }
#else
    map(const_ptr<value_type, arch_type> __first, const_ptr<value_type, arch_type> __last)
	: _M_t(_Compare(), allocator_type())
	{ _M_t.insert_unique(__first, __last); }

    map(const_ptr<value_type, arch_type> __first,
	const_ptr<value_type, arch_type> __last, const _Compare &__comp,
	const allocator_type &__a = allocator_type())
	: _M_t(__comp, __a) { _M_t.insert_unique(__first, __last); }

    map(const_iterator __first, const_iterator __last)
	: _M_t(_Compare(), allocator_type()) 
	{ _M_t.insert_unique(__first, __last); }

    map(const_iterator __first, const_iterator __last, const _Compare &__comp,
	const allocator_type &__a = allocator_type())
	: _M_t(__comp, __a) { _M_t.insert_unique(__first, __last); }

#endif /* __PSTL_MEMBER_TEMPLATES */

    map(const map &__x) : _M_t(__x._M_t) {}

    // pnew
    static ptr<map, arch_type> pnew(const pheap_ptr __ph) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map();
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const _Compare &__comp) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__comp, allocator_type(__ph));
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const _Compare &__comp, const allocator_type &__a) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__comp, __a);
	return __p;
    }
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, _InputIterator __first, _InputIterator __last) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last);
	return __p;
    }
    template <class _InputIterator>
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, _InputIterator __first, _InputIterator __last, const _Compare &__comp) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last, __comp, allocator_type(__ph));
	return __p;
    }
    template <class _InputIterator>
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, _InputIterator __first, _InputIterator __last, const _Compare &__comp, const allocator_type &__a) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last, __comp, __a);
	return __p;
    }
#else
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const_ptr<value_type, arch_type> __first, const_ptr<value_type, arch_type> __last) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last);
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const_ptr<value_type, arch_type> __first, const_ptr<value_type, arch_type> __last, const _Compare &__comp) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last, __comp, allocator_type(__ph));
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const_ptr<value_type, arch_type> __first, const_ptr<value_type, arch_type> __last, const _Compare &__comp, const allocator_type &__a) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last, __comp, __a);
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const_iterator __first, const_iterator __last) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last);
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const_iterator __first, const_iterator __last, const _Compare &__comp) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last, __comp, allocator_type(__ph));
	return __p;
    }
    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const_iterator __first, const_iterator __last, const _Compare &__comp, const allocator_type &__a) {
	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
	ref<map, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) map(__first, __last, __comp, __a);
	return __p;
    }

#endif /* __PSTL_MEMBER_TEMPLATES */

    static ptr<map, arch_type> pnew(const pheap_ptr __ph, const map &__x) {
       	ptr<map, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(map)), __ph);
        ref<map, arch_type> r = *__p;
        new ((void *)&(r.get_obj())) map(__x);
	return __p;
    }
    
    map<_Key, _Tp, _Arch, _Compare, _Alloc> &
    operator=(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x)
	{
	    _M_t = __x._M_t;
	    return *this; 
	}

    // accessors:

    key_compare key_comp() const { return _M_t.key_comp(); }
    value_compare value_comp() const { return value_compare(_M_t.key_comp()); }
    allocator_type get_allocator() const { return _M_t.get_allocator(); }

    iterator begin() { return _M_t.begin(); }
    const_iterator begin() const { return _M_t.begin(); }
    iterator end() { return _M_t.end(); }
    const_iterator end() const { return _M_t.end(); }
    reverse_iterator rbegin() { return _M_t.rbegin(); }
    const_reverse_iterator rbegin() const { return _M_t.rbegin(); }
    reverse_iterator rend() { return _M_t.rend(); }
    const_reverse_iterator rend() const { return _M_t.rend(); }
    bool empty() const { return _M_t.empty(); }
    size_type size() const { return _M_t.size(); }
    size_type max_size() const { return _M_t.max_size(); }
    _Tp &operator[](const key_type &__k) {
	iterator __i = lower_bound(__k);
	// __i->first is greater than or equivalent to __k.
	if (__i == end() || key_comp()(__k, (*__i)->get_first(__i)))
	    __i = insert(__i, value_type(__k, _Tp()));
	return *(*__i)->get_second(__i);
    }
    void swap(map &__x) { _M_t.swap(__x._M_t); }

    // insert/erase

    pstl::pair<iterator, bool, arch_type> insert(const value_type &__x) 
	{ return _M_t.insert_unique(__x); }
    iterator insert(iterator position, const value_type &__x)
	{ return _M_t.insert_unique(position, __x); }
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    void insert(_InputIterator __first, _InputIterator __last) {
	_M_t.insert_unique(__first, __last);
    }
#else
    void insert(const_ptr<value_type, arch_type> __first, const_ptr<value_type, arch_type> __last) {
	_M_t.insert_unique(__first, __last);
    }
    void insert(const_iterator __first, const_iterator __last) {
	_M_t.insert_unique(__first, __last);
    }
#endif /* __PSTL_MEMBER_TEMPLATES */

    void erase(iterator __position) { _M_t.erase(__position); }
    size_type erase(const key_type &__x) { return _M_t.erase(__x); }
    void erase(iterator __first, iterator __last)
	{ _M_t.erase(__first, __last); }
    void clear() { _M_t.clear(); }

    // map operations:

    iterator find(const key_type &__x) { return _M_t.find(__x); }
    const_iterator find(const key_type &__x) const { return _M_t.find(__x); }
    size_type count(const key_type &__x) const {
	return _M_t.find(__x) == _M_t.end() ? 0 : 1; 
    }
    iterator lower_bound(const key_type &__x) {return _M_t.lower_bound(__x); }
    const_iterator lower_bound(const key_type &__x) const {
	return _M_t.lower_bound(__x); 
    }
    iterator upper_bound(const key_type &__x) {return _M_t.upper_bound(__x); }
    const_iterator upper_bound(const key_type &__x) const {
	return _M_t.upper_bound(__x); 
    }
  
    pstl::pair<iterator, iterator, arch_type> equal_range(const key_type &__x) {
	return _M_t.equal_range(__x);
    }
    pstl::pair<const_iterator, const_iterator, arch_type> equal_range(const key_type &__x) const {
	return _M_t.equal_range(__x);
    }

#ifdef __PSTL_TEMPLATE_FRIENDS 
    template <class _K1, class _T1, class _Ar1, class _C1, class _A1>
    friend bool operator== (const map<_K1, _T1, _Ar1, _C1, _A1> &,
			    const map<_K1, _T1, _Ar1, _C1, _A1> &);
    template <class _K1, class _T1, class _Ar1, class _C1, class _A1>
    friend bool operator< (const map<_K1, _T1, _Ar1, _C1, _A1> &,
			   const map<_K1, _T1, _Ar1, _C1, _A1> &);
#else /* __PSTL_TEMPLATE_FRIENDS */
    friend bool __STD_QUALIFIER
    operator== __PSTL_NULL_TMPL_ARGS (const map &, const map &);
    friend bool __STD_QUALIFIER
    operator< __PSTL_NULL_TMPL_ARGS (const map &, const map &);
#endif /* __PSTL_TEMPLATE_FRIENDS */

    template <class _Obj> friend struct replace_pheap;
};

template <class _Key, class _Tp, class _Arch,
	  class _Compare,
	  class _Alloc>
struct replace_pheap<map<_Key, _Tp, _Arch, _Compare, _Alloc> > {
    void operator()(const pheap_ptr __ph, map<_Key, _Tp, _Arch, _Compare, _Alloc> &__m) {
	__replace_pheap(__ph, __m._M_t);
    }
};

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator==(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                       const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    return __x._M_t == __y._M_t;
}

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator<(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                      const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    return __x._M_t < __y._M_t;
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator!=(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                       const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    return !(__x == __y);
}

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator>(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                      const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    return __y < __x;
}

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator<=(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                       const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    return !(__y < __x);
}

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline bool operator>=(const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                       const map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    return !(__x < __y);
}

template <class _Key, class _Tp, class _Arch, class _Compare, class _Alloc>
inline void swap(map<_Key, _Tp, _Arch, _Compare, _Alloc> &__x, 
                 map<_Key, _Tp, _Arch, _Compare, _Alloc> &__y) {
    __x.swap(__y);
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_MAP_H */

// Local Variables:
// mode:C++
// End:
