// -*- C++ -*-
/*
 * Copyright (c) 1996-1997
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

#ifndef __SGI_PSTL_INTERNAL_ALLOC_H
#define __SGI_PSTL_INTERNAL_ALLOC_H

#ifdef __PSTL_STATIC_TEMPLATE_MEMBER_BUG
#  define __PSTL_USE_MALLOC
#endif

// This implements some standard node allocators.  These are
// NOT the same as the allocators in the C++ draft standard or in
// in the original STL.  They do not encapsulate different pointer
// types; indeed we assume that there is only one pointer type.
// The allocation primitives are intended to allocate individual objects,
// not larger arenas as with the original STL allocators.

#ifndef __PSTL_THROW_BAD_ALLOC
#  if defined(__PSTL_NO_BAD_ALLOC) || !defined(__PSTL_USE_EXCEPTIONS)
#    include <cstdio>
#    include <cstdlib>
#    define __PSTL_THROW_BAD_ALLOC __STD::cerr << "out of memory" << __STD::endl; exit(1)
//#    define __PSTL_THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
#  else /* Standard conforming out-of-memory handling */
#    include <new>
#    define __PSTL_THROW_BAD_ALLOC throw std::bad_alloc()
#  endif
#endif

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cassert>
#ifndef __PSTL_RESTRICT
#  define __PSTL_RESTRICT
#endif

#ifdef __PSTL_THREADS
//# include "pstl_threads.h"
# define __PSTL_NODE_ALLOCATOR_THREADS true
# ifdef __PSTL_SGI_THREADS
// We test whether threads are in use before locking.
// Perhaps this should be moved into stl_threads.h, but that
// probably makes it harder to avoid the procedure call when
// it isn't needed.
extern "C" {
    extern int __pstl_us_rsthread_malloc;
}
// The above is copied from malloc.h.  Including <malloc.h>
// would be cleaner but fails with certain levels of standard
// conformance.
#   define __PSTL_NODE_ALLOCATOR_LOCK if (threads && __pstl_us_rsthread_malloc)	\
	{ _S_node_allocator_lock._M_acquire_lock(); }
#   define __PSTL_NODE_ALLOCATOR_UNLOCK if (threads && __pstl_us_rsthread_malloc) \
	{ _S_node_allocator_lock._M_release_lock(); }
# else /* !__PSTL_SGI_THREADS */
#   define __PSTL_NODE_ALLOCATOR_LOCK				\
    { if (threads) _S_node_allocator_lock._M_acquire_lock(); }
#   define __PSTL_NODE_ALLOCATOR_UNLOCK				\
    { if (threads) _S_node_allocator_lock._M_release_lock(); }
# endif
#else
//  Thread-unsafe
#   define __PSTL_NODE_ALLOCATOR_LOCK
#   define __PSTL_NODE_ALLOCATOR_UNLOCK
#   define __PSTL_NODE_ALLOCATOR_THREADS false
#endif

#include <unistd.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include "pstl_pheap.h"

#if defined( HAVE_EXT_HASH_MAP )
#   include <ext/hash_map>
#else
#   include <hash_map>
#endif

__PSTL_BEGIN_NAMESPACE

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1174
#endif

template <class _Tp, class _Arch>
class allocator {
public:
    typedef _Arch                                     arch_type;
    typedef typename arch_type::size_type             size_type;
    typedef typename arch_type::difference_type       difference_type;
    typedef ptr<_Tp, arch_type>                       pointer;
    typedef const_ptr<_Tp, arch_type>                 const_pointer;
    typedef ref<_Tp, arch_type>                       value_reference;
    typedef const_ref<_Tp, arch_type>                 const_value_reference;
    typedef _Tp                                       value_type;
    typedef typename arch_type::dev_type              dev_type;
    typedef typename arch_type::bit_type              bit_type;
    typedef pheap<arch_type>                          pheap_type;
    typedef ref<allocator, arch_type>                 reference;
    typedef const_ref<allocator, arch_type>           const_reference;
    template <class _Tp1, class _Arch1> struct rebind {
	typedef allocator<_Tp1, _Arch1> other;
    };

private:
    pheap_ptr ph;

public:
    allocator() __PSTL_NOTHROW : ph(0) {}
    allocator(pheap_ptr __ph) __PSTL_NOTHROW : ph(__ph) {}
    allocator(const allocator &__a) __PSTL_NOTHROW : ph(__a.get_pheap()) {}
    template <class _Tp1, class _Arch1>
    allocator(const allocator<_Tp1, _Arch1> &__a) __PSTL_NOTHROW : ph(__a.get_pheap()) {}
    ~allocator() __PSTL_NOTHROW {}

    pointer address(value_reference __x) const { return &__x; }
    const_pointer address(const_value_reference __x) const { return &__x; }

    // __n is permitted to be 0.  The C++ standard says nothing about what
    // the return value is when __n == 0.
    ptr<_Tp, arch_type> allocate(size_type __n, const ptr<void, arch_type> = 0) {
	return __n != 0 ? ptr<_Tp, arch_type>(pheap_type::pheap_convert(ph)->malloc(__n * sizeof(_Tp)), ph)
	    : ptr<_Tp, arch_type>(0);
    }

    // __p is not permitted to be a null pointer.
    void deallocate(pointer __p, size_type /* __n */ )
	{ pheap_type::pheap_convert(ph)->free(static_cast<ptr<void, arch_type> >(__p)); }

    size_type max_size() const __PSTL_NOTHROW 
	{ return size_type(-1) / sizeof(_Tp); }

    static ptr<allocator, arch_type> pnew(pheap_ptr __ph) {
	ptr<allocator, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(allocator)), __ph);
        ref<allocator, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) allocator(__ph);
	return __p;
    }
    
    pheap_ptr get_pheap() const { return ph; }

    template <class _Obj> friend struct replace_pheap;
};

template <class _Tp, class _Arch>
struct replace_pheap<allocator<_Tp, _Arch> > {
    void operator()(pheap_ptr __ph, allocator<_Tp, _Arch> &__a) {
	__a.ph = __ph;
    }
};
    
template <class _Arch>
class allocator<void, _Arch> {
public:
    typedef _Arch                                      arch_type;
    typedef size_t                                     size_type;
    typedef ptrdiff_t                                  difference_type;
    typedef ptr<void, arch_type>                       pointer;
    typedef const_ptr<void, arch_type>                 const_pointer;
    typedef void                                       value_type;
    typedef typename arch_type::dev_type               dev_type;
    typedef typename arch_type::bit_type               bit_type;
    typedef pheap<arch_type>                           pheap_type;
    typedef ref<allocator<void, arch_type>, arch_type> reference;
    typedef const_ref<allocator<void, arch_type>, arch_type>
    const_reference;

    template <class _Tp1, class _Arch1> struct rebind {
	typedef allocator<_Tp1, _Arch1> other;
    };
};


template <class _T1, class _T2, class _Arch1, class _Arch2>
inline bool operator==(const allocator<_T1, _Arch1> &, const allocator<_T2, _Arch2> &) 
{
    return true;
}

template <class _T1, class _T2, class _Arch1, class _Arch2>
inline bool operator!=(const allocator<_T1, _Arch1> &, const allocator<_T2, _Arch2> &)
{
    return false;
}



// Another allocator adaptor: _Alloc_traits.  This serves two
// purposes.  First, make it possible to write containers that can use
// either SGI-style allocators or standard-conforming allocator.
// Second, provide a mechanism so that containers can query whether or
// not the allocator has distinct instances.  If not, the container
// can avoid wasting a word of memory to store an empty object.

// This adaptor uses partial specialization.  The general case of
// _Alloc_traits<_Tp, _Alloc> assumes that _Alloc is a
// standard-conforming allocator, possibly with non-equal instances
// and non-static members.  (It still behaves correctly even if _Alloc
// has static member and if all instances are equal.  Refinements
// affect performance, not correctness.)

// There are always two members: allocator_type, which is a standard-
// conforming allocator type for allocating objects of type _Tp, and
// _S_instanceless, a static const member of type bool.  If
// _S_instanceless is true, this means that there is no difference
// between any two instances of type allocator_type.  Furthermore, if
// _S_instanceless is true, then _Alloc_traits has one additional
// member: _Alloc_type.  This type encapsulates allocation and
// deallocation of objects of type _Tp through a static interface; it
// has two member functions, whose signatures are
//    static _Tp* allocate(size_t)
//    static void deallocate(_Tp*, size_t)

// The fully general version.

template <class _Tp, class _Arch, class _Allocator>
struct _Alloc_traits
{
    static const bool _S_instanceless = false;
    typedef typename _Allocator::__PSTL_TEMPLATE rebind<_Tp, _Arch>::other 
    allocator_type;
};

template <class _Tp, class _Arch, class _Allocator>
const bool _Alloc_traits<_Tp, _Arch, _Allocator>::_S_instanceless;


#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1174
#endif

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_INTERNAL_ALLOC_H */

// Local Variables:
// mode:C++
// End:
