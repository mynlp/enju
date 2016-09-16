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

#ifndef __SGI_PSTL_INTERNAL_CONSTRUCT_H
#define __SGI_PSTL_INTERNAL_CONSTRUCT_H

#include <iostream>
#include <new>

#include "pstl_type_traits.h"

__PSTL_BEGIN_NAMESPACE

//////////////////////////////
/// Proto Type Declaration
#ifndef __PSTL_PROTO_TYPE_DECLARATION
#define __PSTL_PROTO_TYPE_DECLARATION
template <class _Obj, class _Arch> class ptr;
template <class _Obj, class _Arch> class const_ptr;
template <class _Obj, class _Arch> class ref;
template <class _Obj, class _Arch> class const_ref;
#endif // __PSTL_PROTO_TYPE_DECLARATION

// construct and destroy.  These functions are not part of the C++ standard,
// and are provided for backward compatibility with the HP STL.  We also
// provide internal names _Construct and _Destroy that can be used within
// the library, so that standard-conforming pieces don't have to rely on
// non-standard extensions.

// Internal names

template <class _T1, class _T2, class _Arch>
inline void pstl_Construct(const ptr<_T1, _Arch> &__p, const _T2 &__value) {
    const ref<_T1, _Arch> r = *__p;
    new ((void *)&(r.get_obj())) _T1(__value);
}

template <class _T1, class _Arch>
inline void pstl_Construct(const ptr<_T1, _Arch> &__p) {
    const ref<_T1, _Arch> r = *__p;
    new ((void *)&(r.get_obj())) _T1();
}

// ref
template <class _T1, class _T2, class _Arch>
inline void pstl_Construct(const ref<_T1, _Arch> &__r, const _T2 &__value) {
    new ((void *)&(__r.get_obj())) _T1(__value);
}

template <class _T1, class _T2, class _Arch>
inline void pstl_Construct(const ref<_T1, _Arch> &__r, const ref<_T2, _Arch> &__s) {
    new ((void *)&(__r.get_obj())) _T1(*__s);
}

template <class _T1, class _Arch>
inline void pstl_Construct(const ref<_T1, _Arch> &__r) {
    new ((void *)&(__r.get_obj())) _T1();
}

template <class _Tp, class _Arch>
inline void pstl_Destroy(ptr<_Tp, _Arch> __pointer) {
    (*__pointer)->~_Tp();
}

template <class _ForwardIterator>
void
pstl__destroy_aux(_ForwardIterator __first, _ForwardIterator __last, __false_type)
{
    for ( ; __first != __last; ++__first) {
	pstl_destroy(__first);
    }
}

template <class _ForwardIterator> 
inline void pstl__destroy_aux(_ForwardIterator, _ForwardIterator, __true_type) {}

template <class _ForwardIterator, class _Tp, class _Arch>
inline void 
pstl__destroy(_ForwardIterator __first, _ForwardIterator __last, ptr<_Tp, _Arch>)
{
    typedef typename __type_traits<_Tp>::has_trivial_destructor
	_Trivial_destructor;
    pstl__destroy_aux(__first, __last, _Trivial_destructor());
}

template <class _ForwardIterator>
inline void pstl_Destroy(_ForwardIterator __first, _ForwardIterator __last) {
    pstl__destroy(__first, __last, __VALUE_TYPE(__first));
}

template <class _Arch>
inline void pstl_Destroy(ptr<char, _Arch>, ptr<char, _Arch>) {}
template <class _Arch>
inline void pstl_Destroy(ptr<int, _Arch>, ptr<int, _Arch>) {}
template <class _Arch>
inline void pstl_Destroy(ptr<long, _Arch>, ptr<long, _Arch>) {}
template <class _Arch>
inline void pstl_Destroy(ptr<float, _Arch>, ptr<float, _Arch>) {}
template <class _Arch>
inline void pstl_Destroy(ptr<double, _Arch>, ptr<double, _Arch>) {}
#ifdef __PSTL_HAS_WCHAR_T
template <class _Arch>
inline void pstl_Destroy(ptr<wchar_t, _Arch>, ptr<wchar_t, _Arch>) {}
#endif /* __PSTL_HAS_WCHAR_T */

// --------------------------------------------------
// Old names from the HP STL.

template <class _T1, class _T2, class _Arch>
inline void pstl_construct(const ptr<_T1, _Arch> &__p, const _T2 &__value) {
    pstl_Construct(__p, __value);
}

template <class _T1, class _Arch>
inline void pstl_construct(const ptr<_T1, _Arch> &__p) {
    pstl_Construct(__p);
}

template <class _Tp, class _Arch>
inline void pstl_destroy(ptr<_Tp, _Arch> __pointer) {
    pstl_Destroy(__pointer);
}

template <class _ForwardIterator>
inline void pstl_destroy(_ForwardIterator __first, _ForwardIterator __last) {
    pstl_Destroy(__first, __last);
}

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_CONSTRUCT_H */

// Local Variables:
// mode:C++
// End:
