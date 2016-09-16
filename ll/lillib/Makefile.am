#*
#* $Id: Makefile.am,v 1.8 2005-03-12 06:22:32 yusuke Exp $
#*
#*    Copyright (c) 1997-1998, Makino Takaki
#*
#*    You may distribute this file under the terms of the Artistic License.
#*

LIBSRC = \
        cgi.lil \
        cgiconsole.lil \
        dattypes.lil \
        debug.lil \
        fs.lil \
        http_server.lil \
        io.lil \
	is.lil \
        lists.lil \
        ordsets.lil \
        strings.lil \
        system.lil \
        tools.lil \
        xml.lil

HTML = \
	cgiconsole.css

lillibdir = $(pkgdatadir)/lillib

lillib_DATA = $(LIBSRC) $(HTML)

EXTRA_DIST = $(lillib_DATA)

doc :
	@PERL@ ../lildoc.prl -e -b . -s -t "LiLLiB" -o ../manual -c style-builtin.css -f lillib- . lillib-index.html
jdoc :
	@PERL@ ../lildoc.prl -j -b . -s -t "LiLLiB" -o ../manual -c style-builtin.css -f lillib- . lillib-index.ja.html

