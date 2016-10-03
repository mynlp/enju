/* $Id: basic.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $ 
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "basic.h"
#include <iostream>
#include <string>

namespace lilfes {

using std::string;


static const char rcsid[] = "$Id: basic.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $";

string int2str(int i)
{
	if( i < 0 )
		return "-" + int2str(-i);
	else if( i < 10 )
		return repeat_string((char)('0'+i),1);
	else
		return int2str(i/10) + repeat_string((char)('0'+(i%10)),1);
}

#ifndef HAVE_LSEARCH
extern "C" void *
lsearch(  const void *key,
          void *base,
          size_t *nelp,
          size_t width,
          int (*compar) (const void *, const void *) )
{
	int i=0;
	char *base_c = (char *)base;
	char *sentinel = base_c + (*nelp)*width;

	memcpy(base_c + (*nelp)*width, key, width);
	
	while( (*compar)(base_c, key) )
		base_c += width;

	if( base_c == sentinel )
		(*nelp)++;

	return base_c;
}
#endif
#ifndef HAVE_LFIND
extern "C" void *
lfind(  const void *key,
          void *base,
          size_t *nelp,
          size_t width,
          int (*compar) (const void *, const void *) )
{
	int i=0;
	char *base_c = (char *)base;
	char *sentinel = base_c + (*nelp)*width;
	
	while( base_c < sentinel )
	{
		if( (*compar)(base_c, key) == 0 )
			return base_c;
		base_c += width;
	}

	return NULL;
}
#endif
} // namespace lilfes
