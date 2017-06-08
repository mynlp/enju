LiLFeS 1.4 -- Installation Procedure
====================================

See "manual/index.html" for details.


1. Unpack the package
---------------------

   This will create a new directory "liblilfes-x.x.x"
   (x.x.x is a version of LiLFeS).

   % gzip -dc < liblilfes-x.x.x.tar.gz | tar xvf -


2. Generate "Makefile" by "configure"
-------------------------------------

   You can see many options specific to LiLFeS by "-h" option.

   % ./configure

   If you want to install lilfes in your home directory, specify
   "--prefix=$HOME" option.

   % ./configure --prefix=$HOME


3. make
-------


4. make install
---------------



### For maintainers:

Generate "configure" file by the following procedure:

  % libtoolize --force; aclocal; autoheader; automake --add-missing --foreign; autoconf

and run the "configure" file with "--enable-maintainer-mode":

  % ./configure --enable-maintainer-mode

  (* Bison(yacc) and flex is necessary to make.)
