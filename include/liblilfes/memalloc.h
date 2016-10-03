/*  $Id: memalloc.h,v 1.6 2011-05-02 08:48:59 matuzaki Exp $
 */

#ifndef __memalloc_h
#define __memalloc_h

#include "lconfig.h"
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <new>
#include <ostream>

namespace lilfes {

#ifdef new
#undef new
#endif //new

class lilfes_allocator_instance
{
private:
	struct linklist
	{
		linklist *next;
		size_t size;
	};

	struct malloclist
	{
		malloclist *next;
	};

	linklist *freelink;
	malloclist *root;
	int nfree;
	int freelimit;
	int nref;

	static const size_t allocunit = 65536 - sizeof(malloclist);
	static const int inifreelimit = 256;

public:
	lilfes_allocator_instance()  throw() { freelink = NULL; root = NULL; nfree = 0; freelimit = inifreelimit; nref = 1;}
	~lilfes_allocator_instance() throw();
	void garbage_collection() throw(std::bad_alloc);

	int IncRef() { return ++nref; }
	int DecRef() { return --nref; }

	void *allocate(size_t n) throw(std::bad_alloc);
	void deallocate(void *p, size_t n) throw(std::bad_alloc);
	void unlink_malloclist(malloclist *x) throw(std::bad_alloc);

private:
	void *reserve_new_area(size_t n) throw(std::bad_alloc);
	void free_area(void *p) throw();
};

template <class T>
class lilfes_allocator
{
public:
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	template <class U> struct rebind { typedef lilfes_allocator<U> other; };
private:

	lilfes_allocator_instance *inst;

public:
	lilfes_allocator() throw() { inst = new lilfes_allocator_instance; }
	~lilfes_allocator() throw() { if( inst && inst->DecRef() == 0 ) delete inst; }
	lilfes_allocator(const lilfes_allocator<T> &t) throw() { inst = t.inst; inst->IncRef(); }

	void destroy_allocator() throw() { delete inst; inst = NULL; }

	lilfes_allocator<T>& operator=(const lilfes_allocator<T> &t) throw() { inst->DecRef(); inst = t.inst; inst->IncRef(); }

	template <class U> lilfes_allocator(const lilfes_allocator<U> &t) throw() { inst = t.GetInstance(); inst->IncRef(); }
	lilfes_allocator_instance *GetInstance() const throw() { return inst; }

	pointer allocate(size_type n) throw(std::bad_alloc) { return (pointer) inst->allocate(n * sizeof(T)); }
	pointer allocate(size_type n, const void* hint) throw(std::bad_alloc) { return allocate(n); }
	void *allocate_bytes(size_t n) throw(std::bad_alloc) { return inst->allocate(n); }

	void deallocate(pointer p, size_type n) throw(std::bad_alloc) { inst->deallocate(p, n * sizeof(T)); }
	void deallocate_bytes(void *p, size_t n) throw(std::bad_alloc) { return inst->deallocate(p, n); }

	size_type max_size() const throw() { return 0x7ffffffc / sizeof(T); }

	void construct(pointer p, const T& val) { new(p) T(val); }
	void destroy(pointer p) { p->~T(); }

	pointer address(reference r) const { return &r; }
	const_pointer address(const_reference r) const { return &r; }

	void gc() { inst->garbage_collection(); }

	friend bool operator==(const lilfes_allocator<T> &a, const lilfes_allocator<T> &b) throw() { return a.inst == b.inst; }
	friend bool operator!=(const lilfes_allocator<T> &a, const lilfes_allocator<T> &b) throw() { return a.inst != b.inst; }

	friend class lilfes_allocator_instance;
};

template <class T, class A> struct Alloc_traits;

template <class T>
struct Alloc_traits<T, lilfes_allocator<T> >
{
	static const bool _S_instanceless = false;
	typedef lilfes_allocator<T> allocator_type;
};

inline void *lilfes_allocator_instance::reserve_new_area(size_t sz) throw(std::bad_alloc)
{
//	std::cout << "malloc" << std::endl;
	void *x = malloc(sz);
	if( x == NULL )
		throw std::bad_alloc();
	return x;
}

inline void lilfes_allocator_instance::free_area(void *p) throw()
{
//	std::cout << "free" << std::endl;
	free(p);
}

} // namespace lilfes

#endif // __memalloc_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/09/28 06:00:32  yusuke
 *
 * removed warning messages
 *
 * Revision 1.4  2004/05/30 08:40:26  yusuke
 *
 * memory leak check のプログラムを実装。それにともない、memory leak を減
 * らした。
 *
 * Revision 1.3  2004/04/06 11:21:41  tsuruoka
 * dos2unix
 *
 */
