/*
 * $Id: lildbm-basic.h,v 1.4 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-basic.h
//  Database Manager Interface
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_basic_h
#define __lildbm_basic_h

#include "lconfig.h"
#include "ftype.h"
#include "module.h"
#include "cell.h"
#include "structur.h"

#include <iomanip>
#include <iostream>
#include <list>
#include <vector>

/////////////////////////
/// basic definitions
///
namespace lilfes {
    namespace lildbm {
        
        enum t_ldbm {
            t_ldbm_env = 0,
            t_ldbm_file = 1,
            t_ldbm_db = 2,
            t_ldbm_index = 3
        };

            // Ldbm Class for Template Variables

        class lil;
        class ldb;
        class mem;
        class bdb;
        class pbi {};
        class dti {};

        template <class StorageT> class dtype {
        public:
            typedef uint32 serial_type;
        };
        template <> class dtype<mem> {
        public:
            typedef tserial serial_type;
        };
        template <> class dtype<bdb> {
        public:
            typedef uint32 serial_type;
        };
        class undef {
        public:
            typedef uint32 serial_type;
        };
    
        template <class LdbmC> class c_ldbm {};
        template <class LdbmC> class serial {
        public:
            typename LdbmC::serial_type v;
            serial(typename LdbmC::serial_type x) : v(x) {};
            ~serial() {};
        };
        template <class LdbmC> bool operator==(serial<LdbmC> x, serial<LdbmC> y) {
            return x.v == y.v;
        }
        template <class LdbmC> struct serial_hash {
            size_t operator()(serial<LdbmC> __x) const { return __x.v; };
        };
        template <class LdbmC> struct serial_equal_to {
            bool operator()(serial<LdbmC> x, serial<LdbmC> y) {
                return x.v == y.v;
            };
        };
        typedef uint32 dbt_serial;
            // instance of template variables in LdbmSchema
        class c_root {};
        class c_key {};
        class c_value {};
        class c_entry {
            typedef uint32 serial_type;
        };
        class c_dontcare {};
        class c_integer {};
        class c_inthash {};
        class c_float {};
        class c_string {};
        class c_type {};
        class c_fs {};
    
        template <class C, class D, class E> class triple {
        public:
            triple(C x, D y, E z) { first = x; second = y; third = z; };
            ~triple() {};
            C first;
            D second;
            E third;
        };
    }

////////////////////////
//// dtag

    namespace lildbm {
        enum dtag_t { 
            DT_PTR  =0,/// ポインタ
            DT_PTR1 =1,
            DT_PTR2 =2,
            DT_PTR3 =3,
            DT_STR  =4,/// structure
            DT_FSTR =5,/// freeze structure
            DT_VAR  =6,/// 変数
            DT_FVAR =7,/// freeze 変数
            DT_INT  =8,/// 整数
            DT_FLO  =9,/// 実数
            DT_CHR =10,/// 文字
            DT_STG =11,/// 文字列
            DT_NFT =12,/// 素性数
            DT_FTR =13,/// 素性
            DT_SYS =14 /// システム用データ
        };
    }

////////////////////////
//// definitions for bdbm interface for both bdbm mode and non-bdbm mode
////

    namespace lildbm {
    
        template <class StorageT> class LIT {
        public:
            typename StorageT::dcore_p pointer;
            serial<StorageT> entry_id;
            LIT() :entry_id(0) {};
            LIT(typename StorageT::dcore_p p) :entry_id(0) { pointer = p; };
            LIT(serial<StorageT> e) :entry_id(e) { };
            LIT(typename StorageT::dcore_p p, serial<StorageT> e) :entry_id(e) { pointer = p; };
            ~LIT() {};
        };
        template <> class LIT<ldb> { // LIT for LdbmDb<mem or bdb>
        public:
            mint entry_id;
            LIT() {};
            LIT(mint eid) : entry_id(eid) {};
            ~LIT() {};

        };
        template <> class LIT<lil> :public LIT<ldb> { // LIT for lilfes interface
        public:
            const type* db;
            LIT() {};
            LIT(const type* d, mint eid) : LIT<ldb>(eid), db(d) {};
            LIT(const type* d, LIT<ldb> lit): LIT<ldb>(lit), db(d) {};
            ~LIT() {};
        };
    
////////////////////////////////////////////
////  InstanceManager
////
        template <class C> class InstanceManager {
        private:
            std::vector<C*> table;
        public:
            serial<C> add() {
                table.push_back((C*) 0);
                return serial<C>(table.size() - 1);
            };
            serial<C> add(C* x) {
                table.push_back(x);
                return serial<C>(table.size() - 1);
            };
            bool set(serial<C> h, C* x) {
                if(h.v < table.size()) {
                    table[h.v] = x;
                    return true;
                }
                return false;
            };
            bool erase(serial<C> h) {
                if(h.v < table.size()) {
                    C* y = table[h.v];
                    if(! y) return false;
                    delete table[h.v];
                    table[h.v] = (C*) 0;
                    return true;
                }
                return false;
            };
            C* get(serial<C> h) {
                if(h.v < table.size())
                    return table[h.v];
                return NULL;
            };
            bool close() {
                typename std::vector<C*>::iterator it = table.begin(), last = table.end();
                for(; it != last ; it++) {
                    if(*it) (*it)->close();
                }
                return true;
            };
            InstanceManager() {};
            ~InstanceManager() {
                int n = table.size();
                for(int i = 0 ; i < n ; i++) {
                    if(table[i]) {
                        delete table[i];
                        table[i] = (C*) 0;
                    }
                }
            }
        };

/////
///// Instance Pointer
/////    
        template <class C> class InstancePointer {
        protected:
            InstanceManager<C> *instance_manager;
            serial<C> serial_no;
        public:
            InstancePointer(InstanceManager<C> *im, serial<C> sn) : instance_manager(im), serial_no(sn) {};
            InstancePointer(InstanceManager<C> &im, serial<C> sn) : instance_manager(&im), serial_no(sn) {};
            ~InstancePointer() {};
            C* get() { return instance_manager->get(serial_no); };
            bool erase() { return instance_manager->erase(serial_no); };
        
        };
    
    } // end of namespace lildbm
} // end of namespace lilfes

#endif // __lildbm_basic_h
// end of  lildbm-basic.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2004/04/23 02:42:56  ninomi
 * いらなくなったLDBM_CREATEなどのマクロ定義を消しました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
