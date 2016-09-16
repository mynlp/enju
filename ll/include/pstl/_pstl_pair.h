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
 *
 * Copyright (c) 2005
 * Takashi Tsunakawa and Takashi Ninomiya
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

#ifndef __SGI_PSTL_INTERNAL_PAIR_H
#define __SGI_PSTL_INTERNAL_PAIR_H

#include "pstl_pheap.h"
#include "pstl_alloc.h"

__PSTL_BEGIN_NAMESPACE

template <class _T1, class _T2, class _Arch>
struct pair {
    typedef _T1                                    first_type;
    typedef _T2                                    second_type;
    typedef _Arch                                  arch_type;
    typedef pheap<arch_type>                       pheap_type;
    typedef ref<pair, arch_type>                   reference;

    _T1 first;
    _T2 second;
    
    pair() {}
    pair(const _T1 &__a, const _T2 &__b) : first(__a), second(__b) {}
    template <class _U1, class _U2, class _U_Arch>
    pair(const pair<_U1, _U2, _U_Arch> &__p) : first(__p.first), second(__p.second) {}
    ~pair() {}	

    ptr<_T1, arch_type> get_first_pointer(const_ptr<pair, arch_type> _M_this) const {
	ptr_int first_addr = reinterpret_cast<ptr_int>(&first);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return ptr<_T1, arch_type>(_M_this.getAddress() + (first_addr - this_addr), _M_this.get_pheap());
    }
    ref<_T1, arch_type> get_first(const_ptr<pair, arch_type> _M_this) const {
        return *get_first_pointer(_M_this);
    }
    ptr<_T2, arch_type> get_second_pointer(const_ptr<pair, arch_type> _M_this) const {
	ptr_int second_addr = reinterpret_cast<ptr_int>(&second);
	ptr_int this_addr = reinterpret_cast<ptr_int>(this);
	return ptr<_T2, arch_type>(_M_this.getAddress() + (second_addr - this_addr), _M_this.get_pheap());
    }
    ref<_T2, arch_type> get_second(const_ptr<pair, arch_type> _M_this) const {
        return *get_second_pointer(_M_this);
    }
    
    static ptr<pair, arch_type> pnew(pheap_ptr __ph) {
	ptr<pair, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(pair)), __ph);
        ref<pair, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) pair();
	return __p;
    }
    static ptr<pair, arch_type> pnew(pheap_ptr __ph, const _T1 &__a, const _T2 &__b) {
	ptr<pair, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(pair)), __ph);
        ref<pair, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) pair(__a, __b);
	return __p;
    }
    template <class _U1, class _U2, class _U_Arch>
    static ptr<pair, arch_type> pnew(pheap_ptr __ph, const pair<_U1, _U2, _U_Arch> &__pa) {
	ptr<pair, arch_type> __p(pheap_type::pheap_convert(__ph)->malloc(sizeof(pair)), __ph);
        ref<pair, arch_type> r = *__p;
	new ((void *)&(r.get_obj())) pair(__pa);
	return __p;
    }    

    template <class _U1, class _U2>
    pair &operator=(const __STD::pair<_U1, _U2> &__p) {
	first = __p.first;
	second = __p.second;
    }

    template <class _Obj> friend struct replace_pheap;
};

template <class _T1, class _T2, class _Arch>
struct replace_pheap<pair<_T1, _T2, _Arch> > {
    void operator()(pheap_ptr __ph, pair<_T1, _T2, _Arch> &__pa) {
	__replace_pheap(__ph, __pa.first);
	__replace_pheap(__ph, __pa.second);
    }
};

template <class _T1, class _T2, class _Arch>
inline pair<_T1, _T2, _Arch> make_pair(const _T1 &__x, const _T2 &__y)
{
    return pair<_T1, _T2, _Arch>(__x, __y);
}

__PSTL_END_NAMESPACE

#endif // __SGI_PSTL_INTERNAL_PAIR_H
