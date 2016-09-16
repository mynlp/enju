// -*- C++ -*-
/*
 * $Id: pstl_hash_fun.h,v 1.8 2011-05-02 10:39:13 matuzaki Exp $
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
 *
 * Copyright (c) 2004
 * Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Ninomiya makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef pstl_hash_fun_h
#define pstl_hash_fun_h

#include <stddef.h>
#include "pstl_pheap.h"

namespace pstl {

    template <class _Key> struct hash { };
     template <class _Dev, class _Bit> inline
     typename _Bit::size_type __stl_hash_string(pheap<_Dev, _Bit> *ph, ptr<char, _Bit> __s) {
         typename _Bit::size_type __h = 0;
         ref<char, _Dev, _Bit> s = __s(ph);
         for ( ; s != 0;) {
             __h = 5*__h + __s;
             ++__s;
             s.reassign(__s(ph));
         }
         return __h;
     }
    template <class _BitDummy>
    struct hash<ptr<char, _BitDummy> > {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *ph, const ptr<char, _Bit> __s) const { return __stl_hash_string(ph, __s); }
    };
    template <class _BitDummy>
    struct hash<const ptr<char, _BitDummy> > {
        template <class _Dev, class _Bit>
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *ph, const ptr<char, _Bit> __s) const { return __stl_hash_string(ph, __s); }
    };
    template <>
    struct hash<char> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, char __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<unsigned char> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, unsigned char __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<signed char> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_tyep operator()(pheap<_Dev, _Bit> *, unsigned char __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<short> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, short __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<unsigned short> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, unsigned short __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<int> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, int __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<unsigned int> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, unsigned int __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<long> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, long __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<unsigned long> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, unsigned long __x) const { return static_cast<typename _Bit::size_type>(__x); }
    };
    template <>
    struct hash<long long int> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, long long int __x) const { return static_cast<typename _Bit::size_type>(__x);}
    };
    template <>
    struct hash<long long unsigned int> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, long long unsigned int __x) const { return static_cast<typename _Bit::size_type>(__x);}
    };
    template <>
    struct hash<float> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, float __x) const { return static_cast<typename _Bit::size_type>(static_cast<uint32>(static_cast<uint32*>(&(__x))));}
    };
    template <>
    struct hash<double> {
        template <class _Dev, class _Bit> 
        typename _Bit::size_type operator()(pheap<_Dev, _Bit> *, double __x) const { return static_cast<typename _Bit::size_type>(static_cast<uint64>(static_cast<uint64*>(&(__x))));}
    };
  
}


#endif // pstl_hash_fun_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2005/05/30 02:40:19  tuna
 * const に関する整理、castを新形式に統一、その他細かい修正をしました
 *
 * Revision 1.6  2005/05/20 06:11:35  tuna
 * const に関係する追加 (const_ptr, const_ref) とその他の修正
 *
 * Revision 1.5  2005/04/04 04:25:01  ninomi
 * a light check for hash key is introduced.  ldbm-type is still under construction.
 *
 * Revision 1.4  2005/04/01 10:09:21  ninomi
 * equalやhashのめそっどの引数のためのpheapを引数に加えました。
 *
 * Revision 1.3  2005/03/25 05:59:25  ninomi
 * hash functionの第一引数にpheapをいれるようにしました。
 *
 * Revision 1.2  2004/12/09 07:40:39  ninomi
 * pheapのページングアルゴリズムを改良しました。
 * hash_mapとhash_multimapに専用のreferenceを加えました。
 *
 * Revision 1.1  2004/11/23 16:02:57  ninomi
 * ファイルを加えたり、分解したりしまいた。pstl_allocator->pstl_pheap+pstl_alloc.h
 *
 */
