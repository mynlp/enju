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

/* NOTE: This is an internal header file, included by other STL headers.
 * You should not attempt to use it directly.
 */

#ifndef __SGI_PSTL_INTERNAL_RAW_STORAGE_ITERATOR_H
#define __SGI_PSTL_INTERNAL_RAW_STORAGE_ITERATOR_H

__PSTL_BEGIN_NAMESPACE

template <class _ForwardIterator, class _Tp>
class raw_storage_iterator {
protected:
    _ForwardIterator _M_iter;
public:
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit raw_storage_iterator(_ForwardIterator __x) : _M_iter(__x) {}
    raw_storage_iterator &operator*() { return *this; }
    raw_storage_iterator &operator=(const _Tp &__element) {
	pstl_construct(&(*_M_iter).get_obj(), __element);
	return *this;
    }        
    raw_storage_iterator<_ForwardIterator, _Tp> &operator++() {
	++_M_iter;
	return *this;
    }
    raw_storage_iterator<_ForwardIterator, _Tp> operator++(int) {
	raw_storage_iterator<_ForwardIterator, _Tp> __tmp = *this;
	++_M_iter;
	return __tmp;
    }

    template <class _Obj> friend struct replace_pheap;
};

template <class _ForwardIterator, class _Tp>
struct replace_pheap<raw_storage_iterator<_ForwardIterator, _Tp> > {
    void operator()(pheap_ptr __ph, raw_storage_iterator<_ForwardIterator, _Tp> &__it) {
	__replace_pheap(__ph, __it._M_iter);
    }
};

#ifndef __PSTL_CLASS_PARTIAL_SPECIALIZATION

template <class _ForwardIterator, class _Tp>
inline output_iterator_tag
iterator_category(const raw_storage_iterator<_ForwardIterator, _Tp> &)
{
    return output_iterator_tag();
}

#endif /* __PSTL_CLASS_PARTIAL_SPECIALIZATION */

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_RAW_STORAGE_ITERATOR_H */

// Local Variables:
// mode:C++
// End:
