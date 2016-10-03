/*
 * $Id: lildbm-dbtree.h,v 1.8 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-dbtree.h
//  LdbmDbTree
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_dbtree_h
#define __lildbm_dbtree_h

#include "lildbm-basic.h"
#include "lildbm-cell.h"

#include <iostream>
#include <list>
#include <ostream>
#include <utility>
#include <vector>

namespace lilfes {
    namespace lildbm {
        class LdbmSchemaBase;
        class DbTreeBase {
        public:
            virtual std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator, std::vector<FSP>::iterator) = 0;
            virtual triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator, std::vector<FSP>::iterator) = 0;
            virtual triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator, std::vector<FSP>::iterator) = 0;
            virtual void print() = 0;
            DbTreeBase() {};
            virtual ~DbTreeBase() {};
        };

            //template class
        template<class KorV, class DataT> class __DbTree : public DbTreeBase {};
        template<class DataT> class __DbTree<c_value, DataT> : public DbTreeBase {
        protected:
            DbTreeBase* next;
        public:
            __DbTree() {next = (DbTreeBase*)0; };
            ~__DbTree() {
                if(next) delete next;
            };
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry) {
                if(! next) return std::pair<bool, LIT<mem> >(false, LIT<mem>());
                schema++;
                entry++;
                return next->find(schema, entry);
            };
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry) {
                if(! next) return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
                schema++;
                entry++;
                triple<bool, bool, LIT<mem> > r = next->erase(schema, entry);
                if(r.second) { delete next; next = (DbTreeBase*) 0; }
                return r;
            };
        };

            // substance class
        template<class KorV, class DataT> class DbTree {};
        template <> class DbTree<c_root, c_dontcare> : public __DbTree<c_root, c_dontcare> {
        protected:
            DbTreeBase* next;
        public:
            DbTree() { next = (DbTreeBase*) 0; };
            ~DbTree() {
                if(next) delete next;
            };
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                std::cout << "(root";
                if(!next) {std::cout << "no next in root" << std::endl; return; }
                next->print();
                std::cout << ")" << std::endl;
            };
        };
        template <> class DbTree<c_entry, c_dontcare> : public __DbTree<c_entry, c_dontcare> {
        protected:
            bool allocated;
            LIT<mem> it;
        public:
            DbTree() { allocated = false; };
            ~DbTree() {};
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator, std::vector<FSP>::iterator);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator, std::vector<FSP>::iterator);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                std::cout << "(entry " << it.pointer << " " << it.entry_id.v << ")";
            };
        };
        template <> class DbTree<c_key, c_integer> : public __DbTree<c_key, c_integer> {
        protected:
            int base;
            int refc;
            std::vector<DbTreeBase*> next;
        public:
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                std::vector<DbTreeBase*>::iterator it = next.begin(), last = next.end();
                std::cout << "(key, integer";
                for(int i = 0; it != last ; it++, i++) {
                    if(*it) {
                        std::cout << "[" << i+base << " ";
                        (*it)->print();
                        std::cout << "]";
                    }
                }
                std::cout << ")";
            };
            DbTree(int b, int s) : next(s, (DbTreeBase*)0) {refc = 0; base = b;};
            ~DbTree() {
                std::vector<DbTreeBase*>::iterator it = next.begin(), last = next.end();
                for(; it != last ; it++)
                    if(*it) delete (*it);
            };
        };
        template <> class DbTree<c_key, c_inthash> : public __DbTree<c_key, c_inthash> {
        protected:
            typedef _HASHMAP<mint, DbTreeBase*, hash<mint>, equal_to<mint> > dbtree_hash;
            dbtree_hash next;
        public:
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                std::cout << "(key, inthash";
                for(; it != last ; it++) {
                    std::cout << "[" << it->first << " ";
                    it->second->print();
                    std::cout << "]";
                }
                std::cout << ")";
            };
            DbTree(int s) : next(s) {};
            ~DbTree() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                for(; it != last ; it++)
                    delete (it->second);
            };
        };
        template <> class DbTree<c_key, c_string> : public __DbTree<c_key, c_string> {
        protected:
            typedef _HASHMAP<std::vector<mem::dcell>*, DbTreeBase*, mem::hash_vcp, mem::equal_vcp> dbtree_hash;
            dbtree_hash next;
        public:
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                std::cout << "(key, string";
                for(; it != last ; it++) {
                    std::cout << "[";
                    it->second->print();
                    std::cout << "]";
                }
                std::cout << ")";
            };
        public:
            DbTree(int s) : next(s) {};
            ~DbTree() {
                std::vector<std::vector<mem::dcell>*> buf;
                dbtree_hash::iterator it = next.begin(), last = next.end();
                for(; it != last ; it++){
                    buf.push_back(it->first);
                    delete (it->second);
                }
                std::vector<std::vector<mem::dcell>*>::iterator bufit = buf.begin(), buflast = buf.end();
                for(; bufit != buflast ; bufit++) {
                    delete (*bufit);
                }
            }
        };
        template <> class DbTree<c_key, c_float> : public __DbTree<c_key, c_float> {
        protected:
            typedef _HASHMAP<mfloat, DbTreeBase*, hash<mfloat>, equal_to<mfloat> > dbtree_hash;
            dbtree_hash next;
        public:
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                std::cout << "(key, float";
                for(; it != last ; it++) {
                    std::cout << "[" << it->first << " ";
                    it->second->print();
                    std::cout << "]";
                }
                std::cout << ")";
            };
        public:
            DbTree(int s) : next(s) {};
            ~DbTree() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                for(; it != last ; it++)
                    delete (it->second);
            };
        };
        template <> class DbTree<c_key, c_type> : public __DbTree<c_key, c_type> {
        protected:
            typedef _HASHMAP<tserial, DbTreeBase*, hash<tserial>, equal_to<tserial> > dbtree_hash;
            dbtree_hash next;
        public:
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                std::cout << "(key, type";
                for(; it != last ; it++) {
                    std::cout << "[" << type::Serial(it->first)->GetName() << " ";
                    it->second->print();
                    std::cout << "]";
                }
                std::cout << ")";
            };
        public:
            DbTree(int s) : next(s) {};
            ~DbTree() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                for(; it != last ; it++)
                    delete (it->second);
            };
        };
        template <> class DbTree<c_key, c_fs> : public __DbTree<c_key, c_fs> {
        protected:
            typedef _HASHMAP<std::vector<mem::dcell>*, DbTreeBase*, mem::hash_vcp, mem::equal_vcp> dbtree_hash;
            dbtree_hash next;
        public:
            std::pair<bool, LIT<mem> > find(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem>* > insert(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            triple<bool, bool, LIT<mem> > erase(std::list<LdbmSchemaBase*>::iterator schema, std::vector<FSP>::iterator entry);
            void print() {
                dbtree_hash::iterator it = next.begin(), last = next.end();
                std::cout << "(key, fs";
                for(; it != last ; it++) {
                    std::cout << "[";
                    it->second->print();
                    std::cout << "]";
                }
                std::cout << ")";
            };
        public:
            DbTree(int s) : next(s) {};
            ~DbTree() {
                std::vector<std::vector<mem::dcell>*> buf;
                dbtree_hash::iterator it = next.begin(), last = next.end();
                for(; it != last ; it++){
                    buf.push_back(it->first);
                    delete (it->second);
                }
                std::vector<std::vector<mem::dcell>*>::iterator bufit = buf.begin(), buflast = buf.end();
                for(; bufit != buflast ; bufit++) {
                    delete (*bufit);
                }
            }
        };
        template <> class DbTree<c_value, c_integer> : public __DbTree<c_value, c_integer> {
            void print() {
                if(! next) return;
                std::cout << "(value, integer ";
                next->print();
                std::cout << ")";
                return;
            };
        };
        template <> class DbTree<c_value, c_float> : public __DbTree<c_value, c_float> {
            void print() {
                if(! next) return;
                std::cout << "(value, float ";
                next->print();
                std::cout << ")";
                return;
            };
        };
        template <> class DbTree<c_value, c_string> : public __DbTree<c_value, c_string> {
            void print() {
                if(! next) return;
                std::cout << "(value, string";
                next->print();
                std::cout << ")";
                return;
            };
        };
        template <> class DbTree<c_value, c_type> : public __DbTree<c_value, c_type> {
            void print() {
                if(! next) return;
                std::cout << "(value, type";
                next->print();
                std::cout << ")";
                return;
            };
        };
        template <> class DbTree<c_value, c_fs> : public __DbTree<c_value, c_fs> {
            void print() {
                if(! next) return;
                std::cout << "(value, fs";
                next->print();
                std::cout << ")";
                return;
            };
        };

    } // end of namespace lildbm
} // end of namespace lilfes

#endif // __lildbm_dbtree_h
// end of lildbm-dbtree.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2005/05/18 10:29:10  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.6  2004/08/09 13:34:57  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.5  2004/05/27 16:23:17  ninomi
 * デバッグ用coutを抜き忘れていました。
 *
 * Revision 1.4  2004/05/27 16:14:36  ninomi
 * DbTreeのdestructの時にkey_string, key_fsのキーが削除されていなかったの
 * で、するようにしました。
 *
 * Revision 1.3  2004/04/23 08:28:39  yusuke
 *
 * std:: をつけました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
