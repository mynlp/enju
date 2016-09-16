/**
 * LiLFeS Configuration File For Windows 
 *
 * Reference Header File is "config_liblilfes.h.in"
 */
#ifndef __lconfig_win_h
#define __lconfig_win_h

/* Defined if 64bit cell is enabled */
/* #undef CELL_64BIT */

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Defined if in debug. */
/* #undef DEBUG */

/* Defined if debugging label */
#define DEBUG_LABEL 1

/* Defined if debugging memory. */
/* #undef DEBUG_MEM */

/* Defined if assertion check */
/* #undef DOASSERT */

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix). */
/* #undef HAVE_ALLOCA_H */

/* Define to 1 if you have the <bzlib.h> header file. */
#define HAVE_BZLIB_H 1

/* Define to 1 if you have the <cstring.h> header file. */
/* #undef HAVE_CSTRING_H */

/* Define to 1 if you have the `ctime' function. */
#define HAVE_CTIME 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <ext/hash_map> header file. */
/* #undef HAVE_EXT_HASH_MAP */

/* Define to 1 if you have the <fcntl.h> header file. */
/* #undef HAVE_FCNTL_H */

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define to 1 if you have the `grantpt' function. */
/* #undef HAVE_GRANTPT 1 */

/* Define to 1 if you have the <tr1/unordered> header file. */
#define HAVE_TR1_HASH_MAP 1

/* Define to 1 if you have the <hash_map> header file. */
/* #undef HAVE_HASH_MAP */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the `kill' function. */
/* #undef HAVE_KILL */

/* Define to 1 if you have the `lfind' function. */
#define HAVE_LFIND 1

/* Define to 1 if you have the `bz2' library (-lbz2). */
#define HAVE_LIBBZ2 1

/* Define to 1 if you have the `db_cxx-4.1' library (-ldb_cxx-4.1). */
/* #undef HAVE_LIBDB_CXX_4_1 */

/* Define to 1 if you have the `dl' library (-ldl). */
/* #undef HAVE_LIBDL */

/* Define to 1 if you have the `fl' library (-lfl). */
/* #define HAVE_LIBFL */

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `readline' library (-lreadline). */
/* #undef HAVE_LIBREADLINE */

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the `util' library (-lutil). */
/* #undef HAVE_LIBUTIL */

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the `lsearch' function. */
#define HAVE_LSEARCH 1

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `openpty' function. */
/* #undef HAVE_OPENPTY */

/* Define to 1 if you have the <pty.h> header file. */
/* #undef HAVE_PTY_H */

/* Define to 1 if you have the <readline/readline.h> header file. */
/* #undef HAVE_READLINE_READLINE_H */

/* Define to 1 if you have the <regex.h> header file. */
/* #undef HAVE_REGEX_H 1 */

/* Define to 1 if you have the <boost/regex.h> header file. */
#define HAVE_BOOST_REGEX 1

/* Defined if rl_command_func_t is found */
#define HAVE_RL_COMMAND_FUNC_T 1

/* Define to 1 if you have the <sstream> header file. */
#define HAVE_SSTREAM 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <stropts.h> header file. */
/* #undef HAVE_STROPTS_H */

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
/* #undef HAVE_SYS_IOCTL_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
/* #undef HAVE_SYS_RESOURCE_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
/* #undef HAVE_SYS_TYPES_H */

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
/* #undef HAVE_SYS_WAIT_H */

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to 1 if you have the <util.h> header file. */
/* #undef HAVE_UTIL_H */

/* Define to 1 if you have the <winsock.h> header file. */
#define HAVE_WINSOCK_H 1

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Name of package */
#define LILFES_PACKAGE "liblilfes"

/* Version number of package */
#define LILFES_VERSION "1.3.9"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Defined if passive profiling is used */
/* #undef PASSIVE_PROFILE */

/* The profiling level, larger is deeper. */
#define PROFILE 1

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `int*', as computed by sizeof. */
#define SIZEOF_INTP 4

/* The size of `long int', as computed by sizeof. */
#define SIZEOF_LONG_INT 4

/* The size of `short int', as computed by sizeof. */
#define SIZEOF_SHORT_INT 2

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Defined if type table hash is used. */
#define TYPE_TABLE_HASH 1

/* Defined if string gc */
#define USE_STRING_GC 1

/* defined if BDBM is available */
/* #undef WITH_BDBM */

/* Defined if bzlib is used */
#define WITH_BZLIB 1

/* Defined if Dynamic loading library is available */
#define WITH_DYNLOAD 1

/* Defined if readline is used */
/* #undef WITH_READLINE */

/* Defined if Regex library is available */
#define WITH_REGEX 1

/* Defined if zlib is used */
#define WITH_ZLIB 1

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Defined if complex constraints are enabled */
#define XCONSTR 1

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
/* #undef YYTEXT_POINTER */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

// For LiLFeS Windows Porting
//#define YYERROR_VERBOSE 1
//#define YYDEBUG 1
#define DEBUG_WIN 1

#define YY_NO_UNISTD_H 1

#define HAVE_WINDOWS_H 1

#define PKGDATADIR "..\\share\\liblilfes"

#define LILFES_WIN_ 1

#endif // __lconfig_win_h

