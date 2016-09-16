// -*- C++ -*-
/*
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

#ifndef __PSTL_INTERNAL_ALLOC_FWD_H
#define __PSTL_INTERNAL_ALLOC_FWD_H

__PSTL_BEGIN_NAMESPACE

/*
  
#ifndef __PSTL_CLASS_HEADER
template <class _Arch> class pheap;
template <class _Arch> class header;
template <class _Arch> void replace_pheap(pheap_ptr, header<_Arch> &);
#endif // __PSTL_CLASS_HEADER

class b32;
class b64;
class pmem;
class vmem;
template <class _Dev, class _Bit> class arch;

template <int __inst, class _Arch> class __malloc_alloc_template;
template <int __inst, class _Arch> void replace_pheap(pheap_ptr, __malloc_alloc_template<__inst, _Arch> &);

template <class _Tp, class _Arch, class _Alloc> class simple_alloc;
template <class _Tp, class _Arch, class _Alloc> void replace_pheap(pheap_ptr, simple_alloc<_Tp, _Arch, _Alloc> &);

template <class _Arch, class _Alloc> class debug_alloc;
template <class _Arch, class _Alloc> void replace_pheap(pheap_ptr, debug_alloc<_Arch, _Alloc> &);

template <class _Tp, class _Arch> class allocator;
template <class _Tp, class _Arch> void replace_pheap(pheap_ptr, allocator<_Tp, _Arch> &);

template <class _Arch> struct __default_alloc_template_Obj;
template <class _Arch> void replace_pheap(pheap_ptr, __default_alloc_template_Obj<_Arch> &);

template <bool threads, int inst, class _Arch> class __default_alloc_template;
template <bool threads, int inst, class _Arch> void replace_pheap(pheap_ptr, __default_alloc_template<threads, inst, _Arch> &);

// ...

template <class _T1, class _T2, class _Arch> class pair;
template <class _T1, class _T2, class _Arch> void replace_pheap(pheap_ptr, pair<_T1, _T2, _Arch> &);

#ifdef __PSTL_USE_STD_ALLOCATORS
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic> class _Vector_alloc_base;
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic> void replace_pheap(pheap_ptr, _Vector_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> &);
template <class _Tp, class _Arch, class _Allocator> void replace_pheap(pheap_ptr, _Vector_alloc_base<_Tp, _Arch, _Allocator, true> &);
#else // __PSTL_USE_STD_ALLOCATORS
template <class _Tp, class _Arch, class _Alloc> class _Vector_base;
template <class _Tp, class _Arch, class _Alloc> void replace_pheap(pheap_ptr, _Vector_base<_Tp, _Arch, _Alloc> &);
#endif // __PSTL_USE_STD_ALLOCATORS

template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc> class hash_map;
template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc> void replace_pheap(pheap_ptr, hash_map<_Key, _Tp, _Arch, _HashFcn, _EqualKey, _Alloc> &);

template <class _Val, class _Arch> class _Hashtable_node;
template <class _Val, class _Arch> void replace_pheap(pheap_ptr, _Hashtable_node<_Val, _Arch> &);

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc> class _Hashtable_iterator;
template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc> void replace_pheap(pheap_ptr, _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> &);

template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc> class hashtable;
template <class _Val, class _Key, class _HashFcn, class _ExtractKey, class _EqualKey, class _Arch, class _Alloc> void replace_pheap(pheap_ptr, hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Arch, _Alloc> &);

template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc> class hash_multimap;
template <class _Key, class _Tp, class _Arch, class _HashFcn, class _EqualKey, class _Alloc> void replace_pheap(pheap_ptr, hash_multimap<_Key, _Tp, _Arch, _HashFcn, _EqualKey, _Alloc> &);

template <class _Arch> struct _List_node_base;
template <class _Arch> void replace_pheap(pheap_ptr, _List_node_base<_Arch> &);

template <class _Tp, class _Arch> struct _List_node;
template <class _Tp, class _Arch> void replace_pheap(pheap_ptr, _List_node<_Tp, _Arch> &);

template <class _Arch> struct _List_iterator_base;
template <class _Arch> void replace_pheap(pheap_ptr, _List_iterator_base<_Arch> &);

#ifdef __PSTL_USE_STD_ALLOCATORS
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic> class _List_alloc_base;
template <class _Tp, class _Arch, class _Allocator, bool _IsStatic> void replace_pheap(pheap_ptr, _List_alloc_base<_Tp, _Arch, _Allocator, _IsStatic> &);
template <class _Tp, class _Arch, class _Allocator> void replace_pheap(pheap_ptr, _List_alloc_base<_Tp, _Arch, _Allocator, true> &);
#else // __PSTL_USE_STD_ALLOCATORS
template <class _Tp, class _Arch, class _Alloc> class _List_base;
template <class _Tp, class _Arch, class _Alloc> void replace_pheap(pheap_ptr, _List_base<_Tp, _Arch, _Alloc> &);
#endif // __PSTL_USE_STD_ALLOCATORS

*/

__PSTL_END_NAMESPACE

#endif // __PSTL_INTERNAL_ALLOC_FWD_H
