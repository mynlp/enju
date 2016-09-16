// -*- C++ -*-
/*
 * $Id: pstl_config.h,v 1.2 2011-05-02 10:39:13 matuzaki Exp $
 */
#ifndef pstl_config_h
#define pstl_config_h

#include "../liblilfes/config_liblilfes.h"

//define int16
#if SIZEOF_SHORT_INT == 2
namespace pstl {

  typedef          short  int16;
  typedef   signed short sint16;
  typedef unsigned short uint16;
}
#endif

//define int32
#if SIZEOF_INT == 4
namespace pstl {
  typedef          int    int32;
  typedef   signed int   sint32;
  typedef unsigned int   uint32;
}

#elif SIZEOF_SHORT_INT == 4
namespace pstl {
  typedef          short   int32;
  typedef   signed short   sint32;
  typedef unsigned short   uint32;
}

#elif SIZEOF_LONG_INT == 4
namespace pstl {
  typedef          long   int32;
  typedef   signed long   sint32;
  typedef unsigned long   uint32;
}

#endif

//define int64
#if SIZEOF_INT == 8
namespace pstl {
  typedef          int    int64;
  typedef   signed int   sint64;
  typedef unsigned int   uint64;
}

#elif  SIZEOF_LONG_INT == 8
namespace pstl {
  typedef          long    int64;
  typedef   signed long   sint64;
  typedef unsigned long   uint64;
}

#elif SIZEOF_LONG_INT == 4
namespace pstl {
  typedef          long long    int64;
  typedef   signed long long   sint64;
  typedef unsigned long long   uint64;
}

#endif

#if SIZEOF_INTP == 2
namespace pstl {
  typedef uint16   ptr_int;
}

#elif SIZEOF_INTP == 4
namespace pstl {
  typedef uint32    ptr_int;
}

#elif SIZEOF_INTP == 8
namespace pstl {
  typedef uint64    ptr_int;
}
#define HAS_INT64
#endif

#endif //pstl_config_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/11/23 16:02:57  ninomi
 * ファイルを加えたり、分解したりしまいた。pstl_allocator->pstl_pheap+pstl_alloc.h
 *
 */
