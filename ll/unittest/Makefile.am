AM_CPPFLAGS = -isystem $(top_srcdir)/ext/gtest/include -I$(top_srcdir)/include/liblilfes $(GTEST_CPPFLAGS)
AM_CXXFLAGS = $(GTEST_CXXFLAGS)
AM_LDFLAGS = $(GTEST_LDFLAGS) $(GTEST_LIBS) \
	     $(top_srcdir)/ext/gtest/lib/libgtest_main.la \
	     $(top_srcdir)/ext/gtest/lib/libgtest.la

# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run
TESTS = 
check_PROGRAMS = 

TESTS += bitmap_test
check_PROGRAMS += bitmap_test
bitmap_test_SOURCES = \
	bitmap_test.cpp
bitmap_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += basic_test
check_PROGRAMS += basic_test
basic_test_SOURCES = \
	basic_test.cpp
basic_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += cell_test
check_PROGRAMS += cell_test
cell_test_SOURCES = \
	cell_test.cpp
cell_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += codelist_test
check_PROGRAMS += codelist_test
codelist_test_SOURCES = \
	codelist_test.cpp
codelist_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += ftype_test
check_PROGRAMS += ftype_test
ftype_test_SOURCES = \
	ftype_test.cpp
ftype_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += lregex_test
check_PROGRAMS += lregex_test
lregex_test_SOURCES = \
	lregex_test.cpp
lregex_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += option_test
check_PROGRAMS += option_test
option_test_SOURCES = \
	option_test.cpp
option_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += outform_test
check_PROGRAMS += outform_test
outform_test_SOURCES = \
	outform_test.cpp
outform_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += search_path_test
check_PROGRAMS += search_path_test
search_path_test_SOURCES = \
	search_path_test.cpp
search_path_test_LDADD = $(top_srcdir)/src/liblilfes.la

TESTS += structur_test
check_PROGRAMS += structur_test
structur_test_SOURCES = \
	structur_test.cpp
structur_test_LDADD = $(top_srcdir)/src/liblilfes.la
