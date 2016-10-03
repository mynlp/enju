#########################################################################
# $Id: Makefile.am,v 1.4 2005-11-22 08:12:33 tuna Exp $

SUBDIRS=

pkginclude_HEADERS = \
	pstl_algobase.h \
	pstl_alloc.h \
	pstl_construct.h \
	pstl_function.h \
	pstl_hashtable.h \
	pstl_hash_map.h \
	pstl_vector.h

#pkginclude_DATA =

noinst_DATA = stamp-file

stamp-file: $(pkginclude_HEADERS) $(pkginclude_DATA)
	touch stamp-file
