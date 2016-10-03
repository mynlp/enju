/*
 * $Id: lildbm-type.h,v 1.4 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-type.h
//  Database Manager Interface
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_type_h
#define __lildbm_type_h

#include "lildbm-basic.h"
#include "lildbm-cell.h"
#ifdef WITH_BDBM
#include <db_cxx.h>
#endif

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace lilfes {
    namespace lildbm {

/////////////////////////
/// class propername
///

        class propername {
        public:
			std::string module_name;
			std::string simple_name;
            propername(const std::string x, const std::string y) {module_name = x; simple_name = y;};
            propername(const std::string x) {
                module_name = x;
				size_t pos = x.find('\0');
                simple_name = x.substr(pos + 1);
            };
            propername(const type* t) {
                module_name = t->GetModule()->GetName();
                simple_name = t->GetSimpleName();
            };
    
            void Serialize(std::vector<char>& buf) {
                int x = module_name.length() + 1;
                int y = simple_name.length() + 1;
                for(int i = 0; i < x ; i++)
                    buf.push_back(module_name[i]);
                for(int i = 0; i < y ; i++)
                    buf.push_back(simple_name[i]);
            };
            void Serialize(std::vector<char>* buf) {
                Serialize(*buf);
            };
            
            ~propername() {};
        };

#ifdef WITH_BDBM
        struct hash_serial_dtype_bdb {
            size_t operator()(serial<dtype<bdb> > sn) const {
                return (size_t) sn.v;
            }
        };

        struct equal_to_serial_dtype_bdb {
            bool operator()(serial<dtype<bdb> > sn1, serial<dtype<bdb> > sn2) const {
                return (sn1.v == sn2.v);
            }
        };
#endif //WITH_BDBM


/////////////////////////
/// DbType(=DataBase Type) Manager
/// データベース上のタイプ管理クラス
/// データベース上ではポインタのような効率的かつ一意にさだまるデータ構造がないので、
/// 名前(= propername)がタイプのアイデンティティになる
/// このクラスは名前とシリアルナンバーのマッピングを保持する

        template <class C> class DbTypeManager {};

        template <> class DbTypeManager<mem> {
        public:
            typedef serial<dtype<mem> > serial_type;
            DbTypeManager() {};
            ~DbTypeManager() {};
    
            serial<dtype<mem> > GetSerialCount() {return (serial<dtype<mem> >) type::GetSerialCount();};
            std::pair<bool, serial<dtype<mem> > > GetDbtSerial(propername tn) {
                module *m;
                type *t;
                if(!(m = module::SearchModule(tn.module_name))) return std::pair<bool, serial<dtype<mem> > >(false, 0);
                if(!(t = m->Search(tn.simple_name))) return std::pair<bool, serial<dtype<mem> > >(false, 0);
                return std::pair<bool, serial<dtype<mem> > >(true, t->GetSerialNo());
            }
            std::pair<bool, propername> GetPropername(serial<dtype<mem> > ds) {
                const type *t = type::Serial((tserial) ds.v);
                if(t)
                    return std::pair<bool, propername>(true, propername(t->GetModule()->GetName(),
																		(t->GetSimpleName())));
                else
                    return std::pair<bool, propername>(false, propername("", ""));
            }
            std::pair<bool, serial<dtype<mem> > > add(propername) { return std::pair<bool, serial<dtype<mem> > >(false, 0); };
            void getAppFeatures(serial<dtype<mem> > ts, std::vector<serial<dtype<mem> > >&ff) {
                const type *t = type::Serial((tserial) ts.v);
                int nf = t->GetNFeatures();
                for(int i = 0 ; i < nf ; i++) {
                    ff.push_back((serial<dtype<mem> >) t->Feature(i)->GetRepType()->GetSerialNo());
                }
            };
        };
#ifdef WITH_BDBM
        template <> class DbTypeManager<bdb> {
            Db* class_dbp;
            Db* data_dbp;
            serial<dtype<bdb> > serial_count;
            typedef _HASHMAP<propername, serial<dtype<bdb> >, hash_propername, equal_to_propername> pn2ds_hash;
            typedef _HASHMAP<serial<dtype<bdb> >, propername, hash_serial_dtype_bdb, equal_to_serial_dtype_bdb> ds2pn_hash;
            pn2ds_hash pn2ds;
            ds2pn_hash ds2pn;
        public:
            typedef serial<dtype<bdb> > serial_type;
            DbTypeManager(Db* x, Db* y) : serial_count(0) {
                class_dbp = x;
                data_dbp = y;
    
                    // resume serial_count
                Dbt key((void*)"serial_count", strlen("serial_count") + 1);
                Dbt val(&serial_count, sizeof(serial<dtype<bdb> >));
                val.set_flags(DB_DBT_USERMEM);
                val.set_ulen(sizeof(serial<dtype<bdb> >));
    
                if( class_dbp->get(NULL, &key, &val, 0) != 0) {
                        // create new serial_count
                    serial_count.v = 0;
                    Dbt val(&serial_count, sizeof(serial<dtype<bdb> >));
                    class_dbp->put(NULL, &key, &val, 0);
                }

                std::cout << "dtype serial_count: " << serial_count.v << std::endl;
    
                    // resume propername<->id mapping
                    // note: propername is allocated by malloc

                serial<dtype<bdb> > ds(0);
                Dbt key2(&ds, sizeof(serial<dtype<bdb> >));
                key2.set_flags(DB_DBT_USERMEM);
                key2.set_ulen(sizeof(serial<dtype<bdb> >));
                Dbt val2;
                val2.set_flags(DB_DBT_MALLOC);

    
                Dbc *dbcp;
                data_dbp->cursor(NULL, &dbcp, 0);

                DB_BTREE_STAT *statp;
                data_dbp->stat(&statp, 0);
    
                    //    cout << "BTREE: bt_magic: " << (u_int32_t) statp->bt_magic << endl;
                    //    cout << "BTREE: bt_version: " << (u_int32_t) statp->bt_version << endl;
                    //    cout << "BTREE: bt_nkeys: " << (u_int32_t) statp->bt_nkeys << endl;
                    //    cout << "BTREE: bt_ndata: " << (u_int32_t) statp->bt_ndata << endl;
                    //    cout << "BTREE: bt_pagesize: " << (u_int32_t) statp->bt_pagesize << endl;
                    //    cout << "BTREE: bt_minkey: " << (u_int32_t) statp->bt_minkey << endl;
                    //    cout << "BTREE: bt_re_len: " << (u_int32_t) statp->bt_re_len << endl;
                    //    cout << "BTREE: bt_re_pad: " << (u_int32_t) statp->bt_re_pad << endl;
                    //    cout << "BTREE: bt_levels: " << (u_int32_t) statp->bt_levels << endl;
                    //    cout << "BTREE: bt_int_pg: " << (u_int32_t) statp->bt_int_pg << endl;
                    //    cout << "BTREE: bt_leaf_pg: " << (u_int32_t) statp->bt_leaf_pg << endl;
                    //    cout << "BTREE: bt_dup_pg: " << (u_int32_t) statp->bt_dup_pg << endl;
                    //    cout << "BTREE: bt_over_pg: " << (u_int32_t) statp->bt_over_pg << endl;
                    //    cout << "BTREE: bt_free: " << (u_int32_t) statp->bt_free << endl;
                    //    cout << "BTREE: bt_int_pgfree: " << (u_int32_t) statp->bt_int_pgfree << endl;
                    //    cout << "BTREE: bt_leaf_pgfree: " << (u_int32_t) statp->bt_leaf_pgfree << endl;        
                    //    cout << "BTREE: bt_dup_pgfree: " << (u_int32_t) statp->bt_dup_pgfree << endl;
                    //    cout << "BTREE: bt_over_pgfree: " << (u_int32_t) statp->bt_over_pgfree << endl;

                while(dbcp->get(&key2, &val2, DB_NEXT) == 0) {

                    propername pn((char*)val2.get_data());
        
                    std::cout << "resuming DtypeManagerPersistent " << pn.module_name << ":" << pn.simple_name << " <-> " << ds.v << std::endl;
        
                    pn2ds.insert(pn2ds_hash::value_type(pn2ds_hash::key_type(pn),
                                                        pn2ds_hash::mapped_type(ds)));
        
                    ds2pn.insert(ds2pn_hash::value_type(ds2pn_hash::key_type(ds),
                                                        ds2pn_hash::mapped_type(pn)));
                }
                dbcp->close();

            };
            ~DbTypeManager() {};
    
            serial<dtype<bdb> > GetSerialCount() { return serial_count; };
            std::pair<bool, serial<dtype<bdb> > > GetDbtSerial(propername tn) {
                pn2ds_hash::iterator it = pn2ds.find(tn);
                if(it == pn2ds.end() ) // not found
                    return std::pair<bool, serial<dtype<bdb> > >(false, serial<dtype<bdb> >(0));
                return std::pair<bool, serial<dtype<bdb> > >(true, it->second);
            };
            std::pair<bool, propername> GetPropername(serial<dtype<bdb> > ds) {
                ds2pn_hash::iterator it = ds2pn.find(ds);
                if(it == ds2pn.end() ) // not found
                    return std::pair<bool, propername>(false, propername((char*) NULL, (char*) NULL));
                return std::pair<bool, propername>(true, it->second);
            };
            std::pair<bool, serial<dtype<bdb> > > add(propername pn) {
                pn2ds_hash::iterator it = pn2ds.find(pn);

                if(it != pn2ds.end()) { //exist
                    return std::pair<bool, serial<dtype<bdb> > >(false, it->second);
                } else { // not exist
                    serial<dtype<bdb> > ds = serial_count;
                    serial_count.v++;

                        //        cout << "add dserial_count: " << ds << endl;
        
                    pn2ds.insert(pn2ds_hash::value_type(pn2ds_hash::key_type(pn),
                                                        pn2ds_hash::mapped_type(ds)));
        
                    ds2pn.insert(ds2pn_hash::value_type(ds2pn_hash::key_type(ds),
                                                        ds2pn_hash::mapped_type(pn)));

                        // update serial_count
                    Dbt key((void*)"serial_count", strlen("serial_count") + 1);
                    Dbt val(&serial_count, sizeof(serial<dtype<bdb> >));
                    class_dbp->put(NULL, &key, &val, 0);
        
                        // add data
                    std::vector<char> buffer;
                    pn.Serialize(buffer);
                    Dbt key2(&ds, sizeof(serial<dtype<bdb> >));
                    Dbt val2(&(buffer[0]), buffer.size());
                    data_dbp->put(NULL, &key2, &val2, 0);
        
                    return std::pair<bool, serial<dtype<bdb> > >(true, ds);
                }
	    
            };
            void getAppFeatures(serial<dtype<bdb> >, std::vector<serial<dtype<bdb> > >&ff) {
                int n = serial_count.v;
                for(int i = 0; i < n ; i++)
                    ff.push_back(serial<dtype<bdb> >(i));
            };
        };
#endif // WITH_BDBM

/////////////////////////
///  DbTypeMapper
///  ２つのDbTypeManager a, bの間のマッピングをとるクラス

        template <class A, class B> class DbTypeMapper {
        protected:
            typedef _HASHMAP<serial<dtype<A> >, serial<dtype<B> >, serial_hash<dtype<A> >, serial_equal_to<dtype<A> > > dsa2dsb_hash;
            typedef _HASHMAP<serial<dtype<B> >, serial<dtype<A> >, serial_hash<dtype<B> >, serial_equal_to<dtype<B> > > dsb2dsa_hash;
            dsa2dsb_hash a2b;
            dsb2dsa_hash b2a;
            DbTypeManager<A> *a;
            DbTypeManager<B> *b;
            serial<dtype<A> > valid_serial_count_a;
            serial<dtype<B> > valid_serial_count_b;
            void syncA() {
                serial<dtype<A> > serial_count_a = a->GetSerialCount();
	    
                for(; valid_serial_count_a.v < serial_count_a.v ; valid_serial_count_a.v++) {

                    if( a2b.find(valid_serial_count_a) == a2b.end() ) { // no mapping
		    
                        std::pair<bool, propername> r = a->GetPropername(valid_serial_count_a);
                        std::pair<bool, serial<dtype<B> > > s = b->GetDbtSerial(r.second);
                        if(s.first) { // propername of da is found in B
                            serial<dtype<B> > db = s.second;
                            a2b.insert(dsa2dsb_hash::value_type(dsa2dsb_hash::key_type(valid_serial_count_a),
                                                                dsa2dsb_hash::mapped_type(db)));
                            b2a.insert(dsb2dsa_hash::value_type(dsb2dsa_hash::key_type(db),
                                                                dsb2dsa_hash::mapped_type(valid_serial_count_a)));
                        }
                    }
                }
            };
            void syncB() {
                serial<dtype<B> > serial_count_b = b->GetSerialCount();
	    
                for(; valid_serial_count_b.v < serial_count_b.v ; valid_serial_count_b.v++) {
		
                    if( b2a.find(valid_serial_count_b) == b2a.end() ) { // no mapping
		    
                        std::pair<bool, propername> r = b->GetPropername(valid_serial_count_b);
                        std::pair<bool, serial<dtype<A> > > s = a->GetDbtSerial(r.second);
                        if(s.first) { // propername of db is found in A
                            serial<dtype<A> > da = s.second;
                            a2b.insert(dsa2dsb_hash::value_type(dsa2dsb_hash::key_type(da),
                                                                dsa2dsb_hash::mapped_type(valid_serial_count_b)));
                            b2a.insert(dsb2dsa_hash::value_type(dsb2dsa_hash::key_type(valid_serial_count_b),
                                                                dsb2dsa_hash::mapped_type(da)));
                        }
                    }
                }
            };
            void sync() {
                syncA();
                syncB();
            };
        public:
            DbTypeMapper(DbTypeManager<A>* x, DbTypeManager<B>* y) : a(x), b(y), valid_serial_count_a(0), valid_serial_count_b(0) {};
            ~DbTypeMapper() {};
            std::pair<bool, serial<dtype<B> > > A2B(serial<dtype<A> > da) {
                typename dsa2dsb_hash::iterator it = a2b.find(da);
                if( it != a2b.end() ) { // found
                    return std::pair<bool, serial<dtype<B> > >(true, it->second);
                }
                    // not found -- sync and try again
                sync();
                it = a2b.find(da);
                if( it != a2b.end() ) { // found
                    return std::pair<bool, serial<dtype<B> > >(true, it->second);
                }
                return std::pair<bool, serial<dtype<B> > >(false, 0);
            };
            std::pair<bool, serial<dtype<A> > > B2A(serial<dtype<B> > db) {
                typename dsb2dsa_hash::iterator it = b2a.find(db);
                if( it != b2a.end() ) { // found
                    return std::pair<bool, serial<dtype<A> > >(true, it->second);
                }
                    // not found -- sync and try again
                sync();
                it = b2a.find(db);
                if( it != b2a.end() ) { // found
                    return std::pair<bool, serial<dtype<A> > >(true, it->second);
                }
                return std::pair<bool, serial<dtype<A> > >(false, 0);
            };
            std::pair<bool, serial<dtype<B> > > CoerceA2B(serial<dtype<A> > da) {
                typename dsa2dsb_hash::iterator it = a2b.find(da);
                if( it != a2b.end() ) { // found
                    return std::pair<bool, serial<dtype<B> > >(true, it->second);
                }
                    // not found -- sync and try again
                sync();
                it = a2b.find(da);
                if( it != a2b.end() ) { // found
                    return std::pair<bool, serial<dtype<B> > >(true, it->second);
                }
                    // not found -- add new type
                if( da.v >= a->GetSerialCount().v ) { // da does not exist in A, CoerceA2B fails iff da does not exist in A
                    return std::pair<bool, serial<dtype<B> > >(false, 0);
                }
                return b->add(a->GetPropername(da).second);
            };
            std::pair<bool, serial<dtype<A> > > CoerceB2A(serial<dtype<B> > db) {
                typename dsb2dsa_hash::iterator it = b2a.find(db);
                if( it != b2a.end() ) { // found
                    return std::pair<bool, serial<dtype<A> > >(true, it->second);
                }
                    // not found
                sync();
                it = b2a.find(db);
                if( it != b2a.end() ) { // found
                    return std::pair<bool, serial<dtype<A> > >(true, it->second);
                }
                    // not found -- add new type
	    
                if( db.v >= b->GetSerialCount().v ) { // db does not exist in B, CoerceB2A fails iff db does not exist in B
                    return std::pair<bool, serial<dtype<A> > >(false, 0);
                }
                return a->add(b->GetPropername(db).second);
            };
            void print() {
                typename dsa2dsb_hash::iterator it = a2b.begin(), last = a2b.end();
                for(; it != last ; it++) {
                    std::cout << "A2B: " << it->first << "<->" << it->second
                         << " (" << a->GetPropername(it->first).second.module_name
                         << ":" << a->GetPropername(it->first).second.simple_name
                         << "<->" << b->GetPropername(it->second).second.module_name
                         << ":" << b->GetPropername(it->second).second.simple_name
                         << ")" << std::endl;
                }
                it = b2a.begin(), last = b2a.end();
                for(; it != last ; it++) {
                    std::cout << "B2A: " << it->first << "<->" << it->second
                         << " (" << b->GetPropername(it->first).second.module_name
                         << ":" << b->GetPropername(it->first).second.simple_name
                         << "<->" << a->GetPropername(it->second).second.module_name
                         << ":" << a->GetPropername(it->second).second.simple_name
                         << ")" << std::endl;
                }
            };
        };

/////////////////////////////
/// DbFeatureMapper
///

        template <class A, class B> class DbFeatureMapper {
        protected:
            template <class C> struct hash_mapping {
                size_t operator()(std::pair<serial<dtype<C> >, serial<dtype<C> > > x) const { return size_t(x.first.v + x.second.v); }
            };

            template <class C> struct equal_to_mapping {
                bool operator()(std::pair<serial<dtype<C> >, serial<dtype<C> > > x,
                                std::pair<serial<dtype<C> >, serial<dtype<C> > > y) const {
                    return x == y;
                }
            };
            typedef _HASHMAP<serial<dtype<A> >, serial<dtype<A> >, serial_hash<dtype<A> >, serial_equal_to<dtype<A> > > dsa2dsa_hash;
            typedef _HASHMAP<serial<dtype<B> >, serial<dtype<B> >, serial_hash<dtype<B> >, serial_equal_to<dtype<B> > > dsb2dsb_hash;
            typedef _HASHMAP<std::pair<serial<dtype<A> >, serial<dtype<A> > >, int, hash_mapping<A>, equal_to_mapping<A> > a2b_hash;
            typedef _HASHMAP<std::pair<serial<dtype<B> >, serial<dtype<B> > >, int, hash_mapping<B>, equal_to_mapping<B> > b2a_hash;


            a2b_hash a2b;
            b2a_hash b2a;

            DbTypeManager<A> *a;
            DbTypeManager<B> *b;
            DbTypeMapper<A, B> *type_mapper;

            dsa2dsa_hash valid_serial_count_a;
            dsb2dsb_hash valid_serial_count_b;

                //        dbt2dbt_hash nmap_a;
                //        dbt2dbt_hash nmap_b;

            void sync(serial<dtype<A> > t, serial<dtype<B> > u) {
                    // t and u are assumed to have the same propername
                serial<dtype<A> > serial_count_a = a->GetSerialCount();
                serial<dtype<B> > serial_count_b = b->GetSerialCount();

                typename dsa2dsa_hash::iterator ita = valid_serial_count_a.find(t);
                typename dsb2dsb_hash::iterator itb = valid_serial_count_b.find(u);

                if( ita == valid_serial_count_a.end() ) { // not found
                    valid_serial_count_a.insert(dsa2dsa_hash::value_type(dsa2dsa_hash::key_type(t),
                                                                         dsa2dsa_hash::mapped_type(0)));
                        //		nmap_a.insert(dbt2dbt_hash::value_type(dbt2dbt_hash::key_type(t),
                        //						       dbt2dbt_hash::mapped_type(0)));

                }

                if( itb == valid_serial_count_b.end() ) { // not found
                    valid_serial_count_b.insert(dsb2dsb_hash::value_type(dsb2dsb_hash::key_type(u),
                                                                         dsb2dsb_hash::mapped_type(0)));
                        //		nmap_b.insert(dbt2dbt_hash::value_type(dbt2dbt_hash::key_type(u),
                        //						       dbt2dbt_hash::mapped_type(0)));
                }

                ita = valid_serial_count_a.find(t);
                itb = valid_serial_count_b.find(u);

                bool need_new_mapping = false;

                for(; ita->second.v < serial_count_a.v ; ita->second.v++) {
                    std::pair<bool, serial<dtype<B> > > b = type_mapper->A2B(ita->second);
                    if( b.first ) {
                        typename a2b_hash::iterator mp = a2b.find(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, ita->second));
                        if( mp == a2b.end() ) { // not found -- new mapping
                            need_new_mapping = true;
                        }
                    }
                }

                for(; itb->second.v < serial_count_b.v ; itb->second.v++) {
                    std::pair<bool, serial<dtype<A> > > a = type_mapper->B2A(itb->second);
                    if( a.first ) {
                        typename b2a_hash::iterator mp = b2a.find(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, itb->second));
                        if( mp == b2a.end() ) { // not found -- new mapping
                            need_new_mapping = true;
                        }
                    }
                }

                if( need_new_mapping ) {
                    make_new_mapping(t, u);
                }

            };
            void make_new_mapping(serial<dtype<A> > t, serial<dtype<B> > u) {
                std::vector<serial<dtype<A> > > ff;
                std::vector<serial<dtype<B> > > gg;
                int fcounter = 0, gcounter = 0;
                dsa2dsa_hash ff_hash, ff_order;
                dsb2dsb_hash gg_hash, gg_order;

                a->getAppFeatures(t, ff);
                b->getAppFeatures(u, gg);

                for(uint i = 0; i < ff.size() ; i++) {
                    ff_hash.insert(dsa2dsa_hash::value_type(dsa2dsa_hash::key_type(ff[i]),
                                                            dsa2dsa_hash::mapped_type(0)));
                }
                for(uint j = 0; j < gg.size() ; j++) {
                    gg_hash.insert(dsb2dsb_hash::value_type(dsb2dsb_hash::key_type(gg[j]),
                                                            dsb2dsb_hash::mapped_type(0)));
                }

                for(uint i = 0; i < ff.size() ; i++) {
                    std::pair<bool, serial<dtype<B> > > g = type_mapper->A2B(ff[i]);
                    if( g.first && (gg_hash.find(g.second) != gg_hash.end()) ) {
                        ff_order.insert(dsa2dsa_hash::value_type(dsa2dsa_hash::key_type(ff[i]),
                                                                 dsa2dsa_hash::mapped_type(fcounter++)));
                    }
                }
                for(uint j = 0; j < gg.size() ; j++) {
                    std::pair<bool, serial<dtype<A> > > f = type_mapper->B2A(gg[j]);
                    if( f.first && (ff_hash.find(f.second) != ff_hash.end()) ) {
                        gg_order.insert(dsb2dsb_hash::value_type(dsb2dsb_hash::key_type(gg[j]),
                                                                 dsb2dsb_hash::mapped_type(gcounter++)));
                    }
                }

                typename dsa2dsa_hash::iterator it = ff_order.begin(), last = ff_order.end();
                for(; it != last ; it++) {
                    serial<dtype<A> > f = it->first;
                    int fidx = it->second.v;
                    serial<dtype<B> > g = type_mapper->A2B(f).second;
                    typename dsb2dsb_hash::iterator gr = gg_order.find(g);
                    ASSERT(gr != gg_order.end() );
                    int gidx = gr->second.v;
                    typename a2b_hash::iterator itidx = a2b.find(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, f));
                    if( itidx != a2b.end() ) { //found
                        itidx->second = gidx;
                    } else { // not found -- add mapping
                        a2b.insert(a2b_hash::value_type(a2b_hash::key_type(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, f)),
                                                        a2b_hash::mapped_type(gidx)));
                    }
		
                    typename b2a_hash::iterator itidx2 = b2a.find(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, g));
                    if( itidx2 != b2a.end() ) { // found
                        itidx2->second = fidx;
                    } else { // not found -- add mapping
                        b2a.insert(b2a_hash::value_type(b2a_hash::key_type(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, g)),
                                                        b2a_hash::mapped_type(fidx)));
                    }
                }
            };
  
        public:
            DbFeatureMapper(DbTypeManager<A> *x, DbTypeManager<B> *y, DbTypeMapper<A,B> *z) : a(x), b(y), type_mapper(z) {};
            ~DbFeatureMapper() {};
            bool A2B(serial<dtype<A> > t, std::vector<serial<dtype<A> > >& feature, std::vector<int>& feature_order) {
                    // feature_order is supposed to be empty std::vector
                bool perfect_mapping = true;
                typename std::vector<serial<dtype<A> > >::iterator f = feature.begin(), flast = feature.end();

                for(; f != flast ; f++) {
    
                    typename a2b_hash::iterator it = a2b.find(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, *f));

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

                    std::pair<bool, serial<dtype<B> > > uu = type_mapper->A2B(t);
                    if( ! uu.first ) { return false; }

                    sync(t, uu.second);

                    for(f = feature.begin() ; f != flast ; f++) {

                        typename a2b_hash::iterator it = a2b.find(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, *f));
      
                        if( it != a2b.end() ) { // found
                            feature_order.push_back(it->second);
                        } else {
                            feature_order.push_back(-1);
                        }
                    }
                    return true;
                }
            };
            bool B2A(serial<dtype<B> > u, std::vector<serial<dtype<B> > >& feature, std::vector<int>& feature_order) {
                    // feature_order is assumed to be empty std::vector
                bool perfect_mapping = true;
                typename std::vector<serial<dtype<B> > >::iterator g = feature.begin(), glast = feature.end();

                for(; g != glast ; g++) {
    
                    typename b2a_hash::iterator it = b2a.find(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, *g));

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

    
                    std::pair<bool, serial<dtype<A> > > tt = type_mapper->B2A(u);
                    if( ! tt.first ) { return false; }

                    sync(tt.second, u);

                    for(g = feature.begin() ; g != glast ; g++) {

                        typename b2a_hash::iterator it = b2a.find(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, *g));
      
                        if( it != b2a.end() ) { // found
                            feature_order.push_back(it->second);
                        } else {
                            feature_order.push_back(-1);
                        }
                    }
                    return true;
                }
            };
            bool CoerceA2B(serial<dtype<A> > t, std::vector<serial<dtype<A> > >& feature, std::vector<int>& feature_order) {
                    // feature_order is supposed to be empty std::vector
                bool perfect_mapping = true;
                typename std::vector<serial<dtype<A> > >::iterator f = feature.begin(), flast = feature.end();

                for(; f != flast ; f++) {
    
                    typename a2b_hash::iterator it = a2b.find(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, *f));

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

                    std::pair<bool, serial<dtype<B> > > uu = type_mapper->CoerceA2B(t);
                    if( ! uu.first ) { return false; }
                    for(f = feature.begin() ; f != flast ; f++) { // ensure g (<- f)
                        if(! type_mapper->CoerceA2B(*f).first )
                            return false;
                    }

                    sync(t, uu.second);

                    for(f = feature.begin() ; f != flast ; f++) {

                        typename a2b_hash::iterator it = a2b.find(std::pair<serial<dtype<A> >, serial<dtype<A> > >(t, *f));
      
                        if( it != a2b.end() ) { // found
                            feature_order.push_back(it->second);
                        } else {
                            std::cerr << "internal error in DbFeatureMapper::CoerceA2B" << std::endl;
                            return false;
                        }
                    }
                    return true;
                }
            };
            bool CoerceB2A(serial<dtype<B> > u, std::vector<serial<dtype<B> > >& feature, std::vector<int>& feature_order) {
                    // feature_order is supposed to be empty std::vector
                bool perfect_mapping = true;
                typename std::vector<serial<dtype<B> > >::iterator g = feature.begin(), glast = feature.end();

                for(; g != glast ; g++) {
    
                    typename b2a_hash::iterator it = b2a.find(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, *g));

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

                    std::pair<bool, serial<dtype<A> > > tt = type_mapper->CoerceB2A(u);
                    if( ! tt.first ) { return false; }
                    for(g = feature.begin() ; g != glast ; g++) { // ensure f (<- g)
                        if(! type_mapper->CoerceB2A(*g).first )
                            return false;
                    }
                        //		cout << "tt.second" << tt.second.v << endl;

                    sync(tt.second, u);
                        //		cout << "sync end" << endl;

                    for(g = feature.begin() ; g != glast ; g++) {

                        typename b2a_hash::iterator it = b2a.find(std::pair<serial<dtype<B> >, serial<dtype<B> > >(u, *g));
      
                        if( it != b2a.end() ) { // found
                            feature_order.push_back(it->second);
                        } else {
                            std::cerr << "internal error in DbFeatureMapper::CoerceB2A" << std::endl;
                            return false;
                        }
                    }
                    return true;
                }
            };
        };

    } // end of namespace lildbm
} // end of namespace lilfes

#endif // __lildbm_type_h
// end of lildbm-type.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2004/05/07 15:47:05  ninomi
 * FSPのserialize, unserializeの際にstringの中身をvector<cell>に書き込むようにした
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
