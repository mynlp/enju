/*
 * $Id: ldbm-type.h,v 1.10 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __ldbm_type_h
#define __ldbm_type_h

#include "../pstl/pstl_pheap.h"
#include "../pstl/pstl_vector.h"
#include "../pstl/pstl_hash_map.h"

#include <exception>
#include <iostream>
#include <ostream>
#include <utility>
#include <vector>

namespace ldbm {
    
    template <class _Dev, class _Bit>
    void print_ptr_char(pstl::pheap<_Dev, _Bit> *ph, std::ostream& os, pstl::ptr<char, _Bit> str) {
        pstl::ref<char, _Dev, _Bit> c = str(ph);
        while(c != 0) {
            os << c;
            ++str;
            c.reassign(str(ph));
        }
    }
    template <class _Dev, class _Bit>
    int __pstl_strcmp(pstl::pheap<_Dev, _Bit> *ph,
                      pstl::ptr<char, _Bit> str1, pstl::ptr<char, _Bit> str2) {
        pstl::ref<char, _Dev, _Bit> c1 = str1(ph), c2 = str2(ph);
        while(!(c1 == 0 && c2 == 0)) {
            if( c1 != c2) return false;
            ++str1; ++str2;
            c1.reassign(str1(ph)); c2.reassign(str2(ph));
        }
        return true;
    }

    class type_declaration_on_pmem : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "New type is added to DB on the memory\n";
        };
    };

    class double_type_declaration : public std::exception {
    public:
        virtual const char* what() const throw() {
            return "the added type is already registered\n";
        };
    };
    

    template <class _Dev, class _Bit> class propername {// transient class
    private:
        typedef _Dev dev_type;
        typedef _Bit bit_type;
        typedef pstl::allocator<dev_type, bit_type> allocator_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        pstl::ptr<char, bit_type> module_name;
        pstl::ptr<char, bit_type> simple_name;
    public:
        propername() { module_name = 0; simple_name = 0; }
        propername(const pstl::ptr<char, bit_type> x, const pstl::ptr<char, bit_type> y) {
            module_name = x;
            simple_name = y;
        }
        ~propername() {}
        void setPropername(const pstl::ptr<char, bit_type> x, const pstl::ptr<char, bit_type> y) {
            module_name = x;
            simple_name = y;
        }
        static propername invalid_propername() { return propername(); }
        pstl::ptr<char, bit_type> getModuleName() const { return module_name; }
        pstl::ptr<char, bit_type> getSimpleName() const { return simple_name; }
        bool isValid() const { return (module_name !=0 && simple_name != 0); }
    };
    struct hash_propername {
        template <class _Dev, class _Bit>
        typename _Bit::size_type operator()(pstl::pheap<_Dev, _Bit> *ph, const propername<_Dev, _Bit> tn) const {
            return (pstl::__stl_hash_string(ph, tn.getModuleName()) +
                    pstl::__stl_hash_string(ph, tn.getSimpleName())); }
    };

    struct equal_to_propername {
        template <class _Dev1, class _Bit1, class _Dev2, class _Bit2> 
        bool operator()(pstl::pheap<_Dev1, _Bit1> *ph1, propername<_Dev1, _Bit1> tn1,
                        pstl::pheap<_Dev2, _Bit2> *ph2, propername<_Dev2, _Bit2> tn2) const {
            return ((pstl::strcmp(ph1, tn1.getModuleName(), ph2, tn2.getModuleName()) == 0) &&
                    (pstl::strcmp(ph1, tn1.getSimpleName(), ph2, tn2.getSimpleName()) == 0));
        }
        template <class _Dev, class _Bit>
        bool operator()(pstl::pheap<_Dev, _Bit> *ph,
                        propername<_Dev, _Bit> tn1,
                        propername<_Dev, _Bit> tn2) const {
            return ((pstl::strcmp(ph, tn1.getModuleName(), ph, tn2.getModuleName()) == 0) &&
                    (pstl::strcmp(ph, tn1.getSimpleName(), ph, tn2.getSimpleName()) == 0));
        }
    };

    template <class _Dev, class _Bit> class TypeManager;
    template <class _Bit> class TypeManager<pstl::pmem, _Bit> {
    public:
        typedef pstl::pmem dev_type;
        typedef _Bit bit_type;
        typedef pstl::allocator<dev_type, bit_type> allocator_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename allocator_type::size_type size_type;
        typedef typename allocator_type::pointer pointer;
    public:
        static const size_type invalid_size_type = _Bit::max_size_type;
        TypeManager() {}
        ~TypeManager() {}
/// constructor, destructor and pnew
        void construct(pheap_type*, pstl::ptr<TypeManager, bit_type>, pstl::ptr<allocator_type, bit_type>) {}
        static pstl::ptr<TypeManager, bit_type> pnew(pheap_type*, pstl::ptr<allocator_type, bit_type>) {
            return pstl::ptr<TypeManager, bit_type>((pointer) new TypeManager<dev_type, bit_type>);
        }
////  
        size_type size() { return lilfes::type::GetSerialCount(); }
        template <class _Bit2>
        size_type getSerialNo(pheap_type *, pstl::pheap<pstl::pmem, _Bit2> *, propername<pstl::pmem, _Bit2>& pn2) {
            lilfes::module *m;
            lilfes::type *t;
            if(!(m = lilfes::module::SearchModule(pn2.getModuleName().getAddress()))) {
                return invalid_size_type;
	    }
            if(!(t = m->Search(pn2.getSimpleName().getAddress()))) {
                return invalid_size_type;
	    }
            return (size_type) t->GetSerialNo();
        }
        template <class _Bit2>
        size_type getSerialNo(pheap_type *ph1, pstl::pheap<pstl::vmem, _Bit2> *ph2, propername<pstl::vmem, _Bit2>& pn2) {
            lilfes::module *m;
            lilfes::type *t;
	    pstl::ptr<char, _Bit2> modname = pn2.getModuleName();
	    pstl::ptr<char, _Bit2> simname = pn2.getSimpleName();

	    typename _Bit2::size_type modlen = pstl::strlen(ph2, modname);
	    typename _Bit2::size_type simlen = pstl::strlen(ph2, simname);

	    char *modstr = (char*) malloc(sizeof(char) * (modlen + 1));
	    char *simstr = (char*) malloc(sizeof(char) * (simlen + 1));

	    pstl::strcpy(ph2, modname, ph1, pstl::ptr<char, bit_type>((pointer) modstr));
	    pstl::strcpy(ph2, simname, ph1, pstl::ptr<char, bit_type>((pointer) simstr));

	    modstr[modlen] = '\0';
	    simstr[simlen] = '\0';

            if(!(m = lilfes::module::SearchModule(modstr))) {
                free(modstr); free(simstr);
                return invalid_size_type;
	    }
            if(!(t = m->Search(simstr))) {
                free(modstr); free(simstr);
                return invalid_size_type;
	    }
	    free(modstr); free(simstr);
            return (size_type) t->GetSerialNo();
        }
        propername<dev_type, bit_type> getPropername(pheap_type *, size_type sn) {
            const lilfes::type *t = lilfes::type::Serial((lilfes::tserial) sn);
            if(!t) return propername<dev_type, bit_type>::invalid_propername();
            return propername<dev_type, bit_type>(pstl::ptr<char, bit_type>((typename bit_type::pointer)
									    t->GetModule()->GetName()),
                                                  pstl::ptr<char, bit_type>((typename bit_type::pointer)
									    t->GetSimpleName()));
        }
        template <class _Dev2, class _Bit2>
        size_type add(pheap_type *, pstl::pheap<_Dev2, _Bit2> *, propername<_Dev2, _Bit2>&) throw(type_declaration_on_pmem) { throw type_declaration_on_pmem(); return 0;}
        void getAppFeatures(size_type type_serial, std::vector<size_type>& ff) {
            const lilfes::type *t = lilfes::type::Serial((lilfes::tserial) type_serial);
            int nf = t->GetNFeatures();
            for(int i = 0 ; i < nf ; i++)
                ff.push_back((size_type) t->Feature(i)->GetRepType()->GetSerialNo());
        }
    };

    template <class _Bit> class TypeManager<pstl::vmem, _Bit> {
    public:
        typedef pstl::vmem dev_type;
        typedef _Bit bit_type;
        typedef pstl::allocator<dev_type, bit_type> allocator_type;
        typedef pstl::pheap<dev_type, bit_type> pheap_type;
        typedef typename allocator_type::size_type size_type;
        typedef typename allocator_type::pointer pointer;

        typedef pstl::hash_map<size_type, propername<dev_type, bit_type>,
                               pstl::hash<size_type>,
                               pstl::equal_to<size_type>,
                               allocator_type> sn2pn_type;
        typedef pstl::hash_map<propername<dev_type, bit_type>, size_type,
                               hash_propername,
                               equal_to_propername,
                               allocator_type> pn2sn_type;
    private:
        sn2pn_type sn2pn;
        pn2sn_type pn2sn;
        pstl::ptr<allocator_type, bit_type> allocp;
        
    public:
        static const size_type invalid_size_type = _Bit::max_size_type;
        TypeManager() {}
        ~TypeManager() {}

/// constructor, destructor and pnew
    private:
        pstl::ptr<sn2pn_type, bit_type> get_sn2pn_pointer(pstl::ptr<TypeManager, bit_type> _M_this) {
            pstl::ptr_int sn2pn_addr = (pstl::ptr_int) (& sn2pn);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ptr<sn2pn_type, bit_type>(_M_this.getAddress() + (sn2pn_addr - this_addr));
        }
        pstl::ptr<pn2sn_type, bit_type> get_pn2sn_pointer(pstl::ptr<TypeManager, bit_type> _M_this) {
            pstl::ptr_int pn2sn_addr = (pstl::ptr_int) (& pn2sn);
            pstl::ptr_int this_addr = (pstl::ptr_int) this;
            return pstl::ptr<pn2sn_type, bit_type>(_M_this.getAddress() + (pn2sn_addr - this_addr));
        }
    public:
        void construct(pheap_type* ph, pstl::ptr<TypeManager, bit_type> _M_this, pstl::ptr<allocator_type, bit_type> __allocp) {
            allocp = __allocp;
            sn2pn.construct(ph, get_sn2pn_pointer(_M_this), allocp);
            pn2sn.construct(ph, get_pn2sn_pointer(_M_this), allocp);
        }
        static pstl::ptr<TypeManager, bit_type> pnew(pheap_type* ph, pstl::ptr<allocator_type, bit_type> allocp) {
            typename allocator_type::reference alloc(ph, allocp);
            pstl::ptr<TypeManager, bit_type> p = (pstl::ptr<TypeManager, bit_type>) alloc.allocate(sizeof(TypeManager));
            p(ph)->construct(ph, p, allocp);
            return p;
        }
        
////  
        size_type size() { return sn2pn.size(); }
        template <class _Dev2, class _Bit2>
        size_type getSerialNo(pheap_type *ph1, pstl::pheap<_Dev2, _Bit2> *ph2, propername<_Dev2, _Bit2>& pn2) {
            typename pn2sn_type::iterator it = pn2sn.find(ph1, ph2, pn2);
            if(it == pn2sn.end(ph1)) { return invalid_size_type;}
            return it->second;
        }
        propername<dev_type, bit_type> getPropername(pheap_type *ph, size_type sn) {
            typename sn2pn_type::iterator it = sn2pn.find(ph, sn);
            if(it == sn2pn.end(ph)) return propername<dev_type, bit_type>::invalid_propername();
            return it->second;
        }
        template <class _Dev2, class _Bit2>
        size_type add(pheap_type *ph1, pstl::pheap<_Dev2, _Bit2> *ph2, propername<_Dev2, _Bit2> pn2) throw(double_type_declaration) {

            if(pn2sn.find(ph1, ph2, pn2) != pn2sn.end(ph1)) throw double_type_declaration();
            
                /// allocate propername
            pstl::b32::size_type modlen = strlen(ph2, pn2.getModuleName());
            pstl::b32::size_type namlen = strlen(ph2, pn2.getSimpleName());

            typename allocator_type::reference alloc(ph1, allocp);
            pstl::ptr<char, bit_type> newmod = (pstl::ptr<char, bit_type>) alloc.allocate(modlen + 1);
            pstl::ptr<char, bit_type> newnam = (pstl::ptr<char, bit_type>) alloc.allocate(namlen + 1);

            strcpy(ph2, pn2.getModuleName(), ph1, newmod);
            strcpy(ph2, pn2.getSimpleName(), ph1, newnam);
            

                /// mapping propername
            size_type newsn = pn2sn.size();
	    propername<dev_type, bit_type> pn1(newmod, newnam);
	    pn2sn.insert(ph1, typename pn2sn_type::value_type(typename pn2sn_type::key_type(pn1),
                                                              typename pn2sn_type::mapped_type(newsn)));
            sn2pn.insert(ph1, typename sn2pn_type::value_type(typename sn2pn_type::key_type(newsn),
                                                              typename sn2pn_type::mapped_type(pn1)));
            return newsn;
        }
        void getAppFeatures(size_type, std::vector<size_type>& ff) {
            size_type n = sn2pn.size();
            for(size_type i = 0 ; i < n ; i++)
                ff.push_back(i);
        }
    };

    template <class X> struct my_hash
    {
        size_t operator()(X __x) const { return (size_t) __x; }
    };
    template <class X> struct my_equal_to
    {
        bool operator()(const X& __x, const X& __y) const { return __x == __y; }
    };

    template <class A, class B> class TypeMapper {
    private:
        typedef typename A::size_type size_type_a;
        typedef typename B::size_type size_type_b;
        typedef typename A::dev_type dev_type_a;
        typedef typename B::dev_type dev_type_b;
        typedef typename A::bit_type bit_type_a;
        typedef typename B::bit_type bit_type_b;
        typedef pstl::pheap<dev_type_a, bit_type_a> pheap_type_a;
        typedef pstl::pheap<dev_type_b, bit_type_b> pheap_type_b;
        typedef pstl::allocator<dev_type_a, bit_type_a> allocator_type_a;
        typedef pstl::allocator<dev_type_b, bit_type_b> allocator_type_b;
        typedef __gnu_cxx::hash_map<size_type_a, size_type_b, my_hash<size_type_a>, my_equal_to<size_type_a> > sna2snb_type;
        typedef __gnu_cxx::hash_map<size_type_b, size_type_a, my_hash<size_type_b>, my_equal_to<size_type_b> > snb2sna_type;
        static const size_type_a invalid_size_type_a = A::invalid_size_type;
        static const size_type_b invalid_size_type_b = B::invalid_size_type;
        pheap_type_a *pha;
        pheap_type_b *phb;
        sna2snb_type a2b;
        snb2sna_type b2a;
        pstl::ref<TypeManager<dev_type_a, bit_type_a>, dev_type_a, bit_type_a> a;
        pstl::ref<TypeManager<dev_type_b, bit_type_b>, dev_type_b, bit_type_b> b;
        typename allocator_type_a::reference alloca;
        typename allocator_type_b::reference allocb;
        size_type_a valid_serial_count_a;
        size_type_b valid_serial_count_b;
    public:
        
        TypeMapper(pheap_type_a *__pha,
                   pstl::ref<TypeManager<dev_type_a, bit_type_a>, dev_type_a, bit_type_a> __a,
                   typename allocator_type_a::reference __alloca,
                   pheap_type_b *__phb,
                   pstl::ref<TypeManager<dev_type_b, bit_type_b>, dev_type_b, bit_type_b> __b,
                   typename allocator_type_b::reference __allocb)
                : pha(__pha), phb(__phb), valid_serial_count_a(0), valid_serial_count_b(0) {
            a.reassign(__a); b.reassign(__b);
            alloca.reassign(__alloca); allocb.reassign(__allocb);
        }
        ~TypeMapper() {}
        
        void syncA() {
//            std::cout << "TypeMapper::syncA" << std::endl;
            size_type_a serial_count_a = a->size();
            for(; valid_serial_count_a < serial_count_a ; valid_serial_count_a++) {
                if( a2b.find(valid_serial_count_a) == a2b.end() ) { // no mapping
                    propername<dev_type_a, bit_type_a> r = a->getPropername(pha, valid_serial_count_a);
                    if(r.isValid()) {
		        size_type_b s = b->getSerialNo(phb, pha, r);
                        if( s != invalid_size_type_b ) {
                            a2b.insert(typename sna2snb_type::value_type(typename sna2snb_type::key_type(valid_serial_count_a),
                                                                         typename sna2snb_type::mapped_type(s)));
                            b2a.insert(typename snb2sna_type::value_type(typename snb2sna_type::key_type(s),
                                                                         typename snb2sna_type::mapped_type(valid_serial_count_a)));
                        }
                    }
                }
            }
        }
        void syncB() {
//            std::cout << "TypeMapper::syncB" << std::endl;
            size_type_b serial_count_b = b->size();
            for(; valid_serial_count_b < serial_count_b ; valid_serial_count_b++) {
                if( b2a.find(valid_serial_count_b) == b2a.end() ) { // no mapping
                    propername<dev_type_b, bit_type_b> r = b->getPropername(phb, valid_serial_count_b);
                    if(r.isValid()) {
                        size_type_a s = a->getSerialNo(pha, phb, r);
                        if( s != invalid_size_type_a ) {
                            a2b.insert(typename sna2snb_type::value_type(typename sna2snb_type::key_type(s),
                                                                         typename sna2snb_type::mapped_type(valid_serial_count_b)));
                            b2a.insert(typename snb2sna_type::value_type(typename snb2sna_type::key_type(valid_serial_count_b),
                                                                         typename snb2sna_type::mapped_type(s)));
                        }
                    }
                }
            }
        }
        void sync() {
            syncA();
            syncB();
        }
        size_type_b A2B(size_type_a sna) {
//            std::cout << "TypeMapper::A2B" << std::endl;
            typename sna2snb_type::iterator it = a2b.find(sna);
            if( it != a2b.end() ) return it->second;
            sync();
            it = a2b.find(sna);
            if( it != a2b.end() ) return it->second;
            return invalid_size_type_b;
        }
        size_type_a B2A(size_type_b snb) {
//            std::cout << "TypeMapper::B2A" << std::endl;
            typename snb2sna_type::iterator it = b2a.find(snb);
            if( it != b2a.end() ) return it->second;
            sync();
            it = b2a.find(snb);
            if( it != b2a.end() ) return it->second;
            return invalid_size_type_a;
        }
        size_type_b CoerceA2B(size_type_a sna) {
//            std::cout << "TypeMapper::CoerceA2B" << std::endl;
            typename sna2snb_type::iterator it = a2b.find(sna);
            if( it != a2b.end() ) return it->second;
            sync();
            it = a2b.find(sna);
            if( it != a2b.end() ) return it->second;
            if( sna >= a->size() ) return invalid_size_type_b;
            return b->add(phb, pha, a->getPropername(pha, sna));
        }
        size_type_a CoerceB2A(size_type_b snb) {
//            std::cout << "TypeMapper::CoerceB2A" << std::endl;
//            std::cout << "CoereceB2A: " << snb << std::endl;
            typename snb2sna_type::iterator it = b2a.find(snb);
            if( it != b2a.end() ) return it->second;
            sync();
            it = b2a.find(snb);
            if( it != b2a.end() ) return it->second;
            if( snb >= b->size() ) return invalid_size_type_a;
            return a->add(pha, phb, b->getPropername(phb, snb));
        }
        void print(pheap_type_a* pha, pheap_type_b* phb) {
            typename sna2snb_type::iterator it = a2b.begin(), last = a2b.end();
            for(; it != last ; it++) {
                std::cout << "A2B: " << it->first << "<->" << it->second;
                std::cout << " (";
                print_ptr_char(pha, std::cout, a->getPropername(pha, it->first).getModuleName());
                std::cout << ":";
                print_ptr_char(pha, std::cout, a->getPropername(pha, it->first).getSimpleName());
                std::cout << "<->";
                print_ptr_char(phb, std::cout, b->getPropername(phb, it->second).getModuleName());
                std::cout << ":";
                print_ptr_char(phb, std::cout, b->getPropername(phb, it->second).getSimpleName());
                std::cout << ")" << std::endl;
            }
            typename snb2sna_type::iterator it2 = b2a.begin(), last2 = b2a.end();
            for(; it2 != last2 ; it2++) {
                std::cout << "B2A: " << it2->first << "<->" << it2->second;
                std::cout << " (";
                print_ptr_char(phb, std::cout, b->getPropername(phb, it2->first).getModuleName());
                std::cout << ":";
                print_ptr_char(phb, std::cout, b->getPropername(phb, it2->first).getSimpleName());
                std::cout << "<->";
                print_ptr_char(pha, std::cout, a->getPropername(pha, it2->second).getModuleName());
                std::cout << ":";
                print_ptr_char(pha, std::cout, a->getPropername(pha, it2->second).getSimpleName());
                std::cout << ")" << std::endl;
            }
        }
    };
    
    template <class A, class B> class FeatureMapper {
    private:
        typedef typename A::size_type size_type_a;
        typedef typename B::size_type size_type_b;
        typedef typename A::dev_type dev_type_a;
        typedef typename B::dev_type dev_type_b;
        typedef typename A::bit_type bit_type_a;
        typedef typename B::bit_type bit_type_b;
        typedef std::pair<size_type_a, size_type_a> pair_size_type_a;
        typedef std::pair<size_type_b, size_type_b> pair_size_type_b;
        struct hash_mapping_a {
            size_t operator()(pair_size_type_a x) const { return size_t(x.first + x.second); }
        };
        struct hash_mapping_b {
            size_t operator()(pair_size_type_b x) const { return size_t(x.first + x.second); }
        };

        struct equal_to_mapping_a {
            bool operator()(pair_size_type_a x, pair_size_type_a y) const {
                return x == y;
            }
        };
        struct equal_to_mapping_b {
            bool operator()(pair_size_type_b x, pair_size_type_b y) const {
                return x == y;
            }
        };
        typedef __gnu_cxx::hash_map<size_type_a, size_type_a, my_hash<size_type_a>, my_equal_to<size_type_a> > sna2sna_type;
        typedef __gnu_cxx::hash_map<size_type_b, size_type_b, my_hash<size_type_b>, my_equal_to<size_type_b> > snb2snb_type;
        typedef __gnu_cxx::hash_map<pair_size_type_a, int, hash_mapping_a, equal_to_mapping_a> a2b_type;
        typedef __gnu_cxx::hash_map<pair_size_type_b, int, hash_mapping_b, equal_to_mapping_b> b2a_type;

        static const size_type_a invalid_size_type_a = A::invalid_size_type;
        static const size_type_b invalid_size_type_b = B::invalid_size_type;

        a2b_type a2b;
        b2a_type b2a;

        pstl::ref<TypeManager<dev_type_a, bit_type_a>, dev_type_a, bit_type_a> a;
        pstl::ref<TypeManager<dev_type_b, bit_type_b>, dev_type_b, bit_type_b> b;
        TypeMapper<A, B> *type_mapper;

        sna2sna_type valid_serial_count_a;
        snb2snb_type valid_serial_count_b;

        void sync(size_type_a t, size_type_b u) {
//            std::cout << "FeatureMapper::sync" << std::endl;
            size_type_a serial_count_a = a->size();
            size_type_b serial_count_b = b->size();

            typename sna2sna_type::iterator ita = valid_serial_count_a.find(t);
            typename snb2snb_type::iterator itb = valid_serial_count_b.find(u);

            if( ita == valid_serial_count_a.end() )
                valid_serial_count_a.insert(typename sna2sna_type::value_type(typename sna2sna_type::key_type(t),
                                                                              typename sna2sna_type::mapped_type(0)));

            if( itb == valid_serial_count_b.end() )
                valid_serial_count_b.insert(typename snb2snb_type::value_type(typename snb2snb_type::key_type(u),
                                                                              typename snb2snb_type::mapped_type(0)));

            ita = valid_serial_count_a.find(t);
            itb = valid_serial_count_b.find(u);

            bool need_new_mapping = false;
            for(; ita->second < serial_count_a ; ita->second++)
                if( type_mapper->A2B(ita->second) != invalid_size_type_b )
                    if( a2b.find(pair_size_type_a(t, ita->second)) == a2b.end() )
                        need_new_mapping = true;
            
            for(; itb->second < serial_count_b ; itb->second++)
                if( type_mapper->B2A(itb->second) != invalid_size_type_a )
                    if( b2a.find(pair_size_type_b(u, itb->second)) == b2a.end() )
                        need_new_mapping = true;

            if( need_new_mapping ) make_new_mapping(t, u);
        }
        void make_new_mapping(size_type_a t, size_type_b u) {
//            std::cout << "FeatureMapper::make_new_mapping" << std::endl;
            std::vector<size_type_a> ff;
            std::vector<size_type_b> gg;
            int fcounter = 0, gcounter = 0;
            sna2sna_type ff_map, ff_order;
            snb2snb_type gg_map, gg_order;

            a->getAppFeatures(t, ff);
            b->getAppFeatures(u, gg);

            for(uint i = 0; i < ff.size() ; i++)
                ff_map.insert(typename sna2sna_type::value_type(typename sna2sna_type::key_type(ff[i]),
                                                                typename sna2sna_type::mapped_type(0)));
            for(uint j = 0; j < gg.size() ; j++)
                gg_map.insert(typename snb2snb_type::value_type(typename snb2snb_type::key_type(gg[j]),
                                                                typename snb2snb_type::mapped_type(0)));
            for(uint i = 0; i < ff.size() ; i++) {
                size_type_b g = type_mapper->A2B(ff[i]);
                if( g != invalid_size_type_b && (gg_map.find(g) != gg_map.end()) )
                    ff_order.insert(typename sna2sna_type::value_type(typename sna2sna_type::key_type(ff[i]),
                                                                      typename sna2sna_type::mapped_type(fcounter++)));
            }
            for(uint j = 0; j < gg.size() ; j++) {
                size_type_a f = type_mapper->B2A(gg[j]);
                if( f != invalid_size_type_a && (ff_map.find(f) != ff_map.end()) )
                    gg_order.insert(typename snb2snb_type::value_type(typename snb2snb_type::key_type(gg[j]),
                                                                      typename snb2snb_type::mapped_type(gcounter++)));
            }

            typename sna2sna_type::iterator it = ff_order.begin(), last = ff_order.end();
            for(; it != last ; it++) {
                size_type_a f = it->first;
                int fidx = it->second;
                size_type_b g = type_mapper->A2B(f);
                typename snb2snb_type::iterator gr = gg_order.find(g);
                ASSERT(gr != gg_order.end() );
                int gidx = gr->second;
                typename a2b_type::iterator itidx = a2b.find(pair_size_type_a(t, f));
                if( itidx != a2b.end() ) { //found
                    itidx->second = gidx;
                } else { // not found -- add mapping
                    a2b.insert(typename a2b_type::value_type(typename a2b_type::key_type(pair_size_type_a(t, f)),
                                                             typename a2b_type::mapped_type(gidx)));
                }
		
                typename b2a_type::iterator itidx2 = b2a.find(pair_size_type_b(u, g));
                if( itidx2 != b2a.end() ) { // found
                    itidx2->second = fidx;
                } else { // not found -- add mapping
                    b2a.insert(typename b2a_type::value_type(typename b2a_type::key_type(pair_size_type_b(u, g)),
                                                             typename b2a_type::mapped_type(fidx)));
                }
            }
        }
  
    public:
        FeatureMapper(pstl::ref<TypeManager<dev_type_a, bit_type_a>, dev_type_a, bit_type_a> x,
                      pstl::ref<TypeManager<dev_type_b, bit_type_b>, dev_type_b, bit_type_b> y,
		      TypeMapper<A,B> *z) : type_mapper(z) { a.reassign(x); b.reassign(y); }
        ~FeatureMapper() {};
        bool A2B(size_type_a t, std::vector<size_type_a>& feature, std::vector<int>& feature_order) {
//            std::cout << "FeatureMapper::A2B" << std::endl;
                // feature_order is supposed to be empty
            bool perfect_mapping = true;
            typename std::vector<size_type_a>::iterator f = feature.begin(), flast = feature.end();

            for(; f != flast ; f++) {
    
                typename a2b_type::iterator it = a2b.find(pair_size_type_a(t, *f));

                if( it != a2b.end() ) { // found
                    feature_order.push_back(it->second);
                } else {
                    perfect_mapping = false;
                    feature_order.clear();
                    break;
                }
            }
            if( perfect_mapping ) {
                return true;
            } else {

                size_type_b uu = type_mapper->A2B(t);
                if( uu == invalid_size_type_b ) return false;

                sync(t, uu);

                for(f = feature.begin() ; f != flast ; f++) {

                    typename a2b_type::iterator it = a2b.find(pair_size_type_a(t, *f));
      
                    if( it != a2b.end() ) { // found
                        feature_order.push_back(it->second);
                    } else {
                        feature_order.push_back(-1);
                    }
                }
                return true;
            }
        }
        bool B2A(size_type_b u, std::vector<size_type_b>& feature, std::vector<int>& feature_order) {
//            std::cout << "FeatureMapper::B2A" << std::endl;
                // feature_order is assumed to be empty std::vector
            bool perfect_mapping = true;
            typename std::vector<size_type_b>::iterator g = feature.begin(), glast = feature.end();

            for(; g != glast ; g++) {
    
                typename b2a_type::iterator it = b2a.find(pair_size_type_b(u, *g));

                if( it != b2a.end() ) { // found
                    feature_order.push_back(it->second);
                } else {
                    perfect_mapping = false;
                    feature_order.clear();
                    break;
                }
            }
            if( perfect_mapping ) {
                return true;
            } else {
                size_type_a tt = type_mapper->B2A(u);
                if( tt == invalid_size_type_a ) return false;

                sync(tt, u);

                for(g = feature.begin() ; g != glast ; g++) {

                    typename b2a_type::iterator it = b2a.find(pair_size_type_b(u, *g));
      
                    if( it != b2a.end() ) { // found
                        feature_order.push_back(it->second);
                    } else {
                        feature_order.push_back(-1);
                    }
                }
                return true;
            }
        }
        bool CoerceA2B(size_type_a t, std::vector<size_type_a>& feature, std::vector<int>& feature_order) {
//            std::cout << "FeatureMapper::CoerceA2B" << std::endl;
                // feature_order is supposed to be empty
            bool perfect_mapping = true;
            typename std::vector<size_type_a>::iterator f = feature.begin(), flast = feature.end();

            for(; f != flast ; f++) {
    
                typename a2b_type::iterator it = a2b.find(pair_size_type_a(t, *f));

                if( it != a2b.end() ) { // found
                    feature_order.push_back(it->second);
                } else {
                    perfect_mapping = false;
                    feature_order.clear();
                    break;
                }
            }
            if( perfect_mapping ) {
                return true;
            } else {

                size_type_b uu = type_mapper->CoerceA2B(t);
                if( uu == invalid_size_type_b ) return false;
                for(f = feature.begin() ; f != flast ; f++) { // ensure g (<- f)
                    if(type_mapper->CoerceA2B(*f) == invalid_size_type_b )
                        return false;
                }

                sync(t, uu);

                for(f = feature.begin() ; f != flast ; f++) {

                    typename a2b_type::iterator it = a2b.find(pair_size_type_a(t, *f));
      
                    if( it != a2b.end() ) { // found
                        feature_order.push_back(it->second);
                    } else {
                        std::cerr << "internal error in DbFeatureMapper::CoerceA2B" << std::endl;
                        return false;
                    }
                }
                return true;
            }
        }
        bool CoerceB2A(size_type_b u, std::vector<size_type_b>& feature, std::vector<int>& feature_order) {
                // feature_order is supposed to be empty std::vector
//            std::cout << "FeatureMapper::CoerceB2A" << std::endl;
//            std::cout << "feature_size: " << feature.size() << "feature_order_size: " << feature_order.size() << std::endl;
            bool perfect_mapping = true;
            typename std::vector<size_type_b>::iterator g = feature.begin(), glast = feature.end();

            for(; g != glast ; g++) {

                typename b2a_type::iterator it = b2a.find(pair_size_type_b(u, *g));

                if( it != b2a.end() ) { // found
                    feature_order.push_back(it->second);
                } else {
                    perfect_mapping = false;
                    feature_order.clear();
                    break;
                }
            }
            if( perfect_mapping ) {
                return true;
            } else {

                size_type_a tt = type_mapper->CoerceB2A(u);
                if( tt == invalid_size_type_a ) return false;
                for(g = feature.begin() ; g != glast ; g++) { // ensure f (<- g)
                    if(type_mapper->CoerceB2A(*g) == invalid_size_type_a )
                        return false;
                }
                sync(tt, u);
                
                for(g = feature.begin() ; g != glast ; g++) {

                    typename b2a_type::iterator it = b2a.find(pair_size_type_b(u, *g));
      
                    if( it != b2a.end() ) { // found
                        feature_order.push_back(it->second);
                    } else {
                        std::cerr << "internal error in DbFeatureMapper::CoerceB2A" << std::endl;
                        return false;
                    }
                }
                return true;
            }
        }
    };
}


#endif // __ldbm_type_h

/*
 *$Log: not supported by cvs2svn $
 *Revision 1.9  2005/05/23 08:57:12  ninomi
 *push_back
 *FSO = [10&X, 20&Y, 30&Z] kvpair = [(1, X), (2, Y), (3, Z)]
 *に成功
 *
 *Revision 1.8  2005/05/23 07:02:40  ninomi
 *ばぐとり中です。
 *
 *Revision 1.7  2005/05/23 03:21:53  ninomi
 *gcc3.4でもコンパイルできるようにしました。
 *
 *Revision 1.6  2005/05/23 01:18:55  ninomi
 *push_backまで実装。バグがでている状態。
 *
 *Revision 1.5  2005/04/04 07:55:11  ninomi
 *ldbm-typeが動くようになった。TypeMapperまでは動作確認終了。
 *
 *Revision 1.4  2005/04/04 04:25:01  ninomi
 *a light check for hash key is introduced.  ldbm-type is still under construction.
 *
 *Revision 1.3  2005/04/01 10:08:01  ninomi
 *equalやhashのめそっどの引数のためのpheapを引数に加えました。
 *
 *Revision 1.2  2005/03/25 10:12:30  ninomi
 *ldbm-type.hをコンパイルできるようにしました。
 *
 *Revision 1.1  2005/03/25 06:50:59  ninomi
 *type managerを追加しました。
 *
 */
