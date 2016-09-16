########################################################################
# $Id: GNUmakefile,v 1.4 2007-04-03 13:33:55 yusuke Exp $
#
# Bootstrap makefile for running automake/autoconf
# by Makino Takaki
#
########################################################################

include Makefile

Makefile$(PACKAGE):
	@echo Makefile does not exist. running autotools...
	rm -rf aclocal.m4 config.cache config.status
	libtoolize --force --automake
	aclocal -I m4
	autoheader
	automake --add-missing --foreign
	autoconf
	./configure

all-clean: clean
	rm -rf Makefile stamp-h stamp-h.in config.h config.cache config.status config.log config.h.in configure Makefile.in aclocal.m4
