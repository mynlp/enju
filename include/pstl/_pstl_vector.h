// -*- C++ -*-
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

/* NOTE: This is an internal header file, included by other PSTL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_PSTL_INTERNAL_VECTOR_H
#define __SGI_PSTL_INTERNAL_VECTOR_H

#include "pstl_concept_checks.h"

__PSTL_BEGIN_NAMESPACE 

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

// The vector base class serves two purposes.  First, its constructor
// and destructor allocate (but don't initialize) storage.  This makes
// exception safety easier.  Second, the base class encapsulates all of
// the differences between SGI-style allocators and standard-conforming
// allocators.

#ifdef __PSTL_USE_STD_ALLOCATORS

// Base class for ordinary allocators.
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic>
class _Vector_alloc_base {
public:
    typedef typename _Alloc_traits<_Tp, _Arch, _Allocator>::allocator_type
    allocator_type;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::difference_type difference_type;
    typedef _Arch                                    arch_type;
    typedef typename allocator_type::dev_type        dev_type;
    typedef typename allocator_type::bit_type        bit_type;
    typedef _Tp                                      value_type;
    typedef forward_iterator_tag                     iterator_category;
    typedef pheap<arch_type>                         pheap_type;
    allocator_type get_allocator() const { return _M_data_allocator; }

    _Vector_alloc_base(const allocator_type &__a)
	: _M_data_allocator(__a), _M_start(0, __a.get_pheap()), _M_finish(0, __a.get_pheap()), _M_end_of_storage(0, __a.get_pheap()) 
	{}
  
protected:
    allocator_type _M_data_allocator;
    ptr<_Tp, arch_type> _M_start;
    ptr<_Tp, arch_type> _M_finish;
    ptr<_Tp, arch_type> _M_end_of_storage;

    ptr<_Tp, arch_type> _M_allocate(size_type __n)
	{ return _M_data_allocator.allocate(__n); }
    void _M_deallocate(ptr<_Tp, arch_type> __p, size_type __n)
	{ if (__p) _M_data_allocator.deallocate(__p, __n); }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Allocator, bool _IsStatic>
struct replace_pheap<_Vector_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> > {
    void operator()(pheap_ptr __ph, _Vector_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> &__obj) {
	__replace_pheap(__ph, __obj._M_data_allocator);
	__replace_pheap(__ph, __obj._M_start);
	__replace_pheap(__ph, __obj._M_finish);
	__replace_pheap(__ph, __obj._M_end_of_storage);
    }
};

// Specialization for allocators that have the property that we don't
// actually have to store an allocator object.  
template <class _Tp, class _Arch, class _Allocator>
class _Vector_alloc_base<_Tp, _Arch, _Allocator, true> {
public:
    typedef typename _Alloc_traits<_Tp, _Arch, _Allocator>::allocator_type
    allocator_type;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::difference_type difference_type;
    typedef _Arch                                    arch_type;
    typedef typename allocator_type::dev_type        dev_type;
    typedef typename allocator_type::bit_type        bit_type;
    typedef _Tp                                      value_type;
    typedef forward_iterator_tag                     iterator_category;
    typedef pheap<arch_type>                         pheap_type;
    allocator_type get_allocator() const { return allocator_type(_M_start.get_pheap()); }

    _Vector_alloc_base(const allocator_type &__a)
	: _M_start(0, __a.get_pheap()), _M_finish(0, __a.get_pheap()),
	  _M_end_of_storage(0, __a.get_pheap()) 
	{}
  
protected:
    ptr<_Tp, arch_type> _M_start;
    ptr<_Tp, arch_type> _M_finish;
    ptr<_Tp, arch_type> _M_end_of_storage;

    typedef typename _Alloc_traits<_Tp, arch_type, _Allocator>::_Alloc_type
    _Alloc_type;
    ptr<_Tp, arch_type> _M_allocate(size_type __n) {
	return _Alloc_type::allocate(__n);
    }
    void _M_deallocate(ptr<_Tp, arch_type> __p, size_type __n) {
	_Alloc_type::deallocate(__p, __n);
    }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Allocator>
struct replace_pheap<_Vector_alloc_base<_Tp, _Arch, _Allocator, true> > {
    void operator()(pheap_ptr __ph, _Vector_alloc_base<_Tp, _Arch, _Allocator, true> &__obj) {
	__replace_pheap(__ph, __obj._M_start);
	__replace_pheap(__ph, __obj._M_finish);
	__replace_pheap(__ph, __obj._M_end_of_storage);
    }
};

template <class _Tp, class _Arch, class _Alloc>
struct _Vector_base
    : public _Vector_alloc_base<_Tp, _Arch, _Alloc, _Alloc_traits<_Tp, _Arch, _Alloc>::_S_instanceless>
{
    typedef _Vector_alloc_base<_Tp, _Arch, _Alloc, _Alloc_traits<_Tp, _Arch, _Alloc>::_S_instanceless>
    _Base;
    typedef typename _Base::allocator_type           allocator_type;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::difference_type difference_type;
    typedef _Arch                                    arch_type;
    typedef typename allocator_type::dev_type        dev_type;
    typedef typename allocator_type::bit_type        bit_type;
    typedef _Tp                                      value_type;
    typedef typename _Base::iterator_category        iterator_category;
    typedef pheap<arch_type>                         pheap_type;

    _Vector_base(const allocator_type &__a) : _Base(__a) {}
    _Vector_base(size_type __n, const allocator_type &__a) : _Base(__a) {
	this->_M_start = this->_M_allocate(__n);
	this->_M_finish = this->_M_start;
	this->_M_end_of_storage = this->_M_start + __n;
    }

    ~_Vector_base() {
	//this->_M_deallocate(this->_M_start, this->_M_end_of_storage - this->_M_start);
    }
#ifdef __PSTL_HAS_NAMESPACES
protected:
    using _Base::_M_allocate;
    using _Base::_M_deallocate;
    using _Base::_M_start;
    using _Base::_M_finish;
    using _Base::_M_end_of_storage;
#endif /* __PSTL_HAS_NAMESPACES */
    
    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<_Vector_base<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _Vector_base<_Tp, _Arch, _Alloc> &__obj) {
	typedef _Vector_alloc_base<_Tp, _Arch, _Alloc, _Alloc_traits<_Tp, _Arch, _Alloc>::_S_instanceless> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

#else /* __PSTL_USE_STD_ALLOCATORS */

template <class _Tp, class _Arch, class _Alloc> 
class _Vector_base {
public:
    typedef _Alloc                                   allocator_type;
    typedef typename allocator_type::size_type       size_type;
    typedef typename allocator_type::pointer         pointer;
    typedef typename allocator_type::difference_type difference_type;
    typedef _Arch                                    arch_type;
    typedef typename allocator_type::dev_type        dev_type;
    typedef typename allocator_type::bit_type        bit_type;
    typedef _Tp                                      value_type;
    typedef forward_iterator_tag                     iterator_category;
    typedef pheap<arch_type>                         pheap_type;

    allocator_type get_allocator() const { return allocator_type(_M_start.get_pheap()); }

    _Vector_base(const _Alloc &__a)
	: _M_start(0, __a.get_pheap()), _M_finish(0, __a.get_pheap()),
	  _M_end_of_storage(0, __a.get_pheap()) {}
    _Vector_base(size_type __n, const _Alloc &__a)
	: _M_start(0, __a.get_pheap()), _M_finish(0, __a.get_pheap()),
	  _M_end_of_storage(0, __a.get_pheap()) 
	{
	    _M_start = _M_allocate(__n);
	    _M_finish = _M_start;
	    _M_end_of_storage = _M_start + __n;
	}

    ~_Vector_base() {
	_M_deallocate(_M_start, _M_end_of_storage - _M_start);
    }

protected:
    ptr<_Tp, arch_type> _M_start;
    ptr<_Tp, arch_type> _M_finish;
    ptr<_Tp, arch_type> _M_end_of_storage;

    typedef simple_alloc<_Tp, arch_type, _Alloc> _M_data_allocator;
    ptr<_Tp, arch_type> _M_allocate(size_type __n) {
	return _M_data_allocator::allocate(__n);
    }
    void _M_deallocate(ptr<_Tp, arch_type> __p, size_type __n) {
	if (__p) _M_data_allocator::deallocate(__p. __n);
    }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<_Vector_base<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _Vector_base<_Tp, _Arch, _Alloc> &__obj) {
	__replace_pheap(__ph, __obj._M_start);
	__replace_pheap(__ph, __obj._M_finish);
	__replace_pheap(__ph, __obj._M_end_of_storage);
    }
};

#endif /* __PSTL_USE_STD_ALLOCATORS */

//template <class _Tp, class _Arch, class _Alloc> class vector_reference;
//template <class _Tp, class _Arch, class _Alloc> class const_vector_reference;

template <class _Tp, class _Arch, class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_Tp, _Arch) >
class vector : public _Vector_base<_Tp, _Arch, _Alloc> 
{
    // requirements:
    __PSTL_CLASS_REQUIRES(_Tp, _Assignable);

private:
    typedef _Vector_base<_Tp, _Arch, _Alloc> _Base;
public:
    typedef _Tp                                       value_type;
    typedef typename _Base::allocator_type            allocator_type;
    typedef typename allocator_type::size_type        size_type;
    typedef typename allocator_type::difference_type  difference_type;
    typedef _Arch                                     arch_type;
    typedef typename allocator_type::dev_type         dev_type;
    typedef typename allocator_type::bit_type         bit_type;
    typedef typename _Base::iterator_category         iterator_category;
    typedef ptr<vector, arch_type>                    pointer;
    typedef const_ptr<vector, arch_type>              const_pointer;
    typedef ptr<value_type, arch_type>                iterator;
    typedef const_ptr<value_type, arch_type>          const_iterator;
    typedef ref<vector, arch_type>                    reference;
    typedef const_ref<vector, arch_type>              const_reference;
    typedef pheap<arch_type>                          pheap_type;
    typedef ref<value_type, arch_type>                value_reference;
    typedef const_ref<value_type, arch_type>          const_value_reference;

    allocator_type get_allocator() const { return _Base::get_allocator(); }

#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
    typedef reverse_iterator<const_iterator> const_reverse_iterator;
    typedef reverse_iterator<iterator>       reverse_iterator;
#else /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
    typedef reverse_iterator<const_iterator, value_type, arch_type,
			     const_value_reference, difference_type>
    const_reverse_iterator;
    typedef reverse_iterator<iterator, value_type, arch_type,
			     value_reference, difference_type>
    reverse_iterator;
#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

#ifdef __PSTL_HAS_NAMESPACES
protected:
    using _Base::_M_allocate;
    using _Base::_M_deallocate;
    using _Base::_M_start;
    using _Base::_M_finish;
    using _Base::_M_end_of_storage;
#endif /* __PSTL_HAS_NAMESPACES */

protected:
    void _M_insert_aux(iterator __position, const _Tp &__x);
    void _M_insert_aux(iterator __position);

public:
    iterator begin() { return _M_start; }
    const_iterator begin() const { return _M_start; }
    iterator end() { return _M_finish; }
    const_iterator end() const { return _M_finish; }

    reverse_iterator rbegin()
	{ return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const
	{ return const_reverse_iterator(end()); }
    reverse_iterator rend()
	{ return reverse_iterator(begin()); }
    const_reverse_iterator rend() const
	{ return const_reverse_iterator(begin()); }

    size_type size() const
	{ return size_type(_M_finish - _M_start); }
    size_type max_size() const
	{ return size_type(-1) / sizeof(_Tp); }
    size_type capacity() const
	{ return size_type(_M_end_of_storage - _M_start); }
    bool empty() const
	{ return begin() == end(); }

    value_reference operator[](size_type __n) { return (begin() + __n).get_ref(); }
    const_value_reference operator[](size_type __n) const { return (begin() + __n).get_ref(); }

#ifdef __PSTL_THROW_RANGE_ERRORS
    void _M_range_check(size_type __n) const {
	if (__n >= this->size())
	    __stl_throw_range_error("vector");
    }

    value_reference at(size_type __n)
	{ _M_range_check(__n); return (*this)[__n]; }
    const_value_reference at(size_type __n) const
	{ _M_range_check(__n); return (*this)[__n]; }
#endif /* __PSTL_THROW_RANGE_ERRORS */

    vector() : _Base(allocator_type()) {
	throw null_ph_construction();
    }
    
    explicit vector(pheap_ptr __ph)
	: _Base(allocator_type(__ph)) {}

    explicit vector(const allocator_type &__a)
	: _Base(__a) {}

    vector(pheap_ptr __ph, size_type __n, const _Tp &__value)
	: _Base(__n, allocator_type(__ph))
	{ _M_finish = pstl_uninitialized_fill_n(_M_start, __n, __value); }

    vector(size_type __n, const _Tp &__value, const allocator_type &__a)
	: _Base(__n, __a)
	{ _M_finish = pstl_uninitialized_fill_n(_M_start, __n, __value); }

    vector(pheap_ptr __ph, size_type __n)
	: _Base(__n, allocator_type(__ph))
	{ _M_finish = pstl_uninitialized_fill_n(_M_start, __n, _Tp()); }

    vector(const vector &__x) 
	: _Base(__x.size(), __x.get_allocator())
	{ _M_finish = pstl_uninitialized_copy(__x.begin(), __x.end(), _M_start); }

#ifdef __PSTL_MEMBER_TEMPLATES
    // Check whether it's an integral type.  If so, it's not an iterator.
    template <class _InputIterator>
    vector(pheap_ptr __ph, _InputIterator __first, _InputIterator __last)
	: _Base(allocator_type(__ph)) {
	typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
	_M_initialize_aux(__first, __last, _Integral());
    }

    template <class _InputIterator>
    vector(_InputIterator __first, _InputIterator __last,
	   const allocator_type &__a) : _Base(__a) {
	typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
	_M_initialize_aux(__first, __last, _Integral());
    }

    template <class _Integer>
    void _M_initialize_aux(_Integer __n, _Integer __value, __true_type, pheap_ptr __ph = 0) {
	_M_start = _M_allocate(__n);
	_M_end_of_storage = _M_start + __n; 
	_M_finish = pstl_uninitialized_fill_n(_M_start, __n, __value);
    }

    template <class _InputIterator>
    void _M_initialize_aux(_InputIterator __first, _InputIterator __last,
			   __false_type) {
	_M_range_initialize(__first, __last, __ITERATOR_CATEGORY(__first));
    }

#else
    vector(pheap_ptr __ph,
	   const_ptr<_Tp, arch_type> __first, const_ptr<_Tp, arch_type> __last)
	: _Base(__last - __first, allocator_type(__ph)) 
	{ _M_finish = pstl_uninitialized_copy(__first, __last, _M_start); }

    vector(const_ptr<_Tp, arch_type> __first, const_ptr<_Tp, arch_type> __last,
	   const allocator_type &__a)
	: _Base(__last - __first, __a) 
	{ _M_finish = pstl_uninitialized_copy(__first, __last, _M_start); }
#endif /* __PSTL_MEMBER_TEMPLATES */
    
    ~vector() {
	pstl_destroy(_M_start, _M_finish);
    }

public:
    static ptr<vector, arch_type> pnew(pheap_ptr __ph) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__ph);
	return __p;
    }
    static ptr<vector, arch_type> pnew(pheap_ptr __ph, const allocator_type &__a) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__a);
	return __p;
    }
    static ptr<vector, arch_type> pnew(pheap_ptr __ph,
				       size_type __n, const _Tp &__value) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__ph, __n, __value);
	return __p;
    }
    static ptr<vector, arch_type> pnew(pheap_ptr __ph,
				       size_type __n, const _Tp &__value,
				       const allocator_type &__a) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__n, __value, __a);
	return __p;
    }
#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    static ptr<vector, arch_type> pnew(pheap_ptr __ph,
				       _InputIterator __first,
				       _InputIterator __last) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__ph, __first, __last);
	return __p;
    }
    template <class _InputIterator>
    static ptr<vector, arch_type> pnew(pheap_ptr __ph,
				       _InputIterator __first,
				       _InputIterator __last,
				       const allocator_type &__a) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__first, __last, __a);
	return __p;
    }
#else
    static ptr<vector, arch_type> pnew(pheap_ptr __ph,
				       const_ptr<_Tp, arch_type> __first,
				       const_ptr<_Tp, arch_type> __last) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__ph, __first, __last);
	return __p;
    }
    static ptr<vector, arch_type> pnew(pheap_ptr __ph,
				       const_ptr<_Tp, arch_type> __first,
				       const_ptr<_Tp, arch_type> __last,
				       const allocator_type &__a) {
	ptr<vector, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(vector)), __ph);
        ref<vector, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) vector(__first, __last, __a);
	return __p;
    }
#endif /* __PSTL_MEMBER_TEMPLATES */

    vector &operator=(const vector &__x);
    void reserve(size_type __n) {
	if (capacity() < __n) {
	    const size_type __old_size = size();
	    iterator __tmp = _M_allocate_and_copy(__n, _M_start, _M_finish);
	    pstl_destroy(_M_start, _M_finish);
	    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
	    _M_start = __tmp;
	    _M_finish = __tmp + __old_size;
	    _M_end_of_storage = _M_start + __n;
	}
    }

    // assign(), a generalized assignment member function.  Two
    // versions: one that takes a count, and one that takes a range.
    // The range version is a member template, so we dispatch on whether
    // or not the type is an integer.

    void assign(size_type __n, const _Tp &__val) { _M_fill_assign(__n, __val); }
    void _M_fill_assign(size_type __n, const _Tp &__val);

#ifdef __PSTL_MEMBER_TEMPLATES
  
    template <class _InputIterator>
    void assign(_InputIterator __first, _InputIterator __last) {
	typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
	_M_assign_dispatch(__first, __last, _Integral());
    }

    template <class _Integer>
    void _M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
	{ _M_fill_assign(static_cast<size_type>(__n), static_cast<_Tp>(__val)); }

    template <class _InputIter>
    void _M_assign_dispatch(_InputIter __first, _InputIter __last, __false_type)
	{ _M_assign_aux(__first, __last, __ITERATOR_CATEGORY(__first)); }

    template <class _InputIterator>
    void _M_assign_aux(_InputIterator __first, _InputIterator __last,
		       input_iterator_tag);

    template <class _ForwardIterator>
    void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
		       forward_iterator_tag); 

#endif /* __PSTL_MEMBER_TEMPLATES */

    value_reference front() { return *begin(); }
    const_value_reference front() const { return *begin(); }
    value_reference back() { return *(end() - 1); }
    const_value_reference back() const { return *(end() - 1); }

    void push_back(const _Tp &__x) {
	if (_M_finish != _M_end_of_storage) {
            pstl_construct(_M_finish, __x);
            ++_M_finish;
	}
	else {
	    _M_insert_aux(end(), __x);
	}
    }
    void push_back() {
	if (_M_finish != _M_end_of_storage) {
	    pstl_construct(_M_finish);
	    ++_M_finish;
	}
	else {
	    _M_insert_aux(end());
	}
    }
    void swap(vector &__x) {
	pstl_swap(_M_start.getAddressRef(), __x._M_start.getAddressRef());
	pstl_swap(_M_finish.getAddressRef(), __x._M_finish.getAddressRef());
	pstl_swap(_M_end_of_storage.getAddressRef(), __x._M_end_of_storage.getAddressRef());
    }

    iterator insert(iterator __position, const _Tp &__x) {
	size_type __n = __position - begin();
	if (_M_finish != _M_end_of_storage && __position == end()) {
	    pstl_construct(_M_finish, __x);
	    ++_M_finish;
	}
	else {
	    _M_insert_aux(__position, __x);
	}
	return begin() + __n;
    }
    iterator insert(iterator __position) {
	size_type __n = __position - begin();
	if (_M_finish != _M_end_of_storage && __position == end()) {
	    pstl_construct(_M_finish);
	    ++_M_finish;
	}
	else {
	    _M_insert_aux(__position);
	}
	return begin() + __n;
    }
#ifdef __PSTL_MEMBER_TEMPLATES
    // Check whether it's an integral type.  If so, it's not an iterator.
    template <class _InputIterator>
    void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
	typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
	_M_insert_dispatch(__pos, __first, __last, _Integral());
    }

    template <class _Integer>
    void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __val,
			    __true_type)
	{ _M_fill_insert(__pos, static_cast<size_type>(__n), static_cast<_Tp>(__val)); }

    template <class _InputIterator>
    void _M_insert_dispatch(iterator __pos,
			    _InputIterator __first, _InputIterator __last,
			    __false_type) {
	_M_range_insert(__pos, __first, __last, __ITERATOR_CATEGORY(__first));
    }
#else /* __PSTL_MEMBER_TEMPLATES */
    void insert(iterator __position,
		const_iterator __first, const_iterator __last);
#endif /* __PSTL_MEMBER_TEMPLATES */

    void insert(iterator __pos, size_type __n, const _Tp &__x)
	{ _M_fill_insert(__pos, __n, __x); }

    void _M_fill_insert(iterator __pos, size_type __n, const _Tp &__x);

    void pop_back() {
	--_M_finish;
	pstl_destroy(_M_finish);
    }
    iterator erase(iterator __position) {
	if (__position + 1 != end())
	    pstl_copy(__position + 1, _M_finish, __position);
	--_M_finish;
	pstl_destroy(_M_finish);
	return __position;
    }
    iterator erase(iterator __first, iterator __last) {
	iterator __i = pstl_copy(__last, _M_finish, __first);
	pstl_destroy(__i, _M_finish);
	_M_finish = _M_finish - (__last - __first);
	return __first;
    }

    void resize(size_type __new_size, const _Tp &__x) {
	if (__new_size < size()) 
	    erase(begin() + __new_size, end());
	else
	    insert(end(), __new_size - size(), __x);
    }
    void resize(size_type __new_size) { resize(__new_size, _Tp()); }
    void clear() { erase(begin(), end()); }

protected:

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _ForwardIterator>
    iterator _M_allocate_and_copy(size_type __n, _ForwardIterator __first, 
				  _ForwardIterator __last)
	{
	    iterator __result = _M_allocate(__n);
	    __PSTL_TRY {
		pstl_uninitialized_copy(__first, __last, __result);
		return __result;
	    }
	    __PSTL_UNWIND(_M_deallocate(__result, __n));
	}
#else /* __PSTL_MEMBER_TEMPLATES */
    iterator _M_allocate_and_copy(size_type __n, const_iterator __first, 
				  const_iterator __last)
	{
	    iterator __result = _M_allocate(__n);
	    __PSTL_TRY {
		pstl_uninitialized_copy(__first, __last, __result);
		return __result;
	    }
	    __PSTL_UNWIND(_M_deallocate(__result, __n));
	}
#endif /* __PSTL_MEMBER_TEMPLATES */


#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _InputIterator>
    void _M_range_initialize(_InputIterator __first,  
			     _InputIterator __last, input_iterator_tag)
	{
	    for ( ; __first != __last; ++__first) {
		value_reference __first_ref = __first.get_ref();
		push_back(__first_ref.get_obj());
	    }
	}

    // This function is only called by the constructor. 
    template <class _ForwardIterator>
    void _M_range_initialize(_ForwardIterator __first,
			     _ForwardIterator __last, forward_iterator_tag,
			     pheap_ptr __ph)
	{
	    size_type __n = 0;
	    distance(__first, __last, __n);
	    _M_start = _M_allocate(__n, __ph);
	    _M_end_of_storage = _M_start + __n;
	    _M_finish = pstl_uninitialized_copy(__first, __last, _M_start);
	}

    template <class _InputIterator>
    void _M_range_insert(iterator __pos,
			 _InputIterator __first, _InputIterator __last,
			 input_iterator_tag);

    template <class _ForwardIterator>
    void _M_range_insert(iterator __pos,
			 _ForwardIterator __first, _ForwardIterator __last,
			 forward_iterator_tag);

#endif /* __PSTL_MEMBER_TEMPLATES */
    void validcheck() {}
//     void validcheck() {
// 	for (size_type i = 0; i < size(); i++) {
// 	    if (static_cast<int>(i) != (*this)[i]) {
// 		for (size_type j = 0; j < size(); j++) {
// 		    std::cerr << "ERROR MODE: vec[" << j << "] (" << std::hex << (&(*this)[j]).getAddress() << std::dec << ") = " << (*this)[j].get_obj() << std::endl;
// 		}
// 		exit(01);
// 	    }
// 	}
//     }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<vector<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, vector<_Tp, _Arch, _Alloc> &__obj) {
	typedef _Vector_base<_Tp, _Arch, _Alloc> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

template <class _Tp, class _Arch, class _Alloc>
inline bool 
operator==(const vector<_Tp, _Arch, _Alloc> &__x, const vector<_Tp, _Arch, _Alloc> &__y)
{
    return __x.size() == __y.size() &&
	equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Tp, class _Arch, class _Alloc>
inline bool 
operator<(const vector<_Tp, _Arch, _Alloc> &__x, const vector<_Tp, _Arch, _Alloc> &__y)
{
    return pstl_lexicographical_compare(__x.begin(), __x.end(), 
					__y.begin(), __y.end());
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Tp, class _Arch, class _Alloc>
inline void swap(vector<_Tp, _Arch, _Alloc> &__x, vector<_Tp, _Arch, _Alloc> &__y)
{
    __x.swap(__y);
}

template <class _Tp, class _Arch, class _Alloc>
inline bool
operator!=(const vector<_Tp, _Arch, _Alloc> &__x, const vector<_Tp, _Arch, _Alloc> &__y) {
    return !(__x == __y);
}

template <class _Tp, class _Arch, class _Alloc>
inline bool
operator>(const vector<_Tp, _Arch, _Alloc> &__x, const vector<_Tp, _Arch, _Alloc> &__y) {
    return __y < __x;
}

template <class _Tp, class _Arch, class _Alloc>
inline bool
operator<=(const vector<_Tp, _Arch, _Alloc> &__x, const vector<_Tp, _Arch, _Alloc> &__y) {
    return !(__y < __x);
}

template <class _Tp, class _Arch, class _Alloc>
inline bool
operator>=(const vector<_Tp, _Arch, _Alloc> &__x, const vector<_Tp, _Arch, _Alloc> &__y) {
    return !(__x < __y);
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Tp, class _Arch, class _Alloc>
vector<_Tp, _Arch, _Alloc> &
vector<_Tp, _Arch, _Alloc>::operator=(const vector<_Tp, _Arch, _Alloc> &__x)
{
    if (&__x != this) {
	const size_type __xlen = __x.size();
	if (__xlen > capacity()) {
	    iterator __tmp = _M_allocate_and_copy(__xlen, __x.begin(), __x.end());
	    pstl_destroy(_M_start, _M_finish);
	    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
	    _M_start = __tmp;
	    _M_end_of_storage = _M_start + __xlen;
	}
	else if (size() >= __xlen) {
	    iterator __i = pstl_copy(__x.begin(), __x.end(), begin());
	    pstl_destroy(__i, _M_finish);
	}
	else {
	    pstl_copy(__x.begin(), __x.begin() + size(), _M_start);
	    pstl_uninitialized_copy(__x.begin() + size(), __x.end(), _M_finish);
	}
	_M_finish = _M_start + __xlen;
    }
    return *this;
}

template <class _Tp, class _Arch, class _Alloc>
void vector<_Tp, _Arch, _Alloc>::_M_fill_assign(size_type __n, const value_type &__val) 
{
    if (__n > capacity()) {
	vector<_Tp, _Arch, _Alloc> __tmp(__n, __val, get_allocator());
	__tmp.swap(*this);
    }
    else if (__n > size()) {
	fill(begin(), end(), __val);
	_M_finish = pstl_uninitialized_fill_n(_M_finish, __n - size(), __val);
    }
    else
	erase(fill_n(begin(), __n, __val), end());
}

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> template <class _InputIter>
void vector<_Tp, _Arch, _Alloc>::_M_assign_aux(_InputIter __first, _InputIter __last,
					       input_iterator_tag) {
    iterator __cur = begin();
    for ( ; __first != __last && __cur != end(); ++__cur, ++__first) {
	value_reference __cur_ref = *__cur;
	value_reference __first_ref = *__first;
	if (__first) {
	    __first_ref.reassign(*__first);
	}
	else {
	    __STD::cerr << "error in vector::_M_assign_aux(_InputIter, _InputIter, input_iterator_tag" << __STD::endl;
	    __STD::exit(01);
	}
	__cur_ref.get_obj() = __first_ref.get_obj();
    }
    if (__first == __last) {
	erase(__cur, end());
    }
    else {
	insert(end(), __first, __last);
    }
}

template <class _Tp, class _Arch, class _Alloc> template <class _ForwardIter>
void
vector<_Tp, _Arch, _Alloc>::_M_assign_aux(_ForwardIter __first, _ForwardIter __last,
					  forward_iterator_tag) {
    size_type __len = 0;
    distance(__first, __last, __len);

    if (__len > capacity()) {
	iterator __tmp = _M_allocate_and_copy(__len, __first, __last);
	pstl_destroy(_M_start, _M_finish);
	_M_deallocate(_M_start, _M_end_of_storage - _M_start);
	_M_start = __tmp;
	_M_end_of_storage = _M_finish = _M_start + __len;
    }
    else if (size() >= __len) {
	iterator __new_finish = pstl_copy(__first, __last, _M_start);
	pstl_destroy(__new_finish, _M_finish);
	_M_finish = __new_finish;
    }
    else {
	_ForwardIter __mid = __first;
	advance(__mid, size());
	pstl_copy(__first, __mid, _M_start);
	_M_finish = pstl_uninitialized_copy(__mid, __last, _M_finish);
    }
}

#endif /* __PSTL_MEMBER_TEMPLATES */

template <class _Tp, class _Arch, class _Alloc>
void 
vector<_Tp, _Arch, _Alloc>::_M_insert_aux(iterator __position, const _Tp &__x)
{
    if (_M_finish != _M_end_of_storage) {
	value_reference _M_finish1_ref = (_M_finish - 1).get_ref();
	pstl_construct(_M_finish, _M_finish1_ref.get_obj());
	++_M_finish;
	_Tp __x_copy = __x;
	pstl_copy_backward(__position, _M_finish - 2, _M_finish - 1);
	value_reference __position_ref = __position.get_ref();
	__position_ref.get_obj() = __x_copy;
    }
    else {
	const size_type __old_size = size();
	const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
	iterator __new_start = _M_allocate(__len);
	iterator __new_finish = __new_start;
	__PSTL_TRY {
	    __new_finish = pstl_uninitialized_copy(_M_start, __position, __new_start);
	    pstl_construct(__new_finish, __x);
	    ++__new_finish;
	    __new_finish = pstl_uninitialized_copy(__position, _M_finish, __new_finish);
	}
	__PSTL_UNWIND((pstl_destroy(__new_start,__new_finish), 
		       _M_deallocate(__new_start,__len)));
	pstl_destroy(begin(), end());
	_M_deallocate(_M_start, _M_end_of_storage - _M_start);
	_M_start = __new_start;
	_M_finish = __new_finish;
	_M_end_of_storage = __new_start + __len;
    }
}

template <class _Tp, class _Arch, class _Alloc>
void 
vector<_Tp, _Arch, _Alloc>::_M_insert_aux(iterator __position)
{
    if (_M_finish != _M_end_of_storage) {
	value_reference _M_finish1_ref = (_M_finish - 1).get_ref();
	pstl_construct(_M_finish, _M_finish1_ref.get_obj());
	++_M_finish;
	pstl_copy_backward(__position, _M_finish - 2, _M_finish - 1);
	value_reference __position_ref = __position.get_ref();
	__position_ref.get_obj() = _Tp();
    }
    else {
	const size_type __old_size = size();
	const size_type __len = __old_size != 0 ? 2 * __old_size : 1;
	iterator __new_start = _M_allocate(__len);
	iterator __new_finish = __new_start;
	__PSTL_TRY {
	    __new_finish = pstl_uninitialized_copy(_M_start, __position, __new_start);
	    pstl_construct(__new_finish);
	    ++__new_finish;
	    __new_finish = pstl_uninitialized_copy(__position, _M_finish, __new_finish);
	}
	__PSTL_UNWIND((pstl_destroy(__new_start,__new_finish), 
		       _M_deallocate(__new_start,__len)));
	pstl_destroy(begin(), end());
	_M_deallocate(_M_start, _M_end_of_storage - _M_start);
	_M_start = __new_start;
	_M_finish = __new_finish;
	_M_end_of_storage = __new_start + __len;
    }
}

template <class _Tp, class _Arch, class _Alloc>
void vector<_Tp, _Arch, _Alloc>::_M_fill_insert(iterator __position, size_type __n, 
						const _Tp &__x)
{
    if (__n != 0) {
	if (size_type(_M_end_of_storage - _M_finish) >= __n) {
	    _Tp __x_copy = __x;
	    const size_type __elems_after = _M_finish - __position;
	    iterator __old_finish = _M_finish;
	    if (__elems_after > __n) {
		pstl_uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
		_M_finish += __n;
		pstl_copy_backward(__position, __old_finish - __n, __old_finish);
		pstl_fill(__position, __position + __n, __x_copy);
	    }
	    else {
		pstl_uninitialized_fill_n(_M_finish, __n - __elems_after, __x_copy);
		_M_finish += __n - __elems_after;
		pstl_uninitialized_copy(__position, __old_finish, _M_finish);
		_M_finish += __elems_after;
		pstl_fill(__position, __old_finish, __x_copy);
	    }
	}
	else {
	    const size_type __old_size = size();
	    const size_type __len = __old_size + pstl_max(__old_size, __n);
	    iterator __new_start = _M_allocate(__len);
	    iterator __new_finish = __new_start;
	    __PSTL_TRY {
		__new_finish = pstl_uninitialized_copy(_M_start, __position, __new_start);
		__new_finish = pstl_uninitialized_fill_n(__new_finish, __n, __x);
		__new_finish
		    = pstl_uninitialized_copy(__position, _M_finish, __new_finish);
	    }
	    __PSTL_UNWIND((pstl_destroy(__new_start,__new_finish), 
			   _M_deallocate(__new_start,__len)));
	    pstl_destroy(_M_start, _M_finish);
	    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
	    _M_start = __new_start;
	    _M_finish = __new_finish;
	    _M_end_of_storage = __new_start + __len;
	}
    }
}

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> template <class _InputIterator>
void 
vector<_Tp, _Arch, _Alloc>::_M_range_insert(iterator __pos, 
					    _InputIterator __first, 
					    _InputIterator __last,
					    input_iterator_tag)
{
    for ( ; __first != __last; ++__first) {
	value_reference __first_ref = __first.get_ref();
	__pos = insert(__pos, __first_ref.get_obj());
	++__pos;
    }
}

template <class _Tp, class _Arch, class _Alloc> template <class _ForwardIterator>
void 
vector<_Tp, _Arch, _Alloc>::_M_range_insert(iterator __position,
					    _ForwardIterator __first,
					    _ForwardIterator __last,
					    forward_iterator_tag)
{
    if (__first != __last) {
	size_type __n = 0;
	distance(__first, __last, __n);
	if (size_type(_M_end_of_storage - _M_finish) >= __n) {
	    const size_type __elems_after = _M_finish - __position;
	    iterator __old_finish = _M_finish;
	    if (__elems_after > __n) {
		pstl_uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
		_M_finish += __n;
		pstl_copy_backward(__position, __old_finish - __n, __old_finish);
		pstl_copy(__first, __last, __position);
	    }
	    else {
		_ForwardIterator __mid = __first;
		advance(__mid, __elems_after);
		pstl_uninitialized_copy(__mid, __last, _M_finish);
		_M_finish += __n - __elems_after;
		pstl_uninitialized_copy(__position, __old_finish, _M_finish);
		_M_finish += __elems_after;
		pstl_copy(__first, __mid, __position);
	    }
	}
	else {
	    const size_type __old_size = size();
	    const size_type __len = __old_size + pstl_max(__old_size, __n);
	    iterator __new_start = _M_allocate(__len);
	    iterator __new_finish = __new_start;
	    __PSTL_TRY {
		__new_finish = pstl_uninitialized_copy(_M_start, __position, __new_start);
		__new_finish = pstl_uninitialized_copy(__first, __last, __new_finish);
		__new_finish
		    = pstl_uninitialized_copy(__position, _M_finish, __new_finish);
	    }
	    __PSTL_UNWIND((pstl_destroy(__new_start,__new_finish), 
			   _M_deallocate(__new_start,__len)));
	    pstl_destroy(_M_start, _M_finish);
	    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
	    _M_start = __new_start;
	    _M_finish = __new_finish;
	    _M_end_of_storage = __new_start + __len;
	}
    }
}

#else /* __PSTL_MEMBER_TEMPLATES */

template <class _Tp, class _Arch, class _Alloc>
void 
vector<_Tp, _Arch, _Alloc>::insert(iterator __position, 
				   const_iterator __first, 
				   const_iterator __last)
{
    if (__first != __last) {
	size_type __n = 0;
	distance(__first, __last, __n);
	if (size_type(_M_end_of_storage - _M_finish) >= __n) {
	    const size_type __elems_after = _M_finish - __position;
	    iterator __old_finish = _M_finish;
	    if (__elems_after > __n) {
		pstl_uninitialized_copy(_M_finish - __n, _M_finish, _M_finish);
		_M_finish += __n;
		pstl_copy_backward(__position, __old_finish - __n, __old_finish);
		pstl_copy(__first, __last, __position);
	    }
	    else {
		pstl_uninitialized_copy(__first + __elems_after, __last, _M_finish);
		_M_finish += __n - __elems_after;
		pstl_uninitialized_copy(__position, __old_finish, _M_finish);
		_M_finish += __elems_after;
		pstl_copy(__first, __first + __elems_after, __position);
	    }
	}
	else {
	    const size_type __old_size = size();
	    const size_type __len = __old_size + max(__old_size, __n);
	    iterator __new_start = _M_allocate(__len);
	    iterator __new_finish = __new_start;
	    __PSTL_TRY {
		__new_finish = pstl_uninitialized_copy(_M_start, __position, __new_start);
		__new_finish = pstl_uninitialized_copy(__first, __last, __new_finish);
		__new_finish
		    = pstl_uninitialized_copy(__position, _M_finish, __new_finish);
	    }
	    __PSTL_UNWIND((pstl_destroy(__new_start,__new_finish),
			   _M_deallocate(__new_start,__len)));
	    pstl_destroy(_M_start, _M_finish);
	    _M_deallocate(_M_start, _M_end_of_storage - _M_start);
	    _M_start = __new_start;
	    _M_finish = __new_finish;
	    _M_end_of_storage = __new_start + __len;
	}
    }
}

#endif /* __PSTL_MEMBER_TEMPLATES */

// forward declaration
// template <class _Tp, class _Alloc> class const_vector_reference;

///////////////////////////////
//// vector reference

// template <class _Tp, class _Alloc>
// class vector_reference : public ref<vector<_Tp, _Alloc>, typename _Alloc::arch_type> {
// protected:
//     typedef typename _Alloc::arch_type          arch_type;
//     typedef typename arch_type::dev_type        dev_type;
//     typedef typename arch_type::bit_type        bit_type;
//     typedef typename arch_type::size_type       size_type;
//     typedef typename arch_type::pointer         pointer;
//     typedef typename arch_type::difference_type difference_type;
//     typedef _Tp                                 value_type;
//     typedef forward_iterator_tag                iterator_category;
//     typedef pheap<arch_type>                    pheap_type;
//     typedef vector<_Tp, _Alloc>                 vector_type;
//     typedef ptr<vector_type, arch_type>         iterator;
//     typedef const_ptr<vector_type, arch_type>   const_iterator;
//     typedef ref<vector_type, arch_type>         source_type;
//     typedef vector_reference                    reference;
//     typedef const_vector_reference<_Tp, _Alloc> const_reference;
//     typedef ref<value_type, arch_type>          value_reference;
//     typedef const_ref<value_type, arch_type>    const_value_reference;
// #ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
//     typedef reverse_iterator<const_iterator> const_reverse_iterator;
//     typedef reverse_iterator<iterator>       reverse_iterator;
// #else /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
//     typedef reverse_iterator<const_iterator, value_type, arch_type,
// 			     const_reference, difference_type>
//     const_reverse_iterator;
//     typedef reverse_iterator<iterator, value_type, arch_type,
// 			     reference, difference_type>
//     reverse_iterator;
// #endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
// public:
//     vector_reference() : source_type() {}
//     vector_reference(pheap_ptr __ph) : source_type(__ph) {}
//     vector_reference(pheap_ptr __ph, const ptr<vector_type, arch_type> &__p) : source_type(__ph, __p) {}
//     vector_reference(const source_type &r) : source_type(r) {}
//     ~vector_reference() {}

// #ifdef __PSTL_HAS_NAMESPACES
//     using source_type::get_obj;
// #endif // __PSTL_HAS_NAMESPACES

//     iterator begin() { return get_obj().begin(); }
//     const_iterator begin() const { return get_obj().begin(); }
//     iterator end() { return get_obj().end(); }
//     const_iterator end() const { return get_obj().end(); }

//     reverse_iterator rbegin() { return get_obj().rbegin(); }
//     const_reverse_iterator rbegin() const { return get_obj().rbegin(); }
//     reverse_iterator rend() { return get_obj().rend(); }
//     const_reverse_iterator rend() const { return get_obj().rend(); }

//     size_type size() const { return get_obj().size(); }
//     size_type max_size() const { return get_obj().maxsize(); }
//     size_type capacity() const { return get_obj().capacity(); }
//     bool empty() const { return get_obj().empty(); }

//     value_reference operator[](size_type __n) { return get_obj()[__n]; }
//     const_value_reference operator[](size_type __n) const { return get_obj()[__n]; }

// #ifdef __PSTL_THROW_RANGE_ERRORS
//     void _M_range_check(size_type __n) const { get_obj()._M_range_check(__n); }
//     value_reference at(size_type __n) { return get_obj().at(__n); }
//     const_value_reference at(size_type __n) const { return get_obj().at(__n); }
// #endif /* __PSTL_THROW_RANGE_ERRORS */

//     void reserve(size_type __n) { get_obj().reserve(__n); }

//     void assign(size_type __n, const _Tp &__val) { get_obj().assign(__n, __val); }

// #ifdef __PSTL_MEMBER_TEMPLATES
//     template <class _InputIterator>
//     void assign(_InputIterator __first, _InputIterator __last) { get_obj().assign(__first, __last); }
// #endif /* __PSTL_MEMBER_TEMPLATES */

//     value_reference front() { return get_obj().front(); }
//     const_value_reference front() const { return get_obj().front(); }
//     value_reference back() { return get_obj().back(); }
//     const_value_reference back() const { return get_obj().back(); }

//     void push_back(const _Tp &__x) { get_obj().push_back(__x); }
//     void push_back() { get_obj().push_back(); }
//     void swap(vector<_Tp, _Alloc> &__x) { get_obj().swap(__x); }
//     iterator insert(iterator __position, const _Tp &__x) { return get_obj().insert(__position, __x); }
//     iterator insert(iterator __position) { return get_obj().insert(__position); }
// #ifdef __PSTL_MEMBER_TEMPLATES
//     template <class _InputIterator>
//     void insert(iterator __pos, _InputIterator __first, _InputIterator __last) { get_obj().insert(__pos, __first, __last); }
// #else /* __PSTL_MEMBER_TEMPLATES */
//     void insert(iterator __position, const_iterator __first, const_iterator __last) { get_obj().insert(__position, __first, __last); }
// #endif /* __PSTL_MEMBER_TEMPLATES */
//     void insert(iterator __pos, size_type __n, const _Tp &__x) { get_obj().insert(__pos, __n, __x); }

//     void pop_back() { get_obj().pop_back(); }
//     iterator erase(iterator __position) { return get_obj().erase(__position); }
//     iterator erase(iterator __first, iterator __last) { return get_obj().erase(__first, __last); }
    
//     void resize(size_type new_size, const _Tp &__x) { get_obj().resize(new_size, __x); }
//     void resize(size_type new_size) { get_obj().resize(new_size); }
//     void clear() { get_obj().clear(); }
// };

// ///////////////////////////////
// //// const vector reference

// template <class _Tp, class _Alloc>
// class const_vector_reference : public const_ref<vector<_Tp, _Alloc>, typename _Alloc::arch_type> {
// protected:
//     typedef typename _Alloc::arch_type          arch_type;
//     typedef typename arch_type::dev_type        dev_type;
//     typedef typename arch_type::bit_type        bit_type;
//     typedef typename arch_type::size_type       size_type;
//     typedef typename arch_type::pointer         pointer;
//     typedef typename arch_type::difference_type difference_type;
//     typedef _Tp                                 value_type;
//     typedef forward_iterator_tag                iterator_category;
//     typedef pheap<arch_type>                    pheap_type;
//     typedef vector<_Tp, _Alloc>                 vector_type;
//     typedef ptr<vector_type, arch_type>         iterator;
//     typedef const_ptr<vector_type, arch_type>   const_iterator;
//     typedef const_ref<vector_type, arch_type>   source_type;
//     typedef vector_reference<_Tp, _Alloc>       reference;
//     typedef const_vector_reference              const_reference;
//     typedef ref<value_type, arch_type>          value_reference;
//     typedef const_ref<value_type, arch_type>    const_value_reference;
// #ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
//     typedef reverse_iterator<const_iterator> const_reverse_iterator;
//     typedef reverse_iterator<iterator>       reverse_iterator;
// #else /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
//     typedef reverse_iterator<const_iterator, value_type, arch_type,
// 			     const_reference, difference_type>
//     const_reverse_iterator;
//     typedef reverse_iterator<iterator, value_type, arch_type,
// 			     reference, difference_type>
//     reverse_iterator;
// #endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
// public:
//     const_vector_reference() : source_type() {}
//     const_vector_reference(pheap_ptr __ph) : source_type(__ph) {}
//     const_vector_reference(pheap_ptr __ph, const const_ptr<vector_type, arch_type> &__p) : source_type(__ph, __p) {}
//     const_vector_reference(const source_type &__r) : source_type(__r) {}
//     ~const_vector_reference() {}

// #ifdef __PSTL_HAS_NAMESPACES
//     using source_type::get_obj;
// #endif // __PSTL_HAS_NAMESPACES

//     const_iterator begin() const { return get_obj().begin(); }
//     const_iterator end() const { return get_obj().end(); }

//     const_reverse_iterator rbegin() const { return get_obj().rbegin(); }
//     const_reverse_iterator rend() const { return get_obj().rend(); }

//     size_type size() const { return get_obj().size(); }
//     size_type max_size() const { return get_obj().maxsize(); }
//     size_type capacity() const { return get_obj().capacity(); }
//     bool empty() const { return get_obj().empty(); }

//     const_value_reference operator[](size_type __n) const { return get_obj()[__n]; }

// #ifdef __PSTL_THROW_RANGE_ERRORS
//     void _M_range_check(size_type __n) const { get_obj()._M_range_check(__n); }
//     const_value_reference at(size_type __n) const { return get_obj().at(__n); }
// #endif /* __PSTL_THROW_RANGE_ERRORS */

//     void reserve(size_type __n) { get_obj().reserve(__n); }

//     const_value_reference front() const { return get_obj().front(); }
//     const_value_reference back() { return get_obj().back(); }
// };

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__PSTL_END_NAMESPACE 

#endif /* __SGI_PSTL_INTERNAL_VECTOR_H */

// Local Variables:
// mode:C++
// End:
