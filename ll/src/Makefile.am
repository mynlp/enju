#*
#* $Id: Makefile.am,v 1.38 2009-07-25 00:58:02 uematsu Exp $
#*
#*    Copyright (c) 1997-1998, Makino Takaki
#*
#*    You may distribute this file under the terms of the Artistic License.
#*

AM_CPPFLAGS = -I@top_srcdir@/include -I@top_srcdir@/include/liblilfes -I$(srcdir)

lib_LTLIBRARIES = liblilfes.la

bin_PROGRAMS = lilfes

EXTRA_clean = FlexLexer.h

pkginclude_HEADERS = \
	yacc.hh \
	FlexLexer.h

liblilfes_la_SOURCES = \
	yacc.yy \
	lex.ll \
	\
	basic.cpp \
	bitmap.cpp \
	builtin.cpp \
        builtinfs.cpp \
	cell.cpp \
	code.cpp \
	code2.cpp \
	compile.cpp \
        display.cpp \
	errors.cpp \
	fariths.cpp \
	ftype.cpp \
	lildbm-db.cpp \
	lildbm-dbtree.cpp \
	lildbm-interface.cpp \
	lildbm-schema.cpp \
	lildbm.cpp \
	machine.cpp \
	machine2.cpp \
	memalloc.cpp \
        memchk.cpp \
        misc.cpp \
	module.cpp \
	option.cpp \
	outform.cpp \
	parse.cpp \
	proc.cpp \
	profile.cpp \
        rl_stream.cpp \
        stream.cpp \
        string.cpp \
	search_path.cpp \
	structur.cpp \
        system.cpp \
	tokenizer.cpp \
        trace.cpp \
	initial.cpp \
        lregex.cpp \
	version.cpp \
        utility.cpp \
        xml.cpp \
		load_so.cpp

lilfes_SOURCES = main.cpp #$(liblilfes_la_SOURCES)
lilfes_CXXFLAGS = $(AM_CXXFLAGS)

lilfes_LDADD = liblilfes.la

AM_YFLAGS = -d -v
AM_LFLAGS = -8

DEFS= @DEFS@ -DPKGDATADIR="\"$(pkgdatadir)\""
#INCLUDES= -I$(srcdir)

if BISON_USE_PARSER_H
yacc.hh: yacc.h
	cp $< $@
endif

# rewritten the rule to adapt flex 2.5.4a with GCC 3.2
# %.cc: %.ll
# 	$(LEX) $(AM_LFLAGS) $(LFLAGS) `test -f $< || echo '$(srcdir)/'`$<
# 	@echo "Converting $(LEX) output $(LEX_OUTPUT_ROOT).c to $@."
# 	@sed -e '/^#/s|$(LEX_OUTPUT_ROOT)\.c|$@|' -e '/FLEX_STD/!s/^class istream;$$/#include <iostream>/' -e 's/\([^A-Za-z0-9:_]\)\([io]stream\)\([^A-Za-z0-9_]\)/\1std::\2\3/g' -e 's/<FlexLexer.h>/"FlexLexer.h"/' $(LEX_OUTPUT_ROOT).c >$@
# 	rm -f $(LEX_OUTPUT_ROOT).c
# 	@echo "Patching FlexLexer.h."
# 	@rm -rf FlexLexer.h
# 	@sed -e '1s/^/\/\* Automatically patched by Makefile. do not edit\*\//' -e 's/#include <iostream.h>/#include <iostream>/' -e '/FLEX_STD/!s/\([^A-Za-z0-9:]\)\([io]stream\)\([^A-Za-z0-9]\)/\1std::\2\3/g' <`echo "#include <FlexLexer.h>" | $(CXXCPP) -x c++ $(CPPFLAGS) -M - 2>/dev/null | sed -ne 's/^.*[ ]\([^ ]*FlexLexer.h[^ ]*\).*$$/\1/g' -e '/FlexLexer/p'` >FlexLexer.h 

if IS_OLD_FLEX # =< flex 2.5.4a
## Patching FlexLexer.h for C++ compilers
if IS_GXX  ## code for GNU C++
lex.cc FlexLexer.h: lex.ll
	$(LEX) $(AM_LFLAGS) $(LFLAGS) `test -f $< || echo '$(srcdir)/'`$<
	@echo "Converting $(LEX) output $(LEX_OUTPUT_ROOT).c to $@."
	@sed -e '/^#/s|$(LEX_OUTPUT_ROOT)\.c|$@|' -e '/FLEX_STD/!s/^class istream;$$/#include <iostream>/' -e 's/\([^A-Za-z0-9:_]\)\([io]stream\)\([^A-Za-z0-9_]\)/\1std::\2\3/g' -e 's/<FlexLexer.h>/"FlexLexer.h"/' $(LEX_OUTPUT_ROOT).c >$@
	rm -f $(LEX_OUTPUT_ROOT).c
	@echo "Patching FlexLexer.h."
	@rm -rf FlexLexer.h
	sed -e '1s/^/\/\* Automatically patched by Makefile. do not edit\*\//' -e 's/#include <iostream.h>/#include <iostream>/' -e '/FLEX_STD/!s/\([^A-Za-z0-9:]\)\([io]stream\)\([^A-Za-z0-9]\)/\1std::\2\3/g' <`echo "#include <FlexLexer.h>" | $(CXXCPP) -x c++ $(CPPFLAGS) -M - 2>/dev/null | sed -ne 's/^.*[ ]\([^ ]*FlexLexer.h[^ ]*\).*$$/\1/g' -e '/FlexLexer/p'` >FlexLexer.h ;
else  ## code for AIX
lex.cc FlexLexer.h: lex.ll
	$(LEX) $(AM_LFLAGS) $(LFLAGS) `test -f $< || echo '$(srcdir)/'`$<
	@echo "Converting $(LEX) output $(LEX_OUTPUT_ROOT).c to $@."
	@sed -e '/^#/s|$(LEX_OUTPUT_ROOT)\.c|$@|' -e '/FLEX_STD/!s/^class istream;$$/#include <iostream>/' -e 's/\([^A-Za-z0-9:_]\)\([io]stream\)\([^A-Za-z0-9_]\)/\1std::\2\3/g' -e 's/<FlexLexer.h>/"FlexLexer.h"/' $(LEX_OUTPUT_ROOT).c >$@
	rm -f $(LEX_OUTPUT_ROOT).c
	@echo "Patching FlexLexer.h."
	@rm -rf FlexLexer.h
	echo "#include <FlexLexer.h>" > tmp.cpp; \
	$(CXXCPP) -M tmp.cpp 1> /dev/null 2>&1; \
	rm -f tmp.cpp tmp.o; \
	sed -e '1s/^/\/\* Automatically patched by Makefile. do not edit\*\//' -e 's/#include <iostream.h>/#include <iostream>/' -e '/FLEX_STD/!s/\([^A-Za-z0-9:]\)\([io]stream\)\([^A-Za-z0-9]\)/\1std::\2\3/g' < `sed -ne 's/^.*[ ]\([^ ]*FlexLexer\.h[^ ]*\)/\1/g' -e '/FlexLexer/p' tmp.u` > FlexLexer.h; \
	rm -f tmp.u;
endif

else # if new flex
if IS_GXX  ## code for GNU C++
lex.cc FlexLexer.h: lex.ll
	$(LEX) $(AM_LFLAGS) $(LFLAGS) `test -f $< || echo '$(srcdir)/'`$<
	@echo "Converting $(LEX) output $(LEX_OUTPUT_ROOT).c to $@."
	@sed -e '/^#/s|$(LEX_OUTPUT_ROOT)\.c|$@|' -e 's/<FlexLexer.h>/"FlexLexer.h"/' $(LEX_OUTPUT_ROOT).c >$@
	rm -f $(LEX_OUTPUT_ROOT).c
	@echo "Copying FlexLexer.h."
	@rm -rf FlexLexer.h
	cat < `echo "#include <FlexLexer.h>" | $(CXXCPP) -x c++ $(CPPFLAGS) -M - 2>/dev/null | sed -ne 's/^.*[ ]\([^ ]*FlexLexer.h[^ ]*\).*$$/\1/g' -e '/FlexLexer/p'` >FlexLexer.h
else  ## code for AIX
lex.cc FlexLexer.h: lex.ll
	$(LEX) $(AM_LFLAGS) $(LFLAGS) `test -f $< || echo '$(srcdir)/'`$<
	@echo "Converting $(LEX) output $(LEX_OUTPUT_ROOT).c to $@."
	@sed -e '/^#/s|$(LEX_OUTPUT_ROOT)\.c|$@|' -e 's/<FlexLexer.h>/"FlexLexer.h"/' $(LEX_OUTPUT_ROOT).c >$@
	rm -f $(LEX_OUTPUT_ROOT).c
	@echo "Copying FlexLexer.h."
	@rm -rf FlexLexer.h
	echo "#include <FlexLexer.h>" > tmp.cpp; \
	$(CXXCPP) -M tmp.cpp 1> /dev/null 2>&1; \
	rm -f tmp.cpp tmp.o; \
	cat < `sed -ne 's/^.*[ ]\([^ ]*FlexLexer\.h[^ ]*\)/\1/g' -e '/FlexLexer/p' tmp.u` > FlexLexer.h; \
	rm -f tmp.u;
endif
endif # IS_OLD_FLEX

lex.o: lex.cc FlexLexer.h

# FlexLexer.h: Makefile
# 	@echo "Patching FlexLexer.h."
# 	@rm -rf FlexLexer.h
# 	@sed -e '1s/^/\/\* Automatically patched by Makefile. do not edit\*\//' -e 's/#include <iostream.h>/#include <iostream>/' -e '/FLEX_STD/!s/\([^A-Za-z0-9:]\)\([io]stream\)\([^A-Za-z0-9]\)/\1std::\2\3/g' <`echo "#include <FlexLexer.h>" | $(CXXCPP) -x c++ $(CPPFLAGS) -M - 2>/dev/null | sed -ne 's/^.*[ ]\([^ ]*FlexLexer.h[^ ]*\).*$$/\1/g' -e '/FlexLexer/p'` >FlexLexer.h 

doc :
	@PERL@ ../lildoc.prl -e -s -b . -t "Builtin" -o ../manual -c style-builtin.css -f builtin- . builtin-index.html
jdoc :
	@PERL@ ../lildoc.prl -j -s -b . -t "組み込み述語" -o ../manual -c style-builtin.css -f builtin- . builtin-index.ja.html

