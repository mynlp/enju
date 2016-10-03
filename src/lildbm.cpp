/* $Id: lildbm.cpp,v 1.12 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

/////////////////////////////
//
//  lildbm.cpp
//  LiLFeS DB Manager
//


static const char rcsid[] = "$Id: lildbm.cpp,v 1.12 2011-05-02 10:38:23 matuzaki Exp $";

#include "lildbm.h"

#include <iostream>
#include <list>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

namespace lilfes {

using std::cout;
using std::endl;
using std::list;
using std::pair;
using std::set;
using std::vector;

    namespace lildbm {
            //////////////////////////////////////////////////////////////////////////////
            /// global variables

        extern module* m_ldbm;
        extern type *ldbm_info;
        extern type *ldbm_info_env;
        extern type *ldbm_info_file;
        extern type *ldbm_iterator;
        extern type *ldbm_pbi_iterator;
        extern type *ldbm_dti_iterator;
        extern type *ldbm_handle;
        extern type *ldbm_handle_env;
        extern type *ldbm_handle_file;
        extern type *ldbm_handle_index;
        extern feature *ldbm_db_f;
        extern feature *ldbm_entry_id_f;
        extern feature *ldbm_handle_id_f;


            //////////////////////////////////////////////////////////////////////////////
            // クラス LdbmManager のメンバ関数

        pair<bool, serial<LdbmEnvBase> > LdbmManager::openEnv(char* home, int mode, int pagesize, int cachesize, int flags) {
            LdbmEnvBase *e = new LdbmEnv();
            if(! e->open(home, mode, pagesize, cachesize, flags) ) { // fail to open
                RUNERR("cannot open environment home: " << home << " mode: " << mode << " pagesize: " << pagesize << " cachesize: " << cachesize << " flags: " << flags);
                delete e;
                return pair<bool, serial<LdbmEnvBase> >(false, serial<LdbmEnvBase>(0));
            }
            serial<LdbmEnvBase> h = env_manager.add(e);
            return pair<bool, serial<LdbmEnvBase> >(true, h);
        }
        bool LdbmManager::openDefaultEnv(int mode, int pagesize, int cachesize, int flags) {
            serial<LdbmEnvBase> h(0);
            if( env_manager.get(h) ) {
                RUNWARN("default environment is already configured or open");
                return false;
            }
            LdbmEnvBase *e = new LdbmEnv();
            if(! e->open(".", mode, pagesize, cachesize, flags) ) { // fail to open
                RUNERR("cannot open default environment mode: " << mode << " pagesize: " << pagesize << " cachesize: " << cachesize << " flags: " << flags);
                delete e;
                return false;
            }
            env_manager.set(h, e);
            return true;
        }
#ifdef WITH_BDBM
        bool LdbmManager::openDbBdb(serial<LdbmEnvBase> eh, char* filename, DBTYPE db_type, int mode, int flags, int cache_size, list<LdbmSchemaBase*> *sc, tserial t) {
	
            if(type2db.find(t) != type2db.end() ) {
                RUNERR(type::Serial(t)->GetName() << " is already open.");
                return false;
            }

            LdbmEnvBase *e = env_manager.get(eh);

            if(eh.v == 0) { // default environment
                if(! e ) {
                    if(! openDefaultEnv(mode, 1024, 1024 * 32, DB_CREATE | DB_INIT_MPOOL | DB_PRIVATE) )
                        return false;
                    e = env_manager.get(eh);
                }
            }
            if(! e ) return false;

            LdbmDb<bdb> *d = new LdbmDb<bdb>();
            if(! d->open(e, filename, db_type, mode, flags, cache_size, sc)) { // fail to open
                RUNWARN("cannot open db<bdb>: " << filename << " mode: " << mode << " cachesize: " << cache_size << " flags: " << flags);
                delete d;
                return false;
            }
            serial<LdbmDbBase> h = db_manager.add(d);
            type2db.insert(type2db_hash::value_type(type2db_hash::key_type(t),
                                                    type2db_hash::mapped_type(h)));

            e->addDb(InstancePointer<LdbmDbBase>(db_manager, h));
            return true;
        };
#endif //WITH_BDBM

        bool LdbmManager::openDbMem(list<LdbmSchemaBase*> *sc, tserial t) {
            if(type2db.find(t) != type2db.end() ) {
                RUNERR(type::Serial(t)->GetName() << " is already open.");
                return false;
            }
            LdbmDb<mem> *d = new LdbmDb<mem>();
            if(! d->open(sc)) { // fail to open
                RUNERR("cannot open db<mem>");
                delete d;
                return false;
            }
            serial<LdbmDbBase> h = db_manager.add(d);
            type2db.insert(type2db_hash::value_type(type2db_hash::key_type(t),
                                                    type2db_hash::mapped_type(h)));
            return true;
        }

        bool LdbmManager::closeDb(tserial t) {
            type2db_hash::iterator it = type2db.find(t);
            if(it == type2db.end()) {
                RUNERR(type::Serial(t)->GetName() << " is not defined or closed");
                return false;
            }
            serial<LdbmDbBase> h = it->second;
            LdbmDbBase* d = db_manager.get(h);
            if(!d || ! d->close() ) { return false; }
            type2db.erase(t);
            db_manager.erase(h);
            return true;
        };
    
        bool LdbmManager::closeEnv(serial<LdbmEnvBase> h) {
            LdbmEnvBase* e = env_manager.get(h);
            if( !e || ! e->close() ) return false;
            env_manager.erase(h);
            return true;
        };
    
        bool LdbmManager::closeDefaultEnv() {
            serial<LdbmEnvBase> h(0);
            LdbmEnvBase *e = env_manager.get(h);
            if( !e || ! e->close() ) return false;
            env_manager.erase(h);
            return true;
        };
    
        bool LdbmManager::syncDb(tserial t) {
            type2db_hash::iterator it = type2db.find(t);
            if(it == type2db.end() ) {
                RUNERR(type::Serial(t)->GetName() << " is not defined or closed");
                return false;
            }
            serial<LdbmDbBase> h = it->second;
            LdbmDbBase* d = db_manager.get(h);
            if(!d) return false;
            return d->sync();
        };
        bool LdbmManager::syncEnv(serial<LdbmEnvBase> h) {
            LdbmEnvBase* e = env_manager.get(h);
            if(!e) return false;
            return e->sync();
        };
        bool LdbmManager::syncDefaultEnv() {
            serial<LdbmEnvBase> h(0);
            LdbmEnvBase *e = env_manager.get(h);
            if(!e) return false;
            return e->sync();
        };
    
        LdbmDbBase* LdbmManager::getDb(tserial t) {
            type2db_hash::iterator it = type2db.find(t);
            if( it == type2db.end() ) {
                RUNERR(type::Serial(t)->GetName() << " is not defined or closed");
                return (LdbmDbBase*) 0;
            }
            serial<LdbmDbBase> h = it->second;
            return db_manager.get(h);
        };
        bool LdbmManager::find(FSP fs) {
            const type* t = fs.GetType();
            tserial ts = t->GetSerialNo();
            LdbmDbBase* db = getDb(ts);
            if(!db) return false;
            if(!t->IsSubType(pred)) return false;
            vector<FSP> arglist;
            int arity = fs.GetArity();
            for(int i = 0 ; i < arity ; i++)
                arglist.push_back(fs.Follow(f_arg[i+1]));
            return db->find(fs.GetMachine(), arglist);
        };
        bool LdbmManager::find(LIT<lil> lit, FSP fs) {
            LdbmDbBase* db = getDb(lit.db->GetSerialNo());
            if(!db) return false;
            const type* t = type::Serial(lit.db->GetSerialNo());
            if(!t->IsSubType(pred)) return false;
            
            int arity = fs.GetArity();
            vector<FSP> arglist;
            if(! db->find(fs.GetMachine(), (LIT<ldb>) lit, arglist)) return false;
            if(((int) arglist.size()) != arity) return false;
            for(int i = 0 ; i < arity ; i++)
                if(! fs.Follow(f_arg[i+1]).Unify(arglist[i])) return false;
            return true;
        };
        bool LdbmManager::insert(FSP fs) {
            const type* t = fs.GetType();
            tserial ts = t->GetSerialNo();
            LdbmDbBase* db = getDb(ts);
            if(!db) return false;
            if(!t->IsSubType(pred)) return false;
            vector<FSP> arglist;
            int arity = fs.GetArity();
            for(int i = 0 ; i < arity ; i++)
                arglist.push_back(fs.Follow(f_arg[i+1]));
            return db->insert(fs.GetMachine(), arglist);
        };
        bool LdbmManager::erase(FSP fs) {
            const type* t = fs.GetType();
            tserial ts = t->GetSerialNo();
            LdbmDbBase* db = getDb(ts);
            if(!db) return false;
            if(!t->IsSubType(pred)) return false;
            vector<FSP> arglist;
            int arity = fs.GetArity();
            for(int i = 0 ; i < arity ; i++)
                arglist.push_back(fs.Follow(f_arg[i+1]));
            return db->erase(fs.GetMachine(), arglist);
        };
        bool LdbmManager::update(FSP fs, FSP) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            return true;
        };
        bool LdbmManager::clear(FSP fs) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            return db->clear();
        };
        bool LdbmManager::findall(FSP fs1, FSP fs2) {
            machine* mach = fs1.GetMachine();
            const type* t = fs1.GetType();
            tserial ts = t->GetSerialNo();
            LdbmDbBase* db = getDb(ts);
            if(!db) return false;
            if(!t->IsSubType(pred)) return false;
            int arity = fs1.GetArity();
            vector<vector<FSP> > arglistlist;
            if(! db->findall(mach, arglistlist) ) return false;

            FSP root(mach);
            FSP r = root;
            vector<vector<FSP> >::iterator it = arglistlist.begin(), last = arglistlist.end();
            for(; it != last ; it++) {
                r.Coerce(cons);
                FSP entry = r.Follow(hd);
                entry.Coerce(t);
                vector<FSP>& arglist = *it;
                if(((int) arglist.size()) != arity) return false;
                for(int i = 0 ; i < arity ; i++)
                    entry.Follow(f_arg[i+1]).Unify(arglist[i]);
                r = r.Follow(tl);
            }
            r.Coerce(nil);
            return fs2.Unify(root);
        };
        pair<bool, LIT<lil> > LdbmManager::first(FSP fs1) {
            const type* t = fs1.GetType();
            LdbmDbBase* db = getDb(t->GetSerialNo());
            if(!db) return pair<bool, LIT<lil> >(false, LIT<lil>());
            pair<bool, LIT<ldb> > r = db->first();
            return pair<bool, LIT<lil> >(r.first, LIT<lil>(t, r.second));
        };
        pair<bool, LIT<lil> > LdbmManager::last(FSP fs1) {
            const type* t = fs1.GetType();
            LdbmDbBase* db = getDb(t->GetSerialNo());
            if(!db) return pair<bool, LIT<lil> >(false, LIT<lil>());
            pair<bool, LIT<ldb> > r = db->last();
            return pair<bool, LIT<lil> >(r.first, LIT<lil>(t, r.second));
        };
        pair<bool, LIT<lil> > LdbmManager::next(LIT<lil> lit) {
            LdbmDbBase* db = getDb(lit.db->GetSerialNo());
            if(!db) return pair<bool, LIT<lil> >(false, LIT<lil>());
            pair<bool, LIT<ldb> > r = db->next((LIT<ldb>) lit);
            return pair<bool, LIT<lil> >(r.first, LIT<lil>(lit.db, r.second));
        };
        pair<bool, LIT<lil> > LdbmManager::prev(LIT<lil> lit) {
            LdbmDbBase* db = getDb(lit.db->GetSerialNo());
            if(!db) return pair<bool, LIT<lil> >(false, LIT<lil>());
            pair<bool, LIT<ldb> > r = db->prev((LIT<ldb>) lit);
            return pair<bool, LIT<lil> >(r.first, LIT<lil>(lit.db, r.second));
        };
        bool LdbmManager::save(const char* filename, FSP fs) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            return db->save(filename);
        };
        bool LdbmManager::load(const char* filename, FSP fs) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            return db->load(fs.GetMachine(), filename);
        };
        void LdbmManager::print_statistics() {
            type2db_hash::iterator it = type2db.begin(), last = type2db.end();
            for(; it != last ; it++) {
                cout << "statistics of " << type::Serial(it->first)->GetName() << " start" << endl;
                LdbmDbBase* d = db_manager.get(it->second);
                if(d)
                    d->print_statistics();
                cout << "statistics of " << type::Serial(it->first)->GetName() << " end" << endl;
            }
        }
        bool LdbmManager::reserve(FSP fs, size_t sz) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            db->reserve((size_t) sz);
            return true;
        }
        size_t LdbmManager::size(FSP fs) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            return db->size();
        }
        size_t LdbmManager::capacity(FSP fs) {
            tserial t = fs.GetType()->GetSerialNo();
            LdbmDbBase* db = getDb(t);
            if(!db) return false;
            return db->capacity();
        }
    } // end of namespace lildbm
} // end of namespace lilfes

// end of lildbm.cpp

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2004/08/10 14:36:29  yusuke
 *
 * db_findall が常に nil を返すようになっていた。
 *
 * Revision 1.10  2004/08/09 13:35:07  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.9  2004/07/26 05:34:27  ninomi
 * C++から直接DBを操作しやすくなるように一部修正した
 *
 * Revision 1.8  2004/06/01 15:25:01  ninomi
 * db_reserve, db_size, db_capacity
 * db_reserve_k, db_size_k, db_capacity_k
 * db_reserve_m, db_size_m, db_capacity_mを加えました。
 *
 * Revision 1.7  2004/05/27 12:59:18  ninomi
 * print_statisticsというbuilt-inをくわえました。
 *
 * Revision 1.6  2004/04/22 07:48:49  ninomi
 * lildbmにdb_saveとdb_loadが加わりました。
 *
 * Revision 1.5  2004/04/20 10:22:41  ninomi
 * save, load機能追加中
 *
 * Revision 1.4  2004/04/19 08:49:21  ninomi
 * sys/waitのincludeの順番によってコンパイルできない現象のバグをとりました。
 * profile.hの中のlilfes namespace内でsignal.hをincludeしているのが原因でした。
 *
 * Revision 1.3  2004/04/14 12:48:38  ninomi
 * Berkeley DB依存のデータや、lilfesの特定のタイプ依存のデータを
 * lildbm-interface.cppに移しました。
 *
 * Revision 1.2  2004/04/14 04:53:04  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
