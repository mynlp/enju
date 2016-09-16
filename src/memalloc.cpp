#include <ios>
#include <iostream>
#include <new>
#include <ostream>
#include <vector>
#include <stdlib.h>
#include "memalloc.h"
#include "profile.h"

namespace lilfes {

using std::bad_alloc;
using std::vector;


void *lilfes_allocator_instance::allocate(size_t sz) throw(bad_alloc)
{
	P1("allocate");
	if( sz < sizeof(linklist) ) sz = sizeof(linklist);
	else sz = (sz + 3) & ~3;
	
	if( sz >= allocunit )
	{
		malloclist *newmal = (malloclist *)reserve_new_area(sz + sizeof(malloclist));
		newmal->next = root;
		root = newmal;
		return (void *)(newmal + 1);
	}

	linklist **p = &freelink;

	while( *p != NULL )
	{
		if( (*p)->size == sz )	// Hitting the same-size block
		{
			void *ret = *p;
			*p = (*p)->next;
			nfree--;
			return ret;
		}
		if( (*p)->size >= sz+sizeof(linklist) )	// Hitting larger block
		{
			(*p)->size -= sz;
			void *ret = (void *)((char *)(*p) + (*p)->size);
			return ret;
		}
		p = &((*p)->next);
	}
	if( *p == NULL )
	{
		malloclist *newmal = (malloclist *)reserve_new_area(allocunit + sizeof(malloclist));
		newmal->next = root;
		root = newmal;

		linklist *newlink = (linklist *)(newmal + 1);
		newlink->next = freelink;
		freelink = newlink;
		nfree++;

		newlink->size = allocunit - sz;
		return (void *)((char *)(newlink) + newlink->size);
	}
	// never comes here
	return NULL;
};


void lilfes_allocator_instance::deallocate(void *p, size_t sz) throw(bad_alloc)
{
	P1("deallocate");
	if( sz < sizeof(linklist) ) sz = sizeof(linklist);
	else sz = (sz + 3) & ~3;

	if( sz >= allocunit )
	{
		unlink_malloclist((malloclist *)p-1);
		free_area((malloclist *)p-1);
	}
	else
	{
		linklist *newlink = (linklist *)(p);
		newlink->next = freelink;
		newlink->size = sz;
		freelink = newlink;
		nfree++;
	}

	if( nfree >= freelimit )
		garbage_collection();
}

lilfes_allocator_instance::~lilfes_allocator_instance() throw()
{
	malloclist *p = root;
	while( p != NULL )
	{
		void *x = p;
		p = p->next;
		free_area(x);
	}
//	root = NULL;
}

void lilfes_allocator_instance::unlink_malloclist(malloclist *x) throw(bad_alloc)
{
	P1("unlink");
	if( root == x )
		root = root->next;
	else
	{
		malloclist *z = root;
		while( z->next != x )
		{
			z = z->next;
			if( z == NULL )
				throw bad_alloc();
		}
		z->next = z->next->next;
	}
}

void lilfes_allocator_instance::garbage_collection() throw(bad_alloc)
{
	P1("defrag");
	if( freelink == NULL )
		return;

	linklist *newlink = freelink;
	freelink = freelink->next;
	newlink->next = NULL;

	int x = 1;

	while( freelink != NULL )
	{
		linklist *next = freelink->next;

		if( freelink < newlink )
		{
			freelink->next = newlink;
			newlink = freelink;
		}
		else
		{
			linklist *p = newlink;
			while( p->next != NULL && freelink > p->next )
			{
				p = p->next;
			}
			freelink->next = p->next;
			p->next = freelink;
		}
		freelink = next;
		x++;
	}

//	freelink = NULL;

	while( newlink->next != NULL )
	{
		if( newlink->size + (char *)newlink == (char *)(newlink->next) )
		{
			newlink->size += newlink->next->size;
			newlink->next = newlink->next->next;
			nfree--;
/*
			if( newlink->size == allocunit )
			{
				malloclist *x = (malloclist *)newlink-1;
				nfree--;
				newlink = newlink->next;
				unlink_malloclist(x);
				free_area(x);
				if( newlink == NULL )
					goto hoge;
			}
*/
		}
		else
		{
			linklist *next = newlink->next;
			newlink->next = freelink;
			freelink = newlink;
			newlink = next;
		}
	}

	newlink->next = freelink;
	freelink = newlink;
//hoge:

//	std::cout << "gc end " << nfree << std::endl;

	if( freelimit < nfree * 7/4 )
		freelimit = nfree * 7/4;
}

//////////////////////////////////////////////////////////////////////
#if 0
int main()
{
	const int size = 49152;
	lilfes_allocator<int> allocator;

	for( int l=0; l<100; l++ )
	{
		P1("Loop");
		vector<int *> vec;
		for( int i=0; i<size; i++ )
		{
			int size = rand()%100+1;
			int *p = allocator.allocate(size);
//			int *p = new int[size];
			*p = size;
			vec.push_back(p);
		}
		for( int i=0; i<size; i++ )
		{
				int k = (i*113+345)%size;
//				std::cout << "free " << std::hex << (int)vec[i] << " size " << *vec[i] << std::dec << std::endl;
				allocator.deallocate(vec[i], *vec[i]);
//				delete[] vec[i];
		}
	}
}
#endif

} // namespace lilfes

