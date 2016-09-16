#define __PSTL_COMPATIBLE_64
#define __PSTL_ALLOC_BIT_CLASS pstl::b64

#include "pstl_pheap.h"
#include "pstl_utility"
#include "pstl_set"

#include <iostream>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <set>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

const int MAX_RAND_INT = 10000;

namespace pstl {
    template <class _Arch> class global_variable {
    private:
	typedef pstl::set<int, _Arch> set_type;
	typedef pheap<_Arch> pheap_type;
    public:
	typedef _Arch arch_type;
	ptr<set_type, arch_type> sp;
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
	typedef pstl::set<int, _Arch> set_type;
	typedef global_variable<_Arch> global_variable_type;

    public:
	typedef _Arch arch_type;
	
    private:
	pheap_type phv;
	pheap_ptr ph;
	ptr<set_type, arch_type> sp;
    public:
	typename set_type::reference s;

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
		(*gvp)->sp = set_type::pnew(ph, typename set_type::key_compare(), typename set_type::allocator_type(ph));
		pheap_type::pheap_convert(ph)->setRoot(ptr<void, arch_type>(gvp, ph));
	    }
	    s.reassign(*(*gvp)->sp);
	}

	void init_set(const std::set<int> &stds, const int sz = 0);
	void func_apply_set(const int trial_func, const int seed, const int seed2, const int seed3);

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
    void b<_Arch>::init_set(const std::set<int> &stds, const int sz) {
	s->clear();
	if (!stds.empty()) {
	    for (typename std::set<int>::const_iterator it = stds.begin();
		 it != stds.end(); ++it) {
		
		bool check = s->insert(*it).second;
		if (!check) {
		    std::cerr << "copy insert failure" << std::endl;
		    exit(01);
		}
	    }
	}
	else {
	    for (int i = 0; i < sz; i++) {
		s->insert(std::rand() % MAX_RAND_INT);
	    }
	}
    }

    template <class _Arch>
    void b<_Arch>::func_apply_set(const int trial_func, const int seed, const int seed2, const int seed3) {
	const int NUM_OF_FUNCS = 17;
	pstl::pair<typename pstl::set<int, _Arch>::iterator, bool, _Arch> retins;
	typename pstl::set<int, _Arch>::iterator it, it1, it2;
	int pos;
	pstl::pair<typename pstl::set<int, _Arch>::iterator,
	    typename pstl::set<int, _Arch>::iterator,
	    _Arch> pa;
	std::cout << "pstl::set[" << _Arch::dev_type::dev_type << "," << _Arch::bit_type::bit_size << "]::";

	switch (trial_func % NUM_OF_FUNCS) {
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
	case 3:
	    break;
	case 4: // swap(set &)
	    std::cout << "swap(set &) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
	    break;
	case 5: // insert(const value_type &)
	    retins = s->insert(seed);
	    std::cout << "insert(" << seed << ")->second = ";
	    std::cout << retins.second << std::endl;
	    break;
	case 6: // insert(iterator, const value_type &)
	    if (s->empty()) {
		std::cout << "insert(iterator, const value_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    pos = seed % s->size();
	    it = s->begin();
	    for (int i = 0; i < pos; i++, ++it);
	    std::cout << "insert(begin()+" << pos << ", (" << seed;
	    std::cout << ", " << seed2 << "))" << std::endl;
	    s->insert(it, seed2);
	    break;
	case 7: // insert(_InputIterator, _InputIterator)
	    if (s->empty()) {
		std::cout << "insert(begin(), end()) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
		break;
	    }
	    break;
	case 8: // erase(iterator)
	    if (s->empty()) {
		std::cout << "erase(iterator) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "erase(begin())" << std::endl;
	    s->erase(s->begin());
	    break;
	case 9: // erase(const key_type &)
	    if (s->empty()) {
		std::cout << "erase(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "erase(" << seed << ") = ";
	    std::cout << s->erase(seed) << std::endl;
	    break;
	case 10: // erase(iterator, iterator);
	    if (s->empty()) {
		std::cout << "erase(iterator, iterator) NOT EXECUTED (empty set)" << std::endl;
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
	case 11: // clear()
	    // std::cout << "clear()" << std::endl;
	    // s->clear();
	    std::cout << "clear() NOT EXECUTED (for test)" << std::endl;
	    break;
	case 12: // find(const key_type &)
	    if (s->empty()) {
		std::cout << "find(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    it = s->find(seed);
	    std::cout << "*find(" << seed << ") = ";
	    if (it == s->end()) {
		std::cout << "end() (NOT FOUND)" << std::endl;
	    }
	    else {
		std::cout << "an iterator (FOUND)" << std::endl;
	    }
	    break;
	case 13: // count(const key_type &)
	    if (s->empty()) {
		std::cout << "count(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "count(" << seed << ") = " << s->count(seed) << std::endl;
	    break;
	case 14: // lower_bound(const key_type &)
	    if (s->empty()) {
		std::cout << "lower_bound(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    it = s->lower_bound(seed);
	    std::cout << "*lower_bound(" << seed << ") = ";
	    if (it == s->end()) {
		std::cout << "end() (NOT FOUND)" << std::endl;
	    }
	    else {
		std::cout << "an iterator" << std::endl;
	    }
	    break;
	case 15: // upper_bound(const key_type &)
	    if (s->empty()) {
		std::cout << "upper_bound(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    it = s->upper_bound(seed);
	    std::cout << "*upper_bound(" << seed << ") = ";
	    if (it == s->end()) {
		std::cout << "end() (NOT FOUND)" << std::endl;
	    }
	    else {
		std::cout << "an iterator (FOUND)" << std::endl;
	    }
	    break;
	case 16: // equal_range(const key_type &)
	    if (s->empty()) {
		std::cout << "equal_range(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    pa = s->equal_range(seed);
	    it1 = pa.first;
	    it2 = pa.second;
	    std::cout << "equal_range(" << seed << ") = (";
	    if (it1 == s->end()) {
		std::cout << "end(), ";
	    }
	    else {
		std::cout << "an iterator, ";
	    }
	    if (it2 == s->end()) {
		std::cout << "end())";
	    }
	    else {
		std::cout << "an iterator)";
	    }
	    std::cout << std::endl;
	    break;
	default:
	    break;
	}
    }

    class c {
    private:
	typedef std::set<int> set_type;
    public:
	set_type s;

	template <class _Arch>
	void init_set(pstl::set<int, _Arch> &pstls, const int sz = 0);
	void func_apply_set(const int trial_func, const int seed, const int seed2, const int seed3);
    };

    template <class _Arch>
    void c::init_set(pstl::set<int, _Arch> &pstls, const int sz) {
	s.clear();
	if (!pstls.empty()) {
	    for (typename pstl::set<int, _Arch>::const_iterator it =
		     pstls.begin(); it != pstls.end(); ++it) {
		if (!s.insert(*it).second) {
		    std::cerr << "copy insert failure" << std::endl;
		    exit(01);
		}
	    }
	}
	else {
	    for (int i = 0; i < sz; i++) {
		s.insert(std::rand() % MAX_RAND_INT);
	    }
	}
    }

    void c::func_apply_set(const int trial_func, const int seed, const int seed2, const int seed3) {
	const int NUM_OF_FUNCS = 17;
	std::set<int>::iterator it, it1, it2;
	int pos;
	std::pair<std::set<int>::iterator, std::set<int>::iterator> pa;
	std::cout << "std::set::";
       
	switch (trial_func % NUM_OF_FUNCS) {
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
	case 3:
	    break;
	case 4: // swap(set &)
	    std::cout << "swap(set &) (TEST PROGRAM UNIMPLEMENTED)" << std::endl;
	    break;
	case 5: // insert(const value_type &)
	    std::cout << "insert(" << seed << ").second = ";
	    std::cout << s.insert(seed).second << std::endl;
	    break;
	case 6: // insert(iterator, const value_type &)
	    if (s.empty()) {
		std::cout << "insert(iterator, const value_type &) NOT EXECUTED (empty set)" << std::endl;
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
		std::cout << "insert(begin(), end()) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "insert(begin(), end())" << std::endl;
	    s.insert(s.begin(), s.end());
	    break;
	case 8: // erase(iterator)
	    if (s.empty()) {
		std::cout << "erase(iterator) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "erase(begin())" << std::endl;
	    s.erase(s.begin());
	    break;
	case 9: // erase(const key_type &)
	    if (s.empty()) {
		std::cout << "erase(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "erase(" << seed << ") = ";
	    std::cout << s.erase(seed) << std::endl;
	    break;
	case 10: // erase(iterator, iterator);
	    if (s.empty()) {
		std::cout << "erase(iterator, iterator) NOT EXECUTED (empty set)" << std::endl;
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
	case 11: // clear()
	    //std::cout << "clear()" << std::endl;
	    //s.clear();
	    std::cout << "clear() NOT EXECUTED (for test)" << std::endl;
	    break;
	case 12: // find(const key_type &)
	    if (s.empty()) {
		std::cout << "find(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    it = s.find(seed);
	    std::cout << "*find(" << seed << ") = ";
	    if (it == s.end()) {
		std::cout << "end() (NOT FOUND)" << std::endl;
	    }
	    else {
		std::cout << "an iterator (FOUND)" << std::endl;
	    }
	    break;
	case 13: // count(const key_type &)
	    if (s.empty()) {
		std::cout << "count(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    std::cout << "count(" << seed << ") = " << s.count(seed) << std::endl;
	    break;
	case 14: // lower_bound(const key_type &)
	    if (s.empty()) {
		std::cout << "lower_bound(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    it = s.lower_bound(seed);
	    std::cout << "*lower_bound(" << seed << ") = ";
	    if (it == s.end()) {
		std::cout << "end() (NOT FOUND)" << std::endl;
	    }
	    else {
		std::cout << "an iterator" << std::endl;
	    }
	    break;
	case 15: // upper_bound(const key_type &)
	    if (s.empty()) {
		std::cout << "upper_bound(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    it = s.upper_bound(seed);
	    std::cout << "*upper_bound(" << seed << ") = ";
	    if (it == s.end()) {
		std::cout << "end() (NOT FOUND)" << std::endl;
	    }
	    else {
		std::cout << "an iterator" << std::endl;
	    }
	    break;
	case 16: // equal_range(const key_type &)
	    if (s.empty()) {
		std::cout << "equal_range(const key_type &) NOT EXECUTED (empty set)" << std::endl;
		break;
	    }
	    pa = s.equal_range(seed);
	    it1 = pa.first;
	    it2 = pa.second;
	    std::cout << "equal_range(" << seed << ") = (";
	    if (it1 == s.end()) {
		std::cout << "end(), ";
	    }
	    else {
		std::cout << "an iterator, ";
	    }
	    if (it2 == s.end()) {
		std::cout << "end())";
	    }
	    else {
		std::cout << "an iterator)";
	    }
	    std::cout << std::endl;
	    break;
	default:
	    break;
	}
    }

    template <class _Arch>
    void validate_set(const b<_Arch> &b_obj, const c &c_obj) {
	std::set<int>::const_iterator std_it = c_obj.s.begin(), std_diff_it;
	typename pstl::set<int, _Arch>::const_iterator pstl_it = b_obj.s->begin(), pstl_diff_it;
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
    const int NUM_OF_FUNCS = 17;
    
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    pstl::c k;
    pstl::b<pstl::arch<pstl::pmem> > i;
    i.open("");
    pstl::b<pstl::arch<pstl::vmem, pstl::b64> > j;
    j.open("hoge.ldb");
    
    k.init_set(*j.s, 100);
    i.init_set(k.s, 100);
    j.init_set(k.s, 100);

    pstl::validate_set(i, k);
    pstl::validate_set(j, k);
    
    for (int s = 0; s < NUM_TRIAL; s++) {
	int func_id = std::rand() % NUM_OF_FUNCS;
	int seed = std::rand() % MAX_RAND_INT,
	    seed2 = std::rand() % MAX_RAND_INT,
	    seed3 = std::rand() % MAX_RAND_INT;
	k.func_apply_set(func_id, seed, seed2, seed3);
	i.func_apply_set(func_id, seed, seed2, seed3);
	j.func_apply_set(func_id, seed, seed2, seed3);
    }
    
    pstl::validate_set(i, k);
    pstl::validate_set(j, k);

    return 0;
}
