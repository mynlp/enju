// -*- C++ -*-
/*
 * Copyright (c) 1998
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

#ifndef __SGI_PSTL_EXCEPTION_H
#define __SGI_PSTL_EXCEPTION_H

// This header exists solely for portability.  Normally it just includes
// the header <exception>.

// The header <exception> contains low-level functions that interact
// with a compiler's exception-handling mechanism.  It is assumed to
// be supplied with the compiler, rather than with the library, because
// it is inherently tied very closely to the compiler itself.

// On platforms where <exception> does not exist, this header defines
// an exception base class.  This is *not* a substitute for everything
// in <exception>, but it suffices to support a bare minimum of STL
// functionality.

#include "_pstl_config.h"

#ifndef __PSTL_NO_EXCEPTION_HEADER

#include <exception>
#define __PSTL_EXCEPTION_BASE exception

#else /* __PSTL_NO_EXCEPTION_HEADER */

__PSTL_BEGIN_NAMESPACE

class _Exception {
public:
  virtual ~_Exception() __PSTL_NOTHROW {}
  virtual const char *what() const __PSTL_NOTHROW { return ""; }
};

#define __PSTL_EXCEPTION_BASE _Exception

__PSTL_END_NAMESPACE

#endif /* __PSTL_NO_EXCEPTION_HEADER */

__PSTL_BEGIN_NAMESPACE

class closed_pheap_access : public std::exception {
public:
    virtual const char* what() const throw() {
        return "warning: access to the closed pheap\n";
    }
};

class null_ph_construction : public std::exception {
public:
    virtual const char* what() const throw() {
	return "error: null ph constructor is called\n";
    }
};

__PSTL_END_NAMESPACE

#endif /* __SGI_PSTL_EXCEPTION_H */

// Local Variables:
// mode:C++
// End:
