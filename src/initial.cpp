/* $Id: initial.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 1997-1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: initial.cpp,v 1.3 2011-05-02 10:38:23 matuzaki Exp $";

#include "initial.h"

namespace lilfes {



InitTermBase::InitTermBase(InitTermBase **table, int ip)
{
	priority = ip;
	while(*table != NULL && (*table)->priority <= ip) 
	{
		table = &((*table)->next);
	}
	
	next = *table;
	*table = this;
}

void InitTermBase::PerformAll(InitTermBase *table)
{
	while( table != NULL )
	{
		table->Work();
		table = table->next;
	}
}

Initializer::Initializer(int ip) 
	: InitTermBase( &initable, ip )
{
	
}

Terminator::Terminator(int ip) 
	: InitTermBase( &trmtable, -ip )
{
	
}

void Initializer::PerformAll()
{
	InitTermBase::PerformAll(initable); 
}

void Terminator::PerformAll()
{
	InitTermBase::PerformAll(trmtable); 
}

InitTermBase *Initializer::initable = NULL;
InitTermBase *Terminator::trmtable = NULL;


InitFunction::InitFunction(void (*f)(), int ip)
	: Initializer(ip) { func = f; } 

void InitFunction::Work() 
{ 
	(*func)();
}

TermFunction::TermFunction(void (*f)(), int ip)
	: Terminator(ip) { func = f; } 

void TermFunction::Work() 
{ 
	(*func)();
}

} // namespace lilfes
