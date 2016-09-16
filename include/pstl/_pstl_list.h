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
 * Kenta Oouchida, Takashi Tsunakawa and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Kenta Oouchida, Takashi Tsunakawa
 * and Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

/* NOTE: This is an internal header file, included by other PSTL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_PSTL_INTERNAL_LIST_H
#define __SGI_PSTL_INTERNAL_LIST_H

#include "pstl_pheap.h"
#include "pstl_concept_checks.h"

__PSTL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

template <class _Arch>
struct _List_node_base {
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;
  
    ptr<_List_node_base, arch_type> _M_next;
    ptr<_List_node_base, arch_type> _M_prev;
  
};

template <class _Arch>
struct replace_pheap<_List_node_base<_Arch> > {
    void operator()(pheap_ptr __ph, _List_node_base<_Arch> &__obj) {
	__replace_pheap(__ph, __obj._M_next);
	__replace_pheap(__ph, __obj._M_prev);
    }
};

template <class _Tp, class _Arch>
struct _List_node : public _List_node_base<_Arch> {
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;
  
    _Tp _M_data;
    
    ptr<_Tp, arch_type> get_M_data_pointer(ptr<_Tp, arch_type> _M_this) {
	ptr_int _M_data_addr = reinterpret_cast<ptr_int>(&_M_data);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return ptr<_Tp, arch_type>(_M_this.getAddress() +
				   (_M_data_addr - this_addr),
				   _M_this.get_pheap());
    }
    const_ptr<_Tp, arch_type> get_M_data_pointer(ptr<_Tp, arch_type> _M_this) const {
	ptr_int _M_data_addr = reinterpret_cast<ptr_int>(&_M_data);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return const_ptr<_Tp, arch_type>(_M_this.getAddress() +
					 (_M_data_addr - this_addr),
					 _M_this.get_pheap());
    }
    ref<_Tp, arch_type> get_M_data(ptr<_Tp, arch_type> _M_this) {
	return *get_M_data_pointer(_M_this);
    }
    const_ref<_Tp, arch_type> get_M_data(ptr<_Tp, arch_type> _M_this) const {
	return *get_M_data_pointer(_M_this);
    }
};

template <class _Tp, class _Arch>
struct replace_pheap<_List_node<_Tp, _Arch> > {
    void operator()(pheap_ptr __ph, _List_node<_Tp, _Arch> &__obj) {
	typedef _List_node_base<_Arch> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
	__replace_pheap(__ph, __obj._M_data);
    }
};
    
template <class _Arch>
struct _List_iterator_base {
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef bidirectional_iterator_tag               iterator_category;
    typedef pheap<arch_type>                         pheap_type;

    ptr<_List_node_base<arch_type>, arch_type> _M_node;
    _List_iterator_base(ptr<_List_node_base<arch_type>, arch_type> __x) : _M_node(__x) {}
    _List_iterator_base() {}

    void _M_incr() { _M_node = (*_M_node)->_M_next; }
    void _M_decr() { _M_node = (*_M_node)->_M_prev; }

    bool operator==(const _List_iterator_base &__x) const {
	return _M_node == __x._M_node;
    }
    bool operator!=(const _List_iterator_base &__x) const {
	return _M_node != __x._M_node;
    }
};

template <class _Arch>
struct replace_pheap<_List_iterator_base<_Arch> > {
    void operator()(pheap_ptr __ph, _List_iterator_base<_Arch> &__obj) {
	__replace_pheap(__ph, __obj._M_node);
    }
};

template <class _Tp, class _Ref, class _Ptr, class _Arch>
struct _List_iterator : public _List_iterator_base<_Arch>{
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;
    typedef _List_iterator<_Tp, ref<_Tp, arch_type>, ptr<_Tp, arch_type>, arch_type>
    iterator;
    typedef _List_iterator<_Tp, const_ref<_Tp, arch_type>, const_ptr<_Tp, arch_type>, arch_type>
    const_iterator;
    typedef _List_iterator<_Tp, _Ref, _Ptr, _Arch>   _Self;

    typedef _Tp value_type;
    typedef _Ptr pointer;
    typedef _Ref reference;
    typedef _List_node<_Tp, _Arch> _Node;

    _List_iterator(ptr<_Node, arch_type> __x) : _List_iterator_base<_Arch>(__x) {}
    _List_iterator() {}
    _List_iterator(const iterator &__x) : _List_iterator_base<_Arch>(__x._M_node) {}
    _List_iterator(const_ref<iterator, arch_type> __x) : _List_iterator_base<_Arch>(__x->_M_node) {}

    reference operator*() const {
	ref<_List_node<_Tp, arch_type>, arch_type> _M_node_ref =
	    *ptr<_List_node<_Tp, arch_type>, arch_type>(this->_M_node);
	return _M_node_ref->get_M_data(this->_M_node);
    }
  

#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    pointer operator->() const {
	ref<_List_node<_Tp, arch_type>, arch_type> _M_node_ref =
	    *ptr<_List_node<_Tp, arch_type>, arch_type>(this->_M_node);
	return _M_node_ref->get_M_data_pointer(this->_M_node);
    }
#endif /* __SGI_PSTL_NO_ARROW_OPERATOR */

    _Self &operator++() { 
	this->_M_incr();
	return *this;
    }
    _Self operator++(int) { 
	_Self __tmp = *this;
	this->_M_incr();
	return __tmp;
    }
    _Self &operator--() { 
	this->_M_decr();
	return *this;
    }
    _Self operator--(int) { 
	_Self __tmp = *this;
	this->_M_decr();
	return __tmp;
    }
    
    ptr<_Node, arch_type> get_node() {
        return _List_iterator_base<_Arch>::_M_node;
    }
};

#ifndef __PSTL_CLASS_PARTIAL_SPECIALIZATION

inline bidirectional_iterator_tag
iterator_category(const _List_iterator_base &)
{
    return bidirectional_iterator_tag();
}

template <class _Tp, class _Ref, class _Ptr, class _Arch>
inline ptr<_Tp, _Arch>
value_type(const _List_iterator<_Tp, _Ref, _Ptr> &)
{
    return ptr<_Tp, _Arch>(0);
}

template <class _Arch>
inline typename _Arch::difference_type *
distance_type(const _List_iterator_base<_Arch> &)
{
    return reinterpret_cast<typename _Arch::difference_type *>(0);
}

#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */


// Base class that encapsulates details of allocators.  Three cases:
// an ordinary standard-conforming allocator, a standard-conforming
// allocator with no non-static data, and an SGI-style allocator.
// This complexity is necessary only because we're worrying about backward
// compatibility and because we want to avoid wasting storage on an 
// allocator instance if it isn't necessary.

#ifdef __PSTL_USE_STD_ALLOCATORS

// Base for general standard-conforming allocators.
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic>
class _List_alloc_base {
public:
    typedef typename _Alloc_traits<_Tp, _Arch, _Allocator>::allocator_type
    allocator_type;
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;
  
    allocator_type get_allocator() const { return _Node_allocator; }

    _List_alloc_base(const allocator_type &__a) : _Node_allocator(__a) {}

protected:
    ptr<_List_node<_Tp, arch_type>, arch_type> _M_get_node()
	{ return _Node_allocator.allocate(1); }
    void _M_put_node(ptr<_List_node<_Tp, arch_type>, arch_type> __p)
	{ _Node_allocator.deallocate(__p, 1); }

protected:
    typename _Alloc_traits<_List_node<_Tp, arch_type>, arch_type, _Allocator>::allocator_type
    _Node_allocator;
    ptr<_List_node<_Tp, arch_type>, arch_type> _M_node;

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Allocator, bool _IsStatic>
struct replace_pheap<_List_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> > {
    void operator()(pheap_ptr __ph, _List_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> &__obj) {
	__replace_pheap(__ph, __obj._Node_allocator);
	__replace_pheap(__ph, __obj._M_node);
    }
};

// Specialization for instanceless allocators.

template <class _Tp, class _Arch, class _Allocator>
class _List_alloc_base<_Tp, _Arch, _Allocator, true> {
public:
    typedef typename _Alloc_traits<_Tp, _Arch, _Allocator>::allocator_type
    allocator_type;
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;

    allocator_type get_allocator() const { return allocator_type(); }

    _List_alloc_base(const allocator_type &__a) : _M_node(0, __a.get_pheap()) {}

protected:
    typedef typename _Alloc_traits<_List_node<_Tp, arch_type>, arch_type, _Allocator>::_Alloc_type
    _Alloc_type;

    ptr<_List_node<_Tp, arch_type>, arch_type> _M_get_node() { 	
	return ptr<_List_node<_Tp, arch_type>, arch_type>(pheap_type::pheap_convert(_M_node.get_pheap())->malloc(sizeof(_List_node<_Tp, arch_type>)), _M_node.get_pheap());
    }
    void _M_put_node(ptr<_List_node<_Tp, arch_type>, arch_type> __p) {
	pheap_type::pheap_convert(_M_node.get_pheap())->free(static_cast<ptr<void, arch_type> >(__p)); //, sizeof(_List_node<_Tp, arch_type>));
    }
  
protected:
    ptr<_List_node<_Tp, arch_type>, arch_type> _M_node;

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Allocator>
struct replace_pheap<_List_alloc_base<_Tp, _Arch, _Allocator, true> > {
    void operator()(pheap_ptr __ph, _List_alloc_base<_Tp, _Arch, _Allocator, true> &__obj) {
	__replace_pheap(__ph, __obj._M_node);
    }
};

template <class _Tp, class _Arch, class _Allocator>
class _List_base 
    : public _List_alloc_base<_Tp, _Arch, _Allocator,
			      _Alloc_traits<_Tp, _Arch, _Allocator>::_S_instanceless> {
public:
    typedef _List_alloc_base<_Tp, _Arch, _Allocator,
			     _Alloc_traits<_Tp, _Arch, _Allocator>::_S_instanceless>
    _Base; 
    typedef typename _Base::allocator_type           allocator_type;
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;

#ifdef __PSTL_HAS_NAMESPACES
    using _Base::_M_node;
    using _Base::_M_get_node;
    using _Base::_M_put_node;
#endif // __PSTL_HAS_NAMESPACES
    
    _List_base(const allocator_type &__a) : _Base(__a) {
	_M_node = _M_get_node();
	(*_M_node)->_M_next = _M_node;
	(*_M_node)->_M_prev = _M_node;
    }
    ~_List_base() {
	clear();
	_M_put_node(_M_node);
    }

    void clear();

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<_List_base<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _List_base<_Tp, _Arch, _Alloc> &__obj) {
	typedef _List_alloc_base<_Tp, _Arch, _Alloc, _Alloc_traits<_Tp, _Arch, _Alloc>::_S_instanceless> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

#else /* __PSTL_USE_STD_ALLOCATORS */

template <class _Tp, class _Arch, class _Alloc>
class _List_base {
public:
    typedef _Alloc                                   allocator_type;
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;

    allocator_type get_allocator() const { return allocator_type(); }

    _List_base(const allocator_type &) {
	_M_node = _M_get_node();
	(*_M_node)->_M_next = _M_node;
	(*_M_node)->_M_prev = _M_node;
    }
    ~_List_base() {
	clear();
	_M_put_node(_M_node);
    }

    void clear();

protected:
    typedef simple_alloc<_List_node<_Tp, arch_type>, arch_type, _Alloc> _Alloc_type;
    ptr<_List_node<_Tp, arch_type>, arch_type> _M_get_node() { return _Alloc_type::allocate(1); }
    void _M_put_node(ptr<_List_node<_Tp, arch_type>, arch_type> __p) { _Alloc_type::deallocate(__p, 1); } 

protected:
    ptr<_List_node<_Tp, arch_type>, arch_type> _M_node;

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<_List_base<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _List_base<_Tp, _Arch, _Alloc> &__obj) {
	__replace_pheap(__ph, __obj._M_node);
    }
};

#endif /* __PSTL_USE_STD_ALLOCATORS */

template <class _Tp, class _Arch, class _Alloc>
void 
_List_base<_Tp, _Arch, _Alloc>::clear() 
{
    typedef typename _Alloc_traits<_Tp, arch_type, _Alloc>::allocator_type
	allocator_type;
    typedef typename allocator_type::arch_type       arch_type;
    ptr<_List_node<_Tp, arch_type>, arch_type> __cur = ptr<_List_node<_Tp, arch_type>, arch_type>((*_M_node)->_M_next);
    while (__cur != _M_node) {
	ptr<_List_node<_Tp, arch_type>, arch_type> __tmp = __cur;
	__cur = ptr<_List_node<_Tp, arch_type>, arch_type>((*__cur)->_M_next);
	pstl_Destroy((*_M_node)->get_M_data_pointer(_M_node));
	_M_put_node(__tmp);
    }
    //
    (*_M_node)->_M_next = _M_node;
    (*_M_node)->_M_prev = _M_node;
}

template <class _Tp, class _Arch, class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_Tp, _Arch) >
class list : public _List_base<_Tp, _Arch, _Alloc> {
    // requirements:

    __PSTL_CLASS_REQUIRES(_Tp, _Assignable);

    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;

    typedef _List_base<_Tp, _Arch, _Alloc> _Base;
protected:
    typedef ptr<void, arch_type> _Void_pointer;

public:      
    typedef _Tp value_type;
    typedef ptr<value_type, arch_type> pointer;
    typedef const_ptr<value_type, arch_type> const_pointer;
    typedef ref<value_type, arch_type> value_reference;
    typedef const_ref<value_type, arch_type> const_value_reference;    
    typedef ref<list, arch_type> reference;
    typedef const_ref<list, arch_type> const_reference;
    typedef _List_node<_Tp, arch_type> _Node;
    typedef _Node node_type;

    typedef typename _Base::allocator_type allocator_type;
    allocator_type get_allocator() const { return _Base::get_allocator(); }

public:
    typedef _List_iterator<_Tp, ref<_Tp, arch_type>, ptr<_Tp, arch_type>, arch_type>
    iterator;
    typedef _List_iterator<_Tp, const_ref<_Tp, arch_type>, const_ptr<_Tp, arch_type>, arch_type>
    const_iterator;

#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION
    typedef reverse_iterator<const_iterator> const_reverse_iterator;
    typedef reverse_iterator<iterator>       reverse_iterator;
#else /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */
    typedef reverse_bidirectional_iterator<const_iterator, value_type,
					   arch_type,
					   const_value_reference,
					   difference_type>
    const_reverse_iterator;
    typedef reverse_bidirectional_iterator<iterator, value_type, arch_type,
					   value_reference, difference_type>
    reverse_iterator; 
#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

protected:
#ifdef __PSTL_HAS_NAMESPACES
    using _Base::_M_node;
    using _Base::_M_put_node;
    using _Base::_M_get_node;
#endif /* __PSTL_HAS_NAMESPACES */

protected:
    ptr<_Node, arch_type> _M_create_node(const _Tp &__x)
	{
	    ptr<_Node, arch_type> __p = _M_get_node();
	    __PSTL_TRY {
		pstl_Construct((*__p)->get_M_data(__p), __x);
	    }
	    __PSTL_UNWIND(_M_put_node(__p));
	    return __p;
	}

    ptr<_Node, arch_type> _M_create_node()
	{
	    ptr<_Node, arch_type> __p = _M_get_node();
	    __PSTL_TRY {
		pstl_Construct((*__p)->get_M_data_pointer(__p));
	    }
	    __PSTL_UNWIND(_M_put_node(__p));
	    return __p;
	}

public:
    list() : _Base(allocator_type()) {}

    explicit list(pheap_ptr __ph)
	: _Base(allocator_type(__ph)) {}
    
    explicit list(const allocator_type &__a)
	: _Base(__a) {}

public:
    static ptr<list, arch_type> pnew(pheap_ptr __ph) {
	ptr<list, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(list)), __ph);
        ref<list, arch_type> r = *__p;
	new ((void*) &(r.get_obj())) list(__ph);
	return __p;
    }
    static ptr<list, arch_type> pnew(pheap_ptr __ph, const allocator_type &__a) {
	ptr<list, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(list)), __ph);
        ref<list, arch_type> r = *__p;
	new ((void*) &(r.get_obj())) list(__a);
	return __p;
    }

    iterator begin()             { return static_cast<ptr<_Node, arch_type> >((*_M_node)->_M_next); }
    const_iterator begin() const { return static_cast<ptr<_Node, arch_type> >((*_M_node)->_M_next); }

    iterator end()             { return _M_node; }
    const_iterator end() const { return _M_node; }

    reverse_iterator rbegin() 
	{ return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const 
	{ return const_reverse_iterator(end()); }

    reverse_iterator rend()
	{ return reverse_iterator(begin()); }
    const_reverse_iterator rend() const
	{ return const_reverse_iterator(begin()); }

    bool empty() const { return (*_M_node)->_M_next == _M_node; }
    size_type size() const {
	size_type __result = 0;
	distance(begin(), end(), __result);
	return __result;
    }
    size_type max_size() const { return size_type(-1); }

    value_reference front() { return *begin(); }
    const_value_reference front() const { return *begin(); }
    value_reference back() { return *(--end()); }
    const_value_reference back() const { return *(--end()); }

    void swap(list<_Tp, _Arch, _Alloc> &__x) { pstl_swap(_M_node, __x._M_node); }

    iterator insert(iterator __position, const _Tp &__x) {
	ptr<_Node, arch_type> __tmp = _M_create_node(__x);
	(*__tmp)->_M_next = __position._M_node;
	(*__tmp)->_M_prev = (*__position._M_node)->_M_prev;
	(*(*__position._M_node)->_M_prev)->_M_next = __tmp;
	(*__position._M_node)->_M_prev = __tmp;
	return __tmp;
    }
    iterator insert(iterator __position) { return insert(__position, _Tp()); }
#ifdef __PSTL_MEMBER_TEMPLATES
    // Check whether it's an integral type.  If so, it's not an iterator.

    template <class _Integer>
    void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __x,
			    __true_type) {
	_M_fill_insert(__pos, (size_type)__n, (_Tp) __x);
    }

    template <class _InputIterator>
    void _M_insert_dispatch(iterator __pos,
			    _InputIterator __first, _InputIterator __last,
			    __false_type);

    template <class _InputIterator>
    void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
	typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
	_M_insert_dispatch(__pos, __first, __last, _Integral());
    }

#else /* __PSTL_MEMBER_TEMPLATES */
    void insert(iterator __position, const_ptr<_Tp, arch_type> __first, const_ptr<_Tp, arch_type> __last);
    void insert(iterator __position,
		const_iterator __first, const_iterator __last);
#endif /* __PSTL_MEMBER_TEMPLATES */
    void insert(iterator __pos, size_type __n, const _Tp &__x)
	{ _M_fill_insert(__pos, __n, __x); }
    void _M_fill_insert(iterator __pos, size_type __n, const _Tp &__x); 

    void push_front(const _Tp &__x) { insert(begin(), __x); }
    void push_front() {insert(begin());}
    void push_back(const _Tp &__x) { insert(end(), __x); }
    void push_back() {insert(end());}

    iterator erase(iterator __position) {
	ptr<_List_node_base<arch_type>, arch_type> __next_node =
	    (*__position._M_node)->_M_next;
	ptr<_List_node_base<arch_type>, arch_type> __prev_node =
	    (*__position._M_node)->_M_prev;
	ptr<_Node, arch_type> __n = ptr<_Node, arch_type>(__position._M_node);
	(*__prev_node)->_M_next = __next_node;
	(*__next_node)->_M_prev = __prev_node;
	pstl_Destroy((*__n)->get_M_data_pointer(__n));
	_M_put_node(__n);
	return iterator(ptr<_Node, arch_type>(__next_node));
    }
    iterator erase(iterator __first, iterator __last);
    void clear() { _Base::clear(); }

    void resize(size_type __new_size, const _Tp &__x);
    void resize(size_type __new_size) { this->resize(__new_size, _Tp()); }

    void pop_front() { erase(begin()); }
    void pop_back() { 
	iterator __tmp = end();
	erase(--__tmp);
    }

    list<_Tp, _Arch, _Alloc> &operator=(const list<_Tp, _Arch, _Alloc> &__x);

public:
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
	{ _M_fill_assign((size_type) __n, (_Tp) __val); }

    template <class _InputIterator>
    void _M_assign_dispatch(_InputIterator __first, _InputIterator __last,
			    __false_type);

#endif /* __PSTL_MEMBER_TEMPLATES */

protected:
    void transfer(iterator __position, iterator __first, iterator __last) {
	if (__position != __last) {
	    // Remove [first, last) from its old position.
	    (*(*__last._M_node)->_M_prev)->_M_next     = __position._M_node;
	    (*(*__first._M_node)->_M_prev)->_M_next    = __last._M_node;
	    (*(*__position._M_node)->_M_prev)->_M_next = __first._M_node; 

	    // Splice [first, last) into its new position.
	    ptr<_List_node_base<arch_type>, arch_type> __tmp =
		(*__position._M_node)->_M_prev;
	    (*__position._M_node)->_M_prev = (*__last._M_node)->_M_prev;
	    (*__last._M_node)->_M_prev     = (*__first._M_node)->_M_prev; 
	    (*__first._M_node)->_M_prev    = __tmp;
	}
    }

public:
    void splice(iterator __position, list &__x) {
	if (!__x.empty()) 
	    this->transfer(__position, __x.begin(), __x.end());
    }
  
    void splice(iterator __position, list &, iterator __i) {
	iterator __j = __i;
	++__j;
	if (__position == __i || __position == __j) return;
	this->transfer(__position, __i, __j);
    }
    void splice(iterator __position, list &, iterator __first, iterator __last) {
	if (__first != __last) 
	    this->transfer(__position, __first, __last);
    }
    void remove(const _Tp &__value);
    void unique();
    void merge(list &__x);
    void reverse();
    void sort();

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _Predicate> void remove_if(_Predicate);
    template <class _BinaryPredicate> void unique(_BinaryPredicate);
    template <class _StrictWeakOrdering> void merge(list &, _StrictWeakOrdering);
    template <class _StrictWeakOrdering> void sort(_StrictWeakOrdering);
#endif /* __PSTL_MEMBER_TEMPLATES */

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<list<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, list<_Tp, _Arch, _Alloc> &__obj) {
	typedef _List_base<_Tp, _Arch, _Alloc> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

template <class _Tp, class _Arch, class _Alloc>
inline bool 
operator==(const list<_Tp, _Arch, _Alloc> &__x,
	   const list<_Tp, _Arch, _Alloc> &__y)
{
    typedef typename list<_Tp, _Arch, _Alloc>::const_iterator const_iterator;
    const_iterator __end1 = __x.end();
    const_iterator __end2 = __y.end();

    const_iterator __i1 = __x.begin();
    const_iterator __i2 = __y.begin();
    while (__i1 != __end1 && __i2 != __end2 && *__i1 == *__i2) {
	++__i1;
	++__i2;
    }
    return __i1 == __end1 && __i2 == __end2;
}

template <class _Tp, class _Arch, class _Alloc>
inline bool operator<(const list<_Tp, _Arch, _Alloc> &__x,
                      const list<_Tp, _Arch, _Alloc> &__y)
{
    return pstl_lexicographical_compare(__x.begin(), __x.end(),
					__y.begin(), __y.end());
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Tp, class _Arch, class _Alloc>
inline bool operator!=(const list<_Tp, _Arch, _Alloc>& __x,
                       const list<_Tp, _Arch, _Alloc>& __y) {
    return !(__x == __y);
}

//&&&2
template <class _Tp, class _Arch, class _Alloc>
inline bool operator>(const list<_Tp, _Arch, _Alloc>& __x,
                      const list<_Tp, _Arch, _Alloc>& __y) {
    return __y < __x;
}

//&&&2
template <class _Tp, class _Arch, class _Alloc>
inline bool operator<=(const list<_Tp, _Arch, _Alloc>& __x,
                       const list<_Tp, _Arch, _Alloc>& __y) {
    return !(__y < __x);
}

//&&&2
template <class _Tp, class _Arch, class _Alloc>
inline bool operator>=(const list<_Tp, _Arch, _Alloc>& __x,
                       const list<_Tp, _Arch, _Alloc>& __y) {
    return !(__x < __y);
}

//&&&2
template <class _Tp, class _Arch, class _Alloc>
inline void 
swap(list<_Tp, _Arch, _Alloc>& __x, list<_Tp, _Arch, _Alloc>& __y)
{
    __x.swap(__y);
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> template <class _InputIter>
void 
list<_Tp, _Arch, _Alloc>::_M_insert_dispatch(iterator __position,
					     _InputIter __first, _InputIter __last,
					     __false_type)
{
    for ( ; __first != __last; ++__first) {
	insert(__position, *__first);
    }
}

#else /* __PSTL_MEMBER_TEMPLATES */

template <class _Tp, class _Arch, class _Alloc>
void 
list<_Tp, _Arch, _Alloc>::insert(iterator __position, 
				 const_ptr<_Tp, _Arch> __first,
				 const_ptr<_Tp, _Arch> __last)
{
    for ( ; __first != __last; ++__first) {
	insert(__position, *__first);
    }
}

template <class _Tp, class _Arch, class _Alloc>
void 
list<_Tp, _Arch, _Alloc>::insert(iterator __position,
				 const_iterator __first, const_iterator __last)
{
    for ( ; __first != __last; ++__first) {
	insert(__position, *__first);
    }
}

#endif /* __PSTL_MEMBER_TEMPLATES */

template <class _Tp, class _Arch, class _Alloc>
void 
list<_Tp, _Arch, _Alloc>::_M_fill_insert(iterator __position,
					 size_type __n, const _Tp &__x)
{
    for ( ; __n > 0; --__n) {
	insert(__position, __x);
    }
}

template <class _Tp, class _Arch, class _Alloc>
typename list<_Tp, _Arch, _Alloc>::iterator
list<_Tp, _Arch, _Alloc>::erase(iterator __first, iterator __last)
{
    while (__first != __last) {
	erase(__first++);
    }
    return __last;
}

template <class _Tp, class _Arch, class _Alloc>
void
list<_Tp, _Arch, _Alloc>::resize(size_type __new_size, const _Tp &__x)
{
    iterator __i = begin();
    size_type __len = 0;
    for ( ; __i != end() && __len < __new_size; ++__i, ++__len);
    if (__len == __new_size) {
	erase(__i, end());
    }
    else {                        // __i == end()
	insert(end(), __new_size - __len, __x);
    }
}

template <class _Tp, class _Arch, class _Alloc>
list<_Tp, _Arch, _Alloc> &
list<_Tp, _Arch, _Alloc>::operator=(const list<_Tp, _Arch, _Alloc> &__x)
{
    if (this != &__x) {
	iterator __first1 = begin();
	iterator __last1 = end();
	const_iterator __first2 = __x.begin();
	const_iterator __last2 = __x.end();
	while (__first1 != __last1 && __first2 != __last2) {
	    *__first1++ = *__first2++;
	}
	if (__first2 == __last2) {
	    erase(__first1, __last1);
	}
	else {
	    insert(__last1, __first2, __last2);
	}
    }
    return *this;
}

template <class _Tp, class _Arch, class _Alloc>
void
list<_Tp, _Arch, _Alloc>::_M_fill_assign(size_type __n, const _Tp &__val) {
    iterator __i = begin();
    for ( ; __i != end() && __n > 0; ++__i, --__n) {
	*__i = __val;
    }
    if (__n > 0) {
	insert(end(), __n, __val);
    }
    else {
	erase(__i, end());
    }
}

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> template <class _InputIter>
void
list<_Tp, _Arch, _Alloc>::_M_assign_dispatch(_InputIter __first2,
					     _InputIter __last2,
					     __false_type)
{
    iterator __first1 = begin();
    iterator __last1 = end();
    for ( ; __first1 != __last1 && __first2 != __last2; ++__first1, ++__first2)
	*__first1 = *__first2;
    if (__first2 == __last2) {
	erase(__first1, __last1);
    }
    else {
	insert(__last1, __first2, __last2);
    }
}

#endif /* __PSTL_MEMBER_TEMPLATES */

template <class _Tp, class _Arch, class _Alloc>
void list<_Tp, _Arch, _Alloc>::remove(const _Tp &__value)
{
    iterator __first = begin();
    iterator __last = end();
    while (__first != __last) {
	iterator __next = __first;
	++__next;
	if (*__first == __value) {
	    erase(__first);
	}
	__first = __next;
    }
}

template <class _Tp, class _Arch, class _Alloc>
void list<_Tp, _Arch, _Alloc>::unique()
{
    iterator __first = begin();
    iterator __last = end();
    if (__first == __last) return;
    iterator __next = __first;
    while (++__next != __last) {
	if (*__first == *__next) {
	    erase(__next);
	}
	else {
	    __first = __next;
	}
	__next = __first;
    }
}

template <class _Tp, class _Arch, class _Alloc>
void list<_Tp, _Arch, _Alloc>::merge(list<_Tp, _Arch, _Alloc> &__x)
{
    iterator __first1 = begin();
    iterator __last1 = end();
    iterator __first2 = __x.begin();
    iterator __last2 = __x.end();
    while (__first1 != __last1 && __first2 != __last2)
	if (*__first2 < *__first1) {
	    iterator __next = __first2;
	    transfer(__first1, __first2, ++__next);
	    __first2 = __next;
	}
	else {
	    ++__first1;
	}
    if (__first2 != __last2) {
	transfer(__last1, __first2, __last2);
    }
}

template <class _Arch>
inline void __List_base_reverse(ptr<_List_node_base<_Arch>, _Arch> __p)
{
    ptr<_List_node_base<_Arch>, _Arch> __tmp = __p;
    do {
        pstl_swap((*__tmp)->_M_next, (*__tmp)->_M_prev);
	__tmp = (*__tmp)->_M_prev;     // Old next node is now prev.
    } while (__tmp != __p);
}

template <class _Tp, class _Arch, class _Alloc>
inline void list<_Tp, _Arch, _Alloc>::reverse() 
{
    __List_base_reverse(ptr<_List_node_base<_Arch>, _Arch>(this->_M_node));
}    

template <class _Tp, class _Arch, class _Alloc>
void list<_Tp, _Arch, _Alloc>::sort()
{
    // Do nothing if the list has length 0 or 1.
    if ((*_M_node)->_M_next != _M_node &&
	(*(*_M_node)->_M_next)->_M_next != _M_node) {
	list<_Tp, _Arch, _Alloc> __carry(_M_node.get_pheap());
	list<_Tp, _Arch, _Alloc> *__counter[64];
	for (int i = 0; i < 64; i++) {
	    __counter[i] = new list<_Tp, _Arch, _Alloc>(_M_node.get_pheap());
	}
	int __fill = 0;
	while (!empty()) {
	    __carry.splice(__carry.begin(), *this, begin());
	    int __i = 0;
	    while (__i < __fill && !__counter[__i]->empty()) {
		__counter[__i]->merge(__carry);
		__carry.swap(*__counter[__i++]);
	    }
	    __carry.swap(*__counter[__i]);         
	    if (__i == __fill) {
		++__fill;
	    }
	} 

	for (int __i = 1; __i < __fill; ++__i) {
	    __counter[__i]->merge(*__counter[__i - 1]);
	}
	swap(*__counter[__fill - 1]);
	for (int i = 0; i < 64; i++) {
	    delete __counter[i];
	}
    }
}

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> template <class _Predicate>
void list<_Tp, _Arch, _Alloc>::remove_if(_Predicate __pred)
{
    iterator __first = begin();
    iterator __last = end();
    while (__first != __last) {
	iterator __next = __first;
	++__next;
	if (__pred(*__first)) {
	    erase(__first);
	}
	__first = __next;
    }
}

template <class _Tp, class _Arch, class _Alloc>
template <class _BinaryPredicate>
void list<_Tp, _Arch, _Alloc>::unique(_BinaryPredicate __binary_pred)
{
    iterator __first = begin();
    iterator __last = end();
    if (__first == __last) {
	return;
    }
    iterator __next = __first;
    while (++__next != __last) {
	if (__binary_pred(*__first, *__next)) {
	    erase(__next);
	}
	else {
	    __first = __next;
	}
	__next = __first;
    }
}

template <class _Tp, class _Arch, class _Alloc>
template <class _StrictWeakOrdering>
void list<_Tp, _Arch, _Alloc>::merge(list<_Tp, _Arch, _Alloc> &__x,
				     _StrictWeakOrdering __comp)
{
    iterator __first1 = begin();
    iterator __last1 = end();
    iterator __first2 = __x.begin();
    iterator __last2 = __x.end();
    //***1?
    while (__first1 != __last1 && __first2 != __last2) {
	if (__comp(*__first2, *__first1)) {
	    iterator __next = __first2;
	    transfer(__first1, __first2, ++__next);
	    __first2 = __next;
	}
	else {
	    ++__first1;
	}
    }
    if (__first2 != __last2) {
	transfer(__last1, __first2, __last2);
    }
}

template <class _Tp, class _Arch, class _Alloc>
template <class _StrictWeakOrdering>
void list<_Tp, _Arch, _Alloc>::sort(_StrictWeakOrdering __comp)
{
    // Do nothing if the list has length 0 or 1.
    if ((*_M_node)->_M_next != _M_node &&
	(*(*_M_node)->_M_next)->_M_next != _M_node) {
	list<_Tp, _Arch, _Alloc> __carry(_M_node.get_pheap());
	list<_Tp, _Arch, _Alloc> *__counter[64];
	for (int i = 0; i < 64; i++) {
	    __counter[i] = new list<_Tp, _Arch, _Alloc>(_M_node.get_pheap());
	}
	int __fill = 0;
	while (!empty()) {
	    __carry.splice(__carry.begin(), *this, begin());
	    int __i = 0;
	    while(__i < __fill && !__counter[__i]->empty()) {
		__counter[__i]->merge(__carry, __comp);
		__carry.swap(*__counter[__i++]);
	    }
	    __carry.swap(*__counter[__i]);         
	    if (__i == __fill) {
		++__fill;
	    }
	} 

	for (int __i = 1; __i < __fill; ++__i) {
	    __counter[__i]->merge(*__counter[__i - 1], __comp);
	}
	swap(*__counter[__fill - 1]);
	for (int i = 0; i < 64; i++) {
	    delete __counter[i];
	}
    }
}

#endif /* __PSTL_MEMBER_TEMPLATES */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__PSTL_END_NAMESPACE 

#endif /* __SGI_PSTL_INTERNAL_LIST_H */

// Local Variables:
// mode:C++
// End:
