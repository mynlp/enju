#define __PSTL_COMPATIBLE_64
#define __PSTL_ALLOC_BIT_CLASS pstl::b64

#include "pstl_pheap.h"
#include "pstl_utility"
#include "pstl_slist"

#include <iostream>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <ext/slist>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

const int MAX_RAND_INT = 10000;

namespace pstl {
    template <class _Arch> class global_variable {
    private:
	typedef pstl::slist<int, _Arch> slist_type;
	typedef pheap<_Arch> pheap_type;
    public:
	typedef _Arch arch_type;
	ptr<slist_type, arch_type> sp;
    };

    template <class _Arch>
    struct replace_pheap<global_variable<_Arch> > {
	void operator()(const pheap_ptr __ph, global_variable<_Arch> &__gv) {
	    __replace_pheap(__ph, __gv.sp);
	}
    };

    template <class _Arch> class b {
    private:
	typedef pheap<_Arch> pheap_type;
	typedef pstl::slist<int, _Arch> slist_type;
	typedef global_variable<_Arch> global_variable_type;

    public:
	typedef _Arch arch_type;
	
    private:
	pheap_type phv;
	pheap_ptr ph;
	ptr<slist_type, arch_type> sp;
    public:
	typename slist_type::reference s;

    public:
	b() : phv(16), ph(pheap_type::pheap_convert(&phv)) {}
	~b() {}
	void open(char *filename) {
	    if (!pheap_type::pheap_convert(ph)->open(filename)) {
		std::cerr << "fail to open" << std::endl;
		return;
	    }

	    ptr<global_variable_type, _Arch> gvp = ptr<global_variable_type, _Arch>(pheap_type::pheap_convert(ph)->getRoot(), ph);
	    if (!gvp) {
		gvp = pheap_type::pheap_convert(ph)->malloc(sizeof(global_variable_type));
		(*gvp)->sp = slist_type::pnew(ph);
		pheap_type::pheap_convert(ph)->setRoot(ptr<void, arch_type>(gvp, ph));
	    }
	    s.reassign(*(*gvp)->sp);
	}

	void init_slist(const __gnu_cxx::slist<int> &stds, const int sz = 0);
	void func_apply_slist(const int trial_func, const int seed, const int seed2, const int seed3);

	pheap_ptr get_pheap() const { return ph; }

	template <class _Obj> friend struct replace_pheap;
    };

    template <class _Arch>
    struct replace_pheap<b<_Arch> > {
	void operator()(const pheap_ptr __ph, b<_Arch> &__obj) {
	    __obj.phv = *pheap<_Arch>::pheap_convert(__ph);
	    __obj.ph = pheap<_Arch>::pheap_convert(&__obj.phv);
	    __replace_pheap(__ph, __obj.sp);
	}
    };

    template <class _Arch>
    void b<_Arch>::init_slist(const __gnu_cxx::slist<int> &stds, const int sz) {
	s->clear();
	if (!stds.empty()) {
	    for (typename __gnu_cxx::slist<int>::const_iterator it = stds.begin();
		 it != stds.end(); ++it) {
		
		s->push_front(*it);
	    }
	    s->reverse();
	}
	else {
	    for (int i = 0; i < sz; i++) {
		s->push_front(std::rand() % MAX_RAND_INT);
	    }
	}
    }

    template <class _Arch>
    void b<_Arch>::func_apply_slist(const int trial_func, const int seed, const int seed2, const int seed3) {
	typename pstl::slist<int, _Arch>::iterator it, it1, it2;
	int pos, tmp;
	std::cout << "pstl::slist[" << _Arch::dev_type::dev_type << "," << _Arch::bit_type::bit_size << "]::";

	switch (trial_func) {
	case 0: // empty()
	    std::cout << "empty() = ";
	    if (s->empty()) {
		std::cout << "true" << std::endl;
	    }
	    else {
		std::cout << "false" << std::endl;
	    }
	    break;
	case 1: // size()
	    std::cout << "size() = " << s->size() << std::endl;
	    break;
	case 2: // max_size()
	    std::cout << "max_size() = " << s->max_size() << std::endl;
	    break;
	case 3: // front()
	    std::cout << "front() = " << s->front().get_obj() << std::endl;
	    break;
	case 4:
	    break;
	case 5: // swap(slist &)
	    std::cout << "swap(slist &) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
	    break;
	case 6: // insert(iterator, const value_type &)
	    if (s->empty()) {
		std::cout << "insert(iterator, const value_type &) NOT EXECUTED (empty slist)" << std::endl;
		break;
	    }
	    pos = seed % s->size();
	    it = s->begin();
	    for (int i = 0; i < pos; i++, ++it);
	    std::cout << "insert(begin()+" << pos << ", " << seed2;
	    std::cout << ")" << std::endl;
	    s->insert(it, seed2);
	    break;
	case 7: // insert(_InputIterator, _InputIterator)
	    if (s->empty()) {
		std::cout << "insert(begin(), end()) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
		break;
	    }
	    break;
	case 8: // insert(iterator, size_type, const value_type &)
	    if (s->empty()) {
		std::cout << "insert(iterator, const value_type &) NOT EXECUTED (empty slist)" << std::endl;
		break;
	    }
	    pos = seed % s->size();
	    it = s->begin();
	    tmp = seed3 % 3 + 1;
	    for (int i = 0; i < pos; i++, ++it);
	    std::cout << "insert(begin()+" << pos << ", " << tmp << ", " << seed2;
	    std::cout << ")" << std::endl;
	    s->insert(it, tmp, seed2);
	    break;
	case 9: // push_front(const value_type &)
	    std::cout << "push_front(" << seed << ")" << std::endl;
	    s->push_front(seed);
	    break;
	case 10:
	    break;	    
	case 11: // erase(iterator, iterator);
	    if (s->empty()) {
		std::cout << "erase(iterator, iterator) NOT EXECUTED (empty slist)" << std::endl;
		break;
	    }
	    it1 = s->begin();
	    it2 = s->begin();
	    if (it2 != s->end()) {
		++it2;
	    }
	    if (it2 != s->end()) {
		++it2;
	    }
	    std::cout << "erase(begin(),begin()+2)" << std::endl;
	    s->erase(it1, it2);
	    break;
	case 12: // clear()
	    // std::cout << "clear()" << std::endl;
	    // s->clear();
	    std::cout << "clear() NOT EXECUTED (for test)" << std::endl;
	    break;
	case 13: // resize(size_type)
	    std::cout << "resize(size()+1)" << std::endl;
	    s->resize(s->size() + 1);
	    break;
	case 14: // pop_front()
	    std::cout << "pop_front()" << std::endl;
	    s->pop_front();
	    break;
	case 15: // pop_back()
	    std::cout << "pop_back()" << std::endl;
	    break;
	    // assign() unimplemented
	    // splice() unimplemented
	case 16: // remove(const value_type &)
	    std::cout << "remove(" << seed << ")" << std::endl;
	    s->remove(seed);
	    break;
	case 17: // unique()
	    std::cout << "unique()" << std::endl;
	    s->unique();
	    break;
	    // merge() unimplemented
	case 18: // reverse()
	    std::cout << "reverse()" << std::endl;
	    s->reverse();
	    break;
	case 19: // sort()
	    std::cout << "sort()" << std::endl;
	    s->sort();
	    break;

	default:
	    break;
	}
    }

    class c {
    private:
	typedef __gnu_cxx::slist<int> slist_type;
    public:
	slist_type s;

	template <class _Arch>
	void init_slist(pstl::slist<int, _Arch> &pstls, const int sz = 0);
	void func_apply_slist(const int trial_func, const int seed, const int seed2, const int seed3);
    };

    template <class _Arch>
    void c::init_slist(pstl::slist<int, _Arch> &pstls, const int sz) {
	s.clear();
	if (!pstls.empty()) {
	    for (typename pstl::slist<int, _Arch>::const_iterator it =
		     pstls.begin(); it != pstls.end(); ++it) {
		s.push_front(*it);
	    }
	    s.reverse();
	}
	else {
	    for (int i = 0; i < sz; i++) {
		s.push_front(std::rand() % MAX_RAND_INT);
	    }
	}
    }

    void c::func_apply_slist(const int trial_func, const int seed, const int seed2, const int seed3) {
	__gnu_cxx::slist<int>::iterator it, it1, it2;
	int pos, tmp;
	std::cout << "__gnu_cxx::slist[gnu stl]::";

	switch (trial_func) {
	case 0: // empty()
	    std::cout << "empty() = ";
	    if (s.empty()) {
		std::cout << "true" << std::endl;
	    }
	    else {
		std::cout << "false" << std::endl;
	    }
	    break;
	case 1: // size()
	    std::cout << "size() = " << s.size() << std::endl;
	    break;
	case 2: // max_size()
	    std::cout << "max_size() = " << s.max_size() << std::endl;
	    break;
	case 3: // front()
	    std::cout << "front() = " << s.front() << std::endl;
	    break;
	case 4:
	    break;
	case 5: // swap(slist &)
	    std::cout << "swap(slist &) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
	    break;
	case 6: // insert(iterator, const value_type &)
	    if (s.empty()) {
		std::cout << "insert(iterator, const value_type &) NOT EXECUTED (empty slist)" << std::endl;
		break;
	    }
	    pos = seed % s.size();
	    it = s.begin();
	    for (int i = 0; i < pos; i++, ++it);
	    std::cout << "insert(begin()+" << pos << ", " << seed2;
	    std::cout << ")" << std::endl;
	    s.insert(it, seed2);
	    break;
	case 7: // insert(_InputIterator, _InputIterator)
	    if (s.empty()) {
		std::cout << "insert(begin(), end()) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
		break;
	    }
	    break;
	case 8: // insert(iterator, size_type, const value_type &)
	    if (s.empty()) {
		std::cout << "insert(iterator, const value_type &) NOT EXECUTED (empty slist)" << std::endl;
		break;
	    }
	    pos = seed % s.size();
	    it = s.begin();
	    tmp = seed3 % 3 + 1;
	    for (int i = 0; i < pos; i++, ++it);
	    std::cout << "insert(begin()+" << pos << ", " << tmp << ", " << seed2;
	    std::cout << ")" << std::endl;
	    s.insert(it, tmp, seed2);
	    break;
	case 9: // push_front(const value_type &)
	    std::cout << "push_front(" << seed << ")" << std::endl;
	    s.push_front(seed);
	    break;
	case 10:
	    break;	    
	case 11: // erase(iterator, iterator);
	    if (s.empty()) {
		std::cout << "erase(iterator, iterator) NOT EXECUTED (empty slist)" << std::endl;
		break;
	    }
	    it1 = s.begin();
	    it2 = s.begin();
	    if (it2 != s.end()) {
		++it2;
	    }
	    if (it2 != s.end()) {
		++it2;
	    }
	    std::cout << "erase(begin(),begin()+2)" << std::endl;
	    s.erase(it1, it2);
	    break;
	case 12: // clear()
	    // std::cout << "clear()" << std::endl;
	    // s.clear();
	    std::cout << "clear() NOT EXECUTED (for test)" << std::endl;
	    break;
	case 13: // resize(size_type)
	    std::cout << "resize(size()+1)" << std::endl;
	    s.resize(s.size() + 1);
	    break;
	case 14: // pop_front()
	    std::cout << "pop_front()" << std::endl;
	    s.pop_front();
	    break;
	case 15: // pop_back()
	    std::cout << "pop_back()" << std::endl;
	    break;
	    // assign() unimplemented
	    // splice() unimplemented
	case 16: // remove(const value_type &)
	    std::cout << "remove(" << seed << ")" << std::endl;
	    s.remove(seed);
	    break;
	case 17: // unique()
	    std::cout << "unique()" << std::endl;
	    s.unique();
	    break;
	    // merge() unimplemented
	case 18: // reverse()
	    std::cout << "reverse()" << std::endl;
	    s.reverse();
	    break;
	case 19: // sort()
	    std::cout << "sort()" << std::endl;
	    s.sort();
	    break;

	default:
	    break;
	}
    }

    template <class _Arch>
    void validate_slist(const b<_Arch> &b_obj, const c &c_obj) {
	__gnu_cxx::slist<int>::const_iterator std_it = c_obj.s.begin(), std_diff_it;
	typename pstl::slist<int, _Arch>::const_iterator pstl_it = b_obj.s->begin(), pstl_diff_it;
	bool matched = true;
	for ( ; std_it != c_obj.s.end(); ++std_it, ++pstl_it) {
	    if (pstl_it == b_obj.s->end()) {
		matched = false;
		break;
	    }
	    if (*std_it != (*pstl_it).get_obj()) {
		std_diff_it = std_it;
		pstl_diff_it = pstl_it;
		matched = false;
		break;
	    }
	}
	std::cout << "std result:" << std::endl;
	for (std_it = c_obj.s.begin(); std_it != c_obj.s.end(); ++std_it) {
	    if (std_it != c_obj.s.begin()) {
		std::cout << ", ";
	    }
	    std::cout << *std_it;
	}
	std::cout << std::endl;
	if (!matched) {
	    std::cout << "pstl result:" << std::endl;
	    for (pstl_it = b_obj.s->begin(); pstl_it != b_obj.s->end();
		 ++pstl_it) {
		if (pstl_it != b_obj.s->begin()) {
		    std::cout << ", ";
		}
		std::cout << *pstl_it << std::endl;
	    }
	    std::cout << std::endl;
	    std::cout << "DIFFERENT" << std::endl;
	    std::cout << "first difference point:" << std::endl;
	    std::cout << "std: ";
	    std::cout << *std_diff_it << std::endl;
	    std::cout << "pstl: ";
	    std::cout << *pstl_diff_it << std::endl;
	}
	else {
	    std::cout << "AGREEMENT" << std::endl;
	}
    }
}

int main() {
    const int NUM_TRIAL = 100;
    const int NUM_OF_FUNCS = 20;
    
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    pstl::c k;
    pstl::b<pstl::arch<pstl::pmem> > i;
    i.open("");
    pstl::b<pstl::arch<pstl::vmem, pstl::b64> > j;
    j.open("hoge.ldb");
    
    k.init_slist(*j.s, 100);
    i.init_slist(k.s, 100);
    j.init_slist(k.s, 100);

    pstl::validate_slist(i, k);
    pstl::validate_slist(j, k);
    
    for (int s = 0; s < NUM_TRIAL; s++) {
	int func_id = std::rand() % NUM_OF_FUNCS;
	int seed = std::rand() % MAX_RAND_INT,
	    seed2 = std::rand() % MAX_RAND_INT,
	    seed3 = std::rand() % MAX_RAND_INT;
	k.func_apply_slist(func_id, seed, seed2, seed3);
	i.func_apply_slist(func_id, seed, seed2, seed3);
	j.func_apply_slist(func_id, seed, seed2, seed3);
    }
    
    pstl::validate_slist(i, k);
    pstl::validate_slist(j, k);

    return 0;
}
