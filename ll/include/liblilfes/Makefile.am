#########################################################################
# $Id: Makefile.am,v 1.18 2007-07-06 14:29:48 matuzaki Exp $

SUBDIRS=

pkginclude_HEADERS = \
	basic.h \
	bfstream.h \
	bitmap.h \
	builtin.h \
	cell.h \
	code.h \
	codelist.h \
	errors.h \
	ftype.h \
	gfstream.h \
	hashdef.h \
	in.h \
	initial.h \
	lconfig.h \
	lconfig_win.h \
	lexer.h \
	lildbm-basic.h \
	lildbm-cell.h \
	lildbm-core.h \
	lildbm-db.h \
	lildbm-dbtree.h \
	lildbm-env.h \
	lildbm-schema.h \
	lildbm-type.h \
	lildbm.h \
	lilfes.h \
	lregex.h \
	machine.h \
	memalloc.h \
	module.h \
	option.h \
	outform.h \
	parse.h \
	proc.h \
	procstream.h \
	profile.h \
	rl_stream.h \
	search_path.h \
	sockstream.h \
	stream.h \
	structur.h \
	tokenizer.h \
	utility.h \
	xml.h \
	CppPred.h \
	ldlfcn_win.h

#pkginclude_DATA = config_liblilfes.h

noinst_DATA = stamp-file

stamp-file: $(pkginclude_HEADERS) $(pkginclude_DATA)
	touch stamp-file
