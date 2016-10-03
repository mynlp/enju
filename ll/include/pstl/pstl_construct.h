// -*- C++ -*-
/*
 * $Id: pstl_construct.h,v 1.7 2011-05-02 10:39:13 matuzaki Exp $
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

#ifndef pstl_construct_h
#define pstl_construct_h

#include "pstl_alloc.h"

namespace pstl {

  ////////////////////////
  /// _Construct
  /// pheap & ptr
  template <class _Dev, class _Bit, class _T1, class _T2>
  inline void pstl_Construct(pheap<_Dev, _Bit>* ph, ptr<_T1, _Bit> __p, const _T2& __value) {
      ref<_T1, _Dev, _Bit> __p_ref = __p(ph); // this is needed because of a bug in gcc 3.2?
    new (static_cast<void*>(&(__p_ref.get_obj()))) _T1(__value);
  }

  template <class _Dev, class _Bit, class _T1, class _T2>
  inline void pstl_Construct(pheap<_Dev, _Bit>* ph,
			 ptr<_T1, _Bit> __p, const ref<_T2, _Dev, _Bit> __r) {
    ref<_T1, _Dev, _Bit> __p_ref = __p(ph); // this is needed because of a bug in gcc 3.2?
    new (static_cast<void*>(&(__p_ref.get_obj()))) _T1(__r.get_obj());
  }

  template <class _Dev, class _Bit, class _T1>
  inline void pstl_Construct(pheap<_Dev, _Bit>* ph, ptr<_T1, _Bit> __p) {
    ref<_T1, _Dev, _Bit> __p_ref = __p(ph); // this is needed because of a bug in gcc 3.2?
    new (static_cast<void*>(&(__p_ref.get_obj()))) _T1();
  }

  /// ref
  template <class _Dev, class _Bit, class _T1, class _T2>
  inline void pstl_Construct(ref<_T1, _Dev, _Bit> __r, const _T2& __value) {
    new (static_cast<void*>(&(__r.get_obj()))) _T1(__value);
  }

  template <class _Dev, class _Bit, class _T1, class _T2>
  inline void pstl_Construct(ref<_T1, _Dev, _Bit> __r, const ref<_T2, _Dev, _Bit> __s) {
    new (static_cast<void*>(&(__r.get_obj()))) _T1(*__s);
  }

  template <class _Dev, class _Bit, class _T1>
  inline void pstl_Construct(ref<_T1, _Dev, _Bit> __r) {
    new (static_cast<void*>(&(__r.get_obj()))) _T1();
  }

  ////////////////////////
  /// _Destroy
  /// pheap & ptr
  template <class _Dev, class _Bit, class _Tp>
  inline void pstl_Destroy(pheap<_Dev, _Bit>* ph, ptr<_Tp, _Bit> __pointer) {
    __pointer(ph)->~_Tp();
  }

  template <class _Bit, class _Dev> inline void pstl_Destroy(pheap<_Dev, _Bit>*, ptr<char, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(pheap<_Dev, _Bit>*, ptr<int, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(pheap<_Dev, _Bit>*, ptr<long, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(pheap<_Dev, _Bit>*, ptr<float, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(pheap<_Dev, _Bit>*, ptr<double, _Bit>) {}

  /// ref
  template <class _Dev, class _Bit, class _Tp>
  inline void pstl_Destroy(ref<_Tp, _Dev, _Bit> __r) {
    __r->~_Tp();
  }

  template <class _Bit, class _Dev> inline void pstl_Destroy(ref<char, _Dev, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ref<int, _Dev, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ref<long, _Dev, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ref<float, _Dev, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ref<double, _Dev, _Bit>) {}

  /// iterator
  template <class _ForwardIterator>
  void pstl_Destroy(_ForwardIterator __first, _ForwardIterator __last) {
    for ( ; __first != __last; ++__first) {
      pstl_Destroy(*__first);
    }
  }

  template <class _Bit, class _Dev> inline void pstl_Destroy(ptr<char, _Bit>, ptr<char, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ptr<int, _Bit>, ptr<int, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ptr<long, _Bit>, ptr<long, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ptr<float, _Bit>, ptr<float, _Bit>) {}
  template <class _Bit, class _Dev> inline void pstl_Destroy(ptr<double, _Bit>, ptr<double, _Bit>) {}

} // end of namespace pstl

#endif // pstl_construct_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/05/30 02:40:19  tuna
 * const に関する整理、castを新形式に統一、その他細かい修正をしました
 *
 * Revision 1.5  2004/12/10 07:04:09  ninomi
 * template variableの_Bitと_Devの順番を逆にしました
 *
 * Revision 1.4  2004/12/10 05:56:05  ninomi
 * amd64でも動くようにしました。
 *
 * Revision 1.3  2004/12/06 05:05:24  ninomi
 * ようやくhash_mapも新しいpage mappingに対応。
 *
 * Revision 1.2  2004/11/23 16:00:10  ninomi
 * ldbmではなくて、pstlとして独立したモジュールにした。allocatorの機能を
 * pheap(persistent heap)とallocatorに分解した。
 *
 * Revision 1.1  2004/11/12 08:15:07  ninomi
 * pstlをinclude/liblilfes/pstlからinclude/pstlにうつしました。
 *
 * Revision 1.1  2004/11/12 07:32:37  ninomi
 * ldbm_*をinclude/liblilfes/pstlに移動しました。
 *
 * Revision 1.3  2004/11/08 10:37:16  ninomi
 * persistent STLのptr, ref, vector, hash_mapまで完成。まだreferenceの
 * life time処理にバグがあるかも。とりあえずcommit。
 *
 * Revision 1.2  2004/10/12 11:27:00  ninomi
 * ref<...>&としている箇所をすべてref<...>とした。
 *
 * Revision 1.1  2004/10/11 09:32:49  ninomi
 * referenceをC++のreferenceに近くした。それにともなって、vectorも変更。
 * iteratorにallocator*をもたせるようにした。
 *
 */
