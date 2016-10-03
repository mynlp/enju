#define __PSTL_COMPATIBLE_64
#define __PSTL_ALLOC_BIT_CLASS pstl::b64

// Forward Declaration for replace_pheap
#include "pstl_pheap.h"
#include "pstl_vector"
#include "pstl_utility"
#include "pstl_hash_map"
#include "pstl_list"

#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>

#include "sys/time.h"
#include "sys/resource.h"
#include "unistd.h"

/*
 * 131072 * 4096 = 536,870,912 byte
 *  65536 * 4096 = 268,435,456 byte
 *  32768 * 4096 = 134,217,728 byte
 *  16384 * 4096 =  67,108,864 byte
 *   8192 * 4096 =  33,554,432 byte
 *   4096 * 4096 =  16,777,216 byte
 *   2048 * 4096 =   8,388,608 byte
 *   1024 * 4096 =   4,194,304 byte
 *    512 * 4096 =   2,097,152 byte
 */


namespace pstl {
    void get_time(double &ut, double &st, double &et) {
        struct rusage ru;
        struct timeval tv;
        getrusage(RUSAGE_SELF, &ru);
        gettimeofday(&tv, 0);
        ut = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 0.000001;
        st = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 0.000001;
        et = tv.tv_sec          + tv.tv_usec          * 0.000001;
    }

    template <class _Arch> class global_variable {
    private:
	typedef pstl::allocator<int, _Arch> int_allocator_type;
	typedef pstl::list<int, _Arch> list_type;
	typedef pstl::hash_map<int, int, _Arch, pstl::hash<int>, pstl::equal_to<int> > hash_map_type;
	typedef pstl::vector<int, _Arch> vector_type;
	typedef pstl::pair<int, int, _Arch> pair_type;
	typedef pstl::pheap<_Arch> pheap_type;
    public:
	ptr<list_type, _Arch> lstp;
	ptr<hash_map_type, _Arch> hsp;
	ptr<vector_type, _Arch> vecp;
	ptr<pair_type, _Arch> pairp;
    };

    template <class _Arch>
    struct replace_pheap<global_variable<_Arch> > {
	void operator()(pheap_ptr __ph, global_variable<_Arch> &__gv) {
	    __replace_pheap(__ph, __gv.lstp);
	    __replace_pheap(__ph, __gv.hsp);
	    __replace_pheap(__ph, __gv.vecp);
	    __replace_pheap(__ph, __gv.pairp);
	}
    };

    template <class _Arch> class b {
    private:
        typedef pstl::pheap<_Arch> pheap_type;
	typedef pstl::allocator<int, _Arch> int_allocator_type;
	typedef pstl::list<int, _Arch> list_type;
	typedef pstl::hash_map<int, int, _Arch, pstl::hash<int>, pstl::equal_to<int> > hash_map_type;
        typedef pstl::vector<int, _Arch> vector_type;
	typedef pstl::pair<int, int, _Arch> pair_type;
	typedef pstl::global_variable<_Arch> global_variable_type;

        pheap_type phv;
        pheap_ptr ph;
	ptr<list_type, _Arch> lstp;
	ptr<hash_map_type, _Arch> hsp;
	ptr<vector_type, _Arch> vecp;
	ptr<pair_type, _Arch> pairp;
	typename list_type::reference lst;
	typename hash_map_type::reference hs;
	typename vector_type::reference vec;
	typename pair_type::reference pa;
	typename int_allocator_type::reference alloc;
    public:
	bool print_val;

    public:
        b() : phv(16), ph(pheap_type::pheap_convert(&phv)), print_val(false) {}
        ~b() {}
        void open(char *filename) {

            if (!pheap_type::pheap_convert(ph)->open(filename)) {
		std::cerr << "fail to open" << std::endl;
		return;
	    }

	    ptr<global_variable_type, _Arch> gvp = ptr<global_variable_type, _Arch>(pheap_type::pheap_convert(ph)->getRoot(), ph); 
	    if (!gvp) {
		gvp = pheap_type::pheap_convert(ph)->malloc(sizeof(global_variable_type));
		(*gvp)->lstp = list_type::pnew(ph, typename list_type::allocator_type(ph));
		(*gvp)->hsp = hash_map_type::pnew(ph, typename hash_map_type::allocator_type(ph));
		(*gvp)->vecp = vector_type::pnew(ph, typename vector_type::allocator_type(ph));
		(*gvp)->pairp = pair_type::pnew(ph);
		pheap_type::pheap_convert(ph)->setRoot(ptr<void, _Arch>(gvp, ph));
	    }
	    lst.reassign(*(*gvp)->lstp);
	    hs.reassign(*(*gvp)->hsp);
	    vec.reassign(*(*gvp)->vecp);
	    pa.reassign(*(*gvp)->pairp);
        }

	void test_list_insert(int sz);
	void test_list_iterate();
	void test_list_erase();
        void test_hash_map_insert(int sz);
        void test_hash_map_lookup(int sz);
        void test_hash_map_iterate();
        void test_hash_map_erase(int sz);
        void test_vector_push_back(int sz);
        void test_vector_lookup(int sz);
        void test_vector_iterate();
        void test_vector_pop_back(int sz);
	void test_pair_set();
	void test_pair_get();

	pheap_ptr get_pheap() const { return ph; }

	template <class _Obj> friend struct replace_pheap;
    };

    template <class _Arch>
    struct replace_pheap<b<_Arch> > {
	void operator()(pheap_ptr __ph, b<_Arch> &__obj) {
	    __obj.phv = *pheap<_Arch>::pheap_convert(__ph);
	    __obj.ph = pheap<_Arch>::pheap_convert(&__obj.phv);
	    __replace_pheap(__ph, __obj.lstp);
	    __replace_pheap(__ph, __obj.hsp);
	    __replace_pheap(__ph, __obj.vecp);
	    __replace_pheap(__ph, __obj.pairp);
	    __replace_pheap(__ph, __obj.alloc);
	}
    };
    
    template <class _Arch>
    void b<_Arch>::test_list_insert(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for (int i = 0 ; i < sz ; i++) {
	    if (print_val) {
		std::cerr << "list_insert " << i << " / " << sz << "      " << std::endl;
		std::cerr << "\033[A";
	    }
            lst->insert(lst->end(), i);
	}
	if (print_val) {
	    std::cerr << std::endl;
	}
        get_time(aut, ast, aet);
        std::cout << "list_insert [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_list_iterate() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        typename list_type::iterator it = lst->begin(), last = lst->end();
        for(; it != last ; ++it) {
	    if (print_val) {
		std::cout << "list_iterate = " << *it << std::endl;
	    }
	    else {
		*it;
	    }
        }
        get_time(aut, ast, aet);
        std::cout << "list_iterate [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_list_erase() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
	typename list_type::iterator it = lst->begin(), last = lst->end();
        for( ; it != last; ) {
            it = lst->erase(it);
        }
        get_time(aut, ast, aet);
        std::cout << "list_erase [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_hash_map_insert(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for (int i = 0 ; i < sz ; i++) {
	    if (print_val) {
		std::cerr << "hash_map_insert " << i << " / " << sz << "      " << std::endl;
		std::cerr << "\033[A";
	    }
            hs->insert(typename hash_map_type::value_type(
			   typename hash_map_type::key_type(i),
			   typename hash_map_type::mapped_type(i+10)));
            //hs->operator[](i) = i+10+1;
        }
	if (print_val) {
	    std::cerr << std::endl;
	}
        get_time(aut, ast, aet);
        std::cout << "hash_map_insert [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_hash_map_lookup(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
	    typename hash_map_type::iterator it = hs->find(i);
	    if (print_val) {
		if (it != hs->end()) {
		    std::cout << "hash_map_lookup[" << i << "] = " << (*it)->second << std::endl;
		}
		else {
		    std::cout << "hash_map_lookup[" << i << "] = undef" << std::endl;
		}
	    }
	    else {
		if (it != hs->end()) {
		    (*it)->second;
		}
		else {
		    std::cout << "not found in hash_map_lookup" << std::endl;
		}
	    }
        }
        get_time(aut, ast, aet);
        std::cout << "hash_map_lookup [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_hash_map_iterate() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        typename hash_map_type::iterator it = hs->begin(), last = hs->end();
        for(; it != last ; ++it) {
	    if (print_val) {
		std::cout << "hash_map_iterate = " << (*it)->second << std::endl;
	    }
	    else {
		(*it)->second;
	    }
        }
        get_time(aut, ast, aet);
        std::cout << "hash_map_iterate [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_hash_map_erase(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
	    if (print_val) {
		std::cerr << "hash_map_erase" << i << " / " << sz << "      " << std::endl;
		std::cerr << "\033[A";
	    }
            hs->erase(i);
        }
	if (print_val) {
	    std::cerr << std::endl;
	}
        get_time(aut, ast, aet);
        std::cout << "hash_map_erase [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_vector_push_back(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for (int i = 0 ; i < sz ; i++) {
	    if (print_val) {
		std::cerr << "vectop_push_back " << i << " / " << sz << "      " << std::endl;
		std::cerr << "\033[A";
	    }
            vec->push_back(i);
        }
	if (print_val) {
	    std::cerr << std::endl;
	}
        get_time(aut, ast, aet);
        std::cout << "vector_push_back [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_vector_lookup(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for (int i = 0 ; i < sz ; i++) {
	    if (print_val) {
		std::cout << "vector_lookup[" << i << "] = " << vec->operator[](i) << std::endl;
	    }
	    else {
		vec->operator[](i);
	    }
        }
        get_time(aut, ast, aet);
        std::cout << "vector_lookup [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_vector_iterate() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        typename vector_type::iterator it = vec->begin(), last = vec->end();
        for (; it != last ; ++it) {
	    if (print_val) {
		std::cout << "vector_iterate = " << *it << std::endl;
	    }
	    else {
		*it;
	    }
        }
        get_time(aut, ast, aet);
        std::cout << "vector_iterate [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    template <class _Arch>
    void b<_Arch>::test_vector_pop_back(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for (int i = 0 ; i < sz ; ++i) {
	    if (print_val) {
		std::cerr << "vector_pop_back " << i << " / " << sz << "      " << std::endl;
		std::cerr << "\033[A";
	    }
            vec->pop_back();
        }
	if (print_val) {
	    std::cerr << std::endl;
	}
        get_time(aut, ast, aet);
        std::cout << "vector_pop_back [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }

    template <class _Arch>
    void b<_Arch>::test_pair_set() {
	pa->first = 10;
	pa->second = 20;
    }
    template <class _Arch>
    void b<_Arch>::test_pair_get() {
	std::cout << "pair test [" << _Arch::bit_type::bit_size << ", " << _Arch::dev_type::dev_type << "], pa = (" << pa->first << ", " << pa->second << ")" << std::endl;
    }
    

    class c {
    private:
	typedef std::list<int> list_type;
        typedef std::vector<int> vector_type;
	typedef __gnu_cxx::hash_map<int, int> hash_map_type;
	typedef std::pair<int, int> pair_type;
    public:
	list_type lst;
	hash_map_type hs;
        vector_type vec;
	pair_type pa;
	void test_list_insert(int sz);
	void test_list_iterate();
	void test_list_erase();
        void test_hash_map_insert(int sz);
        void test_hash_map_lookup(int sz);
        void test_hash_map_iterate();
        void test_hash_map_erase(int sz);
        void test_vector_push_back(int sz);
        void test_vector_lookup(int sz);
        void test_vector_iterate();
        void test_vector_pop_back(int sz);
	void test_pair_set();
	void test_pair_get();

    };

    void c::test_list_insert(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
            lst.insert(lst.end(), i);
        }
        get_time(aut, ast, aet);
        std::cout << "list_insert [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_list_iterate() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        list_type::iterator it = lst.begin(), last = lst.end();
        for(; it != last ; ++it) {
            *it;
        }
        get_time(aut, ast, aet);
        std::cout << "list_iterate [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_list_erase() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
	list_type::iterator it = lst.begin(), last = lst.end();
        for( ; it != last ; ) {
            it = lst.erase(it);
        }
        get_time(aut, ast, aet);
        std::cout << "list_erase [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_hash_map_insert(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
            hs.insert(hash_map_type::value_type(hash_map_type::key_type(i),
                                                hash_map_type::mapped_type(i+10)));
        }
        get_time(aut, ast, aet);
        std::cout << "hash_map_insert [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_hash_map_lookup(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
            hs.find(i);
        }
        get_time(aut, ast, aet);
        std::cout << "hash_map_lookup [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_hash_map_iterate() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        hash_map_type::iterator it = hs.begin(), last = hs.end();
        for(; it != last ; ++it) {
            it->second;
        }
        get_time(aut, ast, aet);
        std::cout << "hash_map_iterate [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_hash_map_erase(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
            hs.erase(i);
        }
        get_time(aut, ast, aet);
        std::cout << "hash_map_erase [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_vector_push_back(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
            vec.push_back(i);
        }
        get_time(aut, ast, aet);
        std::cout << "vector_push_back [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_vector_lookup(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; i++) {
            vec[i];
        }
        get_time(aut, ast, aet);
        std::cout << "vector_lookup [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_vector_iterate() {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        vector_type::iterator it = vec.begin(), last = vec.end();
        for(; it != last ; ++it) {
            *it;
        }
        get_time(aut, ast, aet);
        std::cout << "vector_iterate [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }
    void c::test_vector_pop_back(int sz) {
        double but, bst, bet, aut, ast, aet;
        get_time(but, bst, bet);
        for(int i = 0 ; i < sz ; ++i) {
            vec.pop_back();
        }
        get_time(aut, ast, aet);
        std::cout << "vector_pop_back [gnu stl] user time: " << aut - but << " system time: " << ast - bst << " elapsed time: " << aet - bet << std::endl;
    }

    void c::test_pair_set() {
	pa.first = 10;
	pa.second = 20;
    }
    void c::test_pair_get() {
	std::cout << "pair test [gnu stl], pa = (" << pa.first << ", " << pa.second << ")" << std::endl;
    }
}

int main() {
    int sz = 10000;
    bool gnutest = true;
    bool pmemtest = true;
    bool vmemtest = true;

    bool inserting = true;
    bool erasing = true;

    bool list_test = true;
    bool hash_map_test = true;
    bool vector_test = true;
    bool pair_test = true;
    
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    pstl::c k;
    pstl::b<pstl::arch<pstl::pmem> > i;
    if (pmemtest)
	i.open("");
    pstl::b<pstl::arch<pstl::vmem, pstl::b64> > j;
    if (vmemtest)
	j.open("hoge.ldb");
    j.print_val = false;

    if (list_test) {
	if (gnutest)
	    k.test_list_insert(sz);
	if (pmemtest && inserting)
	    i.test_list_insert(sz);
	if (vmemtest && inserting)
	    j.test_list_insert(sz);

	if (gnutest)
	    k.test_list_iterate();
	if (pmemtest)
	    i.test_list_iterate();
	if (vmemtest)
	    j.test_list_iterate();

	if (gnutest)
	    k.test_list_erase();
	if (pmemtest && erasing)
	    i.test_list_erase();
	if (vmemtest && erasing)
	    j.test_list_erase();
    }
    
    if (hash_map_test) {
	if (gnutest)
	    k.test_hash_map_insert(sz);
	if (pmemtest && inserting)
	    i.test_hash_map_insert(sz);
	if (vmemtest && inserting)
	    j.test_hash_map_insert(sz);

	if (gnutest)
	    k.test_hash_map_lookup(sz);
	if (pmemtest)
	    i.test_hash_map_lookup(sz);
	if (vmemtest)
	    j.test_hash_map_lookup(sz);

	if (gnutest)
	    k.test_hash_map_iterate();
	if (pmemtest)
	    i.test_hash_map_iterate();
	if (vmemtest)
	    j.test_hash_map_iterate();

	if (gnutest)
	    k.test_hash_map_erase(sz);
	if (pmemtest && erasing)
	    i.test_hash_map_erase(sz);
	if (vmemtest && erasing)
	    j.test_hash_map_erase(sz);
    }

    if (vector_test) {
	if (gnutest)
	    k.test_vector_push_back(sz);
	if (pmemtest && inserting)
	    i.test_vector_push_back(sz);
	if (vmemtest && inserting)
	    j.test_vector_push_back(sz);
	
	if (gnutest)
	    k.test_vector_lookup(sz);
	if (pmemtest)
	    i.test_vector_lookup(sz);
	if (vmemtest)
	    j.test_vector_lookup(sz);
	
	if (gnutest)
	    k.test_vector_iterate();
	if (pmemtest)
	    i.test_vector_iterate();
	if (vmemtest)
	    j.test_vector_iterate();
	
	if (gnutest)
	    k.test_vector_pop_back(sz);
	if (pmemtest && erasing)
	    i.test_vector_pop_back(sz);
	if (vmemtest && erasing)
	    j.test_vector_pop_back(sz);
    }
    
    if (pair_test) {
	if (gnutest)
	    k.test_pair_set();
	if (pmemtest && inserting)
	    i.test_pair_set();
	if (vmemtest && inserting)
	    j.test_pair_set();
	
	if (gnutest)
	    k.test_pair_get();
	if (pmemtest && erasing)
	    i.test_pair_get();
	if (vmemtest && erasing)
	    j.test_pair_get();
    }

    if (pmemtest)
	reinterpret_cast<pstl::pheap<pstl::arch<pstl::pmem> > *>(i.get_pheap())->close();
    if (vmemtest)
	reinterpret_cast<pstl::pheap<pstl::arch<pstl::vmem> > *>(j.get_pheap())->close();
    
    return 0;
}
