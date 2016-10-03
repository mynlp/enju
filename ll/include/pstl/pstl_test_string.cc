#define __PSTL_COMPATIBLE_64
#define __PSTL_ALLOC_BIT_CLASS pstl::b64

#include "pstl_pheap.h"
#include "pstl_string"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

namespace pstl {
    template <class _Arch> class global_variable {
    private:
	typedef pstl::string<_Arch> string_type;
	typedef pheap<_Arch> pheap_type;
    public:
	typedef _Arch arch_type;
	ptr<string_type, arch_type> strp;
    };

    template <class _Arch>
    struct replace_pheap<global_variable<_Arch> > {
	void operator()(const pheap_ptr __ph, global_variable<_Arch> &__gv) {
	    __replace_pheap(__ph, __gv.strp);
	}
    };

    template <class _Arch> class b {
    private:
	typedef pheap<_Arch> pheap_type;
	typedef pstl::string<_Arch> string_type;
	typedef global_variable<_Arch> global_variable_type;

    public:
	typedef _Arch arch_type;
	
    private:
	pheap_type phv;
	pheap_ptr ph;
	ptr<string_type, arch_type> strp;
    public:
	typename string_type::reference str;

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
		(*gvp)->strp = string_type::pnew(ph);
		pheap_type::pheap_convert(ph)->setRoot(ptr<void, arch_type>(gvp, ph));
	    }
	    str.reassign(*(*gvp)->strp);
	}

	void init_string(const std::string &stdstr, const int sz = 0);
	void func_apply_string(const int trial_func, const int seed, const int seed2, const int seed3);

	pheap_ptr get_pheap() const { return ph; }

	template <class _Obj> friend struct replace_pheap;
    };

    template <class _Arch>
    struct replace_pheap<b<_Arch> > {
	void operator()(const pheap_ptr __ph, b<_Arch> &__obj) {
	    __obj.phv = *pheap<_Arch>::pheap_convert(__ph);
	    __obj.ph = pheap<_Arch>::pheap_convert(&__obj.phv);
	    __replace_pheap(__ph, __obj.strp);
	}
    };

    template <class _Arch>
    void b<_Arch>::init_string(const std::string &stdstr, const int sz) {
	str->clear();
	if (!stdstr.empty()) {
	    str.get_obj() = stdstr;
	}
	else {
	    for (int i = 0; i < sz; i++) {
		str->operator+=(static_cast<char>('a' + std::rand() % 26));
	    }
	}
    }

    template <class _Arch>
    void b<_Arch>::func_apply_string(const int trial_func, const int seed, const int seed2, const int seed3) {
	const int NUM_OF_FUNCS = 17;
	string_type str2(ph);
	std::string randomstdstr = "" + static_cast<char>('a' + seed % 26) + static_cast<char>('a' + seed2 % 26) + static_cast<char>('a' + seed3 % 26);
	string_type randomstr(ph, randomstdstr);
	typename pstl::string<_Arch>::iterator it, it1, it2;
	int tmp;
	char c;
	ptr<char, _Arch> cp(ph);
	int i, pos;
	std::cout << "pstl::string[" << _Arch::dev_type::dev_type << "," << _Arch::bit_type::bit_size << "]::";

	switch (trial_func % NUM_OF_FUNCS) {
	case 0: // empty()
	    std::cout << "empty() = ";
	    if (str->empty()) {
		std::cout << "true" << std::endl;
	    }
	    else {
		std::cout << "false" << std::endl;
	    }
	    break;
	case 1: // length() (== size())
	    std::cout << "length() = " << str->length() << std::endl;
	    break;
	case 2: // max_size()
	    std::cout << "max_size() = " << str->max_size() << std::endl;
	    break;
	case 3: // resize(size_type, char)
	    tmp = seed % (str->length() * 2);
	    c = 'a' + seed2 % 26;
	    std::cout << "resize(" << tmp << ", " << c << ")" << std::endl;
	    str->resize(tmp, c);
	    break;
	case 4: // reserve(size_type)
	    tmp = str->length() * 2 + 1;
	    std::cout << "reserve(" << tmp << ")" << std::endl;
	    str->reserve(tmp);
	    break;
	case 5: // capacity()
	    std::cout << "capacity() = " << str->capacity() << std::endl;
	    break;
	case 6: // operator[](size_type)
	    if (str->empty()) {
		std::cout << "operator[](size_type) NOT EXECUTED (empty string)" << std::endl;
		break;
	    }
	    i = seed % str->length();
	    c = 'a' + seed2 % 26;
	    std::cout << "operator[](" << i << ") = ";
	    std::cout << str->operator[](i) << std::endl;
	    std::cout << "operator[](" << i << ") = " << c << std::endl;
	    str->operator[](i).get_obj() = c;
	    break;
	case 7: // operator+=(const string &)
	    std::cout << "operator+=((const string &) \"" << randomstr << "\")" << std::endl;
	    str->operator+=(randomstr);
	    break;
	case 8: // operator+=(const_ptr<char>)
	    std::cout << "operator+=((const_ptr<char>) \"" << randomstr << "\")" << std::endl;
	    str->operator+=(randomstr.c_str());
	    break;
	case 9: // operator+=(char)
	    c = 'a' + seed % 26;
	    std::cout << "operator+=('" << c << "')" << std::endl;
	    str->operator+=(c);
	    break;
	case 10: // append(const string &, size_type, size_type)
	    pos = seed % str->length();
	    tmp = seed2 % (str->length() - pos);
	    std::cout << "append(current_str, " << pos << ", " << tmp << ") = ";
	    std::cout << "\"" << str->append(str, pos, tmp) << "\"" << std::endl;
	    break;
	case 11:
	    std::cout << "RESERVED FOR pop_back() (NO EFFECT)" << std::endl;
	    break;
	case 12: // assign(const string &, size_type, size_type)
	    pos = seed % str->length();
	    tmp = seed2 % (str->length() - pos);
	    str2 = str.get_obj();
	    std::cout << "assign(\"" << str2 << "\", " << pos << ", " << tmp << ") = ";
	    std::cout << "\"" << str->assign(str2, pos, tmp) << "\"" << std::endl;
	    break;
	case 13: // insert(size_type, const string &)
	    pos = seed % str->length();
	    std::cout << "insert(" << pos << ", \"" << randomstr << "\") = " << str->insert(pos, randomstr) << std::endl;
	    break;
	case 14: // erase(size_type, size_type)
	    pos = seed % str->length();
	    std::cout << "erase(" << pos << ", " << 3 << ") = ";
	    std::cout << "\"" << str->erase(pos, tmp) << "\"" << std::endl;
	    break;
	case 15: // erase(iterator)
	    std::cout << "erase(begin())" << std::endl;
	    str->erase(str->begin());
	    break;
	case 16: // erase(iterator, iterator)
	    std::cout << "erase(begin(), begin()+2)" << std::endl;
	    it1 = it2 = str->begin();
	    ++it2;
	    ++it2;
	    str->erase(it1, it2);
	    break;
	case 17: // replace(size_type, size_type, const string &)
	    pos = seed % str->length();
	    tmp = seed2 % (str->length() - pos);
	    std::cout << "replace(" << pos << ", " << tmp << ", ";
	    std::cout << "\"" << randomstr << "\") = ";
	    std::cout << "\"" << str->replace(pos, tmp, randomstr) << "\"" << std::endl;
	    break;
	case 18: // copy(ptr<char>, size_type, size_type)
	    pos = seed % str->length();
	    tmp = seed2 % (str->length() - pos);
	    std::cout << "copy(cp, " << tmp << ", " << pos << ") = ";
	    std::cout << str->copy(cp, tmp, pos) << std::endl;
	    std::cout << "cp = \"" << reinterpret_cast<char *>(&(*cp).get_obj()) << "\"" << std::endl;
	    break;
	case 19: // swap(string &) (UNIMPLEMENTED)
	    break;
	case 20: // c_str() (== data())
	    std::cout << "c_str() = ";
	    std::cout << "\"" << reinterpret_cast<const char *>(&(*str->c_str()).get_obj()) << "\"" << std::endl;
	    break;
	case 21: // find(const string &, size_type = 0)
	    std::cout << "find(\"" << randomstr << "\") = " << str->find(randomstr) << std::endl;
	    break;
	case 22: // rfind(const string &, size_type = npos)
	    std::cout << "rfind(\"" << randomstr << "\") = " << str->rfind(randomstr) << std::endl;
	    break;
	case 23: // find_first_of(const string &, size_type = 0)
	    std::cout << "find_first_of(\"" << randomstr << "\") = " << str->find_first_of(randomstr) << std::endl;
	    break;
	case 24: // find_last_of(const string &, size_type = npos)
	    std::cout << "find_last_of(\"" << randomstr << "\") = " << str->find_last_of(randomstr) << std::endl;
	    break;
	case 25: // find_first_not_of(const string &, size_type = 0)
	    std::cout << "find_first_not_of(\"" << randomstr << "\") = " << str->find_first_not_of(randomstr) << std::endl;
	    break;
	case 26: // find_last_not_of(const string &, size_type = npos)
	    std::cout << "find_last_not_of(\"" << randomstr << "\") = " << str->find_last_not_of(randomstr) << std::endl;
	    break;
	case 27: // substr(size_type, size_type)
	    pos = seed % str->length();
	    tmp = seed2 % (str->length() - pos);
	    std::cout << "substr(" << pos << ", " << tmp << ") = ";
	    std::cout << "\"" << str->substr(pos, tmp) << "\"" << std::endl;
	    break;
	case 28: // compare(const string &) (RANDOM)
	    std::cout << "compare(\"" << randomstr << "\") = ";
	    std::cout << str->compare(randomstr) << std::endl;
	    break;
	case 29: // compare(const string &) (IDENTICAL)
	    std::cout << "compare(current_str) = ";
	    std::cout << str->compare(str) << std::endl;
	    break;

	default:
	    break;
	}
    }

    class c {
    private:
	typedef std::string string_type;
    public:
	string_type str;

	template <class _Arch>
	void init_string(pstl::string<_Arch> &pstlstr, const int sz = 0);
	void func_apply_string(const int trial_func, const int seed, const int seed2, const int seed3);
    };

    template <class _Arch>
    void c::init_string(pstl::string<_Arch> &pstlstr, const int sz) {
	str.clear();
	if (!pstlstr.empty()) {
	    str = pstlstr;
	}
	else {
	    for (int i = 0; i < sz; i++) {
		str += 'a' + static_cast<char>(std::rand() % 26);
	    }
	}
    }

    void c::func_apply_string(const int trial_func, const int seed, const int seed2, const int seed3) {
	const int NUM_OF_FUNCS = 17;
	string_type str2;
	string_type::iterator it, it1, it2;
	string_type randomstr = "" + static_cast<char>('a' + seed % 26) + static_cast<char>('a' + seed2 % 26) + static_cast<char>('a' + seed3 % 26);
	char c;
	char *cp;
	int tmp;
	int i, pos;
	std::cout << "std::string::";
       
	switch (trial_func % NUM_OF_FUNCS) {
	case 0: // empty()
	    std::cout << "empty() = ";
	    if (str.empty()) {
		std::cout << "true" << std::endl;
	    }
	    else {
		std::cout << "false" << std::endl;
	    }
	    break;
	case 1: // length()
	    std::cout << "length() = " << str.length() << std::endl;
	    break;
	case 2: // max_size()
	    std::cout << "max_size() = " << str.max_size() << std::endl;
	    break;
	case 3: // resize(size_type, char)
	    tmp = seed % (str.length() * 2);
	    c = 'a' + static_cast<char>(seed2 % 26);
	    std::cout << "resize(" << tmp << ", " << c << ")" << std::endl;
	    str.resize(tmp, c);
	    break;
	case 4: // reserve(size_type)
	    tmp = str.length() * 2 + 1;
	    std::cout << "reserve(" << tmp << ")" << std::endl;
	    str.reserve(tmp);
	    break;
	case 5: // capacity()
	    std::cout << "capacity() = " << str.capacity() << std::endl;
	    break;
	case 6: // operator[](size_type)
	    if (str.empty()) {
		std::cout << "operator[](size_type) NOT EXECUTED (empty string)" << std::endl;
		break;
	    }
	    i = seed % str.length();
	    c = 'a' + static_cast<char>(seed2 % 26);
	    std::cout << "operator[](" << i << ") = ";
	    std::cout << str[i] << std::endl;
	    std::cout << "operator[](" << i << ") = " << c << std::endl;
	    str[i] = c;
	    break;
	case 7: // operator+=(const string &)
	    std::cout << "operator+=((const string &) \"" << randomstr << "\")" << std::endl;
	    str += randomstr;
	    break;
	case 8: // operator+=(const char *)
	    std::cout << "operator+=((const char *) \"" << randomstr << "\")" << std::endl;
	    str += randomstr.c_str();
	    break;
        case 9: // operator+=(char)
	    c = 'a' + static_cast<char>(seed % 26);
	    std::cout << "operator+=('" << c << "')" << std::endl;
	    str += c;
	    break;
	case 10: // append(const string &, size_type, size_type)
	    pos = seed % str.length();
	    tmp = seed2 % (str.length() - pos);
	    std::cout << "append(current_str, " << pos << ", " << tmp << ") = ";
	    std::cout << "\"" << str.append(str, pos, tmp) << "\"" << std::endl;
	    break;
	case 11: 
	    std::cout << "RESERVED for pop_back() (NO EFFECT)" << std::endl;
	    break;
        case 12: // assign(const string &, size_type, size_type)
	    pos = seed % str.length();
	    tmp = seed2 % (str.length() - pos);
	    str2 = str;
	    std::cout << "assign(\"" << str2 << "\", " << pos << ", " << tmp << ") = ";
	    std::cout << "\"" << str.assign(str2, pos, tmp) << "\"" << std::endl;
	    break;
	case 13: // insert(size_type, const string &)
	    pos = seed % str.length();
	    std::cout << "insert(" << pos << ", \"" << randomstr << "\") = " << str.insert(pos, randomstr) << std::endl;
	    break;
	case 14: // erase(size_type, size_type)
	    pos = seed % str.length();
	    std::cout << "erase(" << pos << ", " << 3 << ") = ";
	    std::cout << "\"" << str.erase(pos, tmp) << "\"" << std::endl;
	    break;
	case 15: // erase(iterator)
	    std::cout << "erase(begin())" << std::endl;
	    str.erase(str.begin());
	    break;
	case 16: // erase(iterator, iterator)
	    std::cout << "erase(begin(), begin()+2)" << std::endl;
	    it1 = it2 = str.begin();
	    ++it2;
	    ++it2;
	    str.erase(it1, it2);
	    break;
	case 17: // replace(size_type, size_type, const string &)
	    pos = seed % str.length();
	    tmp = seed2 % (str.length() - pos);
	    std::cout << "replace(" << pos << ", " << tmp << ", ";
	    std::cout << "\"" << randomstr << "\") = ";
	    std::cout << "\"" << str.replace(pos, tmp, randomstr) << "\"" << std::endl;
	    break;
	case 18: // copy(char *, size_type, size_type)
	    pos = seed % str.length();
	    tmp = seed2 % (str.length() - pos);
	    std::cout << "copy(cp, " << tmp << ", " << pos << ") = ";
	    std::cout << str.copy(cp, tmp, pos) << std::endl;
	    std::cout << "cp = \"" << cp << "\"" << std::endl;
	    break;
	case 19: // swap(string &) (UNIMPLEMENTED)
	    break;
	case 20: // c_str() (== data())
	    std::cout << "c_str() = ";
	    std::cout << "\"" << str.c_str() << "\"" << std::endl;
	    break;
	case 21: // find(const string &, size_type = 0)
	    std::cout << "find(\"" << randomstr << "\") = " << str.find(randomstr) << std::endl;
	    break;
	case 22: // rfind(const string &, size_type = npos)
	    std::cout << "rfind(\"" << randomstr << "\") = " << str.rfind(randomstr) << std::endl;
	    break;
	case 23: // find_first_of(const string &, size_type = 0)
	    std::cout << "find_first_of(\"" << randomstr << "\") = " << str.find_first_of(randomstr) << std::endl;
	    break;
	case 24: // find_last_of(const string &, size_type = npos)
	    std::cout << "find_last_of(\"" << randomstr << "\") = " << str.find_last_of(randomstr) << std::endl;
	    break;
	case 25: // find_first_not_of(const string &, size_type = 0)
	    std::cout << "find_first_not_of(\"" << randomstr << "\") = " << str.find_first_not_of(randomstr) << std::endl;
	    break;
	case 26: // find_last_not_of(const string &, size_type = npos)
	    std::cout << "find_last_not_of(\"" << randomstr << "\") = " << str.find_last_not_of(randomstr) << std::endl;
	    break;
	case 27: // substr(size_type, size_type)
	    pos = seed % str.length();
	    tmp = seed2 % (str.length() - pos);
	    std::cout << "substr(" << pos << ", " << tmp << ") = ";
	    std::cout << "\"" << str.substr(pos, tmp) << "\"" << std::endl;
	    break;
	case 28: // compare(const string &) (RANDOM)
	    std::cout << "compare(\"" << randomstr << "\") = ";
	    std::cout << str.compare(randomstr) << std::endl;
	    break;
	case 29: // compare(const string &) (IDENTICAL)
	    std::cout << "compare(current_str) = ";
	    std::cout << str.compare(str) << std::endl;
	    break;

	default:
	    break;
	}
    }

    template <class _Arch>
    void validate_string(const b<_Arch> &b_obj, const c &c_obj) {
	std::cout << b_obj.str << std::endl;
	std::cout << c_obj.str << std::endl;
	if (b_obj.str.get_obj() == c_obj.str) {
	    std::cout << "MATCH" << std::endl;
	}
	else {
	    std::cout << "DISMATCH" << std::endl;
	}
    }
}

int main() {
    const int NUM_TRIAL = 100;
    const int NUM_OF_FUNCS = 30;
    
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    pstl::c k;
    pstl::b<pstl::arch<pstl::pmem> > i;
    i.open("");
    pstl::b<pstl::arch<pstl::vmem, pstl::b64> > j;
    j.open("hoge.ldb");
    
    k.init_string(*j.str, 100);
    i.init_string(k.str, 100);
    j.init_string(k.str, 100);

    pstl::validate_string(i, k);
    pstl::validate_string(j, k);
    
    for (int s = 0; s < NUM_TRIAL; s++) {
	int func_id = std::rand() % NUM_OF_FUNCS;
	int seed = std::rand(),
	    seed2 = std::rand(),
	    seed3 = std::rand();
	k.func_apply_string(func_id, seed, seed2, seed3);
	i.func_apply_string(func_id, seed, seed2, seed3);
	j.func_apply_string(func_id, seed, seed2, seed3);
    }
    
    pstl::validate_string(i, k);
    pstl::validate_string(j, k);

    return 0;
}
