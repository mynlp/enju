/* $Id: memchk.cpp,v 1.4 2004-05-30 09:27:01 yusuke Exp $
 *
 *    Copyright (c) 1998, Makino Takaki
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

static const char rcsid[] = "$Id: memchk.cpp,v 1.4 2004-05-30 09:27:01 yusuke Exp $";

#include "lconfig.h"

//#define LILFES_DEBUG_MEMORY_LEAK
#ifdef LILFES_DEBUG_MEMORY_LEAK

#include "errors.h"
#include "builtin.h"

#ifdef new
#undef new
#endif // new

using namespace lilfes;

namespace lilfes {

using std::bad_alloc;
using std::cerr;
using std::cout;
using std::dec;
using std::endl;
using std::hex;



  static ptr_int new_counter = 0;
  static ptr_int del_counter = 0;
  static ptr_int new_array_counter = 0;
  static ptr_int del_array_counter = 0;

  struct memory_alloc_info {
    const char* file_name;
    int line_no;
    size_t size;
    memory_alloc_info* prev;
    memory_alloc_info* next;
  };

  static memory_alloc_info* memory_alloc_info_root = NULL;

  static void disp_memory_leak() {
    cout << "new_counter: " << new_counter << endl;
    cout << "new_array_counter: " << new_array_counter << endl;
    cout << "del_counter: " << del_counter << endl;
    cout << "del_array_counter: " << del_array_counter << endl;
    memory_alloc_info* it = memory_alloc_info_root->prev;
    while (it != memory_alloc_info_root) {
      cerr << it->file_name << ": " << it->line_no << " size=" << it->size << '\n';
      it = it->prev;
    }
    cerr.flush();
    free(memory_alloc_info_root);
  }

  static void init() {
    atexit(disp_memory_leak);
//     static int* test_int = 0;
//     cerr << "a0" << endl;
//     test_int = new int;
//     cerr << "a1" << endl;
//     delete test_int;
//     cerr << "a2" << endl;
//     test_int = new int;
//     cerr << "a3" << endl;
//     test_int = new int;
//     cerr << "a4" << endl;
//     delete test_int;
//     cerr << "a5" << endl;
//     test_int = new int;
//     cerr << "a6" << endl;
//     test_int = new int;
//     cerr << "a7" << endl;
//     delete test_int;
//     cerr << "a8" << endl;
//     test_int = new int;
//     cerr << "a9" << endl;
//     *test_int = 100;
  }
  static InitFunction IF(init, 0);
}

static void add_to_list(memory_alloc_info* meminfo, size_t size, const char* file_name, int line_no) {
  if (!memory_alloc_info_root) {
    memory_alloc_info_root = static_cast<memory_alloc_info*>(malloc(sizeof(memory_alloc_info)));
    memory_alloc_info_root->prev = memory_alloc_info_root;
    memory_alloc_info_root->next = memory_alloc_info_root;
  }
  meminfo->file_name = file_name;
  meminfo->line_no = line_no;
  meminfo->size = size;
  meminfo->prev = memory_alloc_info_root;
  meminfo->next = memory_alloc_info_root->next;
  memory_alloc_info_root->next->prev = meminfo;
  memory_alloc_info_root->next = meminfo;
}

void* operator new(size_t size, const char* file_name, int line_no) throw (bad_alloc) {
  void* ptr = malloc(size + sizeof(memory_alloc_info));
  if ( ptr == NULL ) {
    printf( "%d bytes memory required at %s:%d\n", size, file_name, line_no );
    ABORT( "Memory allocation failed" );
  }
  ++new_counter;
  memory_alloc_info* meminfo = static_cast<memory_alloc_info*>(ptr);
  add_to_list(meminfo, size, file_name, line_no);
  return static_cast<void*>(meminfo + 1);
}

void* operator new[](size_t size, const char* file_name, int line_no) throw (bad_alloc) {
  void* ptr = malloc(size + sizeof(memory_alloc_info));
  if ( ptr == NULL ) {
    printf( "%d bytes memory required at %s:%d\n", size, file_name, line_no );
    ABORT( "Memory allocation failed" );
  }
  ++new_array_counter;
  memory_alloc_info* meminfo = static_cast<memory_alloc_info*>(ptr);
  add_to_list(meminfo, size, file_name, line_no);
  return static_cast<void*>(meminfo + 1);
}

// for precompiled libraries
void* operator new(size_t size) throw (bad_alloc) {
  void* ptr = malloc(size + sizeof(memory_alloc_info));
  if ( ptr == NULL ) {
    printf( "%d bytes memory required\n", size );
    ABORT( "Memory allocation failed" );
  }
  ++new_counter;
  memory_alloc_info* meminfo = static_cast<memory_alloc_info*>(ptr);
  add_to_list(meminfo, size, "unknown", 0);
  return static_cast<void*>(meminfo + 1);
}

void* operator new[](size_t size) throw (bad_alloc) {
  void* ptr = malloc(size + sizeof(memory_alloc_info));
  if ( ptr == NULL ) {
    printf( "%d bytes memory required\n", size );
    ABORT( "Memory allocation failed" );
  }
  ++new_array_counter;
  memory_alloc_info* meminfo = static_cast<memory_alloc_info*>(ptr);
  add_to_list(meminfo, size, "unknown", 0);
  return static_cast<void*>(meminfo + 1);
}

void operator delete(void* ptr) throw () {
  //printf("delete %x\n", ptr);
  if (ptr) {
    ++del_counter;
    memory_alloc_info* meminfo = static_cast<memory_alloc_info*>(ptr) - 1;
    meminfo->next->prev = meminfo->prev;
    meminfo->prev->next = meminfo->next;
    free(static_cast<void*>(meminfo));
  }
  //printf("end %x\n", ptr);
  return;
}

void operator delete[](void* ptr) throw () {
  //printf("delete[] %x\n", ptr);
  if (ptr) {
    ++del_array_counter;
    memory_alloc_info* meminfo = static_cast<memory_alloc_info*>(ptr) - 1;
    meminfo->next->prev = meminfo->prev;
    meminfo->prev->next = meminfo->next;
    free(static_cast<void*>(meminfo));
  }
  //printf("end %x\n", ptr);
  return;
}

#endif // LILFES_DEBUG_MEMORY_LEAK

//////////////////////////////////////////////////////////////////////

//#define DEBUG_MEM
//#define DEBUG_MEM_CONSISTENCY
#ifdef DEBUG_MEM
#include "basic.h"
#include "initial.h"
#include "errors.h"
#include <stdlib.h>
#include <ios>
#include <iostream>
#include <new>
#include <ostream>
#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#else
#  include <io.h>
#endif

using namespace lilfes;

static const ptr_int SIZE_INVALID = static_cast< ptr_int >( -1 );

ptr_int newcounter = 0;
ptr_int delcounter = 0;

void SanityCheck();

void abesh() 
{
	cout << "abesh " << newcounter << " " << delcounter << endl;
	SanityCheck();
}

#ifdef HAS_INT64
#define MAGICNUM_S 0xaf02c47918d326a9L
#define MAGICNUM_V 0xb31ae06d75326b97L
#define MAGICNUM_D 0xc3107f6d23e591feL
#else
#define MAGICNUM_S 0xaf02c479
#define MAGICNUM_V 0xb31ae06d
#define MAGICNUM_D 0xc31061fe
#endif

//#define TRAP_NEWCOUNTER 13941
//#define TRAP_DELCOUNTER 15687

#ifndef DEBUG_MEM2
#define DISP_ALLOC(i) 
#define DISP_DELETE(i)
#else
#define DISP_ALLOC(i) disp("ALLOC  ", i)
#define DISP_DELETE(i) disp("DELETE ", i)


static const int ppp[] = { 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1 };

static void dispint(int i)
{
	for( int n=0; n<9; n++ )
	{
		char c = i / ppp[n] + '0';
		write(1, &c, 1);
		i -= (c-'0') * ppp[n];
	}
}

static void disp(char *p, int i)
{
	write(1, p, 7);
	
	dispint(i);
/*	write(1, " ", 1);
	dispint(newcounter);
	write(1, " ", 1);
	dispint(delcounter); */
	write(1, "\n", 1);
}
#endif

struct nheader
{
	ptr_int  newc;
	ptr_int  size;
	nheader *next;
	nheader *prev;
};

nheader *mem_root = NULL;

#if 0
static void init() 
{
#ifdef DEBUG
	cout << "Initialize " << __FILE__ << endl;
#endif
	char *p = new char[8];
	strcpy(p, "this a test");
}
static InitFunction IF(init, 0);
#endif

static void term() 
{ 
	cout << "newcounter: " << newcounter << endl;
	cout << "delcounter: " << delcounter << endl;
	SanityCheck();
	if( mem_root != NULL )
		cout << "non-deleted newc: " << mem_root->newc << endl;
}

static TermFunction TF(term, 0);

void * operator new(size_t size)
{
#ifdef TRAP_NEWCOUNTER
	if( newcounter == TRAP_NEWCOUNTER )
		abesh();
#endif

	if( (newcounter & 1023) == 1023 )
		SanityCheck();

#ifdef DEBUG_MEM_CONSISTENCY
	size = (size + 7) & ~7;
	
	void *p = malloc(size + sizeof(ptr_int) + sizeof(nheader));
	
	if( p == NULL )
	{
          printf( "Required size: %d\n", size );
          ABORT( "virtual memory exceeded" );
	}

	DISP_ALLOC(newcounter);
	
	if( mem_root == NULL )
	{
		((nheader *)p)->prev = (nheader *)p;
		((nheader *)p)->next = (nheader *)p;
		mem_root = (nheader *)p;
	}
	else
	{
		((nheader *)p)->prev = mem_root->prev;
		((nheader *)p)->next = mem_root;
		mem_root->prev->next = (nheader *)p;
		mem_root->prev = (nheader *)p;
	}
	((nheader *)p)->newc = newcounter++;
	((nheader *)p)->size = size;
	((ptr_int *)((char *)p+size+sizeof(nheader)))[0] = MAGICNUM_S;
	
	return (char *)p + sizeof(nheader);
#else // DEBUG_MEM_CONSISTENCY
        newcounter++;
	void *p = malloc(size);
	
	if( p == NULL )
	{
          printf( "Required size: %d\n", size );
          ABORT( "virtual memory exceeded" );
	}

        return p;
#endif // DEBUG_MEM_CONSISTENCY
}

void * operator new[](size_t size)
{
#ifdef TRAP_NEWCOUNTER
	if( newcounter == TRAP_NEWCOUNTER )
		abesh();
#endif
	
	if( (newcounter & 1023) == 1023 )
		SanityCheck();
	
#ifdef DEBUG_MEM_CONSISTENCY
	size = (size + 7) & ~7;
	
	void *p = malloc(size + sizeof(ptr_int) + sizeof(nheader));
	
	if( p == NULL )
	{
          printf( "Required size: %d\n", size );
          ABORT( "virtual memory exceeded" );
	}

	DISP_ALLOC(newcounter);

	if( mem_root == NULL )
	{
		((nheader *)p)->prev = (nheader *)p;
		((nheader *)p)->next = (nheader *)p;
		mem_root = (nheader *)p;
	}
	else
	{
		((nheader *)p)->prev = mem_root->prev;
		((nheader *)p)->next = mem_root;
		mem_root->prev->next = (nheader *)p;
		mem_root->prev = (nheader *)p;
	}
	((nheader *)p)->newc = newcounter++;
	((nheader *)p)->size = size;
	((ptr_int *)((char *)p+size+sizeof(nheader)))[0] = MAGICNUM_V;
	
	return (char *)p + sizeof(nheader);
#else // DEBUG_MEM_CONSISTENCY
        newcounter++;
	void *p = malloc(size);
	
	if( p == NULL )
	{
          printf( "Required size: %d\n", size );
          ABORT( "virtual memory exceeded" );
	}

        return p;
#endif // DEBUG_MEM_CONSISTENCY
}

void operator delete(void *p)
{
#ifdef TRAP_DELCOUNTER
	if( delcounter == TRAP_DELCOUNTER )
		abesh();
#endif

	if( (delcounter & 1023) == 1023 )
		SanityCheck();

#ifdef DEBUG_MEM_CONSISTENCY
	if( p != NULL )
	{
		nheader *nh = (nheader *)((char *)p - sizeof(nheader));
		ptr_int newc = nh->newc;
		if( newc >= newcounter )
		{
			cout << "Internal error: illegal delete, newc=" << newc << ", delc=" << delcounter << endl;
			delcounter++;
			return;
		}
		ptr_int size = nh->size;
		if( size == SIZE_INVALID )
		{
			cout << "Internal error: double delete, newc=" << newc << ", delc=" << delcounter << endl;
			delcounter++;
			return;
		}
		ptr_int magicnum = ((ptr_int *)((char *)p + size))[0];

		if( magicnum != MAGICNUM_S )
		{
			if( magicnum == MAGICNUM_V )
				cout << "Internal error: delete for new[], newc=" << newc << ", delc=" << delcounter << endl;
			else
			{
				cout << "Internal error: delete for unknown, newc=" << newc << ", delc=" << delcounter << endl;
				delcounter++;
				return;
			}
		}
		nh->size = SIZE_INVALID;
		
		nh->next->prev = nh->prev;
		nh->prev->next = nh->next;
		
		for( size_t i=0; i<size / sizeof(ptr_int); i++ )
			((ptr_int *)p)[i] = MAGICNUM_D;
//		free( (char *)p - sizeof(ptr_int)*2 );
		DISP_DELETE(newc);
		delcounter++;
	}

	return;
#else // DEBUG_MEM_CONSISTENCY
        free( p );
        delcounter++;
        return;
#endif // DEBUG_MEM_CONSISTENCY
}

void operator delete[](void *p)
{
#ifdef TRAP_DELCOUNTER
	if( delcounter == TRAP_DELCOUNTER )
		abesh();
#endif

	if( (delcounter & 1023) == 1023 )
		SanityCheck();

#ifdef DEBUG_MEM_CONSISTENCY
	if( p != NULL )
	{
		nheader *nh = (nheader *)((char *)p - sizeof(nheader));
		ptr_int newc = nh->newc;
		if( newc >= newcounter )
		{
			cout << "Internal error: illegal delete[], newc=" << newc << ", delc=" << delcounter << endl;
			delcounter++;
			return;
		}
		ptr_int size = nh->size;
		if( size == SIZE_INVALID )
		{
			cout << "Internal error: double delete[], newc=" << newc << ", delc=" << delcounter << endl;
			delcounter++;
			return;
		}
		ptr_int magicnum = ((ptr_int *)((char *)p + size))[0];

		if( magicnum != MAGICNUM_V )
		{
			if( magicnum == MAGICNUM_S )
				cout << "Internal error: delete[] for new, newc=" << newc << ", delc=" << delcounter << endl;
			else
			{
				cout << "Internal error: delete[] for unknown, newc=" << newc << ", delc=" << delcounter << " (ptr: " << hex << (void*)p << dec << endl;
				delcounter++;
				return;
			}
		}
		nh->size = SIZE_INVALID;
		
		nh->next->prev = nh->prev;
		nh->prev->next = nh->next;

		for( size_t i=0; i<size / sizeof(ptr_int); i++ )
			((ptr_int *)p)[i] = MAGICNUM_D;
//		free( (char *)p - sizeof(ptr_int)*2 );
		DISP_DELETE(newc);
		delcounter++;
	}

	return;
#else // DEBUG_MEM_CONSISTENCY
        free( p );
        delcounter++;
        return;
#endif // DEBUG_MEM_CONSISTENCY
}

void SanityCheck()
{
#if defined( DEBUG ) || defined( DEBUG_MEM2 )
	write(1, "SanityCheck\n", 12);
#endif
	size_t ncnt = 0;
	nheader *nh = mem_root;
	
	if( nh != NULL )
		do {
			if( ncnt++ > newcounter )
			{
				cout << "Memory Insane ... counter over  newc:" << nh->newc << endl;
				return;
			}
			if( nh->newc >= newcounter )
			{
				cout << "Memory Insane ... bad newc" << endl;
				return;
			}
			if( nh->next->prev != nh )
			{
				cout << "Memory Insane ... bad link  newc:" << nh->newc << endl;
				return;
			}
			if( *(ptr_int *)((char *)nh+sizeof(nheader)+nh->size) != MAGICNUM_S && 
				*(ptr_int *)((char *)nh+sizeof(nheader)+nh->size) != MAGICNUM_V )
			{
				cout << "MEMORY Insane ... magic number not match  newc:" << nh->newc << endl;
				return;
			}
			nh = nh->next;
		} while( nh != mem_root );
}

#endif

