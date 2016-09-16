/* $Id: lconfig.h,v 1.9 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __lconfig_h
#define __lconfig_h

#include "hashdef.h"

#ifdef _MSC_VER
  #include "lconfig_win.h"
#else
  #include "config_liblilfes.h"
#endif

#include <iostream>

// Big endian or Little endian?
#ifdef WORDS_BIGENDIAN
#  define BIG_ENDIAN_
#else
#  define LITTLE_ENDIAN_
#endif

// 32-bit or 64-bit architecture?

//define int16
#if SIZEOF_SHORT_INT == 2
namespace lilfes {

  typedef          short  int16;
  typedef   signed short sint16;
  typedef unsigned short uint16;
}
#endif

//define int32
#if SIZEOF_INT == 4

namespace lilfes {
  typedef          int    int32;
  typedef   signed int   sint32;
  typedef unsigned int   uint32;
}

#elif SIZEOF_SHORT_INT == 4

namespace lilfes {
  typedef          short   int32;
  typedef   signed short   sint32;
  typedef unsigned short   uint32;
}

#elif SIZEOF_LONG_INT == 4

namespace lilfes {
  typedef          long   int32;
  typedef   signed long   sint32;
  typedef unsigned long   uint32;
}

#endif

//define int64
#if SIZEOF_INT == 8

namespace lilfes {
  typedef          int    int64;
  typedef   signed int   sint64;
  typedef unsigned int   uint64;
}

#elif  SIZEOF_LONG_INT == 8

namespace lilfes {
  typedef          long    int64;
  typedef   signed long   sint64;
  typedef unsigned long   uint64;
}

#elif SIZEOF_LONG_INT == 4

namespace lilfes{
  typedef          long long    int64;
  typedef   signed long long   sint64;
  typedef unsigned long long   uint64;
}

#endif

#if SIZEOF_INTP == 2

namespace lilfes {
  typedef uint16   ptr_int;
}

#elif SIZEOF_INTP == 4

namespace lilfes {
  typedef uint32    ptr_int;
}

#elif SIZEOF_INTP == 8

namespace lilfes{
  typedef uint64    ptr_int;
}
#define HAS_INT64
//#define CELL_64BIT

#endif






#ifdef HAVE_SEARCH_H
#include <search.h>
#else // HAVE_SEARCH_H
#ifndef size_t
#include <sys/types.h>
#endif
#endif // HAVE_SEARCH_H
extern "C" {
namespace lilfes {

#ifndef HAVE_LSEARCH
void *
lsearch(  const void *key,
          void *base,
          size_t *nelp,
          size_t width,
          int (*compar) (const void *, const void *) );
#endif
#ifndef HAVE_LFIND
void *
lfind(  const void *key,
          void *base,
          size_t *nelp,
          size_t width,
          int (*compar) (const void *, const void *) );
#endif
}; // namespace lilfes

}


// どうやってせっていしようかな．

#define DEFAULT_SEARCH_PATH          PKGDATADIR 
#define GLOBAL_INITIALIZATION_FILE   PKGDATADIR "/init.lil"
#ifndef LILFES_WIN_
#define PATH_SEPARATOR "/"
#define LOCAL_INITIALIZATION_PATH_ENV "HOME"
#define LOCAL_INITIALIZATION_FILE ".lilfesrc"
#else
#define PATH_SEPARATOR "\\"
#define LOCAL_INITIALIZATION_PATH_ENV "APPDATA"
#define LOCAL_INITIALIZATION_FILE "LiLFeS"
#endif

// よくわかんないよーん

#define BUILTIN_TYPE_SAVE
#define INLINE_WRITEHEAP
#define BACKHEAP
#define TRAIL
//#define DATA
//#define PROCTRACE

#if defined(WINNT) || defined(WIN95)
#define FASTCALL __fastcall
#define COMPAREFUNC __cdecl
#else
#define FASTCALL
#define COMPAREFUNC
#endif

#ifndef NULL
#define NULL 0
#endif

#ifdef __CYGWIN__
#  define in_addr6 in6_addr
#endif

#ifndef HAVE_EXT_HASH_MAP
namespace lilfes {
namespace __gnu_cxx = std;
}
#endif

#if defined(HAVE_SSTREAM)
#include <sstream>
#else
#include <strstream>

// wrppaer class
namespace std {
class ostringstream {
	std::ostrstream oss;
public:
	ostringstream() { }
	~ostringstream() { delete[] oss.str(); }
	std::string str() { oss << ends; return std::string(oss.str()); }
	template <class T> ostringstream & operator<<(T &t) { oss << t; return *this; }
	template <class T> ostringstream & operator<<(const T &t) { oss << t; return *this; }
};
} // namespace lilfes
#endif

//#define LILFES_DEBUG_MEMORY_LEAK
#ifdef LILFES_DEBUG_MEMORY_LEAK
void* operator new(std::size_t size, const char* file_name, int line_no) throw (std::bad_alloc);
void operator delete(void* ptr) throw();
void* operator new[](std::size_t size, const char* file_name, int line_no) throw (std::bad_alloc);
void operator delete[](void* ptr) throw();
#define new new(__FILE__, __LINE__)
#endif // LILFES_DEBUG_MEMORY_LEAK

#endif // __lconfig_h
