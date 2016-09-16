// -*- C++ -*-
/*
 * $Id: _pstl_hash_fun.h,v 1.4 2011-05-02 10:39:13 matuzaki Exp $
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

#ifndef __SGI_PSTL_HASH_FUN_H
#define __SGI_PSTL_HASH_FUN_H

#include <cstddef>
#include "pstl_pheap.h"

__PSTL_BEGIN_NAMESPACE

template <class _Key> struct hash { };

template <class _Arch>
inline size_t __stl_hash_string(ptr<char, _Arch> __s) {
    size_t __h = 0;
    ref<char, _Arch> s = *__s;
    for ( ; s != static_cast<char>(0); ) {
	__h = 5 * __h + __s;
	++__s;
	s.reassign(*__s);
    }
    return __h;
}

template <class _ArchDummy>
struct hash<ptr<char, _ArchDummy> > {
    template <class _Arch> 
    size_t operator()(const ptr<char, _Arch> __s) const { return __stl_hash_string(__s); }
};

template <class _ArchDummy>
struct hash<const ptr<char, _ArchDummy> > {
    template <class _Arch>
    size_t operator()(const ptr<char, _Arch> __s) const { return __stl_hash_string(__s); }
};

__PSTL_TEMPLATE_NULL
struct hash<char> {
    size_t operator()(char __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<unsigned char> {
    size_t operator()(unsigned char __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<signed char> {
    size_t operator()(unsigned char __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<short> {
    size_t operator()(short __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<unsigned short> {
    size_t operator()(unsigned short __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<int> {
    size_t operator()(int __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<unsigned int> {
    size_t operator()(unsigned int __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<long> {
    size_t operator()(long __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<unsigned long> {
    size_t operator()(unsigned long __x) const { return static_cast<size_t>(__x); }
};
__PSTL_TEMPLATE_NULL
struct hash<long long int> {
    size_t operator()(long long int __x) const { return static_cast<size_t>(__x);}
};
__PSTL_TEMPLATE_NULL
struct hash<long long unsigned int> {
    size_t operator()(long long unsigned int __x) const { return static_cast<size_t>(__x);}
};
__PSTL_TEMPLATE_NULL
struct hash<float> {
    size_t operator()(const float &__x) const { return static_cast<size_t>(reinterpret_cast<uint64>(&__x)); }
};
__PSTL_TEMPLATE_NULL
struct hash<double> {
    size_t operator()(const double &__x) const { return static_cast<size_t>(reinterpret_cast<uint64>(&__x)); }
};

__PSTL_END_NAMESPACE

#endif // __SGI_PSTL_HASH_FUN_H

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/11/22 08:12:34  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.1.2.1  2005/11/21 13:05:18  tuna
 * new-db-5 にファイル名の変更／追加を反映させました。
 *
 * Revision 1.7.2.2.4.1.2.1  2005/08/30 09:29:41  ninomi
 * turbolinuxのg++3.2.2でコンパイルできるようにしました。
 *
 * Revision 1.7.2.2.4.1  2005/08/26 05:00:02  tuna
 * 新しい形式のreplace_pheapを使うバージョンにしました。
 *
 * Revision 1.7.2.2  2005/08/09 09:31:48  tuna
 * *** empty log message ***
 *
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
