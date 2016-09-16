// -*- C++ -*-
/*
 *
 * Copyright (c) 2005
 * Takashi Tsunakawa and Takashi Ninomiya
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Takashi Tsunakawa and Takashi Ninomiya make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __PSTL_PHEAP_H
#define __PSTL_PHEAP_H

#ifndef __PSTL_THROW_BAD_ALLOC
#  if defined(__PSTL_NO_BAD_ALLOC) || !defined(__PSTL_USE_EXCEPTIONS)
#    include <cstdio>
#    include <cstdlib>
#    define __PSTL_THROW_BAD_ALLOC __STD::cerr << "out of memory" << __STD::endl; while(1); exit(1)
//#    define __PSTL_THROW_BAD_ALLOC fprintf(stderr, "out of memory\n"); exit(1)
#  else /* Standard conforming out-of-memory handling */
#    include <new>
#    define __PSTL_THROW_BAD_ALLOC throw std::bad_alloc()
#  endif
#endif

#include "_pstl_config.h"
#include "_pstl_exception.h"
#include "pstl_replace_pheap.h"
#include "pstl_iterator"
#include "_pstl_construct.h"
#include "pstl_type_traits.h"
//#include "pstl_hash_fun"
#include "pstl_fwd.h"
#include <unistd.h>
#include <sys/mman.h>
#include <vector>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <exception>

#if defined( HAVE_EXT_HASH_MAP )
#include <ext/hash_map>
#else
#include <hash_map>
#endif

__PSTL_BEGIN_NAMESPACE

///////////////////////////////////
/// bit size memo
///
/// ia32 (and 32-bit code on amd64)  amd64
/// Type         Byte(s)   Bit(s)    Byte(s)   Bit(s)
/// char         1         8         1         8
/// short        2         16        2         16
/// int          4         32        4         32
/// long         4         32        8         64  <= diff
/// long long    8         64        8         64
/// float        4         32        4         32
/// double       8         64        8         64
/// long double  12        96        16        128 <= diff
    
////////////////////////////////
/// bit size
class b32 {
public:
    typedef uint32 size_type;
    static const uint32 max_size_type = static_cast<size_type>(-1);
    typedef uint32 pointer;
    typedef int32 difference_type;
    typedef uint32 muint;
    typedef int32  mint;
    typedef float mfloat;
    static const size_type bit_size = 32;
    static const size_type pointer_size = 4;
    static const pointer magic_number = 0x9d3bc345UL;
};
class b64 {
public:
    typedef uint64 size_type;
    static const uint64 max_size_type = static_cast<size_type>(-1);
    typedef uint64 pointer;
    typedef int64 difference_type;
    typedef uint64 muint;
    typedef int64 mint;
    typedef double mfloat;
    static const size_type bit_size = 64;
    static const size_type pointer_size = 8;
    static const pointer magic_number = 0xf7284e345a87b5a9ULL;
};

class pmem {
public:
    static const char dev_type = 'p';
};
class vmem {
public:
    static const char dev_type = 'v';
};

template <class _Dev> class _BitTraits;

template <> class _BitTraits<pmem> {
public:
#if SIZEOF_INTP == 4
    typedef b32 default_bit;
#elif SIZEOF_INTP == 8
    typedef b64 default_bit;
#endif
};
template <> class _BitTraits<vmem> {
public:
    typedef b64 default_bit;
};

template <class _Dev, class _Bit= typename _BitTraits<_Dev>::default_bit>
class arch {
public:
    typedef _Dev dev_type;
    typedef _Bit bit_type;
    typedef typename bit_type::size_type       size_type;
    typedef typename bit_type::pointer         pointer;
    typedef typename bit_type::difference_type difference_type;
    typedef typename bit_type::muint           muint;
    typedef typename bit_type::mint            mint;
    typedef typename bit_type::mfloat          mfloat;
    static const size_type max_size_type = bit_type::max_size_type;
};

#if SIZEOF_INTP == 4
typedef b32 real_bit_type;
typedef b64 unreal_bit_type;
typedef arch<pmem, b32> real_arch_type;
#elif SIZEOF_INTP == 8
typedef b64 real_bit_type;
typedef b32 unreal_bit_type;
typedef arch<pmem, b64> real_arch_type;
#endif

//////////////////////////////
/// Proto Type Declaration
#ifndef __PSTL_PROTO_TYPE_DECLARATION
#define __PSTL_PROTO_TYPE_DECLARATION
template <class _Obj, class _Arch> class ptr;
template <class _Obj, class _Arch> class const_ptr;
template <class _Obj, class _Arch> class ref;
template <class _Obj, class _Arch> class const_ref;
#endif // __PSTL_PROTO_TYPE_DECLARATION

#ifndef __PSTL_CLASS_HEADER
#define __PSTL_CLASS_HEADER
template <class _Arch> class pheap;

template <class _Arch> class header {
public:
    typedef typename _Arch::size_type size_type;
    typedef typename _Arch::pointer pointer;
    ptr<header, _Arch> next;
    size_type size;

public:
    header() : next(0), size(0) {}
    header(pheap_ptr __ph) : next(0, __ph), size(0) {}
    header(ptr<header, _Arch> __next, size_type __s = 0) : next(__next), size(__s) {}
};

template <class _Arch>
struct replace_pheap<header<_Arch> > {
    void operator()(pheap_ptr __ph, header<_Arch> &__h) {
	__replace_pheap(__ph, __h.next);
    }
};

#endif // __PSTL_CLASS_HEADER

//// pheap
template <class _Arch> class pheap {};

//// pheap pmem
template <class _Bit> class pheap<arch<pmem, _Bit> > {
public:
    typedef pmem                                dev_type;
    typedef _Bit                                bit_type;
    typedef arch<dev_type, bit_type>            arch_type; 
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef pheap<arch_type>                    pheap_type;
private:
    pointer root;
public:
    pheap(size_type) : root(0) {}
    pheap() : root(0) {}
    bool open(const char *) { return true; }
    bool close() { return true; }
    ptr<void, arch_type> getRoot() {
	return ptr<void, arch_type>(root, pheap_convert(this));
    }
    void setRoot(const ptr<void, arch_type> &p) {
	root = p.getAddress();
    }

    ptr<void, arch_type> malloc(size_type __n) {
	void *__result = __STD_QUALIFIER malloc(__n);
	if (__result == 0) { __PSTL_THROW_BAD_ALLOC; }
	return ptr<void, arch_type>(reinterpret_cast<pointer>(__result), pheap_convert(this));
    }
    template <class _Obj>
    void free(const ptr<_Obj, arch_type> &__p) {
	__STD_QUALIFIER free(reinterpret_cast<void *>(__p.getAddress()));
    }
    template <class _Obj>
    ptr<void, arch_type> realloc(ptr<_Obj, arch_type> __p, size_type __new_sz) {
	void *__result = __STD_QUALIFIER realloc(reinterpret_cast<void *>(__p.getAddress), __new_sz);
	if (__result == NULL) { __PSTL_THROW_BAD_ALLOC; }
	return ptr<void, arch_type>(reinterpret_cast<pointer>(__result), pheap_convert(this));
    }

    void *map(pointer p, size_type sz) {
	return reinterpret_cast<void *>(p);
    }
    void unmap(pointer p, size_type sz) {}
    
    void dump() {}

    static pheap_ptr pheap_convert(pheap_type *__ph) {
	return static_cast<pheap_ptr>(reinterpret_cast<ptr_int>(__ph));
    }
    static pheap_type *pheap_convert(pheap_ptr __ph) {
	return reinterpret_cast<pheap_type *>(static_cast<ptr_int>(__ph)); 
    }

    template <class _Obj, class _Arch> friend class ptr;
    template <class _Obj, class _Arch> friend class const_ptr;
    template <class _Obj, class _Arch> friend class ref;
    template <class _Obj, class _Arch> friend class const_ref;

};

//// pheap vmem
template <class _Bit> class __page_map_key {
public:
    typedef typename _Bit::size_type size_type;
    size_type page_index;
    size_type page_number;
    __page_map_key() : page_index(0), page_number(0) {}
    __page_map_key(size_type idx, size_type num) : page_index(idx), page_number(num) {}
    ~__page_map_key() {}
};

template <class _Bit> class __page_map_mapped {
public:
    typedef typename _Bit::size_type size_type;
    void *page;
    size_type reference_counter;
    typename __STD::list<__page_map_key<_Bit> >::iterator it_in_queue;
    __page_map_mapped(void *pg, size_type rc, typename __STD::list<__page_map_key<_Bit> >::iterator it)
	: page(pg), reference_counter(rc), it_in_queue(it) {}
    ~__page_map_mapped() {}
};

template <class _Bit> class __page_map_hash {
public:
    size_t operator()(const __page_map_key<_Bit> &key) const {
	return key.page_index + key.page_number;
    }
};

template <class _Bit> class __page_map_equal_to {
public:
    bool operator()(const __page_map_key<_Bit> &key1, const __page_map_key<_Bit> &key2) const {
	return (key1.page_index == key2.page_index && key1.page_number == key2.page_number);
    }

};

template <class _Arch> class pheap_persistent_data {
public:
    typedef typename _Arch::size_type size_type;
    typedef typename _Arch::pointer   pointer;
    typedef header<_Arch>             header_type;
    typedef pheap<_Arch>              pheap_type;
    size_type top;
    size_type bit_size;
    size_type magic_number;
    pointer root;
    ptr<header_type, _Arch> basep;
    ptr<header_type, _Arch> allocp;

    template <class _Obj> friend struct replace_pheap;
};

template <class _Arch>
struct replace_pheap<pheap_persistent_data<_Arch> > {
    void operator()(pheap_ptr __ph, pheap_persistent_data<_Arch> &__obj) {
	__replace_pheap(__ph, __obj.basep);
	__replace_pheap(__ph, __obj.allocp);
    }
};

template <class _Bit> class pheap<arch<vmem, _Bit> > {
public:
    typedef vmem                                dev_type;
    typedef _Bit                                bit_type;
    typedef arch<dev_type, bit_type>            arch_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
private:
    typedef __page_map_key<_Bit>                page_map_key;
    typedef __page_map_mapped<_Bit>             page_map_mapped;
    typedef __page_map_hash<_Bit>               page_map_hash;
    typedef __page_map_equal_to<_Bit>           page_map_equal_to;
    typedef __gnu_cxx::hash_map<page_map_key, page_map_mapped,
				page_map_hash, page_map_equal_to>
    page_map;
    typedef pheap_persistent_data<arch_type>    pheap_persistent_data_type;
    typedef header<arch_type>                   header_type;
    typedef pheap<arch_type>                    pheap_type;
    // variables and functions for page manipulation
    size_t bit_size;
    size_t pointer_size;
    size_t page_size;
    size_t page_bit_size;
    size_t offset_bit_size;
    size_t brk_size;
    pointer page_mask;
    pointer offset_mask;
    static const pointer magic_number = _Bit::magic_number;
    size_type max_page_number;
    static const size_type nalloc = 256;

    // file
    FILE *vmem_fp;

    // map
    page_map pages;
    __STD::list<page_map_key> unref_page_queue;

    // __pheap
    pheap_persistent_data_type *persistent_data;

    // is open?
    bool is_open __PSTL_ALIGNED_64;
    
    void __init() {
	bit_size = _Bit::bit_size;
	pointer_size = _Bit::pointer_size;
	brk_size = page_size = getpagesize();

	int __count = 0;
	for(size_t s = 2, os = 1; os < 64 ; s<<=1, os++) {
	    if(s == page_size) {
		__count++;
		offset_bit_size = os;
	    }
	}
	if( __count != 1 ) {
	    __STD::cerr << "system page size is not the size of 2^x" << __STD::endl;
	    exit(EXIT_FAILURE);
	}
	page_bit_size = bit_size - offset_bit_size;
	offset_mask = 0;
	for(size_t i = 0 ; i < offset_bit_size ; i++) {
	    offset_mask <<= 1;
	    offset_mask |= static_cast<pointer>(1);
	}
	page_mask = ~offset_mask;
	//__init_status();
    }

    void __init_status() {
	  __STD::cerr << "bit_size: " << bit_size << __STD::endl;
	  __STD::cerr << "page_size: " << page_size << __STD::endl;
	  __STD::cerr << "page_bit_size: " << page_bit_size << __STD::endl;
	  __STD::cerr << "offset_bit_size: " << offset_bit_size << __STD::endl;
	  __STD::cerr << "page_mask: " << __STD::hex << page_mask << __STD::dec << __STD::endl;
	  __STD::cerr << "offset_mask: " << __STD::hex << offset_mask << __STD::dec << __STD::endl;
    }
    
    size_type get_page_index(pointer p) {
	return static_cast<size_type>(p >> offset_bit_size);
    }
    size_type get_page_offset(pointer p) {
	return static_cast<size_type>(p & offset_mask);
    }
    size_type get_page_number(pointer p, size_type sz) {
	return get_page_index(p + sz - 1) - get_page_index(p) + 1;
    }
    pointer align(pointer x, pointer u) { return (x + (u - 1)) & ~(u - 1); } // u is 2^x

private:
    void __map_status0(pointer p, size_type sz, size_type page_idx, size_type page_num) {
	std::cerr << "p = " << p << ", sz = " << sz << ", page_idx = " << page_idx << ", page_num = " << page_num << std::endl;
	if (p == 0 || p > 10000000) {
	    std::cerr << "error!" << std::endl;
	}
    }
    
public:
    void *map(pointer p, size_type sz) {
        if (!is_open) throw closed_pheap_access();
	size_type page_idx = get_page_index(p);
	size_type page_num = get_page_number(p, sz);
	//__map_status0(p, sz, page_idx, page_num);
	page_map_key key(page_idx, page_num);
	typename page_map::iterator it = pages.find(key);

	if (it != pages.end()) { // found in cache
	    if (it->second.reference_counter == 0) {
		unref_page_queue.erase(it->second.it_in_queue);
	    }
	    it->second.reference_counter++;
	    //__map_status1();
	    return reinterpret_cast<void *>(static_cast<ptr_int>(reinterpret_cast<pointer>(it->second.page) + p - page_idx * page_size));
	}
	// not found in cache
	//__map_status1();
	while (pages.size() >= max_page_number && !unref_page_queue.empty()) { // page out
	    typename page_map::iterator victim = pages.find(unref_page_queue.back());
	    //__map_status2(victim);
	    msync(victim->second.page, victim->first.page_number * page_size, MS_SYNC);
	    munmap(victim->second.page, victim->first.page_number * page_size);
	    pages.erase(victim);
	    unref_page_queue.pop_back();
	}
	// insert a new page to cache
	void *mp = mmap(0, page_num * page_size,
			PROT_WRITE|PROT_READ, MAP_SHARED,
			fileno(vmem_fp), page_idx * page_size);
	pages.insert(typename page_map::value_type(
			 typename page_map::key_type(key),
			 typename page_map::mapped_type(page_map_mapped(mp, 1, unref_page_queue.end()))));
	return reinterpret_cast<void *>(static_cast<ptr_int>(reinterpret_cast<pointer>(mp) + p - page_idx * page_size));
    }
    
    void unmap(pointer p, size_type sz) {
        if (!is_open) throw closed_pheap_access();
	page_map_key key(get_page_index(p), get_page_number(p, sz));
	typename page_map::iterator it = pages.find(key);
	if (it == pages.end()) {
	    __PSTL_THROW_BAD_ALLOC;
	}
	if (it->second.reference_counter == 0) {
	    __PSTL_THROW_BAD_ALLOC;
	}
	it->second.reference_counter--;
	if (it->second.reference_counter == 0) {
	    unref_page_queue.push_front(key);
	    it->second.it_in_queue = unref_page_queue.begin();
	}
	//__map_status1();
	return;
    }

    void __map_status1() {
	__STD::cerr << "pages size: " << pages.size() << __STD::endl;
	__STD::cerr << "try page out pages: ";
	typename page_map::iterator hogeit = pages.begin(), hogelast = pages.end();
	for(; hogeit != hogelast ; hogeit++)
	    __STD::cerr << "{" << hogeit->first.page_index << " " << hogeit->first.page_number << " " << hogeit->second.reference_counter << "}";
	__STD::cerr << __STD::endl;
          
	typename __STD::list<page_map_key>::iterator pogeit = unref_page_queue.begin(), pogelast = unref_page_queue.end();
	for(; pogeit != pogelast ; pogeit++)
	    __STD::cerr << "[" << pogeit->page_index << " " << pogeit->page_number << "]";
	__STD::cerr << __STD::endl;
    }

    void __map_status2(typename page_map::iterator victim) {
	__STD::cerr << "page out: " << victim->first.page_index << " " << victim->first.page_number << __STD::endl;
    }
    
    void __brk(pointer p) __PSTL_THROW(__STD::bad_alloc()) {
	if (fseeko(vmem_fp, p, SEEK_SET) != 0) {
	    __PSTL_THROW_BAD_ALLOC;
	}
	int hoge = 0;
	__STD::fwrite(&hoge, sizeof(int), 1, vmem_fp);
	if (__STD::fflush(vmem_fp) != 0) {
	    __PSTL_THROW_BAD_ALLOC;
	}
    }
    
public:
    pheap(size_type __max_page_number) : max_page_number(__max_page_number), vmem_fp(0), is_open(false) { __init(); }
    pheap() : max_page_number(-1), vmem_fp(0), is_open(false)  { __init(); }
    ~pheap() { if ( is_open ) close(); }

    void brk(pointer p) {
	__brk(p);
	persistent_data->top = p;
    }

    ptr<void, arch_type> sbrk(size_type p) {
	//std::cerr << "sbrk start" << std::endl;
	pointer r = persistent_data->top;
	brk(r + p);
	persistent_data->top = r + p;
	//std::cerr << "sbrk finish" << std::endl;
	return ptr<void, arch_type>(r, pheap_convert(this));
    }
    
    bool open(const char *path) {
        if (is_open) {
            std::cerr << "warning: this pheap is already open" << std::endl;
            return false;
        }
	if ((vmem_fp = __STD::fopen(path, "r+b")) != NULL) { // find the file
	    // global page
            is_open = true;
	    persistent_data = static_cast<pheap_persistent_data_type *>(map(pointer_size, sizeof(pheap_persistent_data_type)));
	   
	    if (persistent_data->magic_number != magic_number) {
		__STD::cerr << path << " is not pstl file." << __STD::endl;
                unmap(pointer_size, sizeof(pheap_persistent_data_type));
                is_open = false;
		return false;
	    }
	    if (persistent_data->bit_size != bit_size) {
		__STD::cerr << path << " is not the same bit size." << __STD::endl;
                unmap(pointer_size, sizeof(pheap_persistent_data_type));
                is_open = false;
		return false;
	    }

	    __replace_pheap(pheap_type::pheap_convert(this), *persistent_data);
	    
	    return true;
	}
	if ((vmem_fp = __STD::fopen(path, "w+b")) != NULL) { // create a new file
            is_open = true;
	    __brk(page_size);
	    persistent_data = static_cast<pheap_persistent_data_type *>(map(pointer_size, sizeof(pheap_persistent_data_type)));
	    persistent_data->magic_number = magic_number;
	    persistent_data->bit_size = bit_size;
	    persistent_data->top = align(pointer_size + sizeof(pheap_persistent_data_type), pointer_size);
	    persistent_data->root = 0;
	    persistent_data->basep = static_cast<ptr<header_type, arch_type> >(sbrk(sizeof(header_type)));
	    persistent_data->allocp = persistent_data->basep;
	    (*persistent_data->basep)->size = 0;
	    (*persistent_data->allocp)->size = 0;
	    (*persistent_data->basep)->next = persistent_data->basep;
	    (*persistent_data->allocp)->next = persistent_data->basep;
	    __replace_pheap(pheap_convert(this), *persistent_data);
	    //check();
	    return true;
	}
	return false;
    }
    bool close() {
        if(!is_open) throw closed_pheap_access();
	typename page_map::iterator it = pages.begin(), last = pages.end();
	for(; it != last ; it++) {
	    msync(it->second.page, it->first.page_number * page_size, MS_SYNC);
	    munmap(it->second.page, it->first.page_number * page_size);
	}
	if( fclose(vmem_fp) != 0) return false;
        is_open = false;
        return true;
    }

    ptr<void, arch_type> getRoot() {
        if(!is_open) throw closed_pheap_access();
	return ptr<void, arch_type>(persistent_data->root, pheap_convert(this));
    }

    void setRoot(const ptr<void, arch_type> &p) {
        if(!is_open) throw closed_pheap_access();
	persistent_data->root = p.getAddress();
    }

private:
    ptr<header_type, arch_type> morecore(size_type nu) {
	size_type rnu = nalloc * ((nu + nalloc - 1) / nalloc);
	ptr<void, arch_type> cp = sbrk(rnu * (sizeof(header_type)));
	ptr<header_type, arch_type> up = static_cast<ptr<header_type, arch_type> >(cp);
	(*up)->size = rnu;
	ptr<header_type, arch_type> up1 = up + 1;
	free(ptr<void, arch_type>(up1, pheap_convert(this)));
	return persistent_data->allocp;
    }

    void printfreelist(const std::string __s) {
	ptr<header_type, arch_type> p = persistent_data->allocp;
	ref<header_type, arch_type> pref = *p;
	std::cerr << __s << std::endl;
	//dump();
	do {
	    std::cerr << "addr = 0x" << std::hex << p.getAddress() << ", size = " << std::dec << pref->size << std::endl;
	    p = pref->next;
	    if (p.getAddress() == 0) {
		std::cerr << "free list link broken" << std::endl;
		return;
	    }
	    pref.reassign(*p);
	} while (p != persistent_data->allocp);
    }

public:
    ptr<void, arch_type> malloc(size_type __n) {
        if (!is_open) throw closed_pheap_access();
	ptr<header_type, arch_type> p, q;
	size_type nunits = 1 + (__n + sizeof(header_type) - 1) / sizeof(header_type);
	q = persistent_data->allocp;
	ref<header_type, arch_type> qref = *q;
	p = qref->next;
	ref<header_type, arch_type> pref = *p;
	for (;;) {
	    if (pref->size >= nunits) { // big enough
		if (pref->size == nunits) { // exactly
		    qref->next = pref->next;
		}
		else {
		    pref->size = pref->size - nunits;
		    p += pref->size;
		    pref.reassign(*p);
		    pref->size = nunits;
		}
		persistent_data->allocp = q;
		ptr<header_type, arch_type> p1 = p + 1;
		//printfreelist("malloc finish");
		return ptr<void, arch_type>(p1, pheap_convert(this));
	    }
	    if (p == persistent_data->allocp) { // wrapped around free list
		p = morecore(nunits);
	    }
	    // postproc
	    q = p;
	    qref.reassign(*q);
	    p = pref->next;
	    pref.reassign(*p);
	}
    }

    void free(const ptr<void, arch_type> &__p) {
        if (!is_open) throw closed_pheap_access();
	ptr<header_type, arch_type> p, q;
	p = static_cast<ptr<header_type, arch_type> >(__p) - 1; // point to header 50
	ref<header_type, arch_type> pref = *p;
	q = persistent_data->allocp; // 40
	ref<header_type, arch_type> qref = *q;
	for ( ; !(p > q && p < qref->next); ) {
	    if (q >= qref->next && (p > q || p < qref->next))
		break; // at one end or other
	    //postproc
	    q = qref->next;
	    qref.reassign(*q);
	}

	if (p + pref->size == qref->next) { // join to upper nbr
	    pref->size += (*qref->next)->size;
	    pref->next = (*qref->next)->next;
	}
	else {
	    pref->next = qref->next;
	}
	if (q + qref->size == p) { // join to lower nbr
	    qref->size += pref->size;
	    qref->next = pref->next;
	}
	else {
	    qref->next = p;
	}
	persistent_data->allocp = q;
	//printfreelist("free finish");
    }

    void dump() {
        if (!is_open) throw closed_pheap_access();
	__STD::cerr << __STD::hex;
	__STD::cerr << "top: 0x" << persistent_data->top << __STD::endl;
	__STD::cerr << "root: 0x" << persistent_data->root << __STD::endl;
	__STD::cerr << "bit_size: 0x" << persistent_data->bit_size << __STD::endl;
	__STD::cerr << "magic_number: 0x" << persistent_data->magic_number << __STD::endl;
	for(pointer i = 0; i < persistent_data->top; i++) {
	    if(i % 16 == 0) {
		__STD::cerr << __STD::endl;
		__STD::cerr << "[" << __STD::setfill('0') << __STD::setw(8) << i << "] ";
	    }
	    unsigned char *p = static_cast<unsigned char *>(map(i, sizeof(unsigned char)));
	    __STD::cerr << __STD::setfill('0') << __STD::setw(2) << (static_cast<unsigned int>(*p)) << " ";
    unmap(i, sizeof(unsigned char));
	}
	__STD::cerr << __STD::dec << __STD::endl;
    }


    void check() {
// 	static unsigned int checksum = 0;

// 	if (persistent_data->top <= 0x2044a0) {
// 	    return;
// 	}
// 	unsigned int checks[16], sum = 0;
// 	pointer pt;
// 	for (pointer i = 0; i < 16; i++) {
// 	    pt = i; // + global_offset;
// 	    unsigned char *p = static_cast<unsigned char *>(map(pt, sizeof(unsigned char)));
// 	    checks[i] = static_cast<unsigned int>(*p);
// 	    sum += checks[i];
// 	    unmap(pt, sizeof(unsigned char));
// 	}
// 	if (sum != checksum) {
// 	    for (pointer i = 0; i < 16; i++) {
// 		pt = i; // + global_offset;
// 		if (pt % 16 == 0) {
// 		    __STD::cerr << "[" << __STD::hex << __STD::setfill('0') << __STD::setw(8) << pt << "] ";
// 		}
// 		__STD::cerr << __STD::setfill('0') << __STD::setw(2) << checks[i] << " ";
// 		if (pt % 16 == 15) {
// 		    __STD::cerr << __STD::endl;
// 		}
// 	    }
// 	    __STD::cerr << __STD::dec << __STD::endl;
// 	    checksum = sum;
// 	}
    }
    
    static pheap_ptr pheap_convert(pheap<arch_type> *__ph) {
	return static_cast<pheap_ptr>(reinterpret_cast<ptr_int>(__ph));
    }
    static pheap<arch_type> *pheap_convert(pheap_ptr __ph) {
	return reinterpret_cast<pheap<arch_type> *>(static_cast<ptr_int>(__ph)); 
    }

    template <class _Obj, class _Arch> friend class ptr;
    template <class _Obj, class _Arch> friend class const_ptr;
    template <class _Obj, class _Arch> friend class ref;
    template <class _Obj, class _Arch> friend class const_ref;

    template <class _Obj> friend struct replace_pheap;
};

template <class _Bit>
struct replace_pheap<pheap<arch<vmem, _Bit> > > {
    void operator()(pheap_ptr __ph, pheap<arch<vmem, _Bit> > &__obj) {
	__replace_pheap(__ph, *__obj.persistent_data);
    }
};

//////////////////
/// Pointer

template <class _Obj, class _Arch>
class ptr {
public:
    typedef _Arch                               arch_type;
    typedef typename arch_type::dev_type        dev_type;
    typedef typename arch_type::bit_type        bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef ref<_Obj, _Arch>                    reference;
    typedef typename arch_type::difference_type difference_type;
    typedef _Obj                                value_type;
    typedef random_access_iterator_tag          iterator_category;
    typedef pheap<arch_type>                    pheap_type;
private:
    pointer addr;
    pheap_ptr ph;
public:
    ptr() : addr(0), ph(0) {}
    ptr(pointer sz) : addr(sz), ph(0) {}
    ptr(pointer sz, pheap_ptr __ph) : addr(sz), ph(__ph) {}
    template <class _Obj1> ptr(const ptr<_Obj1, arch_type> &p)
	: addr(p.getAddress()), ph(p.get_pheap()) {}
    template <class _Obj1> ptr(const ptr<_Obj1, arch_type> &p, pheap_ptr __ph) : addr(p.getAddress()), ph(__ph) {}
    ~ptr() {}
    operator ptr<void, arch_type>() { return ptr<void, arch_type>(addr, ph); }
    operator const_ptr<_Obj, arch_type>() { return const_ptr<_Obj, arch_type>(*this); }

    ref<_Obj, arch_type> operator*() const {
	if (typeid(dev_type) == typeid(vmem) && !addr) {
	    std::cerr << "error in ptr::operator*()" << std::endl;
	    while(1);
	    exit(01);
	}
	ref<_Obj, arch_type> __r(ph, *this);
	return __r;
    }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    _Obj *operator->() const
	{ return ref<_Obj, arch_type>(ph, *this).operator->(); }
#endif /* __SGI_PSTL_NO_ARROW_OPERATOR */
    ref<_Obj, arch_type> operator()() {
	ref<_Obj, arch_type> __r(ph, *this);
	return __r;
    }

    ref<_Obj, arch_type> get_ref() const {
	ref<_Obj, arch_type> __r(ph, *this);
	return __r;
    }
    void get_ref(ref<_Obj, arch_type> &__r) const {
	__r = ref<_Obj, arch_type>(ph, *this);
    }

    pointer getAddress() const { return addr; }
    void setAddress(pointer p) { addr = p; }
    pointer &getAddressRef() { return addr; }
    pheap_ptr get_pheap() const { return ph; }

    ptr &operator++() { addr += sizeof(_Obj); return *this; }
    ptr &operator--() { addr -= sizeof(_Obj); return *this; }
    template <class _Obj1> ptr &operator=(const ptr<_Obj1, arch_type> &p) { addr = p.getAddress(); ph = p.get_pheap(); return *this; }
    ptr &operator+=(difference_type x) { addr += x * sizeof(_Obj); return *this; }
    ptr &operator-=(difference_type x) { addr -= x * sizeof(_Obj); return *this; }
    ptr operator[](size_type n) const { return *this + n; }
    operator bool() const { return addr != 0; }

    template <class _Obj1, class _Arch1, class Y> friend ptr<_Obj1, _Arch1> operator+(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend ptr<_Obj1, _Arch1> operator+(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1>
    friend typename ptr<_Obj1, _Arch1>::difference_type operator-(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend ptr<_Obj1, _Arch1> operator-(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator==(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator==(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator==(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator!=(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator!=(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator!=(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<=(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<=(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<=(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>=(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>=(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>=(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend __STD::ostream &operator<<(__STD::ostream &, const ptr<_Obj1, _Arch1> &);

    template <class _Obj1, class _Arch1> friend class ptr;
    template <class _Obj1, class _Arch1> friend class const_ptr;
    template <class _Obj1, class _Arch1> friend class ref;
    template <class _Obj1, class _Arch1> friend class const_ref;
    template <class _Arch1> friend class pheap;

    template <class _Obj1> friend struct replace_pheap; 
};

template <class _Obj, class _Arch>
struct replace_pheap<ptr<_Obj, _Arch> > {
    void operator()(pheap_ptr __ph, ptr<_Obj, _Arch> &__p) {
	__p.ph = __ph;
    }
};

template <class _Obj, class _Arch>
struct replace_pheap<const ptr<_Obj, _Arch> > {
    void operator()(pheap_ptr __ph, const ptr<_Obj, _Arch> &__p) {
	const_cast<ptr<_Obj, _Arch>&>(__p).ph = __ph;
    }
};

//////////////////////////
//// Void Pointer

template <class _Arch> class ptr<void, _Arch> {
public:
    typedef _Arch                               arch_type;
    typedef typename arch_type::dev_type        dev_type;
    typedef typename arch_type::bit_type        bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef ref<void, _Arch>                    reference;
    typedef typename arch_type::difference_type difference_type;
    typedef void                                value_type;
    typedef random_access_iterator_tag          iterator_category;
    typedef pheap<arch_type>                    pheap_type;
private:
    pointer addr;
    pheap_ptr ph;
public:
    ptr() : addr(0), ph(0) {}
    ptr(pointer sz) : addr(sz), ph(0) {}
    ptr(pointer sz, pheap_ptr __ph) : addr(sz), ph(__ph) {}
    template <class _Obj> ptr(const ptr<_Obj, arch_type> &p) : addr(p.getAddress()), ph(p.get_pheap()) {}
    template <class _Obj> ptr(const ptr<_Obj, arch_type> &p, pheap_ptr __ph) : addr(p.getAddress()), ph(__ph) {}
    ~ptr() {}

    template <class _Obj> operator ptr<_Obj, arch_type>() { return ptr<_Obj, arch_type>(addr, ph); }
    operator const_ptr<void, arch_type>() { return const_ptr<void, arch_type>(*this); }
    
    pointer getAddress() const { return addr; }
    void setAddress(pointer p) { addr = p; }
    pheap_ptr get_pheap() const { return ph; }

    template <class _Obj> ptr &operator=(const ptr<_Obj, arch_type> &p) { addr = p.getAddress(); ph = p.get_pheap(); return *this; }
    operator bool() const { return addr != 0; }
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator==(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator==(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator==(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator!=(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator!=(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator!=(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<=(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<=(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<=(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>=(const ptr<_Obj1, _Arch1> &x, const ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>=(const ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>=(X x, const ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1> friend __STD::ostream &operator<<(__STD::ostream &, const ptr<_Obj1, _Arch1> &);

    template <class _Obj1, class _Arch1> friend class ptr;
    template <class _Obj1, class _Arch1> friend class const_ptr;
    template <class _Obj1, class _Arch1> friend class ref;
    template <class _Obj1, class _Arch1> friend class const_ref;
    template <class _Arch1> friend class pheap;

    template <class _Obj1> friend struct replace_pheap;
};

template <class _Arch>
struct replace_pheap<ptr<void, _Arch> > {
    void operator()(pheap_ptr __ph, ptr<void, _Arch> &__p) {
	__p.ph = __ph;
    }
};
template <class _Arch>
struct replace_pheap<const ptr<void, _Arch> > {
    void operator()(pheap_ptr __ph, const ptr<void, _Arch> &__p) {
	const_cast<ptr<void, _Arch>&>(__p).ph = __ph;
    }
};

//////////////////
/// Const Pointer

template <class _Obj, class _Arch>
class const_ptr {
public:
    typedef _Arch                               arch_type;
    typedef typename arch_type::dev_type        dev_type;
    typedef typename arch_type::bit_type        bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef _Obj                                value_type;
    typedef random_access_iterator_tag          iterator_category;
    typedef pheap<arch_type>                    pheap_type;
private:
    pointer addr;
    pheap_ptr ph;
public:
    const_ptr() : addr(0), ph(0) {}
    const_ptr(pointer sz) : addr(sz), ph(0) {}
    const_ptr(pointer sz, pheap_ptr __ph) : addr(sz), ph(__ph) {}
    template <class _Obj1> const_ptr(const const_ptr<_Obj1, arch_type> &p) : addr(p.getAddress()), ph(p.get_pheap()) {}
    template <class _Obj1> const_ptr(const const_ptr<_Obj1, arch_type> &p, pheap_ptr __ph) : addr(p.getAddress()), ph(__ph) {}
    template <class _Obj1> const_ptr(const ptr<_Obj1, arch_type> &p) : addr(p.getAddress()), ph(p.get_pheap()) {}
    template <class _Obj1> const_ptr(const ptr<_Obj1, arch_type> &p, pheap_ptr __ph) : addr(p.getAddress()), ph(__ph) {}
    ~const_ptr() {}
    operator const_ptr<void, arch_type>() { return const_ptr<void, arch_type>(addr, ph); }

    const_ref<_Obj, arch_type> operator*() const {
	const_ref<_Obj, arch_type> __r(ph, *this);
	return __r;
    }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    const _Obj *operator->() const
        { return const_ref<_Obj, arch_type>(ph, *this).operator->(); }
#endif /* __SGI_PSTL_NO_ARROW_OPERATOR */
    const_ref<_Obj, arch_type> operator()() const {
	const_ref<_Obj, arch_type> __r(ph, *this);
	return __r;
    }

    const_ref<_Obj, arch_type> get_ref() const {
	const_ref<_Obj, arch_type> __r(ph, *this);
	return __r;
    }
    void get_ref(const_ref<_Obj, arch_type> &__r) const {
	__r = const_ref<_Obj, arch_type>(ph, *this);
    }

    pointer getAddress() const { return addr; }
    void setAddress(pointer p) { addr = p; }
    pheap_ptr get_pheap() const { return ph; }

    const_ptr &operator++() { addr += sizeof(_Obj); return *this; }
    const_ptr &operator--() { addr -= sizeof(_Obj); return *this; }
    template <class _Obj1> const_ptr &operator=(const ptr<_Obj1, arch_type> &p) { addr = p.getAddress(); ph = p.get_pheap(); return *this; }
    const_ptr &operator+=(difference_type x) { addr += x * sizeof(_Obj); return *this; }
    const_ptr &operator-=(difference_type x) { addr -= x * sizeof(_Obj); return *this; }
    const_ptr operator[](size_type n) const { return *this + n; }
    operator bool() const { return addr != 0; }

    template <class _Obj1, class _Arch1, class Y> friend const_ptr<_Obj1, _Arch1> operator+(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend const_ptr<_Obj1, _Arch1> operator+(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1>
    friend typename const_ptr<_Obj1, _Arch1>::difference_type operator-(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1, class X> friend const_ptr<_Obj1, _Arch1> operator-(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator==(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator==(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator==(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator!=(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator!=(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator!=(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<=(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<=(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<=(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>=(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>=(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>=(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1> friend __STD::ostream &operator<<(__STD::ostream&, const const_ptr<_Obj1, _Arch1> &);

    template <class _Obj1, class _Arch1> friend class ptr;
    template <class _Obj1, class _Arch1> friend class const_ptr;
    template <class _Obj1, class _Arch1> friend class ref;
    template <class _Obj1, class _Arch1> friend class const_ref;
    template <class _Arch1> friend class pheap;

    template <class _Obj1> friend struct replace_pheap;
};

template <class _Obj, class _Arch>
struct replace_pheap<const_ptr<_Obj, _Arch> > {
    void operator()(pheap_ptr __ph, const_ptr<_Obj, _Arch> &__p) {
	__p.ph = __ph;
    }
};
template <class _Obj, class _Arch>
struct replace_pheap<const const_ptr<_Obj, _Arch> > {
    void operator()(pheap_ptr __ph, const const_ptr<_Obj, _Arch> &__p) {
	__p.ph = __ph;
    }
};

//////////////////////////
//// Void Const Pointer

template <class _Arch> class const_ptr<void, _Arch> {
public:
    typedef _Arch                               arch_type;
    typedef typename arch_type::dev_type        dev_type;
    typedef typename arch_type::bit_type        bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef void                                value_type;
    typedef random_access_iterator_tag          iterator_category;
    typedef pheap<arch_type>                    pheap_type;
private:
    pointer addr;
    pheap_ptr ph;
public:
    const_ptr() : addr(0), ph(0) {}
    const_ptr(pointer sz, pheap_ptr __ph) : addr(sz), ph(__ph) {}
    template <class _Obj> const_ptr(const const_ptr<_Obj, arch_type> &p) : addr(p.getAddress()), ph(p.get_pheap()) {}
    template <class _Obj> const_ptr(const const_ptr<_Obj, arch_type> &p, pheap_ptr __ph) : addr(p.getAddress()), ph(__ph) {}
    template <class _Obj> const_ptr(const ptr<_Obj, arch_type> &p) : addr(p.getAddress()), ph(p.get_pheap()) {}
    template <class _Obj> const_ptr(const ptr<_Obj, arch_type> &p, pheap_ptr __ph) : addr(p.getAddress()), ph(__ph) {}
    ~const_ptr() {}

    pointer getAddress() { return addr; }
    void setAddress(pointer p) { addr = p; }
    pheap_ptr get_pheap() const { return ph; }

    template <class _Obj> const_ptr &operator=(const ptr<_Obj, arch_type> &p) { addr = p.getAddress(); ph = p.get_pheap(); return *this; }
    operator bool() const { return addr != 0; }
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator==(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator==(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator==(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator!=(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator!=(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator!=(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator<=(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator<=(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator<=(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Obj2, class _Arch1> friend bool operator>=(const const_ptr<_Obj1, _Arch1> &x, const const_ptr<_Obj2, _Arch1> &y);
    template <class _Obj1, class _Arch1, class Y> friend bool operator>=(const const_ptr<_Obj1, _Arch1> &x, Y y);
    template <class _Obj1, class _Arch1, class X> friend bool operator>=(X x, const const_ptr<_Obj1, _Arch1> &y);
    template <class _Obj1, class _Arch1> friend __STD::ostream &operator<<(__STD::ostream&, const const_ptr<_Obj1, _Arch1> &);

    template <class _Obj1, class _Arch1> friend class const_ptr;
    template <class _Obj1, class _Arch1> friend class const_ref;
    template <class _Arch1> friend class pheap;

    template <class _Obj1> friend struct replace_pheap;
};

template <class _Arch>
struct replace_pheap<const_ptr<void, _Arch> > {
    void operator()(pheap_ptr __ph, const_ptr<void, _Arch> &__p) {
	__p.ph = __ph;
    }
};
template <class _Arch>
struct replace_pheap<const const_ptr<void, _Arch> > {
    void operator()(pheap_ptr __ph, const const_ptr<void, _Arch> &__p) {
	__p.ph = __ph;
    }
};

//// operators for Pointer

template <class _Obj, class _Arch, class Y>
ptr<_Obj, _Arch> operator+(const ptr<_Obj, _Arch> &x, Y y) {
    return ptr<_Obj, _Arch>(x.getAddress() +
			   (static_cast<typename ptr<_Obj, _Arch>::difference_type>(y)) * sizeof(_Obj), x.get_pheap());
}
template <class _Obj, class _Arch, class X>
ptr<_Obj, _Arch> operator+(X x, const ptr<_Obj, _Arch> &y) {
    return ptr<_Obj, _Arch>((static_cast<typename ptr<_Obj, _Arch>::difference_type>(x)) * sizeof(_Obj) +
			   y.getAddress(), y.get_pheap());
}
template <class _Obj, class _Arch>
typename ptr<_Obj, _Arch>::difference_type operator-(const ptr<_Obj, _Arch> &x,
						    const ptr<_Obj, _Arch> &y) {
    return ((x.getAddress() - y.getAddress()) / sizeof(_Obj));
}
template <class _Obj, class _Arch, class Y>
ptr<_Obj, _Arch> operator-(const ptr<_Obj, _Arch> &x, Y y) {
    return ptr<_Obj, _Arch>(x.getAddress() -
			   (static_cast<typename ptr<_Obj, _Arch>::difference_type>(y)) * sizeof(_Obj), x.get_pheap());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator==(const ptr<_Obj1, _Arch> &x, const ptr<_Obj2, _Arch> &y) {
    return x.getAddress() == y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator==(const ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() == (static_cast<typename ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator==(X x, const ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename ptr<_Obj, _Arch>::pointer>(x)) == y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator!=(const ptr<_Obj1, _Arch> &x, const ptr<_Obj2, _Arch> &y) {
    return x.getAddress() != y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator!=(const ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() != (static_cast<typename ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator!=(X x, const ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename ptr<_Obj, _Arch>::pointer>(x)) != y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator<(const ptr<_Obj1, _Arch> &x, const ptr<_Obj2, _Arch> &y) {
    return x.getAddress() < y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator<(const ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() < (static_cast<typename ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator<(X x, const ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename ptr<_Obj, _Arch>::pointer>(x)) < y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator>(const ptr<_Obj1, _Arch> &x, const ptr<_Obj2, _Arch> &y) {
    return x.getAddress() > y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator>(const ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() > (static_cast<typename ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator>(X x, const ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename ptr<_Obj, _Arch>::pointer>(x)) > y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator<=(const ptr<_Obj1, _Arch> &x, const ptr<_Obj2, _Arch> &y) {
    return x.getAddress() <= y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator<=(const ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() <= (static_cast<typename ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator<=(X x, const ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename ptr<_Obj, _Arch>::pointer>(x)) <= y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator>=(const ptr<_Obj1, _Arch> &x, const ptr<_Obj2, _Arch> &y) {
    return x.getAddress() >= y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator>=(const ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() >= (static_cast<typename ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator>=(X x, const ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename ptr<_Obj, _Arch>::pointer>(x)) >= y.getAddress());
}
template <class _Obj, class _Arch>
__STD::ostream &operator<<(__STD::ostream &os, const ptr<_Obj, _Arch> &p) {
    os << __STD::hex << "0x" << p.getAddress() << __STD::dec;
    return os;
}

//// operators for Const Pointer

template <class _Obj, class _Arch, class Y>
const_ptr<_Obj, _Arch> operator+(const const_ptr<_Obj, _Arch> &x, Y y) {
    return const_ptr<_Obj, _Arch>(x.getAddress() +
				 (static_cast<typename const_ptr<_Obj, _Arch>::difference_type>(y)) * sizeof(_Obj), x.get_pheap());
}
template <class _Obj, class _Arch, class X>
const_ptr<_Obj, _Arch> operator+(X x, const const_ptr<_Obj, _Arch> &y) {
    return const_ptr<_Obj, _Arch>((static_cast<typename const_ptr<_Obj, _Arch>::difference_type>(x)) * sizeof(_Obj) +
				 y.getAddress(), y.get_pheap());
}
template <class _Obj, class _Arch>
typename const_ptr<_Obj, _Arch>::difference_type operator-(const const_ptr<_Obj, _Arch> &x,
							  const const_ptr<_Obj, _Arch> &y) {
    return ((x.getAddress() - y.getAddress()) / sizeof(_Obj));
}
template <class _Obj, class _Arch, class Y>
const_ptr<_Obj, _Arch> operator-(const const_ptr<_Obj, _Arch> &x, Y y) {
    return const_ptr<_Obj, _Arch>(x.getAddress() -
				 (static_cast<typename const_ptr<_Obj, _Arch>::difference_type>(y)) * sizeof(_Obj), x.get_pheap());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator==(const const_ptr<_Obj1, _Arch> &x, const const_ptr<_Obj2, _Arch> &y) {
    return x.getAddress() == y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator==(const const_ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() == (static_cast<typename const_ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator==(X x, const const_ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename const_ptr<_Obj, _Arch>::pointer>(x)) == y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator!=(const const_ptr<_Obj1, _Arch> &x, const const_ptr<_Obj2, _Arch> &y) {
    return x.getAddress() != y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator!=(const const_ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() != (static_cast<typename const_ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator!=(X x, const const_ptr<_Obj, _Arch> &y) {
    return (((typename const_ptr<_Obj, _Arch>::pointer) x) != y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator<(const const_ptr<_Obj1, _Arch> &x, const const_ptr<_Obj2, _Arch> &y) {
    return x.getAddress() < y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator<(const const_ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() < (static_cast<typename const_ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator<(X x, const const_ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename const_ptr<_Obj, _Arch>::pointer>(x)) < y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator>(const const_ptr<_Obj1, _Arch> &x, const const_ptr<_Obj2, _Arch> &y) {
    return x.getAddress() > y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator>(const const_ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() > (static_cast<typename const_ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator>(X x, const const_ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename const_ptr<_Obj, _Arch>::pointer>(x)) > y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator<=(const const_ptr<_Obj1, _Arch> &x, const const_ptr<_Obj2, _Arch> &y) {
    return x.getAddress() <= y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator<=(const const_ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() <= (static_cast<typename const_ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator<=(X x, const const_ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename const_ptr<_Obj, _Arch>::pointer>(x)) <= y.getAddress());
}
template <class _Obj1, class _Obj2, class _Arch>
bool operator>=(const const_ptr<_Obj1, _Arch> &x, const const_ptr<_Obj2, _Arch> &y) {
    return x.getAddress() >= y.getAddress();
}
template <class _Obj, class _Arch, class Y>
bool operator>=(const const_ptr<_Obj, _Arch> &x, Y y) {
    return (x.getAddress() >= (static_cast<typename const_ptr<_Obj, _Arch>::pointer>(y)));
}
template <class _Obj, class _Arch, class X>
bool operator>=(X x, const const_ptr<_Obj, _Arch> &y) {
    return ((static_cast<typename const_ptr<_Obj, _Arch>::pointer>(x)) >= y.getAddress());
}
template <class _Obj, class _Arch>
__STD::ostream &operator<<(__STD::ostream &os, const const_ptr<_Obj, _Arch> &p) {
    os << __STD::hex << "0x" << p.getAddress() << __STD::dec;
    return os;
}

/////////////////////////////////
//// Reference

template <class _Obj, class _Arch>
class ref {
protected:
    typedef _Arch                               arch_type;
    typedef typename arch_type::dev_type        dev_type;
    typedef typename arch_type::bit_type        bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef _Obj                                value_type;
    typedef random_access_iterator_tag          iterator_category;
    typedef pheap<arch_type>                    pheap_type;
    pheap_ptr ph;
    ptr<_Obj, arch_type> addr;
    _Obj *obj;
public:
    ref() : ph(0), addr(0), obj(0) {}
    //ref(pheap_ptr __ph) : ph(__ph), addr(0, __ph), obj(0) {}
    ref(pheap_ptr __ph, const ptr<_Obj, arch_type> &p) : ph(__ph), addr(p, __ph), obj(reinterpret_cast<_Obj *>(pheap_type::pheap_convert(ph)->map(p.getAddress(), sizeof(_Obj)))) {
	if (obj) {
	    __replace_pheap(ph, *obj);
	}
    }
    ref(const ref &r) : ph(r.get_pheap()), addr(&r), obj(reinterpret_cast<_Obj *>(pheap_type::pheap_convert(ph)->map(addr.getAddress(), sizeof(_Obj)))) {
	if (obj) {
	    __replace_pheap(ph, *obj);
	}
    }
    ~ref() { if (addr) { pheap_type::pheap_convert(ph)->unmap(addr.getAddress(), sizeof(_Obj)); } }

    operator const_ref<_Obj, arch_type>() { return const_ref<_Obj, arch_type>(*this); }

          _Obj &get_obj()       { return *obj; }
    const _Obj &get_obj() const { return *obj; }
    pheap_ptr get_pheap() const { return ph; }
          _Obj &operator*()       { return *obj; }
    const _Obj &operator*() const { return *obj; }
    operator       _Obj()       { return *obj; }
    operator const _Obj() const { return *obj; }
    _Obj copy() const { return *obj; }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
          _Obj *operator->()       { return obj; }
    const _Obj *operator->() const { return obj; }
#endif /*  __SGI_PSTL_NO_ARROW_OPERATOR */

          ptr<_Obj, arch_type> operator&()       { return addr; }
    const_ptr<_Obj, arch_type> operator&() const { return const_ptr<_Obj, arch_type>(addr); }
    
    ref &__reassign(const ref &r) {
	if (addr) {
	    pheap_type::pheap_convert(ph)->unmap(addr.getAddress(), sizeof(_Obj));
	}
	ph = r.ph;
	addr = r.addr;
	obj = reinterpret_cast<_Obj *>(pheap_type::pheap_convert(ph)->map(addr.getAddress(), sizeof(_Obj)));
	__replace_pheap(ph, *this);
	return *this;
    }
    ref &reassign(const ref &r) { return __reassign(r); }
    ref &operator=(const _Obj &o) { *obj = o; return *this; }
    ref &operator=(const ref &r) { *obj = r.get_obj(); return *this; }
    ref &operator++() { ++get_obj(); return *this; }
    ref &operator--() { --get_obj(); return *this; }
    ref &operator+=(const _Obj &o) { get_obj() += o; return *this; }
    ref &operator+=(const ref &r) { get_obj() += r.get_obj(); return *this; }
    ref &operator-=(const _Obj &o) { get_obj() -= o; return *this; }
    ref &operator*=(const _Obj &o) { get_obj() *= o; return *this; }
    ref &operator/=(const _Obj &o) { get_obj() /= o; return *this; }
    ref &operator%=(const _Obj &o) { get_obj() %= o; return *this; }
    ref &operator^=(const _Obj &o) { get_obj() ^= o; return *this; }
    ref &operator&=(const _Obj &o) { get_obj() &= o; return *this; }
    ref &operator|=(const _Obj &o) { get_obj() |= o; return *this; }
    ref &operator<<=(const _Obj &o) { get_obj() <<= o; return *this; }
    ref &operator>>=(const _Obj &o) { get_obj() >>= o; return *this; }

    template <class _Obj1> friend struct replace_pheap;
};

template <class _Obj, class _Arch>
struct replace_pheap<ref<_Obj, _Arch> > {
    void operator()(pheap_ptr __ph, ref<_Obj, _Arch> &__r) {
	__r.ph = __ph;
	__replace_pheap(__ph, __r.addr);
	if (__r.obj) {
	    __replace_pheap(__ph, *__r.obj);
	}
    }
};

//// efficient implementation for pmem
template <class _Obj, class _Bit>
class ref<_Obj, arch<pmem, _Bit> > {
protected:
    typedef arch<pmem, _Bit>                    arch_type;
    typedef pmem                                dev_type;
    typedef _Bit                                bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef pheap<arch_type>                    pheap_type;
    ptr<_Obj, arch_type> pt;
public:
    ref() : pt(0) {}
    ref(pheap_ptr __ph, const ptr<_Obj, arch_type> &p) : pt(p, __ph) {}
    ref(const ref &r) : pt(&r) {}
    ~ref() {}
          _Obj &get_obj()       { return *(reinterpret_cast<_Obj *>(pt.getAddress())); }
    const _Obj &get_obj() const { return *(reinterpret_cast<_Obj *>(pt.getAddress())); }
    pheap_ptr get_pheap() const { return 0; }
          _Obj &operator*()       { return get_obj(); }
    const _Obj &operator*() const { return get_obj(); }
    operator       _Obj()       { return get_obj(); }
    operator const _Obj() const { return get_obj(); }
    _Obj copy() const { return get_obj(); }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
          _Obj *operator->()       { return reinterpret_cast<_Obj *>(pt.getAddress()); }
    const _Obj *operator->() const { return reinterpret_cast<const _Obj *>(pt.getAddress()); }
#endif /* __SGI_PSTL_NO_ARROW_OPERATOR */
          ptr<_Obj, arch_type> operator&()       { return pt; }
    const_ptr<_Obj, arch_type> operator&() const { return const_ptr<_Obj, arch_type>(pt); }

    ref &reassign(const ref &r) { pt = r.pt; return *this; }
    ref &reassign(const const_ref<_Obj, arch<pmem, _Bit> > &r) { pt = &r; return *this; }
    ref &operator=(const _Obj &o) { get_obj() = o; return *this; }
    ref &operator=(const ref &r) { get_obj() = r.get_obj(); return *this; }
    ref &operator++() { ++get_obj(); return *this; }
    ref &operator--() { --get_obj(); return *this; }
    ref &operator+=(const _Obj &o) { get_obj() += o; return *this; }
    //    ref &operator+=(const ref &r) { get_obj() += r.get_obj(); return *this; }
    ref &operator-=(const _Obj &o) { get_obj() -= o; return *this; }
    ref &operator*=(const _Obj &o) { get_obj() *= o; return *this; }
    ref &operator/=(const _Obj &o) { get_obj() /= o; return *this; }
    ref &operator%=(const _Obj &o) { get_obj() %= o; return *this; }
    ref &operator^=(const _Obj &o) { get_obj() ^= o; return *this; }
    ref &operator&=(const _Obj &o) { get_obj() &= o; return *this; }
    ref &operator|=(const _Obj &o) { get_obj() |= o; return *this; }
    ref &operator<<=(const _Obj &o) { get_obj() <<= o; return *this; }
    ref &operator>>=(const _Obj &o) { get_obj() >>= o; return *this; }

};

/////////////////////////////////
//// Const Reference

template <class _Obj, class _Arch>
class const_ref {
protected:
    typedef _Arch                               arch_type;
    typedef typename arch_type::dev_type        dev_type;
    typedef typename arch_type::bit_type        bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef _Obj                                value_type;
    typedef random_access_iterator_tag          iterator_category;
    typedef pheap<arch_type>                    pheap_type;
    pheap_ptr ph;
    const_ptr<_Obj, arch_type> addr;
    const _Obj *obj;
public:
    const_ref() : ph(0), addr(0), obj(0) {}
    const_ref(pheap_ptr __ph, const const_ptr<_Obj, arch_type> &p) : ph(__ph), addr(p), obj(reinterpret_cast<const _Obj *>(pheap_type::pheap_convert(ph)->map(p.getAddress(), sizeof(_Obj)))) {
	if (obj) {
	    __replace_pheap(__ph, *(const_cast<_Obj *>(obj)));
	}
    }
    const_ref(const const_ref &r) : ph(r.get_pheap()), addr(&r), obj(reinterpret_cast<const _Obj *>(pheap_type::pheap_convert(ph)->map(addr.getAddress(), sizeof(_Obj)))) {}
    const_ref(const ref<_Obj, arch_type> &r) : ph(r.get_pheap()), addr(&r), obj(reinterpret_cast<const _Obj *>(pheap_type::pheap_convert(ph)->map(addr.getAddress(), sizeof(_Obj)))) {
	if (obj) {
	    __replace_pheap(ph, *(const_cast<_Obj *>(obj)));
	}
    }
    ~const_ref() { if (addr) { pheap_type::pheap_convert(ph)->unmap(addr.getAddress(), sizeof(_Obj)); } }

    const _Obj &get_obj() const { return *obj; }
    pheap_ptr get_pheap() const { return ph; }
    const _Obj &operator*() const { return *obj; }
    operator const _Obj() const { return *obj; }
    _Obj copy() const { return *obj; }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    const _Obj *operator->() const { return obj; }
#endif /* __SGI_PSTL_NO_ARROW_OPERATOR */
    const_ptr<_Obj, arch_type> operator&() const { return addr; }
    const_ref &__reassign(const const_ref &r) {
	if (addr) {
	    pheap_type::pheap_convert(ph)->unmap(addr.getAddress(), sizeof(_Obj));
	}
	ph = r.get_pheap();
	addr = &r;
	obj = reinterpret_cast<const _Obj *>(pheap_type::pheap_convert(ph)->map(addr.getAddress(), sizeof(_Obj)));
	__replace_pheap(ph, *this);
	return *this;
    }
    const const_ref &reassign(const ref<_Obj, arch_type> &r) { return __reassign(const_ref<_Obj, arch_type>(r)); }
    const const_ref &reassign(const const_ref &r) { return __reassign(r); }

    template <class _Obj1> friend struct replace_pheap;
};

template <class _Obj, class _Arch>
struct replace_pheap<const_ref<_Obj, _Arch> > {
    void operator()(pheap_ptr __ph, const_ref<_Obj, _Arch> &__r) {
	__r.ph = __ph;
	__replace_pheap(__ph, __r.addr);
	if (__r.obj) {
	    __replace_pheap(__ph, *(const_cast<_Obj *>(__r.obj)));
	}
    }
};

//// efficient implementation for pmem (const_ref)
template <class _Obj, class _Bit>
class const_ref<_Obj, arch<pmem, _Bit> > {
protected:
    typedef arch<pmem, _Bit>                    arch_type;
    typedef pmem                                dev_type;
    typedef _Bit                                bit_type;
    typedef typename arch_type::size_type       size_type;
    typedef typename arch_type::pointer         pointer;
    typedef typename arch_type::difference_type difference_type;
    typedef pheap<arch_type>                    pheap_type;
    const_ptr<_Obj, arch_type> pt;
public:
    const_ref() : pt(0) {}
    //const_ref(pheap_ptr __ph) : pt(0, __ph) {}
    const_ref(pheap_ptr __ph, const const_ptr<_Obj, arch_type> &p) : pt(p, __ph) {}
    const_ref(const const_ref &r) : pt(r.pt) {}
    const_ref(const ref<_Obj, arch_type> &r) : pt(&r) {}
    ~const_ref() {}
    const _Obj &get_obj() const {
	return *(reinterpret_cast<const _Obj *>(pt.getAddress()));
    }
    pheap_ptr get_pheap() const { return 0; }
    const _Obj &operator*() const { return get_obj(); }
    operator const _Obj() const { return get_obj(); }
    _Obj copy() const { return get_obj(); }
#ifndef __SGI_PSTL_NO_ARROW_OPERATOR
    const _Obj *operator->() const { return reinterpret_cast<const _Obj *>(pt.getAddress()); }
#endif /* __SGI_PSTL_NO_ARROW_OPERATOR */
    const_ptr<_Obj, arch_type> operator&() const { return pt; }
    const_ref &reassign(const const_ref &r) { pt = &r; return *this; }
    const_ref &reassign(const ref<_Obj, arch<pmem, _Bit> > &r) { pt = &r; return *this; }
//    const ref &reassign(const ref &r) const { pt = r.pt; return *this; }
};

////  operator +
////  operator -
////  operator *
////  operator /
////  operator %
////  operator ^
////  operator &
////  operator |
////  operator ~
////  operator !

////  operator ==
template <class _Obj1, class _Arch1, class _Obj2, class _Arch2>
bool operator==(const ref<_Obj1, _Arch1> &x, const ref<_Obj2, _Arch2> &y) {
    return (x.get_obj() == y.get_obj());
}
template <class _Obj, class _Arch>
bool operator==(const ref<_Obj, _Arch> &x, const _Obj &y) {
    return (x.get_obj() == y);
}
template <class _Obj, class _Arch>
bool operator==(const _Obj &x, const ref<_Obj, _Arch> &y) {
    return (x == y.get_obj());
}
////  operator !=
template <class _Obj1, class _Arch1, class _Obj2, class _Arch2>
bool operator!=(const ref<_Obj1, _Arch1> &x, const ref<_Obj2, _Arch2> &y) {
    return (x.get_obj() != y.get_obj());
}
template <class _Obj, class _Arch>
bool operator!=(const ref<_Obj, _Arch> &x, const _Obj &y) {
    return (x.get_obj() != y);
}
template <class _Obj, class _Arch>
bool operator!=(const _Obj &x, const ref<_Obj, _Arch> &y) {
    return (x != y.get_obj());
}
////  operator <
template <class _Obj1, class _Arch1, class _Obj2, class _Arch2>
bool operator<(const ref<_Obj1, _Arch1> &x, const ref<_Obj2, _Arch2> &y) {
    return (x.get_obj() < y.get_obj());
}
template <class _Obj, class _Arch>
bool operator<(const ref<_Obj, _Arch> &x, const _Obj &y) {
    return (x.get_obj() < y);
}
template <class _Obj, class _Arch>
bool operator<(const _Obj &x, const ref<_Obj, _Arch> &y) {
    return (x < y.get_obj());
}
////  operator <=
template <class _Obj1, class _Arch1, class _Obj2, class _Arch2>
bool operator<=(const ref<_Obj1, _Arch1> &x, const ref<_Obj2, _Arch2> &y) {
    return (x.get_obj() <= y.get_obj());
}
template <class _Obj, class _Arch>
bool operator<=(const ref<_Obj, _Arch> &x, const _Obj &y) {
    return (x.get_obj() <= y);
}
template <class _Obj, class _Arch>
bool operator<=(const _Obj &x, const ref<_Obj, _Arch> &y) {
    return (x <= y.get_obj());
}
////  operator >
template <class _Obj1, class _Arch1, class _Obj2, class _Arch2>
bool operator>(const ref<_Obj1, _Arch1> &x, const ref<_Obj2, _Arch2> &y) {
    return (x.get_obj() > y.get_obj());
}
template <class _Obj, class _Arch>
bool operator>(const ref<_Obj, _Arch> &x, const _Obj &y) {
    return (x.get_obj() > y);
}
template <class _Obj, class _Arch>
bool operator>(const _Obj &x, const ref<_Obj, _Arch> &y) {
    return (x > y.get_obj());
}
////  operator >=
template <class _Obj1, class _Arch1, class _Obj2, class _Arch2>
bool operator>=(const ref<_Obj1, _Arch1> &x, const ref<_Obj2, _Arch2> &y) {
    return (x.get_obj() >= y.get_obj());
}
template <class _Obj, class _Arch>
bool operator>=(const ref<_Obj, _Arch> &x, const _Obj &y) {
    return (x.get_obj() >= y);
}
template <class _Obj, class _Arch>
bool operator>=(const _Obj &x, const ref<_Obj, _Arch> &y) {
    return (x >= y.get_obj());
}
//// operator <<
template <class _Obj, class _Arch>
__STD::ostream &operator<<(__STD::ostream &os, const ref<_Obj, _Arch> &x) {
    os << x.get_obj();
    return os;
}
//// operator >>
template <class _Obj, class _Arch>
__STD::istream &operator>>(__STD::istream &is, const ref<_Obj, _Arch> &x) {
    is >> x.get_obj();
    return is;
}

//////////////////////////
//// delete

template<class _Obj, class _Arch>
void pdelete(const ptr<_Obj, _Arch> &__p) {
        // call destructor
    
        // delete from the pheap
    pheap<_Arch>::pheap_convert(__p.get_pheap())->free(__p);
}

__PSTL_END_NAMESPACE

#endif // __PSTL_PHEAP_H

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.17  2005/11/22 08:12:34  tuna
 * ptr, iterator に対する operator->() を解禁しました。
 * （一部、返り値の型が reference から pointer (または _Obj *) に変更）
 *
 * Revision 1.14.8.1  2005/11/21 12:55:54  tuna
 * tag名をnew-db-5に更新しました。
 *
 * Revision 1.14.2.15.2.2.2.1.2.2  2005/09/22 11:08:53  tuna
 * hash_mapのerase,pnewと、listを修正しました。
 * map, multimapを追加しました。
 * テストプログラムpstl_test_hash_map.cc, pstl_test_map.cc, pstl_test_multimap.ccを追加しました。以前のテストプログラムはpstl_bench.ccに移しました。
 *
 * Revision 1.14.2.15.2.2.2.1.2.1  2005/08/31 15:08:31  ninomi
 * turbolinuxでコンパイルできるようにしました。pstl_replace_pheapの中で
 * long longのところを無理矢理読み込ませるようにしました。pstl_pheapの中
 * で、ref<_, arch<pmem,_> >のoperator=(ref<..>)がコメントアウトされてい
 * るのを元にもどしました。
 *
 * Revision 1.14.2.15.2.2.2.1  2005/08/26 05:00:03  tuna
 * 新しい形式のreplace_pheapを使うバージョンにしました。
 *
 * Revision 1.14.2.15.2.2  2005/08/26 02:42:25  ninomi
 * LiLFeSからのDBへのアクセスにtry and catchをつけました。
 *
 * Revision 1.14.2.15.2.1  2005/08/25 09:20:16  ninomi
 * closeしたpheapにアクセスするとexception(closed_pheap_access)がでるようにしました。
 *
 * Revision 1.14.2.15  2005/08/18 09:59:19  ninomi
 * PSTL_without_phで動くようになりました。あと、loadを実装したら完成
 *
 * Revision 1.14.2.14  2005/08/18 07:52:36  ninomi
 * コンパイルがとおるようになりました。
 *
 * Revision 1.14.2.13  2005/08/18 07:40:07  tuna
 * pstl_alloc.h をシンプルにしました。mason上でも動作を確認しました。
 *
 * Revision 1.14.2.12  2005/08/16 04:18:31  ninomi
 * PSTL_without_phに対応しました。でも、旧ldbmとのインターフェース部分が
 * できていないのでコンパイルできません。
 *
 * Revision 1.14.2.11  2005/08/15 10:49:49  ninomi
 * PSTL_without_phでコンパイルできるよう奮闘中
 *
 * Revision 1.14.2.10  2005/08/15 02:35:39  ninomi
 * ldbmをPSTL_without_phに対応させました。コンパイルはこれから。
 *
 * Revision 1.14.2.9  2005/08/12 09:21:26  ninomi
 * 一度referenceにしてからreplace_pheapにオブジェクトを渡していたのをダイ
 * レクトに渡すように変更しました。
 *
 * Revision 1.14.2.8  2005/08/12 08:57:33  ninomi
 * replace_pheapをtemplate化して、あとからinstanciationできるようにしました。
 *
 * Revision 1.14.2.7  2005/08/12 05:53:40  ninomi
 * default_bitクラスを_BitTraitsクラスに変更しました
 *
 * Revision 1.14.2.6  2005/08/09 09:31:48  tuna
 * *** empty log message ***
 *
 * Revision 1.14.2.4  2005/07/11 13:53:56  tuna
 * munmapの前にmsyncを追加しました。一部にptrの参照渡しを使いました。その他コードを整理しました。
 *
 * Revision 1.14.2.3  2005/07/08 13:03:48  tuna
 * デバッグ用の出力をコメントアウトしました。
 *
 * Revision 1.14.2.2  2005/07/08 07:43:21  tuna
 * 開発中...
 *
 * Revision 1.14.2.1  2005/07/08 06:53:01  tuna
 * pheap<arch> *ph を使わないバージョンをbranchさせました。
 * 現在、ganesh14/15上では動くようですが、ganesh12上ではまだうまく動きません。
 *
 * Revision 1.14  2005/05/30 02:40:19  tuna
 * const に関する整理、castを新形式に統一、その他細かい修正をしました
 *
 * Revision 1.13  2005/05/23 02:40:38  tuna
 * correct bugs of operator-()
 *
 * Revision 1.12  2005/05/23 01:18:55  ninomi
 * push_backまで実装。バグがでている状態。
 *
 * Revision 1.11  2005/05/20 06:11:35  tuna
 * const に関係する追加 (const_ptr, const_ref) とその他の修正
 *
 * Revision 1.10  2005/04/22 06:54:34  tuna
 * list, pair, iterator_base are added.
 * alloc, hash_map, pheap, test and vector are a bit modified.
 *
 * Revision 1.9  2005/04/08 08:39:50  ninomi
 * *** empty log message ***
 *
 * Revision 1.8  2005/04/04 04:25:01  ninomi
 * a light check for hash key is introduced.  ldbm-type is still under construction.
 *
 * Revision 1.7  2005/03/25 05:59:25  ninomi
 * hash functionの第一引数にpheapをいれるようにしました。
 *
 * Revision 1.6  2005/03/10 09:08:54  ninomi
 * b32, b64のsize_typeの最大値をmax_size_typeにいれるようにしました。
 *
 * Revision 1.5  2004/12/14 06:37:32  ninomi
 * devが決まった場合のdefaultのbitをdefault_bit classに設定しました。
 * pstl_testの時間計測をuser time, system time, elapsed timeの3種で測定できるようにしました。
 *
 * Revision 1.4  2004/12/10 07:04:09  ninomi
 * template variableの_Bitと_Devの順番を逆にしました
 *
 * Revision 1.3  2004/12/10 05:56:05  ninomi
 * amd64でも動くようにしました。
 *
 * Revision 1.2  2004/12/09 07:40:39  ninomi
 * pheapのページングアルゴリズムを改良しました。
 * hash_mapとhash_multimapに専用のreferenceを加えました。
 *
 * Revision 1.1  2004/11/23 16:02:57  ninomi
 * ファイルを加えたり、分解したりしまいた。pstl_allocator->pstl_pheap+pstl_alloc.h
 *
 */
