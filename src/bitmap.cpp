/* $Id: bitmap.cpp,v 1.7 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1996-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: bitmap.cpp,v 1.7 2011-05-02 10:38:23 matuzaki Exp $";

#include "bitmap.h"
#include <cstring>
#include <iostream>
#include <ostream>

namespace lilfes {

using std::cout;
using std::memset;
using std::memcpy;
using std::ostream;

Bitmap::Bitmap(unsigned is) 
{ 
	size = is; 
	ar = new int32[(size+31)/32]; 
	memset(ar, 0, ((size+31)/32) * sizeof(int32) );
}

Bitmap::Bitmap(const Bitmap &b) 
{ 
	size = b.size; 
	ar = new int32[(size+31)/32]; 
	memcpy(ar, b.ar, ((size+31)/32) * sizeof(int32) );
}

Bitmap& Bitmap::operator =(const Bitmap &b) 
{ 
	delete[] ar;
	size = b.size; 
	ar = new int32[(size+31)/32]; 
	memcpy(ar, b.ar, ((size+31)/32) * sizeof(int32) );
	return *this;
}


void Bitmap::Merge(const Bitmap &b) 
{ 
	for(uint i=0; i<(size+31)/32; i++ )
	{
		ar[i] |= b.ar[i];
	}
}

bool Bitmap::Ident(const Bitmap &b) const
{
	if( b.size != size )
	{
		ABORT("Ident sizes are not same");
		return false;
	}
	for( uint i=0; i<(size+31)/32; i++ )
	{
		if( b.ar[i] != ar[i] ) {
			return false;
		}
	}
	return true;
}

bool Bitmap::IsAllZero() const
{
	for( uint i=0; i<(size+31)/32; i++ )
	{
		if( ar[i] != 0 )
		{
			return false;
		}
	}
	
	return true;
}

void Bitmap::Output(ostream &o) const
{
	o << "<" << size << " bit(s)>";
	for( uint i=0; i<size; i++ )
	{
		o << (Test(i) ? "1" : "_");
	}
}


} // namespace lilfes




/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/05/18 10:29:34  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.5  2004/05/30 12:29:01  yusuke
 *
 * warning message をけした。
 *
 * Revision 1.4  2004/04/23 10:08:42  yusuke
 *
 * strstream -> sstream
 * g++3 でこんぱいるできるようにへんこう。
 *
 * Revision 1.3  2004/04/06 11:15:27  tsuruoka
 * dos2unix, nkf -e
 *
 */
