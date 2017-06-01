/* $Id: lildbm-interface.cpp,v 1.22 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 2001--2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

/////////////////////////////
//
//  lildbm-lil.cpp
//  lilfes database manager -- interface for lilfes
//

#include "lildbm.h"
#include "builtin.h"

#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif
#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#include <iostream>
#include <list>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

////////////////////////////////
//// global variables
////

namespace lilfes {

using std::cout;
using std::endl;
using std::list;
using std::pair;
using std::string;
using std::vector;

    namespace lildbm {
        DbTypeManager<mem> *dtm;
        LdbmManager *ldbm;

        type *hash_key;
        type *key_integer;
        type *key_inthash;
        type *key_float;
        type *key_string;
        type *key_type;
        type *key_fs;
        type *array_value_integer;
        type *array_value_float;
        type *array_value_string;
        type *array_value_type;
        type *array_value_fs;
        
        module* m_ldbm;
        type *ldbm_info, *ldbm_info_env, *ldbm_info_db;
        type *ldbm_iterator;
        type *ldbm_handle, *ldbm_handle_env, *ldbm_handle_file, *ldbm_handle_index;
        feature *ldbm_db_f, *ldbm_entry_id_f, *ldbm_handle_id_f;

        bool print_statistics(machine&, FSP) {
            P1("LDBM");
            ldbm->print_statistics();
            return true;
        }
        bool db_reserve(machine&, FSP db, FSP sz) {
            if(! sz.IsInteger() ) { RUNWARN("2nd argument of db_reserve must be the number of cells in core"); return false; }
            return ldbm->reserve(db, sz.ReadInteger());
        }
        bool db_size(machine& m, FSP db, FSP sz) {
            return sz.Unify(FSP(m, (mint) ldbm->size(db)));
        }
        bool db_capacity(machine& m, FSP db, FSP sz) {
            return sz.Unify(FSP(m, (mint) ldbm->capacity(db)));
        }
        bool db_reserve_k(machine&, FSP db, FSP sz) {
            if(! sz.IsInteger() ) { RUNWARN("2nd argument of db_reserve must be the number of cells in core"); return false; }
            return ldbm->reserve(db, ((size_t) sz.ReadInteger()) * 1000);
        }
        bool db_size_k(machine& m, FSP db, FSP sz) {
            return sz.Unify(FSP(m, (mint) (ldbm->size(db) / 1000)));
        }
        bool db_capacity_k(machine& m, FSP db, FSP sz) {
            return sz.Unify(FSP(m, (mint) (ldbm->capacity(db) / 1000)));
        }
        bool db_reserve_m(machine&, FSP db, FSP sz) {
            if(! sz.IsInteger() ) { RUNWARN("2nd argument of db_reserve must be the number of cells in core"); return false; }
            return ldbm->reserve(db, ((size_t) sz.ReadInteger()) * 1000000);
        }
        bool db_size_m(machine& m, FSP db, FSP sz) {
            return sz.Unify(FSP(m, (mint) (ldbm->size(db) / 1000000)));
        }
        bool db_capacity_m(machine& m, FSP db, FSP sz) {
            return sz.Unify(FSP(m, (mint) (ldbm->capacity(db) / 1000000)));
        }
#define DefaultCacheSize (1048576 * 10)

        list<LdbmSchemaBase*>* newSchema() {
            return new list<LdbmSchemaBase*>();
        }
        
        bool convertSchema(FSP sc, list<LdbmSchemaBase*>* rsc) {

            if( ! sc.GetType()->IsSubType(pred)) return false;

            int arity = sc.GetArity();
    
            for(int i = 0 ; i < arity ; i++) {
                FSP f = sc.Follow(f_arg[i+1]);

                const type *kt = f.GetType();

                if( kt == key_integer ) {

                    FSP b = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_BASE\\")));
                    FSP s = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_SIZE\\")));
                    if( !b.IsInteger() || !s.IsInteger() ) {
                        RUNERR("Error: both ARRAY_BASE and ARRAY_SIZE must be specified for key_integer");
                        return false;
                    } else {
                        if ( s.ReadInteger() > 0 ) {
                            LdbmSchema<c_key, c_integer>* x = new LdbmSchema<c_key, c_integer>(b.ReadInteger(), s.ReadInteger());
                            rsc->push_back(x);
                        } else {
                            RUNERR("Error: ARRAY_SIZE must be a positive integer");
                            return false;
                        }
                    }
                    
                } else if( kt == key_inthash) {

                    int isize = INITIALHASHSIZE;
                    if( f.IsNode() ) {
                        FSP s = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_SIZE\\")));
                        if( !s.IsInteger() ) {
                            RUNERR("Error: ARRAY_SIZE must be integer");
                            return false;
                        }
                        if ( s.ReadInteger() > 0 ) {
                            isize = s.ReadInteger();
                        } else {
                            RUNERR("Error: ARRAY_SIZE must be a positive integer");
                            return false;
                        }
                    }
                    LdbmSchema<c_key, c_inthash>* x = new LdbmSchema<c_key, c_inthash>(isize);
                    rsc->push_back(x);
                    
                } else if( kt == key_string) {
                    int isize = INITIALHASHSIZE;
                    if( f.IsNode() ) {
                        FSP s = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_SIZE\\")));
                        if( !s.IsInteger() ) {
                            RUNERR("Error: ARRAY_SIZE must be integer");
                            return false;
                        }
                        if ( s.ReadInteger() > 0 ) {
                            isize = s.ReadInteger();
                        } else {
                            RUNERR("Error: ARRAY_SIZE must be a positive integer");
                            return false;
                        }
                    }
                    LdbmSchema<c_key, c_string>* x = new LdbmSchema<c_key, c_string>(isize);
                    rsc->push_back(x);

                } else if( kt == key_float) {

                    int isize = INITIALHASHSIZE;
                    if( f.IsNode() ) {
                        FSP s = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_SIZE\\")));
                        if( !s.IsInteger() ) {
                            RUNERR("Error: ARRAY_SIZE must be integer");
                            return false;
                        }
                        if ( s.ReadInteger() > 0 ) {
                            isize = s.ReadInteger();
                        } else {
                            RUNERR("Error: ARRAY_SIZE must be a positive integer");
                            return false;
                        }
                    }
                    LdbmSchema<c_key, c_float>* x = new LdbmSchema<c_key, c_float>(isize);
                    rsc->push_back(x);
                    
                } else if( kt == key_type) {
                    int isize = INITIALHASHSIZE;
                    if( f.IsNode() ) {
                        FSP s = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_SIZE\\")));
                        if( !s.IsInteger() ) {
                            RUNERR("Error: ARRAY_SIZE must be integer");
                            return false;
                        }
                        if ( s.ReadInteger() > 0 ) {
                            isize = s.ReadInteger();
                        } else {
                            RUNERR("Error: ARRAY_SIZE must be a positive integer");
                            return false;
                        }
                    }
                    LdbmSchema<c_key, c_type>* x = new LdbmSchema<c_key, c_type>(isize);
                    rsc->push_back(x);

                } else if( kt == key_fs) {

                    int isize = INITIALHASHSIZE;
                    if( f.IsNode() ) {
                        FSP s = f.Follow(feature::Search(module::BuiltinModule()->Search("ARRAY_SIZE\\")));
                        if( !s.IsInteger() ) {
                            RUNERR("Error: ARRAY_SIZE must be integer");
                            return false;
                        }
                        if ( s.ReadInteger() > 0 ) {
                            isize = s.ReadInteger();
                        } else {
                            RUNERR("Error: ARRAY_SIZE must be a positive integer");
                            return false;
                        }
                    }
                    LdbmSchema<c_key, c_fs>* x = new LdbmSchema<c_key, c_fs>(isize);
                    rsc->push_back(x);
                    
                } else if( kt == array_value_integer) {
                    LdbmSchema<c_value, c_integer>* x = new LdbmSchema<c_value, c_integer>();
                    rsc->push_back(x);
                } else if( kt == array_value_float) {
                    LdbmSchema<c_value, c_float>* x = new LdbmSchema<c_value, c_float>();
                    rsc->push_back(x);
                } else if( kt == array_value_string) {
                    LdbmSchema<c_value, c_string>* x = new LdbmSchema<c_value, c_string>();
                    rsc->push_back(x);
                } else if( kt == array_value_type) {
                    LdbmSchema<c_value, c_type>* x = new LdbmSchema<c_value, c_type>();
                    rsc->push_back(x);
                } else if( kt == array_value_fs) {
                    LdbmSchema<c_value, c_fs>* x = new LdbmSchema<c_value, c_fs>();
                    rsc->push_back(x);
                } else {
                    cout << "Error: predicate db_open requires keys or array_values as arguments" << endl;
                    cout << sc.DisplayAVM();
                    cout << f.DisplayAVM();
                    cout << kt->GetName();
                    return false;
                }
            }
            return true;
        };
        
        bool fixSchema(list<LdbmSchemaBase*> *sc) {
            LdbmSchema<c_entry, c_dontcare>* x = new LdbmSchema<c_entry, c_dontcare>();
            sc->push_back(x);
            return true;
        };

        bool open_db(machine&, FSP arg1) {
            list<LdbmSchemaBase*> *sc = newSchema();
            if(! convertSchema(arg1, sc) ) return false;
            if(! fixSchema(sc) ) return false;
            tserial pred_t = arg1.GetType()->GetSerialNo();
            return ldbm->openDbMem(sc, pred_t);
        }

        bool close(machine&, FSP arg1) {
            P1("LDBM");
            const type* t = arg1.GetType();
            if( t->IsSubType(pred) ) { // db
                if(! ldbm->closeDb(t->GetSerialNo())) {
                    RUNWARN("db_close/1: fail to close db");
                    return false;
                }
            } else if( t->IsSubType(ldbm_handle_env) ) {
                if(! arg1.Follow(ldbm_handle_id_f).IsInteger()) {
                    RUNWARN("db_close/1: invalid value of ldbm:handle_ID\\");
                    return false;
                }
                if(! ldbm->closeEnv(serial<LdbmEnvBase>(arg1.Follow(ldbm_handle_id_f).ReadInteger())) ) {
                    RUNWARN("db_close/1: fail to close env");
                    return false;
                }
            } else {
                RUNWARN("1st argument of db_close/1 must be ldbm:handle_env, ldbm:handle_file, ldbm:handle_index or pred");
                return false;
            }
            return true;
        }
        
        bool sync(machine&, FSP arg1) {
            P1("LDBM");
            const type* t = arg1.GetType();
            if( t->IsSubType(pred) ) { // db
                if(! ldbm->syncDb(t->GetSerialNo())) {
                    RUNWARN("ldbm_sync/1: fail to sync db");
                    return false;
                }
            } else if( t->IsSubType(ldbm_handle_env) ) {
                if(! arg1.Follow(ldbm_handle_id_f).IsInteger()) {
                    RUNWARN("ldbm_sync/1: invalid value of ldbm:handle_ID\\");
                    return false;
                }
                if(! ldbm->syncEnv(serial<LdbmEnvBase>(arg1.Follow(ldbm_handle_id_f).ReadInteger())) ) {
                    RUNWARN("ldbm_sync/1: fail to sync env");
                    return false;
                }
            } else {
                RUNWARN("1st argument of ldbm_sync/1 must be ldbm:handle_env, ldbm:handle_file, ldbm:handle_index or pred");
                return false;
            }
            return true;
        }

        bool find(machine&, FSP arg1) {
            P1("LDBM");
            return ldbm->find(arg1);
        }    

        bool find(machine&, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(! arg1.GetType()->IsSubType(ldbm_iterator)) return false;
            const type* t = arg1.Follow(ldbm_db_f).GetType();
            FSP eid = arg1.Follow(ldbm_entry_id_f);
            if(! arg2.Coerce(t)) return false;
            if(! eid.IsInteger()) return false;
	
            return ldbm->find(LIT<lil>(t, eid.ReadInteger()), arg2);
        }    

        bool insert(machine&, FSP arg1) {
            P1("LDBM");
            return ldbm->insert(arg1);
        }

        bool erase(machine&, FSP arg1) {
            P1("LDBM");
            return ldbm->erase(arg1);
        }

        bool update(machine&, FSP arg1, FSP arg2) {
            P1("LDBM");
            return ldbm->update(arg1, arg2);
        }

        bool clear(machine&, FSP arg1) {
            P1("LDBM");
            return ldbm->clear(arg1);
        }

        bool findall(machine&, FSP arg1, FSP arg2) {
            P1("LDBM");
            return ldbm->findall(arg1, arg2);
        }
        bool first(machine& m, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(!arg2.Coerce(ldbm_iterator)) return false;
            pair<bool, LIT<lil> > lit = ldbm->first(arg1);
            if(! lit.first) return false;
            if(arg2.Follow(ldbm_db_f).Coerce(lit.second.db) &&
               arg2.Follow(ldbm_entry_id_f).Unify(FSP(m, lit.second.entry_id)))
                return true;
            return false;
        }

        bool last(machine& m, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(!arg2.Coerce(ldbm_iterator)) return false;
            pair<bool, LIT<lil> > lit = ldbm->last(arg1);
            if(! lit.first) return false;
            if(arg2.Follow(ldbm_db_f).Coerce(lit.second.db) &&
               arg2.Follow(ldbm_entry_id_f).Unify(FSP(m, lit.second.entry_id)))
                return true;
            return false;
        }

        bool next(machine& m, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(! arg1.GetType()->IsSubType(ldbm_iterator)) return false;
            const type* t = arg1.Follow(ldbm_db_f).GetType();
            FSP eid = arg1.Follow(ldbm_entry_id_f);
            if(! eid.IsInteger()) return false;
	
            pair<bool, LIT<lil> > lit0 = ldbm->next(LIT<lil>(t, eid.ReadInteger()));
            if(!lit0.first) return false;
            LIT<lil>& lit = lit0.second;
            if(! arg2.Coerce(ldbm_iterator)) return false;
            if(arg2.Follow(ldbm_db_f).Coerce(lit.db) &&
               arg2.Follow(ldbm_entry_id_f).Unify(FSP(m, lit.entry_id)))
                return true;
            return false;
        }

        bool prev(machine& m, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(! arg1.GetType()->IsSubType(ldbm_iterator)) return false;
            const type* t = arg1.Follow(ldbm_db_f).GetType();
            FSP eid = arg1.Follow(ldbm_entry_id_f);
            if(! eid.IsInteger()) return false;
	
            pair<bool, LIT<lil> > lit0 = ldbm->prev(LIT<lil>(t, eid.ReadInteger()));
            if(!lit0.first) return false;
            LIT<lil>& lit = lit0.second;
            if(! arg2.Coerce(ldbm_iterator)) return false;
            if(arg2.Follow(ldbm_db_f).Coerce(lit.db) &&
               arg2.Follow(ldbm_entry_id_f).Unify(FSP(m, lit.entry_id)))
                return true;
            return false;
        }
        
        bool save(machine&, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(! arg1.IsString() ) {
                RUNWARN("the first argument of db_save is of string type");
                return false;
            }
            return ldbm->save(arg1.ReadString(), arg2);
        };
        bool load(machine&, FSP arg1, FSP arg2) {
            P1("LDBM");
            if(! arg1.IsString() ) {
                RUNWARN("the first argument of db_load is of string type");
                return false;
            }
            return ldbm->load(arg1.ReadString(), arg2);
        };

#ifdef WITH_BDBM
        pair<bool, DBTYPE> get_type(FSP arg) {

            if (! arg.IsString() ) {
                return pair<bool, DBTYPE>(true, DB_BTREE);
            } else {
                char* x = arg.ReadString();
                if ( strcmp( x, "BTREE" ) == 0 ) {
                    return pair<bool, DBTYPE>(true, DB_BTREE);
                } else if ( strcmp( x, "HASH" ) == 0 ) {
                    return pair<bool, DBTYPE>(true, DB_HASH);
                } else {
                    return pair<bool, DBTYPE>(false, DB_BTREE);
                }
            }
        }

        pair<bool, int> get_env_flags(FSP arg) {
            int flags = 0;
            if ( arg.IsString() ) {
                char* dbm_type = arg.ReadString();

                if( strcmp( dbm_type, "AUTO_COMMIT") == 0) {
                    flags |= DB_AUTO_COMMIT;
                } else if( strcmp( dbm_type, "CREATE") == 0) {
                    flags |= DB_CREATE;
                } else if( strcmp( dbm_type, "DIRTY_READ") == 0) {
                    flags |= DB_DIRTY_READ;
                } else if( strcmp( dbm_type, "EXCL") == 0) {
                    flags |= DB_EXCL;
                } else if( strcmp( dbm_type, "NOMMAP") == 0) {
                    flags |= DB_NOMMAP;
                } else if( strcmp( dbm_type, "RDONLY") == 0) {
                    flags |= DB_RDONLY;
                } else if( strcmp( dbm_type, "THREAD") == 0) {
                    flags |= DB_THREAD;
                } else if( strcmp( dbm_type, "TRUNCATE") == 0) {
                    flags |= DB_TRUNCATE;
                } else if( strcmp( dbm_type, "INIT_CDB") == 0) {
                    flags |= DB_INIT_CDB;
                } else if( strcmp( dbm_type, "CLIENT") == 0) {
                    flags |= DB_CLIENT;
                } else if( strcmp( dbm_type, "INIT_LOCK") == 0) {
                    flags |= DB_INIT_LOCK;
                } else if( strcmp( dbm_type, "INIT_LOG") == 0) {
                    flags |= DB_INIT_LOG;
                } else if( strcmp( dbm_type, "INIT_MPOOL") == 0) {
                    flags |= DB_INIT_MPOOL;
                } else if( strcmp( dbm_type, "INIT_TXN") == 0) {
                    flags |= DB_INIT_TXN;
                } else if( strcmp( dbm_type, "JOINENV") == 0) {
                    flags |= DB_JOINENV;
                } else if( strcmp( dbm_type, "LOCKDOWN") == 0) {
                    flags |= DB_LOCKDOWN;
                } else if( strcmp( dbm_type, "PRIVATE") == 0) {
                    flags |= DB_PRIVATE;
                } else if( strcmp( dbm_type, "RECOVER_FATAL") == 0) {
                    flags |= DB_RECOVER_FATAL;
                } else if( strcmp( dbm_type, "SYSTEM_MEM") == 0) {
                    flags |= DB_SYSTEM_MEM;
                } else if( strcmp( dbm_type, "CXX_NO_EXCEPTIONS") == 0) {
                    flags |= DB_CXX_NO_EXCEPTIONS;
                } else if( strcmp( dbm_type, "FORCE") == 0) {
                    flags |= DB_FORCE;
                } else if( strcmp( dbm_type, "RECOVER") == 0) {
                    flags |= DB_RECOVER;
                } else if( strcmp( dbm_type, "TXN_NOSYNC") == 0) {
                    flags |= DB_TXN_NOSYNC;
                } else if( strcmp( dbm_type, "USE_ENVIRON") == 0) {
                    flags |= DB_USE_ENVIRON;
                } else if( strcmp( dbm_type, "USE_ENVIRON_ROOT") == 0) {
                    flags |= DB_USE_ENVIRON_ROOT;
                } else {
                    return pair<bool, int>(false, flags);
                }
            } else {
                while ( arg.GetType() != nil ) {
                    if ( arg.GetType() == cons ) {
                        FSP p = arg.Follow( hd );
                        arg = arg.Follow( tl );
                        if ( p.IsString() ) {
                            char* dbm_type = p.ReadString();

                            if( strcmp( dbm_type, "AUTO_COMMIT") == 0) {
                                flags |= DB_AUTO_COMMIT;
                            } else if( strcmp( dbm_type, "CREATE") == 0) {
                                flags |= DB_CREATE;
                            } else if( strcmp( dbm_type, "DIRTY_READ") == 0) {
                                flags |= DB_DIRTY_READ;
                            } else if( strcmp( dbm_type, "EXCL") == 0) {
                                flags |= DB_EXCL;
                            } else if( strcmp( dbm_type, "NOMMAP") == 0) {
                                flags |= DB_NOMMAP;
                            } else if( strcmp( dbm_type, "RDONLY") == 0) {
                                flags |= DB_RDONLY;
                            } else if( strcmp( dbm_type, "THREAD") == 0) {
                                flags |= DB_THREAD;
                            } else if( strcmp( dbm_type, "TRUNCATE") == 0) {
                                flags |= DB_TRUNCATE;
                            } else if( strcmp( dbm_type, "INIT_CDB") == 0) {
                                flags |= DB_INIT_CDB;
                            } else if( strcmp( dbm_type, "CLIENT") == 0) {
                                flags |= DB_CLIENT;
                            } else if( strcmp( dbm_type, "INIT_LOCK") == 0) {
                                flags |= DB_INIT_LOCK;
                            } else if( strcmp( dbm_type, "INIT_LOG") == 0) {
                                flags |= DB_INIT_LOG;
                            } else if( strcmp( dbm_type, "INIT_MPOOL") == 0) {
                                flags |= DB_INIT_MPOOL;
                            } else if( strcmp( dbm_type, "INIT_TXN") == 0) {
                                flags |= DB_INIT_TXN;
                            } else if( strcmp( dbm_type, "JOINENV") == 0) {
                                flags |= DB_JOINENV;
                            } else if( strcmp( dbm_type, "LOCKDOWN") == 0) {
                                flags |= DB_LOCKDOWN;
                            } else if( strcmp( dbm_type, "PRIVATE") == 0) {
                                flags |= DB_PRIVATE;
                            } else if( strcmp( dbm_type, "RECOVER_FATAL") == 0) {
                                flags |= DB_RECOVER_FATAL;
                            } else if( strcmp( dbm_type, "SYSTEM_MEM") == 0) {
                                flags |= DB_SYSTEM_MEM;
                            } else if( strcmp( dbm_type, "CXX_NO_EXCEPTIONS") == 0) {
                                flags |= DB_CXX_NO_EXCEPTIONS;
                            } else if( strcmp( dbm_type, "FORCE") == 0) {
                                flags |= DB_FORCE;
                            } else if( strcmp( dbm_type, "RECOVER") == 0) {
                                flags |= DB_RECOVER;
                            } else if( strcmp( dbm_type, "TXN_NOSYNC") == 0) {
                                flags |= DB_TXN_NOSYNC;
                            } else if( strcmp( dbm_type, "USE_ENVIRON") == 0) {
                                flags |= DB_USE_ENVIRON;
                            } else if( strcmp( dbm_type, "USE_ENVIRON_ROOT") == 0) {
                                flags |= DB_USE_ENVIRON_ROOT;
                            } else {
                                return pair<bool, int>(false, flags);
                            }

                            continue;
                        }
                    }
                    return pair<bool, int>(false, flags);
                }
            }
            return pair<bool, int>(true, flags);
        }

        pair<bool, int> get_flags(FSP arg) {
            int flags = 0;
            if ( arg.IsString() ) {
                char* dbm_type = arg.ReadString();
        
                if ( strcmp( dbm_type, "AUTO_COMMIT" ) == 0 ) {
                    flags |= DB_AUTO_COMMIT;
                } else if ( strcmp( dbm_type, "CREATE" ) == 0 ) {
                    flags |= DB_CREATE;
                } else if ( strcmp( dbm_type, "DIRTY_READ" ) == 0 ) {
                    flags |= DB_DIRTY_READ;
                } else if ( strcmp( dbm_type, "EXCL" ) == 0 ) {
                    flags |= DB_EXCL;
                } else if ( strcmp( dbm_type, "NOMMAP" ) == 0 ) {
                    flags |= DB_NOMMAP;
                } else if ( strcmp( dbm_type, "RDONLY" ) == 0 ) {
                    flags |= DB_RDONLY;
                } else if ( strcmp( dbm_type, "THREAD" ) == 0 ) {
                    flags |= DB_THREAD;
                } else if ( strcmp( dbm_type, "TRUNCATE" ) == 0 ) {
                    flags |= DB_TRUNCATE;
                }
            } else {
                while ( arg.GetType() != nil ) {
                    if ( arg.GetType() == cons ) {
                        FSP p = arg.Follow( hd );
                        arg = arg.Follow( tl );
                        if ( p.IsString() ) {
                            char* dbm_type = p.ReadString();
                            if ( strcmp( dbm_type, "AUTO_COMMIT" ) == 0 ) {
                                flags |= DB_AUTO_COMMIT;
                            } else if ( strcmp( dbm_type, "CREATE" ) == 0 ) {
                                flags |= DB_CREATE;
                            } else if ( strcmp( dbm_type, "DIRTY_READ" ) == 0 ) {
                                flags |= DB_DIRTY_READ;
                            } else if ( strcmp( dbm_type, "EXCL" ) == 0 ) {
                                flags |= DB_EXCL;
                            } else if ( strcmp( dbm_type, "NOMMAP" ) == 0 ) {
                                flags |= DB_NOMMAP;
                            } else if ( strcmp( dbm_type, "RDONLY" ) == 0 ) {
                                flags |= DB_RDONLY;
                            } else if ( strcmp( dbm_type, "THREAD" ) == 0 ) {
                                flags |= DB_THREAD;
                            } else if ( strcmp( dbm_type, "TRUNCATE" ) == 0 ) {
                                flags |= DB_TRUNCATE;
                            } else {
                                return pair<bool, int>(false, flags);
                            }
                            continue;
                        }
                    }
                    return pair<bool, int>(false, flags);
                }
            }
            return pair<bool, int>(true, flags);
        }
        bool open_env(machine& m, FSP arg1, FSP arg2, FSP arg3) {
            char* home;
            int mode;
            int bdbmpagesize;
            int bdbmcachesize;
            int flags;
        
            if(! arg1.IsString() ) {
                RUNWARN("1st argument of db_open_env/3 must be string");
                return false;
            }
            if(! arg2.Coerce(ldbm_info_env)) {
                RUNWARN("2nd argument of db_open_env/3 must be compatible with ldbm:info_env");
                return false;
            }
            if(! arg3.Coerce(ldbm_handle_env)) {
                RUNWARN("3rd argument of db_open_env/3 must be compatible with ldbm:handle_env");
                return false;
            }

            FSP fsp_home = arg1;
            FSP fsp_mode = arg2.Follow(m_ldbm->Search("mode\\"));
            FSP fsp_bdbmpagesize = arg2.Follow(m_ldbm->Search("bdbm_page_size\\"));
            FSP fsp_bdbmcachesize = arg2.Follow(m_ldbm->Search("bdbm_cache_size\\"));
            FSP fsp_flags = arg2.Follow(m_ldbm->Search("flags\\"));
    
            home = fsp_home.ReadString();
        
            struct stat s;
            bool directory_exist = (!stat(home, &s) && (s.st_mode & S_IFDIR));
        
            if(!directory_exist)  {
                if( mkdir(home, 0777) < 0) {
                    RUNERR("fail to make database home directory");
                }
            }
        
            if( fsp_mode.IsInteger() ) {
                mode = fsp_mode.ReadInteger();
            } else {
                mode = 0;
            }
            if( fsp_bdbmpagesize.IsInteger() ) {
                bdbmpagesize = fsp_bdbmpagesize.ReadInteger();
            } else {
                bdbmpagesize = 1024;
            }
            if( fsp_bdbmcachesize.IsInteger() ) {
                bdbmcachesize = fsp_bdbmcachesize.ReadInteger();
            } else {
                bdbmcachesize = bdbmpagesize * 32;
            }

            pair<bool, int> __flags = get_flags(fsp_flags);
            if(__flags.first) {
                flags = __flags.second;
            } else {
                RUNERR( "Invalid flags: " );
                return false;
            }

            pair<bool, serial<LdbmEnvBase> > r = ldbm->openEnv(home, mode, bdbmpagesize, bdbmcachesize, flags);
            if(! r.first) return false;
            FSP handle(m, (mint) r.second.v);
            return arg3.Follow(ldbm_handle_id_f).Unify(handle);
        }
    
        bool open_env(machine&, FSP arg1) {
        
            int mode;
            int bdbmpagesize;
            int bdbmcachesize;
            int flags;
        
            if(! arg1.Coerce(ldbm_info_env)) {
                RUNWARN("1st argument of db_config/1 must be compatible with ldbm:info_env");
                return false;
            }
        
            FSP fsp_mode = arg1.Follow(m_ldbm->Search("mode\\"));
            FSP fsp_bdbmpagesize = arg1.Follow(m_ldbm->Search("bdbm_page_size\\"));
            FSP fsp_bdbmcachesize = arg1.Follow(m_ldbm->Search("bdbm_cache_size\\"));
            FSP fsp_flags = arg1.Follow(m_ldbm->Search("flags\\"));
    
            if( fsp_mode.IsInteger() ) {
                mode = fsp_mode.ReadInteger();
            } else {
                mode = 0;
            }
            if( fsp_bdbmpagesize.IsInteger() ) {
                bdbmpagesize = fsp_bdbmpagesize.ReadInteger();
            } else {
                bdbmpagesize = 1024;
            }
            if( fsp_bdbmcachesize.IsInteger() ) {
                bdbmcachesize = fsp_bdbmcachesize.ReadInteger();
            } else {
                bdbmcachesize = bdbmpagesize * 32;
            }

            pair<bool, int> __flags = get_flags(fsp_flags);
            if(__flags.first) {
                flags = __flags.second;
            } else {
                RUNERR( "Invalid flags: " );
                return false;
            }

            return ldbm->openDefaultEnv(mode, bdbmpagesize, bdbmcachesize, flags);
        }

        bool open_db(machine&, FSP arg1, FSP arg2, FSP arg3) {
                // file version
            DBTYPE db_type;
            int mode;
            int flags;
            int cache_size;
            char *filename;

            serial<LdbmEnvBase> handle(0);

            if(! arg1.IsString()) {
                RUNWARN("1st argument of ldbm:open/3 must be string to specify filename");
                return false;
            }
            filename = arg1.ReadString();
        
            if(! arg2.Coerce(ldbm_info_db) ) {
                RUNWARN("2nd argument of ldbm:open/3 must be compatible with ldbm_info_db");
                return false;
            }
        
            FSP fsp_mode = arg2.Follow(m_ldbm->Search("mode\\"));
            FSP fsp_flags = arg2.Follow(m_ldbm->Search("flags\\"));
            FSP fsp_type = arg2.Follow(m_ldbm->Search("type\\"));
            FSP fsp_cache_size = arg2.Follow(m_ldbm->Search("cache_size\\"));

            if( fsp_mode.IsInteger() ) {
                mode = fsp_mode.ReadInteger();
            } else {
                mode = 0;
            }
            pair<bool, DBTYPE> __db_type = get_type(fsp_type);
            if( __db_type.first ) {
                db_type = __db_type.second;
            } else {
                db_type = DB_BTREE;
            }
    
            pair<bool, int> __flags = get_flags(fsp_flags);
            if(__flags.first) {
                flags = __flags.second;
            } else {
                RUNERR( "Invalid flags: " );
                return false;
            }
            if( fsp_cache_size.IsInteger() ) {
                cache_size = fsp_cache_size.ReadInteger();
            } else {
                cache_size = DefaultCacheSize;
            }
            list<LdbmSchemaBase*> *sc = newSchema();
            if(! convertSchema(arg3, sc) ) return false;
            if(! fixSchema(sc) ) return false;
            
            tserial pred_t = arg3.GetType()->GetSerialNo();

            if(! ldbm->openDbBdb(handle, filename, db_type, mode, flags, cache_size, sc, pred_t)) return false;
            return true;
        }

        bool open_db(machine&, FSP arg1, FSP arg2, FSP arg3, FSP arg4) {
                // file version
        
            DBTYPE db_type;
            int mode;
            int flags;
            int cache_size;
            char *filename;

            if(! arg1.GetType()->IsSubType(ldbm_handle_env)) {
                RUNWARN("1st argument of ldbm:open/4 must be ldbm_handle_env to specify environment");
                return false;
            }
            FSP fsp_handle_id = arg1.Follow(ldbm_handle_id_f);
            if(! fsp_handle_id.IsInteger()) {
                RUNWARN("invalid ldbm_handle_file");
                return false;
            }
            serial<LdbmEnvBase> handle(fsp_handle_id.ReadInteger());

            if(! arg2.IsString()) {
                RUNWARN("2nd argument of ldbm:open/4 must be string to specify filename");
                return false;
            }
            filename = arg2.ReadString();
        
            if(! arg3.Coerce(ldbm_info_db) ) {
                RUNWARN("3rd argument of ldbm:open/4 must be compatible with ldbm_info_db");
                return false;
            }
        
            FSP fsp_mode = arg3.Follow(m_ldbm->Search("mode\\"));
            FSP fsp_flags = arg3.Follow(m_ldbm->Search("flags\\"));
            FSP fsp_type = arg3.Follow(m_ldbm->Search("type\\"));
            FSP fsp_cache_size = arg3.Follow(m_ldbm->Search("cache_size\\"));

            if( fsp_mode.IsInteger() ) {
                mode = fsp_mode.ReadInteger();
            } else {
                mode = 0;
            }
            pair<bool, DBTYPE> __db_type = get_type(fsp_type);
            if( __db_type.first ) {
                db_type = __db_type.second;
            } else {
                db_type = DB_BTREE;
            }
    
            pair<bool, int> __flags = get_flags(fsp_flags);
            if(__flags.first) {
                flags = __flags.second;
            } else {
                RUNERR( "Invalid flags: " );
                return false;
            }
            if( fsp_cache_size.IsInteger() ) {
                cache_size = fsp_cache_size.ReadInteger();
            } else {
                cache_size = DefaultCacheSize;
            }
            list<LdbmSchemaBase*> *sc = newSchema();
            if(! convertSchema(arg4, sc) ) return false;
            if(! fixSchema(sc) ) return false;
            
            tserial pred_t = arg4.GetType()->GetSerialNo();

            if(! ldbm->openDbBdb(handle, filename, db_type, mode, flags, cache_size, sc, pred_t)) return false;

            return true;
        }

#else //WITH_BDBM
        bool open_env(machine&, FSP, FSP, FSP) { return false; };
        bool open_env(machine&, FSP) { return false; };
        bool open_db(machine&, FSP, FSP, FSP) { return false; };
        bool open_db(machine&, FSP, FSP, FSP, FSP) { return false; };
        
#endif // WITH_BDBM
        

            /*
              bool ldbm_make_index0(FSP, FSP, machine &) {
              P1("LDBM");
              hoge = &m;

              if (! arg1.GetType()->IsSubType(ldbm_database) ) {
              RUNWARN( "the 1st argument of ldbm_make_index/2 requires a ldbm_database" );
              return false;
              }

              const type *index_t = arg2.GetType();
  
              if (! (index_t->IsSubType(ldbm_index_key) || index_t->IsSubType(ldbm_index_value)) ) {
              RUNWARN( "the 2nd argument of ldbm_make_index/2 requires a ldbm_index_key of ldbm_index_value" );
              return false;
              }
  
              return dbm->openIndex(arg1.GetType(), index_t, (index_t->IsSubType(ldbm_index_key)));

              return false;
              }
      
              bool ldbm_find_usg0(const int, FSP, FSP, FSP, FSP, machine &) {
              if (! arg1.GetType()->IsSubType(ldbm_index) ) {
              RUNWARN( "the 1st argument of ldbm_find_{unifiable, more_specific, more_general}/2 requires a ldbm_index" );
              return false;
              }

              if(! arg3.IsInteger() ) {
              RUNWARN( "the 3rd argument of ldbm_find_{unifiable, more_specific, more_general}/2 requires an integer (the number of index paths)" );
              return false;
              }
    
              if( arg4.GetType() != bot) {
              RUNWARN( "the 4th argument of ldbm_find_{unifiable, more_specific, more_general}/2 must be bot" );
              return false;
              }
  
              DbManagerIndex *index;
  
              if( index = dbm->getDbManagerIndex(arg1.GetType()) ) {

              list<int> r;
              index->FindUSG(mode, arg2, arg3.ReadInteger(), r);

              list<int>::iterator it = r.begin(), last = r.end();
              for(; it != last ; it++) {
              arg4.Follow(hd).Unify(FSP(m, (mint) (*it)));
              arg4 = arg4.Follow(tl);
              }
              arg4.Coerce(nil);
              return true;
        
              } else {
        
              RUNWARN("ldbm_index in the 1st argument is invalid");
              return false;
              }
              return false;
              }
            */
    } //end of namespace lildbm



//////////////////////////////////////////////////////////////////////////////
// LiLFeSの初期化

    static void initialize_ldbm(void) {
#ifdef DEBUG
        cout << "Initialize " << __FILE__ << endl;
#endif
        lildbm::dtm = new lildbm::DbTypeManager<lildbm::mem>();
        lildbm::ldbm = new lildbm::LdbmManager(lildbm::dtm);
    }
    static void terminate_ldbm(void) {
        delete lildbm::dtm;
        delete lildbm::ldbm;
    }

    static InitFunction INIT_ldbm(initialize_ldbm, 103);
    static TermFunction TERM_ldbm(terminate_ldbm);


//////////////////////////////////////////////////////////////////////////////
// LiLFeS DataBase Manager

/// :- module("ldbm").
/// ldbm:iterator <- [bot] + [ldbm:db\, ldbm:entry_ID\].
/// ldbm:pbi_iterator <- [ldbm:iterator] + ??.
/// ldbm:dti_iterator <- [ldbm:iterator] + ??.
///
/// ldbm:info <- [bot] + [ldbm:mode\, ldbm:flags\].
/// ldbm:info_env <- [ldbm:info] + [ldbm:bdbm_page_size, ldbm:bdbm_cache_size\].
/// ldbm:info_db <- [ldbm:info] + [ldbm:type\, ldbm:cache_size\].
/// ldbm:handle <- [bot] + [ldbm:handle_ID\].
/// ldbm:handle_env <- [ldbm:handle].
/// ldbm:handle_file <- [ldbm:handle].
/// ldbm:handle_index <- [ldbm:handle].


    static void initialize_ldbm_type(void) {

        using lildbm::hash_key;
        using lildbm::key_integer;
        using lildbm::key_inthash;
        using lildbm::key_float;
        using lildbm::key_string;
        using lildbm::key_type;
        using lildbm::key_fs;
        using lildbm::array_value_integer;
        using lildbm::array_value_float;
        using lildbm::array_value_string;
        using lildbm::array_value_type;
        using lildbm::array_value_fs;
        
	hash_key = new type("hash_key",module::BuiltinModule());
        hash_key->SetAsChildOf(bot);
        hash_key->AddFeature(new feature("ARRAY_SIZE\\",module::BuiltinModule()), t_int);
        hash_key->Fix();
	key_integer = new type("key_integer",module::BuiltinModule());
        key_integer->SetAsChildOf(hash_key);
        key_integer->AddFeature(new feature("ARRAY_BASE\\",module::BuiltinModule()), t_int);
        key_integer->Fix();
	key_inthash = new type("key_inthash",module::BuiltinModule());
        key_inthash->SetAsChildOf(hash_key);
        key_inthash->Fix();
	key_float = new type("key_float",module::BuiltinModule());
        key_float->SetAsChildOf(hash_key);
        key_float->Fix();
	key_string = new type("key_string",module::BuiltinModule());
        key_string->SetAsChildOf(hash_key);
        key_string->Fix();
	key_type = new type("key_type",module::BuiltinModule());
        key_type->SetAsChildOf(hash_key);
        key_type->Fix();
	key_fs = new type("key_fs",module::BuiltinModule());
        key_fs->SetAsChildOf(hash_key);
        key_fs->Fix();

	array_value_integer = new type("array_value_integer",module::BuiltinModule());
        array_value_integer->SetAsChildOf(hash_key);
        array_value_integer->Fix();
	array_value_float = new type("array_value_float",module::BuiltinModule());
        array_value_float->SetAsChildOf(hash_key);
        array_value_float->Fix();
	array_value_string = new type("array_value_string",module::BuiltinModule());
        array_value_string->SetAsChildOf(hash_key);
        array_value_string->Fix();
	array_value_type = new type("array_value_type",module::BuiltinModule());
        array_value_type->SetAsChildOf(hash_key);
        array_value_type->Fix();
	array_value_fs = new type("array_value",module::BuiltinModule());
        array_value_fs->SetAsChildOf(hash_key);
        array_value_fs->Fix();

        using lildbm::m_ldbm;
    
        using lildbm::ldbm_info;
        using lildbm::ldbm_info_env;
        using lildbm::ldbm_info_db;

        using lildbm::ldbm_iterator;

        using lildbm::ldbm_handle;
        using lildbm::ldbm_handle_env;
        using lildbm::ldbm_handle_file;
        using lildbm::ldbm_handle_index;
    
        using lildbm::ldbm_db_f;
        using lildbm::ldbm_entry_id_f;
        using lildbm::ldbm_handle_id_f;
    
            // declare module
        m_ldbm = new module("ldbm", module::NMT_BUILTIN);

            // declare ldbm_info
        ldbm_info = new type("info", m_ldbm);
        ldbm_info->SetAsChildOf(bot);
        ldbm_info->AddFeature(new feature("mode\\", m_ldbm));
        ldbm_info->AddFeature(new feature("flags\\", m_ldbm));
        ldbm_info->Fix();

        ldbm_info_env = new type("info_env", m_ldbm);
        ldbm_info_env->SetAsChildOf(ldbm_info);
        ldbm_info_env->AddFeature(new feature("bdbm_page_size\\", m_ldbm));
        ldbm_info_env->AddFeature(new feature("bdbm_cache_size\\", m_ldbm));
        ldbm_info_env->Fix();
  
        ldbm_info_db = new type("info_db", m_ldbm);
        ldbm_info_db->SetAsChildOf(ldbm_info);
        ldbm_info_db->AddFeature(new feature("type\\", m_ldbm));
        ldbm_info_db->AddFeature(new feature("cache_size\\", m_ldbm));
        ldbm_info_db->Fix();
  
            /// declare iterator
        ldbm_iterator = new type("iterator", m_ldbm);
        ldbm_iterator->SetAsChildOf(bot);
        ldbm_db_f = new feature("db\\", m_ldbm);
        ldbm_entry_id_f = new feature("entry_ID\\", m_ldbm);
        ldbm_iterator->AddFeature(ldbm_db_f);
        ldbm_iterator->AddFeature(ldbm_entry_id_f);
        ldbm_iterator->Fix();

            /// declare handle
        ldbm_handle = new type("handle", m_ldbm);
        ldbm_handle->SetAsChildOf(bot);
        ldbm_handle_id_f = new feature("handle_ID\\", m_ldbm);
        ldbm_handle->AddFeature(ldbm_handle_id_f);
        ldbm_handle->Fix();
    
        ldbm_handle_env = new type("handle_env", m_ldbm);
        ldbm_handle_env->SetAsChildOf(ldbm_handle);
        ldbm_handle_env->Fix();

        ldbm_handle_file = new type("handle_file", m_ldbm);
        ldbm_handle_file->SetAsChildOf(ldbm_handle);
        ldbm_handle_file->Fix();

        ldbm_handle_index = new type("handle_index", m_ldbm);
        ldbm_handle_index->SetAsChildOf(ldbm_handle);
        ldbm_handle_index->Fix();
    }

    static void terminate_ldbm_type(void) {
    }

    static InitFunction IF_ldbm_type(initialize_ldbm_type, 101);
    static TermFunction TF_ldbm_type(terminate_ldbm_type);

//////////////////////////////////////////////////////////////////////////////
// LiLFeS Built-in Predicates

/**
  * @module = database
  * @cvs    = $Id: lildbm-interface.cpp,v 1.22 2011-05-02 10:38:23 matuzaki Exp $
  * @desc   = Database and Arrays
  * @copyright = Copyright (c) 2003-2004, Takashi Ninomiya
  * @copyright = You may distribute this file under the terms of the Artistic License.
  * Manipulation of database and arrays
  * @jdesc  = データベース
  * @japanese =
  * データベースを操作する述語です
  * @end_japanese
*/
    
    LILFES_BUILTIN_PRED_1(lildbm::print_statistics, print_statistics);
    LILFES_BUILTIN_PRED_2(lildbm::db_reserve, db_reserve);
    LILFES_BUILTIN_PRED_2(lildbm::db_size, db_size);
    LILFES_BUILTIN_PRED_2(lildbm::db_capacity, db_capacity);
    LILFES_BUILTIN_PRED_2(lildbm::db_reserve_k, db_reserve_k);
    LILFES_BUILTIN_PRED_2(lildbm::db_size_k, db_size_k);
    LILFES_BUILTIN_PRED_2(lildbm::db_capacity_k, db_capacity_k);
    LILFES_BUILTIN_PRED_2(lildbm::db_reserve_m, db_reserve_m);
    LILFES_BUILTIN_PRED_2(lildbm::db_size_m, db_size_m);
    LILFES_BUILTIN_PRED_2(lildbm::db_capacity_m, db_capacity_m);

/**
  * @predicate	= db_open_env(+Home, +Info, -Handle) db_config(+Info)
  * @desc	= db_open_env opens LiLDBM environment.  db_config opens `default LiLDBM environment'.  These are not necessary currently.  They might be neccesarry if we develop multi-user processing functions or transaction.
  * @param	= +Home/string : directory name
  * @param      = +Info/'ldbm:info_env' : settings for this environment
  * @param      = -Handle : handle for this environment
  * @note       = Info is supposed to be a following feature structure;<br><br> (<I>ldbm:info_env</I>& <br> ldbm:mode\ +Mode& (optional)<br> ldbm:bdbm_page_size\ +Pagesize& (optional)<br> ldbm:bdbm_cache_size\ +Cachesize& (optional)<br>ldbm:flags\ +Flags)<br><br> <table border=1><tr><td>+Mode:</td><td><i>integer</i></td><td>directory permission (optional, defalut value is the user defined file permission)</td></tr><tr><td>+Pagesize:</td><td><I>integer</I></td><td>the page size in the Berkeley DB (optional, defalut value is 1024)</td></tr><tr><td>+Cachesize:</td><td><I>integer</I></td><td>the cache size in the Berkeley DB (optional, defalut value is 32768)</td></tr><tr><td>+Flags:</td><td><I>list of string</I></td><td>Options for Berkeley DB.  ["CREATE", "INIT_MPOOL", "PRIVATE"] should be specified if you are not familliar with BerkeleyDB.  They are given in a list.  If you open a file without specifying the environment, `default environment' is opened automatically.  The default environment is opened with ["CREATE", "INIT_MPOOL", "PRIVATE"] flags.</td></tr></table>
  
  * @example	= 
  > :- db_open_env("myenv", (ldbm:bdbm_page_size\ 1024 &
                             ldbm:bdbm_cache_size\ 32768 &
                             ldbm:flags\ ["CREATE", "INIT_MPOOL", "PRIVATE"]), $H).
  * @end_example

  * @jdesc      = LiLDBM 環境をオープンします。(今のところは必要のない機能。将来マルチユーザー処理、トランザクション処理の機能が導入された時に必要になる。)

  * @jparam	= +Home/string : 環境のディレクトリ名
  * @jparam     =+Info/'ldbm:info_env' : 環境に対する設定
  * @jparam     = -Handle : この環境に対するハンドル
  * @jnote      = Info は下記の素性構造であることが想定されています.<br><br> (<I>ldbm:info_env</I>& <br> ldbm:mode\ +Mode& (オプション)<br> ldbm:bdbm_page_size\ +Pagesize& (オプション)<br> ldbm:bdbm_cache_size\ +Cachesize& (オプション)<br>ldbm:flags\ +Flags)<br><br> <table border=1><tr><td>+Mode:</td><td><i>integer</i></td><td>この環境に対応するディレクトリのパーミッション(オプション、デフォルトはユーザー指定のファイルパーミッション)</td></tr><tr><td>+Pagesize:</td><td><I>integer</I></td><td>この環境の中で開かれるファイルのBerkeley DBのページサイズ (オプション、デフォルトは1024)</td></tr><tr><td>+Cachesize:</td><td><I>integer</I></td><td>この環境の中で開かれるファイルのBerkeley DBのキャッシュサイズ (オプション, デフォルトは 32768)</td></tr><tr><td>+Flags:</td><td><I>list of string</I></td><td>Berkeley DBに渡すオプション。普通は["CREATE", "INIT_MPOOL", "PRIVATE"]を指定しておけばよい。これらをリスト形式で指定します。環境を指定せずにファイルを開いた場合にはデフォルト環境が生成されますが、そのデフォルト環境は["CREATE", "INIT_MPOOL", "PRIVATE"]が指定されています。</td></tr></table>

*/

    LILFES_BUILTIN_PRED_3(lildbm::open_env, db_open_env);
    LILFES_BUILTIN_PRED_1(lildbm::open_env, db_config);
    
/**
  * @predicate	= db_open(+Pred) db_open(+File, +Info, +Pred) db_open(+Env, +file, +Info, +Pred)
  * @desc	= db_open/1 opens DB that keeps statistic values on memory.  db_open/3 or db_open\4 opens DB on a file.
  * @param	= +Pred/pred : name of DB
  * @param      = +Env/ldbm:handle_env : Env specifies the environment that generates the file.
  * @param      = +File/string : file name
  * @param      = +Info/ldbm:info_db : 
  * @note       = The arguments of Pred are expressed as follows: <br>list up definition of keys and datas that has any dimension by marking off with comma. But, DB is dealt with predicate,so sum of parameters must be under 15.<br>key has two types:<TABLE border=1><TBODY><TR><TH>key_integer<TD>Integer key.Feature "ARRAY_BASE\"(begin of subscript), "ARRAY_SIZE\"(number of subscript) must be defined.<TR><TH>key_inthash<TD>Integer hash key.Feature "ARRAY_SIZE\" can be defined.<tr><th>key_float</th><td>float key</td><TR><TH>key_string<TD>String key.It must be used like associative array.Feature "ARRAY_SIZE\" can be defined.<TR><th>key_type</th><td>type key</td></tr><tr><TH>key_fs<TD>Feature structure key.It must be used like associative array.Feature "ARRAY_SIZE\" can be defined.</TR></TBODY></TABLE>And data has three types:<BR><TABLE border=1><TBODY><TR><TH>array_value<TD>feature structure<TR><TH>array_value_integer<TD>integer<TR><th>array_value_float</th><td> float</td><tr><TH>array_value_string<TD>string </TR><tr><th>array_value_type</th><td>type</td></tr></TBODY></TABLE><br>DB name must be declared as predicate(subtype of pred) in advance.And don't put any space between array name and "(".<br><br> Info is supposed to be the following feature structure;<br>(ldbm:info_db&<br>ldbm:mode\ +Mode& (optional)<br>ldbm:flags\ +Flags& (optional)<br>ldbm:type\ +Type& (optional)<br>ldbm:cache_size\ +Cache (optional))<br><br><table border=1><tr><td>+Mode:</td><td> <I>integer</I></td><td> file permission (optional, defalut value is the user defined file permission)</td></tr><tr><td>+Type:</td><td><I> string</I></td><td> type of index structures, hash ("HASH") or b+tree ("BTREE"). (optional, default value is "BTREE").</td></tr><tr><td>+Flags:</td><td><I> list of string</I></td><td> Options for Berkeley DB.  You should just specify "CREATE", if you are not familliar with BerkeleyDB. When "CREATE" is specified, a file is created if the specifed file does not exist, or a file is opened if the file exists.  When "RDONLY" is specified, the file is opened as a read-only file.  When "TRUNCATE" is specified, a file is created if the specified file does not exist, or a file is truncated if the file exists.  "DIRTY_READ", "EXCL", or "NOMMAP" can also be specified.  See Berkeley DB documents for further details. These options is specified as a list of string.<br> +Cache : <I> integer</I> Specifies the byte size of cache size of `LiLDBM cache', not `Berkeley DB cache'.  The specified value must be more than 16,384 byte.  The default value is 10 * 1048576 byte (=~ 10MB)</td></tr></table>
  * @example	=
>  myarr <- [pred].
>  :- db_open("mydata.db", (ldbm:flags\ ["CREATE"]),
              myarr(key_integer & ARRAY_BASE\0 & ARRAY_SIZE\2000,
                    array_value)).
>  myarr2 <- [pred].
>  :- db_open(myarr2(key_inthash, array_value_string)).<BR><BR>
>  myarr3 <- [pred].
>  :- db_open(myarr3((key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 100),
                     (key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 200),
                     array_value_integer,
                     array_value_integer,
                     array_value_string)).

    The first example is array that that has integer as a key and keeps one feature structure on a file.                        
 　 The second example is array that has integer as a key and keeps a string on memory.
  　The third example is array (which has 2nd dimension) of 100x200,which keeps two integer and one strings.It is similar with 
struct arr1{
    int a;
    int b;
    char *c;
}arr1[100][200];
in C.
  
  * @end_example

  * @jdesc	= open_db/1は静的に値を保持するDBをメモリ上に作ります． open_db/3もしくはopen_db\4は静的に値を保持するDBをファイル上に作ります。
  * @jparam	= +Pred/pred
  * @jparam      = +Env/ldbm:handle_env : ファイルを開く場合の環境を指定します.
  * @jparam      = +File/string : ファイル名.
  * @jparam      = +Info/ldbm:info_db : DBの設定
  * @jnote	= 配列名は、予め、述語（pred型の subtype）として定義されている必要があります。また、配列名と ( の間には空白を置くことはできません。
  * @jnote      = Predの表記方法 : <br>任意の次元のキーの定義, 任意の数のデータの定義をカンマで区切って列挙します。但し、配列は述語として扱われるため、合計の引数は15個までになります。キーには次の6種類があります.<br><TABLE border=1><TBODY><TR><TH>key_integer<TD>整数のキー。素性 ARRAY_BASE\（添字の始まりの数）, ARRAY_SIZE\（添字の数） を必ず指定すること。 <TR><TH>key_inthash<TD>整数ハッシュのキー。素性 ARRAY_SIZE\ を指定することができる． <tr><th>key_float</th><td> floatの実数のキー</td></tr><TR><TH>key_string<TD>文字列のキー。連想配列のように使える。ARRAY_SIZE\ を指定することができる。 <tr><th>key_type</th><td>型のキー</td></tr><TR><TH>key_fs<TD>素性構造のキー。連想配列のように使える。ARRAY_SIZE\ を指定することができる。</TR></TBODY></TABLE>　また、データには、次の5種類があります。<BR><TABLE border=1><TBODY><TR><TH>array_value<TD>任意の素性構造 <TR><TH>array_value_integer<TD>整数値 <tr><th>array_value_float</th><td>floatの実数値</td></tr><TR><TH>array_value_string<TD>文字列 </TR><tr><th>array_value_type</th><td>型</td></tr></TBODY></TABLE> Infoは下記の素性構造であることが想定されています.<br><br> (ldbm:info_db&<br>ldbm:mode\ +Mode& (オプション)<br>ldbm:flags\ +Flags& (オプション)<br>ldbm:type\ +Type& (オプション)<br>ldbm:cache_size\ +Cache (オプション))<br><br><table border=1><tr><td>+Mode:</td><td><I>integer</I></td><td>この環境に対応するファイルのパーミッション(オプション、デフォルトはユーザー指定のファイルパーミッション)</td></tr><tr><td>+Type:</td><td><I> string</I></td><td> ファイルのデータ構造のタイプを指定。ハッシュ("HASH")かB+Tree("BTREE")が選べる。(オプション、デフォルトは"BTREE")</td></tr><tr><td>+Flags:</td><td><I> list of string</I></td><td> Berkeley DBにわたすオプション。普通は"CREATE"だけ指定しておけばよい。"CREATE"(ファイルが存在しない場合はファイルを作成。存在しない場合はファイルをただ開く)、"DIRTY_READ"(??)、"EXCL"(??)、"NOMMAP"(??)、"RDONLY"(リードオンリーでファイルを開きます)、"THREAD"(??)、"TRUNCATE"(ファイルが存在しない場合はファイルを作成。存在する場合はそのファイルを削除します。)。これらをリスト形式で指定します。</td></tr><tr><td> +Cache :</td><td><I> integer</I></td><td> BerkeleyDBのキャッシュではなくて、LiLDBのヒープのキャッシュサイズ。単位はByte。16,384Byte以上を指定しないとエラー。デフォルトは 10 * 1048576 Byte (=~ 10MB)</td></tr></table>
  * @jexample	=
>  myarr <- [pred].
>  :- db_open("mydata.db", (ldbm:flags\ ["CREATE"]),
              myarr(key_integer & ARRAY_BASE\0 & ARRAY_SIZE\2000,
                    array_value)).
>  myarr2 <- [pred].
>  :- db_open(myarr2(key_inthash, array_value_string)).<BR><BR>
>  myarr3 <- [pred].
>  :- db_open(myarr3((key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 100),
                     (key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 200),
                     array_value_integer,
                     array_value_integer,
                     array_value_string)).

    最初の例では、整数がキーで素性構造が値になっているファイルDBを宣言しています。
 　 次の例では、整数がキーで文字列を値としてもつメモリDBを宣言しています。
  　最後の例では、100 x 200の2次元配列を定義しています。C言語での
struct arr1{
    int a;
    int b;
    char *c;
}arr1[100][200];
という定義と似ています。

  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::open_db, db_open);
    LILFES_BUILTIN_PRED_OVERLOAD_3(lildbm::open_db, db_open_3, db_open);
    LILFES_BUILTIN_PRED_OVERLOAD_4(lildbm::open_db, db_open_4, db_open_3);


/**
  * @predicate	= db_sync(+Pred)
  * @desc	= The file DB is sync-ed.
  * @param	= +Pred/pred : name of DB
  * @example	=
>  myarr <- [pred].
>  :- db_open("mydata.db", (ldbm:flags\ ["CREATE"]),
              myarr(key_integer & ARRAY_BASE\0 & ARRAY_SIZE\2000,
                    array_value)).
>  :- db_sync(myarr(_, _)).                        
  * @end_example

  * @jdesc	= ファイルDBをsyncします.
  * @jparam	= +Pred/pred : 配列名
*/
    
    LILFES_BUILTIN_PRED_1(lildbm::sync, db_sync);

/**
  * @predicate	= db_close(+Pred)
  * @desc	= db_close closes the specified DB.
  * @param	= +Pred/pred : name of DB
  * @example	=
>  myarr <- [pred].
>  :- db_open("mydata.db", (ldbm:flags\ ["CREATE"]),
              myarr(key_integer & ARRAY_BASE\0 & ARRAY_SIZE\2000,
                    array_value)).
>  :- db_close(myarr(_, _)).                        
  * @end_example

  * @jdesc	= ファイルDBを閉じます.
  * @jparam	= +Pred/pred : 配列名
*/
    
    
    LILFES_BUILTIN_PRED_1(lildbm::close, db_close);

/**
  * @predicate	= db_find(+Pred) db_find(+Iter, +Pred)
  * @desc	= db_find/1 finds an entry specified keys in Pred.  db_find/2 finds an entry specified by Iter.
  * @param      = +Iter/ldbm:iterator : the identifier for an entry
  * @param	= +Pred/pred : name of DB
  * @example	=
>  myarr <- [pred].
>  :- db_open("hoge.db", ldbm:flags\ ["CREATE"],
              myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_find(my_arr(10, X)), printAVM(X).
>  :- db_first(my_arr(_, _), I), db_find(I, X), printAVM(X).
  * @end_example

  * @jdesc	= db_find/1はキーで指定されたエントリーの値をDBから取り出してきます。db_find/2はIterで指定されたエントリーを取り出してきます。
  * @jparam	= +Pred/pred : 配列名
*/
    
    LILFES_BUILTIN_PRED_1(lildbm::find, db_find);
    LILFES_BUILTIN_PRED_OVERLOAD_2(lildbm::find, db_find_2, db_find);

/**
  * @predicate	= db_insert(+Pred)
  * @desc	= db_insert/1 stores/overwrite an entry.
  * @param	= +Pred/pred : name of DB
  * @example	=
>  myarr <- [pred].
>  :- db_open("hoge.db", ldbm:flags\ ["CREATE"],
              myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_find(my_arr(10, X)), printAVM(X).
>  :- db_first(my_arr(_, _), I), db_find(I, X), printAVM(X).
  * @end_example

  * @jdesc	= db_insertはDBにエントリーを格納/上書きします.
  * @jparam	= +Pred/pred : 配列名
*/
    
    LILFES_BUILTIN_PRED_1(lildbm::insert, db_insert);

/**
  * @predicate	= db_delete(+Pred)
  * @desc	= db_delete/1 deletes an entry specified by keys from the DB.
  * @param	= +Pred/pred : name of DB
  * @example	=
>  myarr <- [pred].
>  :- db_open("hoge.db", ldbm:flags\ ["CREATE"],
              myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_delete(my_arr(10, _)).
  * @end_example

  * @jdesc	= db_deleteはDBのエントリーを削除します.
  * @jparam	= +Pred/pred : 配列名
*/
    
    LILFES_BUILTIN_PRED_1(lildbm::erase, db_delete);

    
    LILFES_BUILTIN_PRED_2(lildbm::update, db_update);

/**
  * @predicate	= db_clear(+Pred)
  * @desc	= db_clear/1 deletes all entries in the DB.
  * @param	= +Pred/pred : name of DB
  * @example	=
>  myarr <- [pred].
>  :- db_open("hoge.db", ldbm:flags\ ["CREATE"],
              myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_clear(my_arr(_, _)).
  * @end_example

  * @jdesc	= データベースの内容を空にします。(全エントリーが消えます。)
  * @jparam	= +Pred/pred : 配列名
*/
    
    LILFES_BUILTIN_PRED_1(lildbm::clear, db_clear);

/**
  * @predicate	= db_findall(+Pred, -EntryList)
  * @desc	= db_findall/1 returns all entries in a list.
  * @param	= +Pred/pred : name of DB
  * @param      = -EntryList : a list of all entries
  * @example	=
>  myarr <- [pred].
>  :- db_declare("hoge.db", ldbm:flags\ ["CREATE"],
                 myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_insert(my_arr(20, [4,5])).
>  :- db_insert(my_arr(30, [6,7,8,9])).
>  :- db_findall(my_arr(_, _), X), printAVM(X).
  * @end_example

  * @jdesc	= データベースの全エントリーをリストにして返します.
  * @jparam	= +Pred/pred : 配列名
  * @jparam     = 全エントリーのリスト
*/
    
    LILFES_BUILTIN_PRED_2(lildbm::findall, db_findall);

/**
  * @predicate	= db_first(+Pred, -Iter)
  * @desc	= db_first returs the identifier of the first entry in the DB.
  * @param	= +Pred/pred : name of DB
  * @param      = -Iter : the identifier of the first entry
  * @example	=
>  :- db_declare("hoge.db", ldbm:flags\ ["CREATE"],
                 myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_first(my_arr(_, _), I), db_find(I, X), printAVM(X).
  * @end_example

  * @jdesc	= データベースの先頭のエントリーのIDを返します。先頭のエントリーがない場合はfailします。
  * @jparam	= +Pred/pred : 配列名
  * @jparam     = -Iter : ID
*/
    
    LILFES_BUILTIN_PRED_2(lildbm::first, db_first);

/**
  * @predicate	= db_last(+Pred, -Iter)
  * @desc	= db_last returs the identifier of the last entry in the DB.
  * @param	= +Pred/pred : name of DB
  * @param      = -Iter : the identifier of the last entry
  * @example	=
>  :- db_declare("hoge.db", ldbm:flags\ ["CREATE"],
                 myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_last(my_arr(_, _), I), db_find(I, X), printAVM(X).
  * @end_example

  * @jdesc	= データベースの末尾のエントリーのIDを返します。先頭のエントリーがない場合はfailします。
  * @jparam	= +Pred/pred : 配列名
  * @jparam     = -Iter : ID
*/

    LILFES_BUILTIN_PRED_2(lildbm::last, db_last);
    
/**
  * @predicate	= db_next(+Iter1, -Iter2)
  * @desc	= db_next returs the identifier of the next entry specified by Iter1.  If there is no next entry, db_next fails.
  * @param      = +Iter1 : the identifier of an entry
  * @param      = -Iter2 : the identifier of the next entry of Iter1
  * @example	=
>  myarr <- [pred].
>  :- db_declare("hoge.db", ldbm:flags\ ["CREATE"],
                 myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_insert(my_arr(30, [4,5,6,7])).
>  :- db_first(my_arr(_, _), I), db_next(I, J), db_find(J, X), printAVM(X).
  * @end_example

  * @jdesc	= Iter1の次のエントリーのIDをIter2に返します.次のエントリーがない場合はfailします.
  * @jparam	= +Iter1 : エントリーのID
  * @jparam     = -Iter2 : Iter1の次のエントリーのID
*/
    LILFES_BUILTIN_PRED_2(lildbm::next, db_next);

/**
  * @predicate	= db_prev(+Iter1, -Iter2)
  * @desc	= db_prev returs the identifier of the previous entry specified by Iter1.  If there is no previous entry, db_prev fails.
  * @param      = +Iter1 : the identifier of an entry
  * @param      = -Iter2 : the identifier of the previous entry of Iter1
  * @example	=
>  myarr <- [pred].
>  :- db_declare("hoge.db", ldbm:flags\ ["CREATE"],
                 myarr(key_inthash, array_value)).
>  :- db_insert(my_arr(10, [1,2,3])).
>  :- db_insert(my_arr(30, [4,5,6,7])).
>  :- db_first(my_arr(_, _), I), db_prev(I, J), db_find(J, X), printAVM(X).
  * @end_example

  * @jdesc	= Iter1の前のエントリーのIDをIter2に返します.前のエントリーがない場合はfailします.
  * @jparam	= +Iter1 : エントリーのID
  * @jparam     = -Iter2 : Iter1の次のエントリーのID
*/

    LILFES_BUILTIN_PRED_2(lildbm::prev, db_prev);

/**
  * @predicate = db_save(+FILE, +NAME)
  * @desc      = Save contents of DB into a file
  * @param     = +FILE/string : file name
  * @param     = +NAME/pred : DB name
  * @example   =
  * :- db_save("array1", arr1(_, _, _, _, _)).
  * @end_example
  * @jdesc     = DBの内容をファイルに保存します。
  * @jparam    = +FILE/string : ファイル名
  * @jparam    = +NAME/pred : 対象となるDB
*/
    
    LILFES_BUILTIN_PRED_2(lildbm::save, db_save);

/**
  * @predicate = db_load(+FILE, +NAME)
  * @desc      = Load contents of DB from a file
  * @param     = +FILE/string : file name
  * @param     = +NAME/pred : DB name
  * @example   =
  * :- db_load("array1", arr1(_, _, _, _, _)).
  * @end_example
  * @jdesc     = DBの内容をファイルから取り込みます。
  * @jparam    = +FILE/string : ファイル名
  * @jparam    = +NAME/pred : 対象となるDB
*/
    
    LILFES_BUILTIN_PRED_2(lildbm::load, db_load);

/**
  * @predicate	= declare_array(+INFO)
  * @desc	= Make arrray that keeps statistic values.
  * @param	= +INFO/pred : name of array(...)<br>How to express  …… : <br>list up definition of keys and datas that has any dimension by marking off with comma. But,array is dealt with predicate,so sum of parameters must be under 15.<br>key has two types:<TABLE border=1><TBODY><TR><TH>key_integer<TD>Integer key.Feature "ARRAY_BASE\"(begin of subscript), "ARRAY_SIZE\"(number of subscript) must be defined.<TR><TH>key_inthash<TD>Integer hash key.Feature "ARRAY_SIZE\" can be defined.<TR><TH>key_string<TD>String key.It must be used like associative array.Feature "ARRAY_SIZE\" can be defined.<TR><TH>key_fs<TD>Feature structure key.It must be used like associative array.Feature "ARRAY_SIZE\" can be defined.</TR></TBODY></TABLE>And data has three types:<BR><TABLE border=1><TBODY><TR><TH>array_value<TD>feature structure<TR><TH>array_value_integer<TD>integer<TR><TH>array_value_string<TD>string </TR></TBODY></TABLE>
  * @note	= Array name must be declared as predicate(subtype of pred) in advance.And don't put any space between array name and "(".
  * @example	= 
  > :- declare_array(arr1((key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 100),
                      (key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 200),
                      array_value_integer,
                      array_value_integer,
                      array_value_string)).
  > :- declare_array(arr2(key_string,
                      array_value)).
  　
  　The first example is array (which has 2nd dimension) of 100x200,which keeps two integer and one strings.It is equal to 
struct arr1{
    int a;
    int b;
    char *c;
}arr1[100][200];
in C.
　The second example is array that has string as key ans keeps one feature structure.
  * @end_example

  * @jdesc	= 静的に値を保持する配列を作ります．
  * @jparam	= +INFO/pred : 配列名(……)<br>……の部分の表記方法 : <br>任意の次元のキーの定義, 任意の数のデータの定義をカンマで区切って列挙します。但し、配列は述語として扱われるため、合計の引数は15個までになります。<br>キーには、次の２種類があります。 key_integer 整数のキー。素性 ARRAY_BASE\（添字の始まりの数）, ARRAY_SIZE\（添字の数） を必ず指定すること。  <br><TABLE border=1><TBODY><TR><TH>key_integer<TD>整数のキー。素性 ARRAY_BASE\（添字の始まりの数）, ARRAY_SIZE\（添字の数） を必ず指定すること。 <TR><TH>key_inthash<TD>整数ハッシュのキー。素性 ARRAY_SIZE\ を指定することができる． <TR><TH>key_string<TD>文字列のキー。連想配列のように使える。ARRAY_SIZE\ を指定することができる。 <TR><TH>key_fs<TD>素性構造のキー。連想配列のように使える。ARRAY_SIZE\ を指定することができる。</TR></TBODY></TABLE>　また、データには、次の３種類があります。<BR><TABLE border=1><TBODY><TR><TH>array_value<TD>任意の素性構造 <TR><TH>array_value_integer<TD>整数値 <TR><TH>array_value_string<TD>文字列 </TR></TBODY></TABLE>
  * @jnote	= 配列名は、予め、述語（pred型の subtype）として定義されている必要があります。また、配列名と ( の間には空白を置くことはできません。 
  * @jexample	=
  > :- declare_array(arr1((key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 100),
                      (key_integer & ARRAY_BASE\ 0 & ARRAY_SIZE\ 200),
                      array_value_integer,
                      array_value_integer,
                      array_value_string)).
  > :- declare_array(arr2(key_string,
                      array_value)).
  　
  　上記の例の１番目は、100×200の２次元配列で、２つの整数値と１つの文字列を保持するものです。およそ、Ｃ言語における 
struct arr1{
    int a;
    int b;
    char *c;
}arr1[100][200];
に相当します。
　２番目の例は、文字列をキーとする配列で、１つの素性構造を保持します。 
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::open_db, declare_array);
      //RUNWARN("declare_array/1 is obsolete.  Instead, use db_open/1.");

/**
  * @predicate	= delete_array(+NAME)
  * @desc	= Delete array <i>NAME</i>.
  * @param	= +NAME/pred : array name
  * @example	= 
  * :- delete_array(arr1(_, _, _, _, _)).
  * @end_example

  * @jdesc	= <i>NAME</i>という名前の配列を削除します．
  * @jparam	= +NAME/pred : 対象となる配列
  * @jexample	=
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::close, delete_array);
      //RUNWARN("delete_array/1 is obsolete.  Instead, use db_close/1.");

/**
  * @predicate	= get_array(+INFO)
  * @desc	= Get value of array <i>INFO</i>.
  * @param	= +INFO/pred : name of array(...)
  * @see        = #set_array/1
  * @jdesc	= 配列の値を取得します．
  * @jparam	= +INFO/pred : 配列名(……)
  * @jexample	=
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::find, get_array);
      //RUNWARN("get_array/1 is obsolete.  Instead, use db_find/1.");

/**
  * @predicate	= set_array(+INFO)
  * @desc	= Set value of array.
  * @param	= +INFO/pred : name of array(...)<br>How to express …… : <br>Following keys and datas in declare_array,describe place for input and value.
  * @see        = #get_array/1
  * @example	= 
  > :- set_array(arr1(3,4,200,55,"abc")).
  > :- set_array(arr2("foo", abc & F1\aa)).
  > ?- get_array(arr1(3,4,X,Y,Z)).
  X: 200
  Y: 55
  Z: "abc"
  > ?- get_array(arr2("foo",X)).
  X: |~abc  ~|
     |_F1:aa_|
  　
  　The first example is substitution of 200, 100, "abc" to the value of arr1[3][4].
  　The second example is entry of feature structure abc & F1\aa to arr2 with key key of "foo".
  　To refer to value of array,describe query with array name as predicate.
  * @end_example

  * @jdesc	= 配列に値を代入します．
  * @jparam	= +INFO/pred : 配列名(……)<br>……の部分の表記方法 : <br>declare_arrayで設定したキーとデータに従って、入力する場所と値を記述します．
  * @jexample	=
  > :- set_array(arr1(3,4,200,55,"abc")).
  > :- set_array(arr2("foo", abc & F1\aa)).
  > ?- get_array(arr1(3,4,X,Y,Z)).
  X: 200
  Y: 55
  Z: "abc"
  > ?- get_array(arr2("foo",X)).
  X: |~abc  ~|
     |_F1:aa_|
  　
  　上記の例の１番目は、arr1[3][4]の値として、 200, 100, "abc" を代入したものです。
  　２番目の例は、"foo" をキーとして、素性構造 abc & F1\aa を登録したものです。
  　配列の値を参照するには、配列名を述語としたクエリーを記述します。 
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::insert, set_array);
      //RUNWARN("set_array/1 is obsolete.  Instead, use db_insert/1.");

/**
  * @predicate	= unset_array(+INFO)
  * @desc	= Unset value of array.
  * @param	= +INFO/pred : name of array(...)<br>How to express …… : <br>Specify all keys of the array
  * @example	= 
  > :- unset_array(arr1(3,4,_,_,_)).
  > ?- get_array(arr1(3,4,X,Y,Z)).
  no
  * @end_example

  * @jdesc	= 配列の値を消去します．
  * @jparam	= +INFO/pred : 配列名(……)<br>……の部分の表記方法 : <br>declare_arrayで設定したキーを指定します．
  * @jexample	=
  > :- unset_array(arr1(3,4,_,_,_)).
  > ?- get_array(arr1(3,4,X,Y,Z)).
  no
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::erase, unset_array);
      //RUNWARN("unset_array/1 is obsolete.  Instead, use db_delete/1.");

/**
  * @predicate	= clear_array(+NAME)
  * @desc	= Clear contents of array <i>NAME</i>.
  * @param	= +NAME/pred : array name
  * @example	= 
  * :- clear_array(arr1(_, _, _, _, _)).
  * @end_example

  * @jdesc	= <i>NAME</i>という名前の配列の内容をクリアします．
  * @jparam	= +NAME/pred : 対象となる配列
  * @jexample	=
  * @end_jexample
*/
    LILFES_BUILTIN_PRED_1(lildbm::clear, clear_array);
      //RUNWARN("clear_array/1 is obsolete.  Instead, use db_clear/1.");

/**
  * @predicate = save_array(+FILE, +NAME)
  * @desc      = Save contents of array into a file
  * @param     = +FILE/string : file name
  * @param     = +NAME/pred : array name
  * @example   =
  * :- save_array("array1", arr1(_, _, _, _, _)).
  * @end_example
  * @jdesc     = 配列の内容をファイルに保存します。
  * @jparam    = +FILE/string : ファイル名
  * @jparam    = +NAME/pred : 対象となる配列
*/
    LILFES_BUILTIN_PRED_2(lildbm::save, save_array);

/**
  * @predicate = load_array(+FILE, +NAME)
  * @desc      = Load contents of array from a file
  * @param     = +FILE/string : file name
  * @param     = +NAME/pred : array name
  * @example   =
  * :- load_array("array1", arr1(_, _, _, _, _)).
  * @end_example
  * @jdesc     = 配列の内容をファイルから取り込みます。
  * @jparam    = +FILE/string : ファイル名
  * @jparam    = +NAME/pred : 対象となる配列
*/
    LILFES_BUILTIN_PRED_2(lildbm::load, load_array);

} // end of namespace lilfes

// end of file lildbm-interface.cpp

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.21  2004/08/09 13:35:06  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.20  2004/06/01 15:25:01  ninomi
 * db_reserve, db_size, db_capacity
 * db_reserve_k, db_size_k, db_capacity_k
 * db_reserve_m, db_size_m, db_capacity_mを加えました。
 *
 * Revision 1.19  2004/05/30 13:42:51  ninomi
 * lildbm終了時にちゃんとごみをdeleteするようにしました
 *
 * Revision 1.18  2004/05/27 12:59:18  ninomi
 * print_statisticsというbuilt-inをくわえました。
 *
 * Revision 1.17  2004/05/18 14:29:59  yusuke
 *
 * マニュアルを更新。
 *
 * Revision 1.16  2004/05/18 10:17:35  ninomi
 * LiLDBMのマニュアルを書きました
 *
 * Revision 1.15  2004/05/18 08:24:38  ninomi
 * とりあえずLiLDBのマニュアルをかきました。
 *
 * Revision 1.14  2004/05/17 13:07:13  yusuke
 *
 * まちがって db_save, db_load を消しちゃってました。
 *
 * Revision 1.13  2004/05/17 12:47:44  yusuke
 *
 * save_array/2, load_array/2 を実装。
 *
 * Revision 1.12  2004/05/14 15:58:29  yusuke
 *
 * 配列のマニュアルをコピー。
 *
 * Revision 1.11  2004/04/23 08:40:52  yusuke
 *
 *  *
 * Revision 1.10  2004/04/23 02:25:32  ninomi
 * このファイルの中で文字化けしている箇所を削りました。
 *
 * Revision 1.9  2004/04/23 02:19:08  ninomi
 * このファイルの漢字コードをeuc-japan-unixに変更しました。
 *
 */
