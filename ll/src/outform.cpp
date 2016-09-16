/* $Id: outform.cpp,v 1.4 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "outform.h"
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>

namespace lilfes {

using std::endl;
using std::ostream;
using std::string;
using std::strlen;


static const char rcsid[] = "$Id: outform.cpp,v 1.4 2011-05-02 10:38:23 matuzaki Exp $";


//////////////////////////////////////////////////////////////////////////////
//
//  class outform
//
//  This class is used for formatting the feature structure.

outform::outform()
{ 
	x = y = 0; 
	data = NULL;
}

outform::outform(const string &s)
{
	x = y = 0; 
	data = NULL; 
	SetString(s); 
}

outform::outform(const outform &o)
{
	x = o.x;
	y = o.y; 
	data = new string[y]; 
	for( int i=0; i<y; i++ )
		data[i] = o.data[i];
}

outform::~outform() 
{
	if( data != NULL )
		delete[] data;
}

outform & outform::operator=(const outform &o)
{
	if( data != NULL )
		delete[] data;
	x = o.x;
	y = o.y; 
	data = new string[y]; 
	for( int i=0; i<y; i++ )
		data[i] = o.data[i];
	return *this;
}

void outform::SetString(const string &s) 
{ 
	delete[] data;
	data = new string[1];
	data[0] = s;
	x = s.length();
	y = 1; 
}

void outform::SurroundByBracket(const char * lt, const char * lm, const char * lb, const char * lx, const char * rt, const char * rm, const char * rb, const char * rx)
{
	if( data == NULL )
		SetString((string)lx + rx);
	else if( y == 1 )
	{
		data[0] = lx + data[0] + rx;
		x += strlen(lx)+strlen(rx);
	} 
	else {
		data[0] = lt + data[0] + rt;
		for( int i=1; i<y-1; i++ )
			data[i] = lm + data[i] + rm;
		data[y-1] = lb + data[y-1] + rb;
		x += strlen(lt)+strlen(rb);
	}
}

void outform::AddAtBottom(const outform &of)
{
	string *newdata = new string[y + of.y];
	int newx = (x > of.x ? x : of.x);
	int i;
	
	for( i=0; i<y; i++ )
		if( x < of.x )
			newdata[i] = data[i] + repeat_string(' ', of.x-x );
		else
			newdata[i] = data[i];

	for(    ; i<y+of.y; i++ )
		if( x > of.x )
			newdata[i] = of.data[i-y] + repeat_string(' ', x-of.x );
		else
			newdata[i] = of.data[i-y];

	x = newx;
	y = y + of.y;

	delete[] data;
	data = newdata;
}

void outform::AddAtRight(const outform &of)
{
	int i;
	if( y >= of.y )
	{
		int topgap = ( y - of.y ) / 2;
		
		for( i=0; i<topgap; i++ )
			data[i] = data[i] + repeat_string(' ',of.x);
		for(    ; i<topgap + of.y; i++ )
			data[i] = data[i] + of.data[i - topgap];
		for(    ; i<y; i++ )
			data[i] = data[i] + repeat_string(' ',of.x);
	}
	else
	{
		string *newdata = new string[of.y];
		int topgap = ( of.y - y ) / 2;

		for( i=0; i<topgap; i++ )
			newdata[i] = repeat_string(' ', x) + of.data[i];
		for(    ; i<topgap + y; i++ )
			newdata[i] = data[i-topgap] + of.data[i];
		for(    ; i<of.y; i++ )
			newdata[i] = repeat_string(' ', x) + of.data[i];

		delete[] data;
		data = newdata;
		y = of.y;
	}
		x = x + of.x;
}

void outform::AddAtRightTop(const outform &of)
{
	int i=0;
	if( y >= of.y )
	{
		for(    ; i<of.y; i++ )
			data[i] = data[i] + of.data[i];
		for(    ; i<y; i++ )
			data[i] = data[i] + repeat_string(' ',of.x);
	}
	else
	{
		string *newdata = new string[of.y];

		for(    ; i<y; i++ )
			newdata[i] = data[i] + of.data[i];
		for(    ; i<of.y; i++ )
			newdata[i] = repeat_string(' ', x) + of.data[i];

		delete[] data;
		data = newdata;
		y = of.y;
	}
		x = x + of.x;
}

void outform::AddAtRightBottom(const outform &of)
{
	int i=0;
	if( y >= of.y )
	{
		for(    ; i<y - of.y; i++ )
			data[i] = data[i] + repeat_string(' ',of.x);
		for(    ; i<y; i++ )
			data[i] = data[i] + of.data[i - (y-of.y)];
	}
	else
	{
		string *newdata = new string[of.y];

		for(    ; i<(of.y-y); i++ )
			newdata[i] = repeat_string(' ', x) + of.data[i];
		for(    ; i<of.y; i++ )
			newdata[i] = data[i - (of.y-y)] + of.data[i];

		delete[] data;
		data = newdata;
		y = of.y;
	}
		x = x + of.x;
}

ostream& operator<<(ostream& os, outform of)
{
	for( int i=0; i<of.y; i++ )
		os << of.data[i] << endl;
	return os;
}

} // namespace lilfes
