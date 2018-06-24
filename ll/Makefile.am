##########################################################################
# $Id: Makefile.am,v 1.12 2008-07-22 02:35:36 yusuke Exp $

SUBDIRS = ext include src lillib manual elisp unittest

pkgdata_DATA = \
	init.lil Artistic COPYING lildoc.prl \
	index.html index.ja.html style.css lilfes.gif favicon.ico \
	fs.xsl fs.css

EXTRA_DIST = $(pkgdata_DATA)

doc:
	cd src; ${MAKE} doc
	cd lillib; ${MAKE} doc

jdoc:
	cd src; ${MAKE} jdoc
	cd lillib; ${MAKE} jdoc

