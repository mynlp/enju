/*
 * Copyright (c) 1997
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef __SGI_PSTL_INTERNAL_SLIST_H
#define __SGI_PSTL_INTERNAL_SLIST_H

#include "pstl_pheap.h"
#include "pstl_concept_checks.h"


__PSTL_BEGIN_NAMESPACE 

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#pragma set woff 1375
#endif

template <class _Arch>
struct _Slist_node_base
{
    ptr<_Slist_node_base, _Arch> _M_next;
};

template <class _Arch>
struct replace_pheap<_Slist_node_base<_Arch> > {
    void operator()(pheap_ptr __ph, _Slist_node_base<_Arch> &__obj) {
	__replace_pheap(__ph, __obj._M_next);
    }
};

template <class _Arch>
inline ptr<_Slist_node_base<_Arch>, _Arch>
__slist_make_link(ptr<_Slist_node_base<_Arch>, _Arch> __prev_node,
                  ptr<_Slist_node_base<_Arch>, _Arch> __new_node)
{
    (*__new_node)->_M_next = (*__prev_node)->_M_next;
    (*__prev_node)->_M_next = __new_node;
    return __new_node;
}

template <class _Arch>
inline ptr<_Slist_node_base<_Arch>, _Arch>
__slist_previous(ptr<_Slist_node_base<_Arch>, _Arch> __head,
                 ptr<_Slist_node_base<_Arch>, _Arch> __node)
{    
    while (__head && (*__head)->_M_next != __node){
	__head = (*__head)->_M_next;
    }

    return __head;
}

template <class _Arch>
inline const_ptr<_Slist_node_base<_Arch>, _Arch>
__slist_previous(const_ptr<_Slist_node_base<_Arch>, _Arch> __head,
                 const_ptr<_Slist_node_base<_Arch>, _Arch> __node)
{
    while (__head && (*__head)->_M_next != __node){
	__head = (*__head)->_M_next;
    }

    return __head;
}

template <class _Arch>
inline void __slist_splice_after(ptr<_Slist_node_base<_Arch>, _Arch> __pos,
                                 ptr<_Slist_node_base<_Arch>, _Arch> __before_first,
                                 ptr<_Slist_node_base<_Arch>, _Arch> __before_last)
{
    if (__pos != __before_first && __pos != __before_last) {
	ptr<_Slist_node_base<_Arch>, _Arch> __first = (*__before_first)->_M_next;
	ptr<_Slist_node_base<_Arch>, _Arch> __after = (*__pos)->_M_next;
	(*__before_first)->_M_next = (*__before_last)->_M_next;
	(*__pos)->_M_next = __first;
	(*__before_last)->_M_next = __after;
    }
}

template <class _Arch>
inline void
__slist_splice_after(ptr<_Slist_node_base<_Arch>, _Arch> __pos, ptr<_Slist_node_base<_Arch>, _Arch> __head)
{
    ptr<_Slist_node_base<_Arch>, _Arch> __before_last = __slist_previous(__head);
    if (__before_last != __head) {
	ptr<_Slist_node_base<_Arch>, _Arch> __after = __pos._M_next;
	__pos._M_next = (*__head)->_M_next;
	(*__head)->_M_next = 0;
	(*__before_last)->_M_next = __after;
    }
}

template <class _Arch>
inline ptr<_Slist_node_base<_Arch>, _Arch> __slist_reverse(ptr<_Slist_node_base<_Arch>, _Arch> __node)
{
    ptr<_Slist_node_base<_Arch>, _Arch> __result = __node;
    __node = (*__node)->_M_next;
    (*__result)->_M_next = 0;
    while(__node) {
	ptr<_Slist_node_base<_Arch>, _Arch> __next = (*__node)->_M_next;
	(*__node)->_M_next = __result;
	__result = __node;
	__node = __next;
    }
    return __result;
}

template <class _Arch>
inline typename _Arch::size_type __slist_size(ptr<_Slist_node_base<_Arch>, _Arch> __node)
{
    typename _Arch::size_type __result = 0;
    for ( ; __node != 0; __node = (*__node)->_M_next)
	++__result;
    return __result;
}


//template <class _Tp>
//struct _Slist_node : public _Slist_node_base
//{
//  _Tp _M_data;
//};



template <class _Tp, class _Arch>
struct _Slist_node : public _Slist_node_base<_Arch> {
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;
  
    _Tp _M_data;
    
    ptr<_Tp, arch_type> get_M_data_pointer(ptr<_Slist_node, arch_type> _M_this) {
	ptr_int _M_data_addr = reinterpret_cast<ptr_int>(&_M_data);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return ptr<_Tp, arch_type>(_M_this.getAddress() +
				   (_M_data_addr - this_addr),
				   _M_this.get_pheap());
    }
    const_ptr<_Tp, arch_type> get_M_data_pointer(ptr<_Slist_node, arch_type> _M_this) const {
	ptr_int _M_data_addr = reinterpret_cast<ptr_int>(&_M_data);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return const_ptr<_Tp, arch_type>(_M_this.getAddress() +
					 (_M_data_addr - this_addr),
					 _M_this.get_pheap());
    }
    ref<_Tp, arch_type> get_M_data(ptr<_Slist_node, arch_type> _M_this) {
	return *get_M_data_pointer(_M_this);
    }
    const_ref<_Tp, arch_type> get_M_data(ptr<_Slist_node, arch_type> _M_this) const {
	return *get_M_data_pointer(_M_this);
    }
};

template <class _Tp, class _Arch>
struct replace_pheap<_Slist_node<_Tp, _Arch> > {
    void operator()(const pheap_ptr __ph, _Slist_node<_Tp, _Arch> &__obj) {
	typedef _Slist_node_base<_Arch> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
	__replace_pheap(__ph, __obj._M_data);
    }
};

template <class _Arch>
struct _Slist_iterator_base {
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef bidirectional_iterator_tag               iterator_category;
    typedef pheap<arch_type>                         pheap_type;

    ptr<_Slist_node_base<arch_type>, arch_type> _M_node;
    _Slist_iterator_base(ptr<_Slist_node_base<arch_type>, arch_type> __x) : _M_node(__x) {}
    _Slist_iterator_base() {}

    void _M_incr() { _M_node = (*_M_node)->_M_next; }

    bool operator==(const _Slist_iterator_base &__x) const {
	return _M_node == __x._M_node;
    }
    bool operator!=(const _Slist_iterator_base &__x) const {
	return _M_node != __x._M_node;
    }
};

template <class _Arch>
struct replace_pheap<_Slist_iterator_base<_Arch> > {
    void operator()(pheap_ptr __ph, _Slist_iterator_base<_Arch> &__obj) {
	__replace_pheap(__ph, __obj._M_node);
    }
};

template <class _Tp, class _Ref, class _Ptr, class _Arch>
struct _Slist_iterator : public _Slist_iterator_base<_Arch>{
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;
    typedef _Slist_iterator<_Tp, ref<_Tp, arch_type>, ptr<_Tp, arch_type>, arch_type>
    iterator;
    typedef _Slist_iterator<_Tp, const_ref<_Tp, arch_type>, const_ptr<_Tp, arch_type>, arch_type>
    const_iterator;
    typedef _Slist_iterator<_Tp, _Ref, _Ptr, _Arch>   _Self;

    typedef _Tp value_type;
    typedef _Ptr pointer;
    typedef _Ref reference;
    typedef _Slist_node<_Tp, _Arch> _Node;

    _Slist_iterator(ptr<_Node, arch_type> __x) : _Slist_iterator_base<_Arch>(__x) {}
    _Slist_iterator() {}
    _Slist_iterator(const iterator &__x) : _Slist_iterator_base<_Arch>(__x._M_node) {}
    _Slist_iterator(const_ref<iterator, arch_type> __x) : _Slist_iterator_base<_Arch>(__x->_M_node) {}

    reference operator*() const {
	ptr<_Slist_node<_Tp, arch_type>, arch_type> _M_node_ptr(this->_M_node);
	return (*_M_node_ptr)->get_M_data(_M_node_ptr);
    }
  

#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    pointer operator->() const {
	ptr<_Slist_node<_Tp, arch_type>, arch_type> _M_node_ptr(this->_M_node);
	return (*_M_node_ptr)->get_M_data_pointer(_M_node_ptr);
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
    
    ptr<_Node, arch_type> get_node() {
	return _Slist_iterator_base<_Arch>::_M_node;
    }
};

#ifndef __PSTL_CLASS_PARTIAL_SPECIALIZATION

template <class _Arch>
inline typename _Arch::difference_type *
distance_type(const _Slist_iterator_base<_Arch> &)
{
    return reinterpret_cast<typename _Arch::difference_type *>(0);
}

inline bidirectional_iterator_tag
iterator_category(const _Slist_iterator_base &)
{
    return bidirectional_iterator_tag();
}

template <class _Tp, class _Ref, class _Ptr, class _Arch>
inline ptr<_Tp, _Arch>
value_type(const _Slist_iterator<_Tp, _Ref, _Ptr> &)
{
    return ptr<_Tp, _Arch>(0);
}

#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

//7

// Base class that encapsulates details of allocators.  Three cases:
// an ordinary standard-conforming allocator, a standard-conforming
// allocator with no non-static data, and an SGI-style allocator.
// This complexity is necessary only because we're worrying about backward
// compatibility and because we want to avoid wasting storage on an 
// allocator instance if it isn't necessary.

#ifdef __PSTL_USE_STD_ALLOCATORS

// Base for general standard-conforming allocators.
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic>
class _Slist_alloc_base {
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

    _Slist_alloc_base(const allocator_type &__a) : _Node_allocator(__a) {
	_M_head = _M_get_node();
    }
    ~_Slist_alloc_base() { _M_put_node(_M_head); }

protected:
    ptr<_Slist_node<_Tp, arch_type>, arch_type> _M_get_node()
	{ return _Node_allocator.allocate(1); }
    void _M_put_node(ptr<_Slist_node<_Tp, arch_type>, arch_type> __p)
	{ _Node_allocator.deallocate(__p, 1); }

protected:
    typename _Alloc_traits<_Slist_node<_Tp, arch_type>, arch_type, _Allocator>::allocator_type
    _Node_allocator;
    // _M_head originally has a type of _Slist_node_base in the SGI STL.
    // PSTL version slightly changed this implementation to that
    // similar to pstl_list.
    ptr<_Slist_node_base<arch_type>, arch_type> _M_head;
  
  
    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Allocator, bool _IsStatic>
struct replace_pheap<_Slist_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> > {
    void operator()(pheap_ptr __ph, _Slist_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> &__obj) {
	__replace_pheap(__ph, __obj._Node_allocator);
	__replace_pheap(__ph, __obj._M_head);
    }
};

// Specialization for instanceless allocators.

template <class _Tp, class _Arch, class _Allocator>
class _Slist_alloc_base<_Tp, _Arch, _Allocator, true> {
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

    _Slist_alloc_base(const allocator_type &__a) {
	__replace_pheap(__a.get_pheap(), _M_head);
	_M_head = _M_get_node();
    }
    ~_Slist_alloc_base() {
	_M_put_node(_M_head);
    }

protected:
    typedef typename _Alloc_traits<_Slist_node<_Tp, arch_type>, arch_type, _Allocator>::_Alloc_type
    _Alloc_type;

    ptr<_Slist_node<_Tp, arch_type>, arch_type> _M_get_node() { 	
	return ptr<_Slist_node<_Tp, arch_type>, arch_type>(pheap_type::pheap_convert(_M_head.get_pheap())->malloc(sizeof(_Slist_node<_Tp, arch_type>)), _M_head.get_pheap());
    }
    void _M_put_node(ptr<_Slist_node<_Tp, arch_type>, arch_type> __p) {
	pheap_type::pheap_convert(_M_head.get_pheap())->free(static_cast<ptr<void, arch_type> >(__p)); //, sizeof(_Slist_node<_Tp, arch_type>));
    }
  
protected:
    // _M_head originally has a type of _Slist_node_base in the SGI STL.
    // PSTL version slightly changed this implementation to that
    // similar to pstl_list.
    ptr<_Slist_node_base<arch_type>, arch_type> _M_head;

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Allocator>
struct replace_pheap<_Slist_alloc_base<_Tp, _Arch, _Allocator, true> > {
    void operator()(pheap_ptr __ph, _Slist_alloc_base<_Tp, _Arch, _Allocator, true> &__obj) {
	__replace_pheap(__ph, __obj._M_head);
    }
};

template <class _Tp, class _Arch, class _Allocator>
struct _Slist_base 
    : public _Slist_alloc_base<_Tp, _Arch, _Allocator,
			       _Alloc_traits<_Tp, _Arch, _Allocator>::_S_instanceless> {
public:
    typedef _Slist_alloc_base<_Tp, _Arch, _Allocator,
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
    using _Base::_M_head;
    using _Base::_M_get_node;
    using _Base::_M_put_node;
#endif // __PSTL_HAS_NAMESPACES
    
    _Slist_base(const allocator_type &__a) : _Base(__a) {
	(*_M_head)->_M_next = 0;
    }
    ~_Slist_base() {
	_M_erase_after(_M_head, 0);
    }
  
    ptr<_Slist_node_base<_Arch>, _Arch> _M_erase_after(ptr<_Slist_node_base<_Arch>, _Arch> __pos)
	{
	    ptr<_Slist_node<_Tp, _Arch>, _Arch> __next = (ptr<_Slist_node<_Tp, _Arch>, _Arch>) ((*__pos)->_M_next);
	    ptr<_Slist_node_base<_Arch>, _Arch> __next_next = (*__next)->_M_next;
	    (*__pos)->_M_next = __next_next;
	    pstl_Destroy((*__next)->get_M_data_pointer(__next));
	    _M_put_node(__next);
	    return __next_next;
	}
    ptr<_Slist_node_base<_Arch>, _Arch> _M_erase_after(ptr<_Slist_node_base<_Arch>, _Arch>, ptr<_Slist_node_base<_Arch>, _Arch>);
    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<_Slist_base<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _Slist_base<_Tp, _Arch, _Alloc> &__obj) {
	typedef _Slist_alloc_base<_Tp, _Arch, _Alloc, _Alloc_traits<_Tp, _Arch, _Alloc>::_S_instanceless> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

#else /* __PSTL_USE_STD_ALLOCATORS */

template <class _Tp, class _Arch, class _Alloc>
struct _Slist_base {
    typedef _Alloc                                   allocator_type;
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;

    allocator_type get_allocator() const { return allocator_type(); }

    _Slist_base(const allocator_type &) {
	(*_M_head)->_M_next = 0;
    }
    ~_Slist_base() { _M_erase_after(_M_head, 0); }

    typedef simple_alloc<_Slist_node<_Tp, arch_type>, arch_type, _Alloc> _Alloc_type;
    ptr<_Slist_node<_Tp, arch_type>, arch_type> _M_get_node() { return _Alloc_type::allocate(1); }
    void _M_put_node(ptr<_Slist_node<_Tp, arch_type>, arch_type> __p) { _Alloc_type::deallocate(__p, 1); } 

    ptr<_Slist_node_base<_Arch>, _Arch> _M_erase_after(ptr<_Slist_node_base<_Arch>, _Arch> __pos)
	{
	    ptr<_Slist_node<_Tp, _Arch>, _Arch>  __next = (ptr<_Slist_node<_Tp, _Arch>, _Arch>) (__pos._M_next);
	    ptr<_Slist_node_base, _Arch> __next_next = (*__next)->_M_next;
	    __pos._M_next = __next_next;
	    pstl_Destroy((*__next)->get_M_data_pointer(__next));
	    _M_put_node(__next);
	    return __next_next;
	}
    ptr<_Slist_node_base<_Arch>, _Arch> _M_erase_after(ptr<_Slist_node_base<_Arch>, _Arch>, ptr<_Slist_node_base<_Arch>, _Arch>);

protected:
    // _M_head originally has a type of _Slist_node_base in the SGI STL.
    // PSTL version slightly changed this implementation to that
    // similar to pstl_list.
    ptr<_Slist_node_base<_Tp, arch_type>, arch_type> _M_head;
  
    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<_Slist_base<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, _Slist_base<_Tp, _Arch, _Alloc> &__obj) {
	__replace_pheap(__ph, __obj._M_head);
    }
};

#endif /* __PSTL_USE_STD_ALLOCATORS */

template <class _Tp, class _Arch, class _Alloc> 
ptr<_Slist_node_base<_Arch>, _Arch>
_Slist_base<_Tp,_Arch, _Alloc>::_M_erase_after(ptr<_Slist_node_base<_Arch>, _Arch> __before_first,
                                               ptr<_Slist_node_base<_Arch>, _Arch> __last_node) {
    ptr<_Slist_node<_Tp, _Arch>, _Arch>__cur = (ptr<_Slist_node<_Tp, _Arch>, _Arch>) ((*__before_first)->_M_next);
    while (__cur != __last_node) {
	ptr<_Slist_node<_Tp, _Arch>, _Arch> __tmp = __cur;
	__cur = (ptr<_Slist_node<_Tp, _Arch>, _Arch>) (*__cur)->_M_next;
	pstl_Destroy((*__tmp)->get_M_data_pointer(__tmp));
	//pstl_Destroy((*__tmp)->_M_data);
	_M_put_node(__tmp);
    }
    (*__before_first)->_M_next = __last_node;
    return __last_node;
}

template <class _Tp, class _Arch, class _Alloc = __PSTL_DEFAULT_ALLOCATOR(_Tp, _Arch) >
class slist : private _Slist_base<_Tp,_Arch,_Alloc>
{
    // requirements:

    __PSTL_CLASS_REQUIRES(_Tp, _Assignable);
  
    typedef _Arch                                    arch_type;
    typedef typename arch_type::dev_type             dev_type;
    typedef typename arch_type::bit_type             bit_type;
    typedef typename arch_type::size_type            size_type;
    typedef typename arch_type::difference_type      difference_type;
    typedef pheap<arch_type>                         pheap_type;

    typedef _Slist_base<_Tp, _Arch, _Alloc> _Base;

protected:
    typedef ptr<void, arch_type> _Void_pointer;
private:
    typedef _Slist_node<_Tp, _Arch>      _Node;
    typedef _Slist_node_base<_Arch>      _Node_base;
    typedef _Slist_iterator_base<_Arch>  _Iterator_base;

public:
    typedef _Tp value_type;
    typedef ptr<value_type, arch_type> pointer;
    typedef const_ptr<value_type, arch_type> const_pointer;
    typedef ref<value_type, arch_type> value_reference;
    typedef const_ref<value_type, arch_type> const_value_reference;    
    typedef ref<slist, arch_type> reference;
    typedef const_ref<slist, arch_type> const_reference;
    typedef _Node node_type;

    //12
    typedef _Slist_iterator<_Tp, ref<_Tp, arch_type>, ptr<_Tp, arch_type>, arch_type>
    iterator;
    typedef _Slist_iterator<_Tp, const_ref<_Tp, arch_type>, const_ptr<_Tp, arch_type>, arch_type>
    const_iterator;

    //13
    typedef typename _Base::allocator_type allocator_type;
    allocator_type get_allocator() const { return _Base::get_allocator(); }

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

    //14
private:
#ifdef __PSTL_HAS_NAMESPACES
    using _Base::_M_head;
    using _Base::_M_put_node;
    using _Base::_M_get_node;
#endif /* __PSTL_HAS_NAMESPACES */
  
    ptr<_Node, arch_type> _M_create_node(const _Tp &__x)
	{
	    ptr<_Node, arch_type> __p = _M_get_node();
	    __PSTL_TRY {
		pstl_Construct((*__p)->get_M_data(__p), __x);
		//std::cout << "_M_create_node 3" << std::endl;
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
    slist() : _Base(allocator_type()) {}

    explicit slist(const pheap_ptr __ph)
	: _Base(allocator_type(__ph))
	{}
    
    explicit slist(const allocator_type &__a)
	: _Base(__a)
	{}
    
    slist(const pheap_ptr __ph, size_type __n, const value_type &__x)
	: _Base(allocator_type(__ph))
	{ _M_insert_after_fill(_M_head, __n, __x); }
    slist(size_type __n, const value_type& __x,
    	  const allocator_type& __a) : _Base(__a)
	{ _M_insert_after_fill(_M_head, __n, __x); }

    slist(const pheap_ptr __ph, size_type __n) :
	_Base(allocator_type(__ph))
	{ _M_insert_after_fill(_M_head, __n, value_type()); }
    slist(size_type __n, const allocator_type &__a) :
	_Base(__a)
	{ _M_insert_after_fill(_M_head, __n, value_type()); }
    
#ifdef __PSTL_MEMBER_TEMPLATES
    // We don't need any dispatching tricks here, because _M_insert_after_range
    // already does them.
    template <class _InputIterator>
    slist(const pheap_ptr __ph, _InputIterator __first, _InputIterator __last) :
	_Base(allocator_type(__ph))
	{ _M_insert_after_range(_M_head, __first, __last); }

    template <class _InputIterator>
    slist(_InputIterator __first, _InputIterator __last,
	  const allocator_type &__a) : _Base(__a)
	{ _M_insert_after_range(_M_head, __first, __last); }

#else /* __PSTL_MEMBER_TEMPLATES */
    slist(const pheap_ptr __ph, const_iterator __first, const_iterator __last) : _Base(allocator_type(__ph))
	{ _M_insert_after_range(_M_head, __first, __last); }
    slist(const_iterator __first, const_iterator __last,
	  const allocator_type &__a) : _Base(__a)
	{ _M_insert_after_range(_M_head, __first, __last); }

    slist(const pheap_ptr __ph, const_ptr <value_type, arch_type> __first, const_ptr<value_type, arch_type> __last) : _Base(allocator_type(__ph))
	{ _M_insert_after_range(_M_head, __first, __last); }
    slist(const_ptr <value_type, arch_type> __first, const_ptr<value_type, arch_type> __last,
	  const allocator_type &__a) : _Base(__a)
	{ _M_insert_after_range(_M_head, __first, __last); }
#endif /* __PSTL_MEMBER_TEMPLATES */

    slist(const slist& __x) : _Base(__x.get_allocator())
	{ _M_insert_after_range(_M_head, __x.begin(), __x.end()); }

    ref<slist, _Arch> operator= (const slist &__x);

    ~slist() {}

public:
    // assign(), a generalized assignment member function.  Two
    // versions: one that takes a count, and one that takes a range.
    // The range version is a member template, so we dispatch on whether
    // or not the type is an integer.

    void assign(size_type __n, const _Tp &__val)
	{ _M_fill_assign(__n, __val); }

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

public:

    static ptr<slist, arch_type> pnew(const pheap_ptr __ph) {
	ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	ref<slist, arch_type> r = *__p;
	new ((void*) &(r.get_obj())) slist(__ph);
	return __p;
    }
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, const allocator_type &__a) {
	ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	ref<slist, arch_type> r = *__p;
	new ((void*) &(r.get_obj())) slist(__a);
	return __p;
    }

    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, size_type __n, const value_type &__x)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__ph, __n, __x);
	    return __p;
	}
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, size_type __n, const value_type &__x,
				      const allocator_type &__a)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__n, __x, __a);
	    return __p;
	}

    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, size_type __n)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__ph, __n);
	    return __p;
	}
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, size_type __n, const allocator_type &__a)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__n, __a);
	    return __p;
	}

#ifdef __PSTL_MEMBER_TEMPLATES
    // We don't need any dispatching tricks here, because _M_insert_after_range
    // already does them.
    template <class _InputIterator>
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, _InputIterator __first, _InputIterator __last)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__ph, __first, __last);
	    return __p;
	}

    template <class _InputIterator>
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, _InputIterator __first, _InputIterator __last,
				      const allocator_type &__a)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__first, __last, __a);
	    return __p;
	}

#else /* __PSTL_MEMBER_TEMPLATES */
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, const_iterator __first, const_iterator __last) : _Base(allocator_type(__ph))
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__ph, __first, __last);
	    return __p;
	}
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, const_iterator __first, const_iterator __last,
				      const allocator_type &__a) : _Base(__a)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__first, __last, __a);
	    return __p;
	}

    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, const_ptr <value_type, arch_type> __first, const_ptr<value_type, arch_type> __last) : _Base(allocate_type(__ph))
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__ph, __first, __last);
	    return __p;
	}
    static ptr<slist, arch_type> pnew(const pheap_ptr __ph, const_ptr <value_type, arch_type> __first, const_ptr<value_type, arch_type> __last,
				      const allocator_type &__a) : _Base(__a)
	{
	    ptr<slist, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(slist)), __ph);
	    ref<slist, arch_type> r = *__p;
	    new ((void*) &(r.get_obj())) slist(__first, __last, __a);
	    return __p;
	}
#endif /* __PSTL_MEMBER_TEMPLATES */
  
    iterator begin()             { return static_cast<ptr<_Node, arch_type> >((*_M_head)->_M_next); }
    const_iterator begin() const { return static_cast<ptr<_Node, arch_type> >((*_M_head)->_M_next); }

    iterator end()             { return iterator(0); }
    const_iterator end() const { return const_iterator(0); }

    // Experimental new feature: before_begin() returns a
    // non-dereferenceable iterator that, when incremented, yields
    // begin().  This iterator may be used as the argument to
    // insert_after, erase_after, etc.  Note that even for an empty 
    // slist, before_begin() is not the same iterator as end().  It 
    // is always necessary to increment before_begin() at least once to
    // obtain end().
    //iterator before_begin() { return iterator((_Node*) &this->_M_head); }
    iterator before_begin() {
	return iterator(ptr<_Node, _Arch>(_M_head));
    }
    //const_iterator before_begin() const
    //{ return const_iterator((_Node*) &this->_M_head); }
    const_iterator before_begin() const
	{ return const_iterator(ptr<_Node, _Arch>(_M_head)); }

    //17
    size_type size() const {
	size_type __result = 0;
	distance(begin(), end(), __result);
	return __result;
    }
    size_type max_size() const { return size_type(-1); }
  
    bool empty() const { return (*_M_head)->_M_next == 0; }

    void swap(slist<_Tp, _Arch, _Alloc> &__x) { pstl_swap((*_M_head)->_M_next, (*__x._M_head)->_M_next); }

public:
    value_reference front() {
	ptr<_Slist_node<_Tp, arch_type>, arch_type> next_ptr =
	    ptr<_Slist_node<_Tp, arch_type>, arch_type>((*_M_head)->_M_next);
	return (*next_ptr)->get_M_data(next_ptr);
    }
    
    const_value_reference front() const 
	{ 
	ptr<_Slist_node<_Tp, arch_type>, arch_type> next_ptr =
	    ptr<_Slist_node<_Tp, arch_type>, arch_type>((*_M_head)->_M_next);
	return (*next_ptr)->get_M_data(next_ptr);
	}
    
    void push_front(const value_type &__x)   {
	__slist_make_link(_M_head, ptr<_Node_base, _Arch>(_M_create_node(__x)));
    }
    void push_front() { __slist_make_link(_M_head, ptr<_Node_base, _Arch>(_M_create_node())); }
    void pop_front() {
	ptr<_Node, _Arch> __node = (*_M_head)->_M_next;
	(*_M_head)->_M_next = (*__node)->_M_next;
	pstl_Destroy((*__node)->get_M_data_pointer(__node));
	_M_put_node(__node);
    }

    iterator previous(const_iterator __pos) {
	return iterator(ptr<_Node, arch_type>(__slist_previous(_M_head, __pos._M_node)));
    }
    const_iterator previous(const_iterator __pos) const {
	return const_iterator(ptr<_Node, arch_type>(__slist_previous(_M_head, __pos._M_node)));
    }

private:
    ptr<_Node, _Arch> _M_insert_after(ptr<_Node_base, _Arch> __pos, const value_type &__x) {
	return ptr<_Node, _Arch>(__slist_make_link(__pos, (ptr<_Node_base, _Arch>) _M_create_node(__x)));
    }

    ptr<_Node, _Arch> _M_insert_after(ptr<_Node_base, _Arch> __pos) {
	return ptr<_Node, _Arch> (__slist_make_link(__pos, (ptr<_Node_base, _Arch>) _M_create_node()));
    }

    void _M_insert_after_fill(ptr<_Node_base, _Arch> __pos,
			      size_type __n, const value_type &__x) {
	for (size_type __i = 0; __i < __n; ++__i)
	    __pos = __slist_make_link(__pos, (ptr<_Node_base, _Arch>) _M_create_node(__x));
    }

#ifdef __PSTL_MEMBER_TEMPLATES

    // Check whether it's an integral type.  If so, it's not an iterator.
    template <class _InIter>
    void _M_insert_after_range(ptr<_Node_base, _Arch> __pos, 
			       _InIter __first, _InIter __last) {
	typedef typename _Is_integer<_InIter>::_Integral _Integral;
	_M_insert_after_range(__pos, __first, __last, _Integral());
    }

    template <class _Integer>
    void _M_insert_after_range(ptr<_Node_base, _Arch> __pos,
			       _Integer __n, _Integer __x,
			       __true_type) {
	_M_insert_after_fill(__pos, __n, __x);
    }

    template <class _InIter>
    void _M_insert_after_range(ptr<_Node_base, _Arch> __pos,
			       _InIter __first, _InIter __last,
			       __false_type) {
	while (__first != __last) {
	    __pos = __slist_make_link(__pos, (ptr<_Node_base, _Arch>) _M_create_node(*__first));
	    ++__first;
	}
    }

#else /* __PSTL_MEMBER_TEMPLATES */

    void _M_insert_after_range(ptr<_Node_base, _Arch> __pos,
			       const_iterator __first, const_iterator __last) {
	while (__first != __last) {
	    __pos = __slist_make_link(__pos, (ptr<_Node_base, _Arch>) _M_create_node(*__first));
	    ++__first;
	}
    }
    void _M_insert_after_range(ptr<_Node_base, _Arch> __pos,
			       const_ptr<value_type, _Arch> __first,
			       const_ptr<value_type, _Arch> __last) {
	while (__first != __last) {
	    __pos = __slist_make_link(__pos, (ptr<_Node_base, _Arch>) _M_create_node(*__first));
	    ++__first;
	}
    }

#endif /* __PSTL_MEMBER_TEMPLATES */

public:

    iterator insert_after(iterator __pos, const value_type &__x) {
	return iterator(_M_insert_after(__pos._M_node, __x));
    }

    iterator insert_after(iterator __pos) {
	return insert_after(__pos, value_type());
    }

    void insert_after(iterator __pos, size_type __n, const value_type &__x) {
	_M_insert_after_fill(__pos._M_node, __n, __x);
    }

#ifdef __PSTL_MEMBER_TEMPLATES

    // We don't need any dispatching tricks here, because _M_insert_after_range
    // already does them.
    template <class _InIter>
    void insert_after(iterator __pos, _InIter __first, _InIter __last) {
	_M_insert_after_range(__pos._M_node, __first, __last);
    }

#else /* __PSTL_MEMBER_TEMPLATES */

    void insert_after(iterator __pos,
		      const_iterator __first, const_iterator __last) {
	_M_insert_after_range(__pos._M_node, __first, __last);
    }
    void insert_after(iterator __pos,
		      const_ptr<value_type, _Arch> __first, const_ptr<value_type, _Arch> __last) {
	_M_insert_after_range(__pos._M_node, __first, __last);
    }

#endif /* __PSTL_MEMBER_TEMPLATES */

    iterator insert(iterator __pos, const value_type &__x) {
	const_ptr<_Slist_node_base<_Arch>, _Arch> __head = _M_head;
	const_ptr<_Slist_node_base<_Arch>, _Arch> __node = __pos._M_node;
	return iterator(_M_insert_after(__slist_previous(_M_head,
							 __pos._M_node),
					__x));
    }

    iterator insert(iterator __pos) {
	return iterator(_M_insert_after(__slist_previous(_M_head,
							 __pos._M_node),
					value_type()));
    }

    void insert(iterator __pos, size_type __n, const value_type &__x) {
	_M_insert_after_fill(__slist_previous(_M_head, __pos._M_node),
			     __n, __x);
    } 
    
#ifdef __PSTL_MEMBER_TEMPLATES
    // We don't need any dispatching tricks here, because _M_insert_after_range
    // already does them.
    template <class _InIter>
    void insert(ptr<slist, arch_type> __this, iterator __pos, _InIter __first, _InIter __last) {
	_M_insert_after_range(__slist_previous((*__this)->_M_head, __pos._M_node), 
			      __first, __last);
    }

#else /* __PSTL_MEMBER_TEMPLATES */

    void insert(ptr<slist, arch_type> __this, iterator __pos, const_iterator __first, const_iterator __last) {
	_M_insert_after_range(__slist_previous((*__this)->_M_head, __pos._M_node), 
			      __first, __last);
    }
    void insert(ptr<slist, arch_type> __this, iterator __pos, const value_type* __first, 
		const value_type* __last) {
	_M_insert_after_range(__slist_previous((*__this)->_M_head, __pos._M_node), 
			      __first, __last);
    }

#endif /* __PSTL_MEMBER_TEMPLATES */

public:
    iterator erase_after(iterator __pos) {
	return iterator(*this->_M_erase_after(__pos._M_node));
    }
    iterator erase_after(iterator __before_first, iterator __last) {
	return iterator(*this->_M_erase_after(__before_first._M_node, 
					      __last._M_node));
    } 

    iterator erase(iterator __pos) {
	return (ptr<_Node, _Arch>) this->_M_erase_after(__slist_previous(_M_head, __pos._M_node));
    }

    iterator erase(iterator __first, iterator __last) {
	return (ptr<_Node, _Arch>) this->_M_erase_after(__slist_previous(_M_head, __first._M_node), __last._M_node);
    }

    void resize(size_type new_size, const _Tp &__x);
    void resize(size_type new_size) { resize(new_size, _Tp()); }
    void clear() { this->_M_erase_after(_M_head, 0); }

public:
    // Moves the range [__before_first + 1, __before_last + 1) to *this,
    //  inserting it immediately after __pos.  This is constant time.
    void splice_after(iterator __pos, 
		      iterator __before_first, iterator __before_last)
	{
	    if (__before_first != __before_last) 
		__slist_splice_after(__pos._M_node, __before_first._M_node, 
				     __before_last._M_node);
	}

    // Moves the element that follows __prev to *this, inserting it immediately
    //  after __pos.  This is constant time.
    void splice_after(iterator __pos, iterator __prev)
	{
	    __slist_splice_after(__pos._M_node,
				 __prev._M_node, __prev._M_node->_M_next);
	}

    // Removes all of the elements from the list __x to *this, inserting
    // them immediately after __pos.  __x must not be *this.  Complexity:
    // linear in __x.size().
    void splice_after(iterator __pos, slist &__x)
	{
	    __slist_splice_after(__pos._M_node, __x._M_head);
	}

    // Linear in distance(begin(), __pos), and linear in __x.size().
    void splice(iterator __pos, slist &__x) {
	if (__x._M_head._M_next)
	    __slist_splice_after(__slist_previous(_M_head, __pos._M_node),
				 __x._M_head, __slist_previous(__x._M_head, 0));
    }

    // Linear in distance(begin(), __pos), and in distance(__x.begin(), __i).
    void splice(iterator __pos, slist &__x, iterator __i) {
	__slist_splice_after(__slist_previous(_M_head, __pos._M_node),
			     __slist_previous(__x._M_head, __i._M_node),
			     __i._M_node);
    }

    // Linear in distance(begin(), __pos), in distance(__x.begin(), __first),
    // and in distance(__first, __last).
    void splice(iterator __pos, slist &__x, iterator __first, iterator __last)
	{
	    if (__first != __last)
		__slist_splice_after(__slist_previous(_M_head, __pos._M_node),
				     __slist_previous(__x._M_head, __first._M_node),
				     __slist_previous(__first._M_node, __last._M_node));
	}

public:
    void reverse() { 
	if ((*_M_head)->_M_next)
	    (*_M_head)->_M_next = __slist_reverse((*_M_head)->_M_next);
    }

    void remove(const _Tp &__val); 
    void unique(); 
    void merge(slist &__x);
    void sort();     

#ifdef __PSTL_MEMBER_TEMPLATES
    template <class _Predicate> 
    void remove_if(_Predicate __pred);

    template <class _BinaryPredicate> 
    void unique(_BinaryPredicate __pred); 

    template <class _StrictWeakOrdering> 
    void merge(slist&, _StrictWeakOrdering);

    template <class _StrictWeakOrdering> 
    void sort(_StrictWeakOrdering __comp); 
#endif /* __PSTL_MEMBER_TEMPLATES */
    template <class _Obj> friend struct replace_pheap;

};


//24
template <class _Tp, class _Arch, class _Alloc>
ref<slist<_Tp, _Arch, _Alloc>, _Arch> slist<_Tp, _Arch, _Alloc>::operator=(const slist<_Tp, _Arch, _Alloc> &__x)
{
    if (&__x != this) {
	ptr<_Node_base, _Arch> __p1 = _M_head;
	ptr<_Node, _Arch> __n1 = (ptr<_Node, _Arch>) (*_M_head)->_M_next;
	const_ptr<_Node, _Arch> __n2 = (const_ptr<_Node, _Arch>) (*(*__x)->_M_head)->_M_next;
	while (__n1 && __n2) {
	    (*__n1)->get_M_data(__n1) = (*__n2)->get_M_data(__n2);
	    __p1 = __n1;
	    __n1 = (ptr<_Node, _Arch>) (*__n1)->_M_next;
	    __n2 = (const_ptr<_Node, _Arch>) (*__n2)->_M_next;
	}
	if (__n2 == 0)
	    this->_M_erase_after(__p1, 0);
	else
	    _M_insert_after_range(__p1, const_iterator((ptr<_Node, _Arch>) __n2),
				  const_iterator(0));
    }
    return *this;
}

template <class _Tp, class _Arch, class _Alloc>
void slist<_Tp, _Arch, _Alloc>::_M_fill_assign(size_type __n, const _Tp &__val) {
    ptr<_Node_base, _Arch> __prev = _M_head;
    ptr<_Node, _Arch> __node = (ptr<_Node, _Arch>) (*_M_head)->_M_next;
    for ( ; __node != 0 && __n > 0 ; --__n) {
	(*__node)->get_M_data(__node) = __val;
	__prev = __node;
	__node = (ptr<_Node, _Arch>) (*__node)->_M_next;
    }
    if (__n > 0)
	_M_insert_after_fill(__prev, __n, __val);
    else
	this->_M_erase_after(__prev, 0);
}

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> template <class _InputIter>
void
slist<_Tp, _Arch, _Alloc>::_M_assign_dispatch(_InputIter __first, _InputIter __last,
                                              __false_type)
{
    ptr<_Node_base, _Arch> __prev = _M_head;
    ptr<_Node, _Arch> __node = (ptr<_Node, _Arch>) (*_M_head)->_M_next;
    while (__node != 0 && __first != __last) {
	(*__node)->get_M_data(__node) = *__first;
	__prev = __node;
	__node = (ptr<_Node, _Arch>) (*__node)->_M_next;
	++__first;
    }
    if (__first != __last)
	_M_insert_after_range(__prev, __first, __last);
    else
	this->_M_erase_after(__prev, 0);
}

#endif /* __PSTL_MEMBER_TEMPLATES */

//23
template <class _Tp, class _Arch, class _Alloc>
struct replace_pheap<slist<_Tp, _Arch, _Alloc> > {
    void operator()(pheap_ptr __ph, slist<_Tp, _Arch, _Alloc> &__obj) {
	typedef _Slist_base<_Tp, _Arch, _Alloc> _Base;
	_Base &__o = static_cast<_Base &>(__obj);
	__replace_pheap(__ph, __o);
    }
};

template <class _Tp, class _Alloc>
inline bool 
operator==(const slist<_Tp,_Alloc> &_SL1, const slist<_Tp,_Alloc> &_SL2)
{
    typedef typename slist<_Tp,_Alloc>::const_iterator const_iterator;
    const_iterator __end1 = _SL1.end();
    const_iterator __end2 = _SL2.end();

    const_iterator __i1 = _SL1.begin();
    const_iterator __i2 = _SL2.begin();
    while (__i1 != __end1 && __i2 != __end2 &&
	   (*__i1).get_obj() == (*__i2).get_obj()) {
	++__i1;
	++__i2;
    }
    return __i1 == __end1 && __i2 == __end2;
}


template <class _Tp, class _Alloc>
inline bool
operator<(const slist<_Tp,_Alloc> &_SL1, const slist<_Tp,_Alloc> &_SL2)
{
    return lexicographical_compare(_SL1.begin(), _SL1.end(), 
				   _SL2.begin(), _SL2.end());
}

#ifdef __PSTL_FUNCTION_TMPL_PARTIAL_ORDER

template <class _Tp, class _Alloc>
inline bool 
operator!=(const slist<_Tp,_Alloc> &_SL1, const slist<_Tp,_Alloc> &_SL2) {
    return !(_SL1 == _SL2);
}

template <class _Tp, class _Alloc>
inline bool 
operator>(const slist<_Tp,_Alloc> &_SL1, const slist<_Tp,_Alloc> &_SL2) {
    return _SL2 < _SL1;
}

template <class _Tp, class _Alloc>
inline bool 
operator<=(const slist<_Tp,_Alloc> &_SL1, const slist<_Tp,_Alloc> &_SL2) {
    return !(_SL2 < _SL1);
}

template <class _Tp, class _Alloc>
inline bool 
operator>=(const slist<_Tp,_Alloc> &_SL1, const slist<_Tp,_Alloc> &_SL2) {
    return !(_SL1 < _SL2);
}

template <class _Tp, class _Alloc>
inline void swap(slist<_Tp,_Alloc> &__x, slist<_Tp,_Alloc> &__y) {
    __x.swap(__y);
}

#endif /* __PSTL_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _Tp, class _Arch, class _Alloc>
void slist<_Tp, _Arch, _Alloc>::resize(size_type __len, const _Tp &__x)
{
    ptr<_Node_base, _Arch> __cur = _M_head;
    while ((*__cur)->_M_next != 0 && __len > 0) {
	--__len;
	__cur = (*__cur)->_M_next;
    }
    if ((*__cur)->_M_next) 
	this->_M_erase_after(__cur, 0);
    else
	_M_insert_after_fill(__cur, __len, __x);
}

template <class _Tp, class _Arch, class _Alloc>
void slist<_Tp, _Arch, _Alloc>::remove(const _Tp &__val)
{
    ptr<_Node_base, _Arch> __cur = _M_head;
    while (__cur && (*__cur)->_M_next) {
	ptr<_Node, _Arch> __next = (ptr<_Node, _Arch>) (*__cur)->_M_next;
	if ((*__next)->get_M_data(__next).get_obj() == __val)
	    this->_M_erase_after(__cur);
	else
	    __cur = (*__cur)->_M_next;
    }
}

template <class _Tp, class _Arch, class _Alloc> 
void slist<_Tp, _Arch, _Alloc>::unique()
{
    ptr<_Node_base, _Arch> __cur = (*_M_head)->_M_next;
    if (__cur) {
	while ((*__cur)->_M_next) {
	    if ((*(ptr<_Node, _Arch>) __cur)->get_M_data((ptr<_Node, _Arch>) __cur) == 
		(*(ptr<_Node, _Arch>) (*__cur)->_M_next)->get_M_data((ptr<_Node, _Arch>) (*__cur)->_M_next))
		this->_M_erase_after(__cur);
	    else
		__cur = (*__cur)->_M_next;
	}
    }
}

template <class _Tp, class _Arch, class _Alloc>
void slist<_Tp, _Arch, _Alloc>::merge(slist<_Tp, _Arch, _Alloc> &__x)
{
    ptr<_Node_base, _Arch> __n1 = _M_head;
    while ((*__n1)->_M_next && (*__x._M_head)->_M_next) {
	ptr<_Node, _Arch> __next = (ptr<_Node, _Arch>) (*__x._M_head)->_M_next;
	ptr<_Node, _Arch> __next1 = (ptr<_Node, _Arch>) (*__n1)->_M_next;
	if ((*__next)->get_M_data(__next) < (*__next1)->get_M_data(__next1))
	    __slist_splice_after(__n1, __x._M_head, (*__x._M_head)->_M_next);
	__n1 = (*__n1)->_M_next;
    }
    if ((*__x._M_head)->_M_next) {
	(*__n1)->_M_next = (*__x._M_head)->_M_next;
	(*__x._M_head)->_M_next = ptr<_Node_base, _Arch>(0);
    }
}

template <class _Tp, class _Arch, class _Alloc>
void slist<_Tp, _Arch, _Alloc>::sort()
{
    if ((*_M_head)->_M_next && (*(*_M_head)->_M_next)->_M_next) {
	slist __carry(_M_head.get_pheap());
	slist *__counter[64];
	for (int i = 0; i < 64; i++) {
	    __counter[i] = new slist<_Tp, _Arch, _Alloc>(_M_head.get_pheap());
	}
	int __fill = 0;
	while (!empty()) {
	    __slist_splice_after(__carry._M_head,
				 _M_head, (*_M_head)->_M_next);
	    int __i = 0;
	    while (__i < __fill && !__counter[__i]->empty()) {
		__counter[__i]->merge(__carry);
		__carry.swap(*__counter[__i]);
		++__i;
	    }
	    __carry.swap(*__counter[__i]);
	    if (__i == __fill)
		++__fill;
	}

	for (int __i = 1; __i < __fill; ++__i)
	    __counter[__i]->merge(*__counter[__i - 1]);
	swap(*__counter[__fill - 1]);
	for (int i = 0; i < 64; i++) {
	    delete __counter[i];
	}
    }
}

#ifdef __PSTL_MEMBER_TEMPLATES

template <class _Tp, class _Arch, class _Alloc> 
template <class _Predicate>
void slist<_Tp, _Arch, _Alloc>::remove_if(_Predicate __pred)
{
    ptr<_Node_base, _Arch> __cur = _M_head;
    while ((*__cur)->_M_next) {
	if (__pred((*(ptr<_Node, _Arch>) (*__cur)->_M_next)->_M_data))
	    this->_M_erase_after(__cur);
	else
	    __cur = (*__cur)->_M_next;
    }
}

template <class _Tp, class _Arch, class _Alloc> template <class _BinaryPredicate> 
void slist<_Tp, _Arch, _Alloc>::unique(_BinaryPredicate __pred)
{
    ptr<_Node, _Arch> __cur = (ptr<_Node, _Arch>) (*_M_head)->_M_next;
    if (__cur) {
	while ((*__cur)->_M_next) {
	    if (__pred((*(ptr<_Node, _Arch>)__cur)->_M_data, 
		       (*(ptr<_Node, _Arch>)(*__cur)->_M_next)->_M_data))
		this->_M_erase_after(__cur);
	    else
		__cur = (ptr<_Node, _Arch>) (*__cur)->_M_next;
	}
    }
}

template <class _Tp, class _Arch, class _Alloc> template <class _StrictWeakOrdering>
void slist<_Tp, _Arch, _Alloc>::merge(slist<_Tp, _Arch, _Alloc> &__x,
                                      _StrictWeakOrdering __comp)
{
    ptr<_Node_base, _Arch> __n1 = _M_head;
    while ((*__n1)->_M_next && (*__x._M_head)->_M_next) {
	ptr<_Node, _Arch> __next = (ptr<_Node, _Arch>) (*__x._M_head)->_M_next;
	ptr<_Node, _Arch> __next1 = (ptr<_Node, _Arch>) (*__n1)->_M_next;
	if (__comp((*__next)->get_M_data(__next).get_obj()
		   (*__next1)->get_M_data(__next1).get_obj()))
	    __slist_splice_after(__n1, __x._M_head,
				 (*__x._M_head)->_M_next);
	__n1 = (*__n1)->_M_next;
    }
    if ((*__x._M_head)->_M_next) {
	(*__n1)->_M_next = (*__x._M_head)->_M_next;
	(*__x._M_head)->_M_next = ptr<_Slist_node_base<_Arch>, _Arch>(0);
    }
}

template <class _Tp, class _Arch, class _Alloc> template <class _StrictWeakOrdering> 
void slist<_Tp, _Arch, _Alloc>::sort(_StrictWeakOrdering __comp)
{
    if ((*_M_head)->_M_next && (*(*_M_head)->_M_next)->_M_next) {
	slist __carry(_M_head.get_pheap());
	slist *__counter[64];
	for (int i = 0; i < 64; i++) {
	    __counter[i] = new slist<_Tp, _Arch, _Alloc>(_M_head.get_pheap());
	}
	int __fill = 0;
	while (!empty()) {
	    __slist_splice_after(__carry._M_head,
				 _M_head, (*_M_head)->_M_next);
	    int __i = 0;
	    while (__i < __fill && !__counter[__i]->empty()) {
		__counter[__i]->merge(__carry, __comp);
		__carry.swap(*__counter[__i]);
		++__i;
	    }
	    __carry.swap(*__counter[__i]);
	    if (__i == __fill)
		++__fill;
	}

	for (int __i = 1; __i < __fill; ++__i)
	    __counter[__i]->merge(*__counter[__i - 1], __comp);
	swap(*__counter[__fill - 1]);
	for (int i = 0; i < 64; i++) {
	    delete __counter[i];
	}
    }
}

#endif /* __PSTL_MEMBER_TEMPLATES */

// Specialization of insert_iterator so that insertions will be constant
// time rather than linear time.

#ifdef __PSTL_CLASS_PARTIAL_SPECIALIZATION

template <class _Tp, class _Arch, class _Alloc>
class insert_iterator<slist<_Tp, _Arch, _Alloc> > {
protected:
    typedef slist<_Tp, _Arch, _Alloc> _Container;
    ptr<_Container, typename _Container::arch_type> container;
    typename _Container::iterator iter;
public:
    typedef _Container          container_type;
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                arch_type;
    typedef void                dev_type;
    typedef void                bit_type;
    typedef void                pointer;
    typedef void                reference;

    insert_iterator(_Container &__x, typename _Container::iterator __i) 
	: container(&__x) {
	if (__i == __x.begin())
	    iter = __x.before_begin();
	else
	    iter = __x.previous(__i);
    }

    insert_iterator<_Container> &
    operator=(const typename _Container::value_type &__value) { 
	iter = container->insert_after(iter, __value);
	return *this;
    }
    insert_iterator<_Container> &operator*() { return *this; }
    insert_iterator<_Container> &operator++() { return *this; }
    insert_iterator<_Container> &operator++(int) { return *this; }
};

#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#pragma reset woff 1375
#endif

__PSTL_END_NAMESPACE 

#endif /* __SGI_PSTL_INTERNAL_SLIST_H */

// Local Variables:
// mode:C++
// End:
