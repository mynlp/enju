/*
 * $Id: lildbm-db.h,v 1.28 2011-05-02 08:48:59 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-db.h
//  LdbmDb
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_db_h
#define __lildbm_db_h

#include "builtin.h"
#include "lildbm-basic.h"
#include "lildbm-schema.h"
#include "lildbm-type.h"
#include "lildbm-cell.h"
#include "lildbm-core.h"

#include "gfstream.h"
#include "tokenizer.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define INITIALHASHSIZE 1024
		// For compatibility; now it means nothing.

namespace lilfes {
	
	namespace lildbm {
		
		extern feature *ldbm_addr0_f;
		extern feature *ldbm_addr1_f;
		extern feature *ldbm_addr2_f;
		extern feature *ldbm_entry_id_f;

		class LdbmEnvBase;
		class LdbmDbBase {
		public:
			typedef uint32 serial_type;
			
		private:
			bool is_open;
			std::list<LdbmSchemaBase*> *schema;
			
		protected:
			bool isOpen() { return is_open; }
			void setOpen(bool x) { is_open = x; }
			std::list<LdbmSchemaBase*>* getSchema() { return schema; }
			void setSchema(std::list<LdbmSchemaBase*> *sc) {
				if( schema ) {
					std::list<LdbmSchemaBase*>::iterator it = schema->begin(), last = schema->end();
					for(; it != last ; it++) {
						delete (*it);
					}
					delete schema;
				}
				schema = sc;
			}
			void setNullSchema() {
				if( schema ) {
					std::list<LdbmSchemaBase*>::iterator it = schema->begin(), last = schema->end();
					for(; it != last ; it++) {
						delete (*it);
					}
					delete schema;
				}
				schema = 0;
			}
			bool sanityCheck(std::vector<FSP>& entry) {
				std::vector<FSP>::iterator it = entry.begin(), last = entry.end();
				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin(), slast = getSchema()->end();
				for(; it != last ; it++, sit++) {
					if(sit == slast) {
						return false;
					}
					if(! (*sit)->SanityCheck(*it) ) {
						return false;
					}
				}
				return true;
			}
			bool sanityCheckKey(std::vector<FSP>& entry) {
				std::vector<FSP>::iterator it = entry.begin(), last = entry.end();
				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin(), slast = getSchema()->end();
				for(; it != last ; it++, sit++) {
					if(sit == slast) {
						return false;
					}
					if(! (*sit)->SanityCheckKey(*it) ) {
						return false;
					}
				}
				return true;
			}
			bool Entry2KeyFSList(std::vector<FSP>& entry, FSP keyfs) {
				std::vector<FSP>::iterator it = entry.begin(), last = entry.end();
				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin(), slast = getSchema()->end();
				for(; it != last; it++, sit++) {
					if(sit == slast) {
						return false;
					}
					FSP arg = *it;
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_KEY_INTEGER:
						case LDBM_SCHEMA_KEY_INTHASH:
						case LDBM_SCHEMA_KEY_STRING:
						case LDBM_SCHEMA_KEY_FLOAT:
						case LDBM_SCHEMA_KEY_TYPE:
						case LDBM_SCHEMA_KEY_FS:
						{
							if(! keyfs.Coerce(cons)) {
								return false;
							}
							if(! keyfs.Follow(hd).Unify(arg)) {
								return false;
							}
							keyfs = keyfs.Follow(tl);
							break;
						}
						case LDBM_SCHEMA_VALUE_INTEGER: 
						case LDBM_SCHEMA_VALUE_STRING:
						case LDBM_SCHEMA_VALUE_FLOAT:
						case LDBM_SCHEMA_VALUE_TYPE:
						case LDBM_SCHEMA_VALUE_FS:
						{
							break;
						}
						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}
				return true;
			}

		public:
			LdbmDbBase() : is_open(false), schema(0) {};
			virtual ~LdbmDbBase() {
				setNullSchema();
			}
			virtual bool find(machine*, std::vector<FSP>&) = 0;
			virtual bool find(machine*, LIT<ldb>, std::vector<FSP>&) = 0;
			virtual bool insert(machine*, std::vector<FSP>&) = 0;
			virtual bool erase(machine*, std::vector<FSP>&) = 0;
			virtual bool clear() = 0;
			virtual bool findall(machine*, std::vector<std::vector<FSP> >&) = 0;
			virtual std::pair<bool, LIT<ldb> > first() = 0;
			virtual std::pair<bool, LIT<ldb> > last() = 0;
			virtual std::pair<bool, LIT<ldb> > next(LIT<ldb>) = 0;
			virtual std::pair<bool, LIT<ldb> > prev(LIT<ldb>) = 0;
			virtual bool close() = 0;
			virtual bool sync() = 0;
			virtual bool save(const char*) = 0;
			virtual bool load(machine*, const char*) = 0;
			virtual void print_statistics() = 0;
			virtual void reserve(size_t) = 0;
			virtual size_t size() = 0;
			virtual size_t capacity() = 0;
		};

		template <class C> class __LdbmDb : public LdbmDbBase {
		protected:
			DCore<C> core;
			typename C::dcore_p base;
			serial<C> serial_count;
		public:

			__LdbmDb() : LdbmDbBase(), serial_count(0) {};
			~__LdbmDb() {}
			bool findEntryValue(machine *mach, typename C::dcore_p addr, std::vector<FSP>& arglist) {
				typename C::dcp2lcp_map mark;
				typename C::dcore_p entry_base = addr + 3;
				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin();
				std::vector<FSP>::iterator ait = arglist.begin(), alast = arglist.end();
				for(int i = 0; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_VALUE_INTEGER: 
						{
							if(! ait->Unify(FSP(mach, C::dc2INT(core.read(entry_base + i))))) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_VALUE_STRING:
						{
							core.readFS(mach, entry_base + i, mark);
							if(! ait->Unify(FSP(mach, mark.find(core.DDeref(entry_base+i))->second)) ) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_VALUE_FLOAT:
						{
							if(! ait->Unify(FSP(mach, (float) C::dc2FLO(core.read(entry_base + i))))) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_VALUE_TYPE:
						{
							if(! ait->Coerce(type::Serial(C::dc2VAR(core.read(entry_base + i)).v))) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_VALUE_FS:
						{
							core.readFS(mach, entry_base + i, mark);
							if(! ait->Unify(FSP(mach, mark.find(core.DDeref(entry_base+i))->second)) ) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_KEY_INTEGER:
						case LDBM_SCHEMA_KEY_INTHASH:
						case LDBM_SCHEMA_KEY_STRING:
						case LDBM_SCHEMA_KEY_FLOAT:
						case LDBM_SCHEMA_KEY_TYPE:
						case LDBM_SCHEMA_KEY_FS:
						{
							break;
						}
						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}
				return true;
			};

			bool findEntry(machine *mach, typename C::dcore_p addr, std::vector<FSP>& arglist) {
				typename C::dcp2lcp_map mark;
				typename C::dcore_p entry_base = addr + 3;
				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin();
				std::vector<FSP>::iterator ait = arglist.begin(), alast = arglist.end();
				for(int i = 0; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_KEY_INTEGER: case LDBM_SCHEMA_KEY_INTHASH: case LDBM_SCHEMA_VALUE_INTEGER: 
						{
							if(! ait->Unify(FSP(mach, C::dc2INT(core.read(entry_base + i))))) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_KEY_STRING: case LDBM_SCHEMA_VALUE_STRING:
						{
							core.readFS(mach, entry_base + i, mark);
							if(! ait->Unify(FSP(mach, mark.find(core.DDeref(entry_base+i))->second)) ) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_KEY_FLOAT: case LDBM_SCHEMA_VALUE_FLOAT:
						{
							if(! ait->Unify(FSP(mach, (float) C::dc2FLO(core.read(entry_base + i))))) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_KEY_TYPE: case LDBM_SCHEMA_VALUE_TYPE:
						{
							if(! ait->Coerce(type::Serial(C::dc2VAR(core.read(entry_base + i)).v))) {
								return false;
							}
							break;
						}
						case LDBM_SCHEMA_KEY_FS: case LDBM_SCHEMA_VALUE_FS:
						{
							core.readFS(mach, entry_base + i, mark);
							if(! ait->Unify(FSP(mach, mark.find(core.DDeref(entry_base+i))->second)) ) {
								return false;
							}
							break;
						}
						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}
				return true;
			};

			std::pair<bool, typename C::dcore_p> newEntry(serial<C> count) {

				typename C::dcore_p entry_p = core.getCorePointer();
				typename C::dcore_p tail = C::dc2PTR(core.read(base+1));

				core.push(C::INT2dc(count.v));
				core.push(C::PTR2dc(tail));
				core.push(C::PTR2dc(base));

				int n = getSchema()->size() - 1;
				for(int i = 0; i < n; i++ ) {
					core.push(C::PTR2dc(0)); // dummy
				}

				core.write(tail + 2, C::PTR2dc(entry_p)); // next of the tail
				core.write(base + 1, C::PTR2dc(entry_p)); // prev of the base

				return std::pair<bool, typename C::dcore_p>(true, entry_p);
			};

			bool eraseEntry(mem::dcore_p dcp) {
				core.write(dcp, C::INT2dc(0)); // entry_id is overwritten by 0
				typename C::dcore_p prev = core.read(dcp+1);
				typename C::dcore_p next = core.read(dcp+2);

				core.write(prev+2, C::PTR2dc(next)); // next of the prev
				core.write(next+1, C::PTR2dc(prev)); // prev of the next
				return true;
			};
			bool findall(machine* mach, std::vector<std::vector<FSP> >& r) {
				typename C::dcore_p dcp = C::dc2PTR(core.read(base + 2)); // the first entry
				while(dcp != base) {
					r.push_back(std::vector<FSP>());
					std::vector<FSP>& entry = r.back();
					int n = getSchema()->size() - 1;
					for(int i = 0 ; i < n ; i++) {
						entry.push_back(FSP(mach));
					}
					if(! findEntry(mach, dcp, entry) ) {
						return false;
					}
					dcp = C::dc2PTR(core.read(dcp+2));
				}
				return true;
			};
			std::pair<bool, LIT<ldb> > first() {
				typename C::dcore_p dcp = C::dc2PTR(core.read(base + 2)); // the first entry
				if(dcp == base) return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				mint eid = C::dc2INT(core.read(dcp));
				return std::pair<bool, LIT<ldb> >(true, LIT<ldb>(eid));
			};
			std::pair<bool, LIT<ldb> > last() {
				typename C::dcore_p dcp = C::dc2PTR(core.read(base + 1)); // the last entry
				if(dcp == base) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}
				mint eid = C::dc2INT(core.read(dcp));
				return std::pair<bool, LIT<ldb> >(true, LIT<ldb>(eid));
			};

		};
	
		template <class C> class LdbmDb {};
		template <> class LdbmDb<mem> : public __LdbmDb<mem> {
		private:
			DbTreeBase* root;
		protected:
			DbTreeBase* getRoot() { return root; }
			void setNewRoot() {
				if( root ) {
					delete root;
				}
				root = new DbTree<c_root, c_dontcare>();
			}
			void setNullRoot() {
				if( root ) {
					delete root;
				}
				root = 0;
			}
			std::vector<mem::dcore_p> entry_addr;
			static const int serialize_size = 32768;

			void writeFS(machine *mach, core_p addr0, core_p base0, mem::dcore_p base1) {
				
				cell c;
				mem::dcore_p addr1;

				while( IsPTR(c = mach->ReadHeap(addr0))) {
					addr0 = c2PTR(c);
				}
	
				switch(Tag(c)) {
				case T_PTR:
				case T_PTR1:
				case T_PTR2:
				case T_PTR3:
					ABORT("internal heap error");
			
				case T_VAR:
				case T_INT:
				case T_FLO:
				case T_CHR:
					{
						addr1 = core.getCorePointer();
						mach->WriteHeap(addr0, PTR2c(base0 + (addr1 - base1)));
						core.push(mem::cell2dcell(c));
						break;
					}
				case T_STG:
					{
						addr1 = core.getCorePointer();
						mach->WriteHeap(addr0, PTR2c(base0 + (addr1 - base1)));

						const char *str = c2STG(c);
						int len = (int) strlen(str);
						core.push(mem::STG2dc(len));
						int k = 0;
						for(; k < len ; ) {
							mem::dcell ic = 0;
							for(int j = 0 ; j < mem::charlen ; j++, k++) {
								ic = ic << 8;
								if( k < len ) {
									unsigned char ustr = *(str++);
									ic |= (mem::dcell) ustr;
								}
							}
							core.push(mem::CHR2dc(ic));
						}
						break;
					}
				case T_STR:
					//					mach->SetTrailPoint();

					{  // Perform actual copy
						static std::vector< std::pair<core_p, mem::dcore_p> > array(serialize_size);
						array.clear();

						addr1 = core.getCorePointer();
						array.push_back( std::make_pair(addr0,addr1) );
						int slot = GetSlot(base0);
						core.resize(addr1+1);

						while( ! array.empty() ) {
							addr0 =  array.back().first;
							addr1 = array.back().second; 
							array.pop_back();
										
							c = mach->ReadHeap(addr0);
								
							if( IsPTR(c) ) {
								core.write(addr1, mem::PTR2dc(base1 + (c2PTR(c) - base0)));
							} else {
								unsigned nf = c2STR(c)->GetNFeatures();

								mem::dcore_p hp1 = core.getCorePointer();
								mach->WriteHeap(addr0, PTR2c(base0 + hp1 - base1));
								core.write(addr1, mem::PTR2dc(hp1));

								core.resize(hp1 + nf + 1);
								core.write(hp1, mem::cell2dcell(c));
						
								for( unsigned i=nf; i>0; i-- ) {
									core_p a = addr0 + i;
									while( IsPTR(c = mach->ReadHeap(a)) ) {
										a = c2PTR(c);
										if( (GetSlot(a) == slot) && a >= base0 ) {
											core.write(hp1 + i, mem::PTR2dc(base1 + (a - base0)));
											goto hogehoge;
										}
									}
									
									if( IsSTR(c) ) {
										array.push_back( std::make_pair(a, hp1 + i) );
									} else if( IsSTG(c) ) {
										addr1 = core.getCorePointer();
										mach->WriteHeap(a, PTR2c(base0 + addr1 - base1));
										core.write(hp1+i, mem::PTR2dc(addr1));
										
										const char *str = c2STG(c);
										int len = (int) strlen(str);
										core.push(mem::STG2dc(len));
										int k = 0;
										for(; k < len ; ) {
											mem::dcell ic = 0;
											for(int j = 0 ; j < mem::charlen ; j++, k++) {
												ic = ic << 8;
												if( k < len ) {
													unsigned char ustr = *(str++);
													ic |= (mem::dcell) ustr;
												}
											}
											core.push(mem::CHR2dc(ic));
										}
									} else {
										mach->WriteHeap(a, PTR2c(base0 + (hp1 + i - base1)));
										core.write(hp1+i, mem::cell2dcell(c));
									}
								hogehoge: ;
								}
							}
						}
						//						mach->TrailBack();
					}
					break;
				default:
					ABORT("Unknown Tag!!!!");
					break;
				}
			}

			bool writeEntry(machine *mach, mem::dcore_p entry_p, std::vector<FSP>& arglist) {

				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin();
				std::vector<FSP>::iterator ait = arglist.begin(), alast = arglist.end();
			
				core_p base0 = mach->GetHP();
				mem::dcore_p base1 = core.getCorePointer();

				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
					case LDBM_SCHEMA_KEY_INTEGER: case LDBM_SCHEMA_KEY_INTHASH:
						{
							core.write(entry_p + 3 + i, mem::INT2dc(ait->ReadInteger()));
							break;
						}
					case LDBM_SCHEMA_KEY_STRING:
						{
							mach->SetTrailPoint();
							mem::dcore_p hp1 = core.getCorePointer();
							writeFS(mach, ait->GetAddress(), base0, base1);
							core.write(entry_p + 3 + i, mem::PTR2dc(hp1));
							mach->TrailBack();
							break;
						}
					case LDBM_SCHEMA_KEY_FLOAT:
						{
							core.write(entry_p + 3 + i, mem::FLO2dc(ait->ReadFloat()));
							break;
						}
					case LDBM_SCHEMA_KEY_TYPE:
						{
							core.write(entry_p + 3 + i, mem::VAR2dc(ait->GetType()->GetSerialNo()));
							break;
						}
					case LDBM_SCHEMA_KEY_FS:
						{
							mach->SetTrailPoint();
							mem::dcore_p hp1 = core.getCorePointer();
							writeFS(mach, ait->GetAddress(), base0, base1);
							core.write(entry_p + 3 + i, mem::PTR2dc(hp1));
							mach->TrailBack();
							break;
						}
					default:
						{
						}
					}
				}

				ait = arglist.begin();
				sit = getSchema()->begin();

				mach->SetTrailPoint();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
					case LDBM_SCHEMA_VALUE_INTEGER:
						{
							core.write(entry_p + 3 + i, mem::INT2dc(ait->ReadInteger()));
							break;
						}
					case LDBM_SCHEMA_VALUE_STRING:
						{
							mem::dcore_p hp1 = core.getCorePointer();
							writeFS(mach, ait->GetAddress(), base0, base1);
							core.write(entry_p + 3 + i, mem::PTR2dc(hp1));
							break;
						}
					case LDBM_SCHEMA_VALUE_FLOAT:
						{
							core.write(entry_p + 3 + i, mem::FLO2dc(ait->ReadFloat()));
							break;
						}
					case LDBM_SCHEMA_VALUE_TYPE:
						{
							core.write(entry_p + 3 + i, mem::VAR2dc(ait->GetType()->GetSerialNo()));
							break;
						}
					case LDBM_SCHEMA_VALUE_FS:
						{
							mem::dcore_p hp1 = core.getCorePointer();
							writeFS(mach, ait->GetAddress(), base0, base1);
							core.write(entry_p + 3 + i, mem::PTR2dc(hp1));
							break;
						}
					default:
						{
						}
					}
				}
				mach->TrailBack();
				return true;
			};
			
			bool overwriteEntry(machine *mach, mem::dcore_p entry_p, std::vector<FSP>& arglist) {

				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin();
				std::vector<FSP>::iterator ait = arglist.begin(), alast = arglist.end();
			
				core_p base0 = mach->GetHP();
				mem::dcore_p base1 = core.getCorePointer();

				ait = arglist.begin();
				sit = getSchema()->begin();

				mach->SetTrailPoint();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
					case LDBM_SCHEMA_VALUE_INTEGER:
						{
							core.write(entry_p + 3 + i, mem::INT2dc(ait->ReadInteger()));
							break;
						}
					case LDBM_SCHEMA_VALUE_STRING:
						{
							mem::dcore_p hp1 = core.getCorePointer();
							writeFS(mach, ait->GetAddress(), base0, base1);
							core.write(entry_p + 3 + i, mem::PTR2dc(hp1));
							break;
						}
					case LDBM_SCHEMA_VALUE_FLOAT:
						{
							core.write(entry_p + 3 + i, mem::FLO2dc(ait->ReadFloat()));
							break;
						}
					case LDBM_SCHEMA_VALUE_TYPE:
						{
							core.write(entry_p + 3 + i, mem::VAR2dc(ait->GetType()->GetSerialNo()));
							break;
						}
					case LDBM_SCHEMA_VALUE_FS:
						{
							mem::dcore_p hp1 = core.getCorePointer();
							writeFS(mach, ait->GetAddress(), base0, base1);
							core.write(entry_p + 3 + i, mem::PTR2dc(hp1));
							break;
						}
					default:
						{
						}
					}
				}
				mach->TrailBack();
				return true;
			};
		public:
			LdbmDb() : __LdbmDb<mem>(), root(0) {}
			~LdbmDb() {
				if( isOpen() ) {
					close();
				}
				setNullRoot();
			}

			void print_statistics() {
					// core
				std::cout << "core: " << core.capacity() << ", " << core.size() << std::endl;
					// entry_addr
				std::cout << "entry_addr: " << entry_addr.capacity() << ", " << entry_addr.size() << std::endl;
			}
			void reserve(size_t sz) {
				core.reserve(sz);
			}
			size_t size() {
				return core.size();
			}
			size_t capacity() {
				return core.capacity();
			}

			bool open(std::list<LdbmSchemaBase*> *sc) {
				setSchema(sc);
				setNewRoot();
				base = core.getCorePointer();
				core.push(mem::INT2dc(serial_count.v));
				core.push(mem::PTR2dc(base)); // previous entry
				core.push(mem::PTR2dc(base)); // next entry
				serial_count.v++;
				entry_addr.push_back(base);
				setOpen(true);
				return true;
			}
			bool close() {
				if( isOpen() ) {
					setNullRoot();
					setNullSchema();
					setOpen(false);
				}
				return true;
			}
	
			bool find(machine* mach, std::vector<FSP>& arglist) {
				if(! sanityCheckKey(arglist) ) {
					return false;
				}
				std::pair<bool, LIT<mem> > r = getRoot()->find(getSchema()->begin(), arglist.begin());
				if(! r.first) {
					return false;
				}
				mem::dcore_p dcp = r.second.pointer;
				return findEntryValue(mach, dcp, arglist);
			};
			bool find(machine* mach, LIT<ldb> lit, std::vector<FSP>& arglist) {
				int n = getSchema()->size() - 1;
				for(int i = 0 ; i < n ; i++) {
					arglist.push_back(FSP(mach));
				}
				
				mem::dcore_p dcp = entry_addr[lit.entry_id];
				if(lit.entry_id != mem::dc2INT(core.read(dcp))) {
					return false;
				}
				return findEntry(mach, dcp, arglist);
			};
			bool insert(machine* mach, std::vector<FSP>& arglist) {
				if(! sanityCheck(arglist) ) {
					return false;
				}
				triple<bool, bool, LIT<mem>* > r = getRoot()->insert(getSchema()->begin(), arglist.begin());
				if(! r.first) {
					return false;
				}

				if(r.second) { // new entry
					std::pair<bool, mem::dcore_p> s = newEntry(serial_count);
					if(!s.first) {
						return false;
					}
					*(r.third) = LIT<mem>(s.second, serial_count);
					serial_count.v++;
					entry_addr.push_back(s.second);
					return writeEntry(mach, r.third->pointer, arglist);
				}
				return overwriteEntry(mach, r.third->pointer, arglist);
			};
			bool erase(machine*, std::vector<FSP>& arglist) {
				if(! sanityCheckKey(arglist) ) {
					return false;
				}
				triple<bool, bool, LIT<mem> > r = getRoot()->erase(getSchema()->begin(), arglist.begin());
				if(! r.first) {
					return false;
				}
				mem::dcore_p dcp = r.third.pointer;
				return eraseEntry(dcp);
			};
			bool clear() {
				setNewRoot();
				serial_count.v = 0;
				core.clear();
				base = core.getCorePointer();
				core.push(mem::INT2dc(serial_count.v));
				core.push(mem::PTR2dc(base)); // previous entry
				core.push(mem::PTR2dc(base)); // next entry
				serial_count.v++;
				entry_addr.clear();
				entry_addr.push_back(base);
				return true;
			};
			std::pair<bool, LIT<ldb> > next(LIT<ldb> lit) {
				mem::dcore_p dcp0 = entry_addr[lit.entry_id];
				if(lit.entry_id != mem::dc2INT(core.read(dcp0))) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}

				mem::dcore_p dcp = mem::dc2PTR(core.read(dcp0 + 2)); // next entry

				if(dcp == base) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}
				mint eid = mem::dc2INT(core.read(dcp));
				return std::pair<bool, LIT<ldb> >(true, LIT<ldb>(eid));
			};
			std::pair<bool, LIT<ldb> > prev(LIT<ldb> lit) {
				mem::dcore_p dcp0 = entry_addr[lit.entry_id];
				if(lit.entry_id != mem::dc2INT(core.read(dcp0))) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}

				mem::dcore_p dcp = mem::dc2PTR(core.read(dcp0 + 1)); // prev entry

				if(dcp == base) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}
				mint eid = mem::dc2INT(core.read(dcp));
				return std::pair<bool, LIT<ldb> >(true, LIT<ldb>(eid));
			};

			bool sync() { return true; };
			bool save(const char *filename) {
#ifndef WITH_ZLIB
				RUNERR( "Saving database requires zlib, but this binary does not support zlib" );
				return false;
#else // ifndef WITH_ZLIB
				ogfstream fout( filename, std::ios::out | std::ios::trunc );

				if ( !fout ) {
					return false;
				}

				fout << "SYSTEM BEGIN\n";
#ifdef CELL_64BIT
				fout << "BIT 64\n";
#else
				fout << "BIT 32\n";
#endif
				fout << "BSP " << base << '\n';

				fout << "CORESIZE " << core.getCorePointer() << '\n';
				//std::cout << "save: " << filename << ": core pointer: " << core.getCorePointer() << std::endl;
				//std::cout << "save: " << filename << ": core size: " << core.size() << std::endl;
				//std::cout << "save: " << filename << ": core capacity: " << core.capacity() << std::endl;

				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin(), slast = getSchema()->end();

				for(; sit != slast ; sit++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_KEY_INTEGER:
							fout << "SCH KEY_INTEGER\n"; break;
						case LDBM_SCHEMA_KEY_INTHASH:
							fout << "SCH KEY_INTHASH\n"; break;
						case LDBM_SCHEMA_KEY_FLOAT:
							fout << "SCH KEY_FLOAT\n"; break;
						case LDBM_SCHEMA_KEY_STRING:
							fout << "SCH KEY_STRING\n"; break;
						case LDBM_SCHEMA_KEY_TYPE:
							fout << "SCH KEY_TYPE\n"; break;
						case LDBM_SCHEMA_KEY_FS:
							fout << "SCH KEY_FS\n"; break;
						case LDBM_SCHEMA_VALUE_INTEGER:
							fout << "SCH VALUE_INTEGER\n"; break;
						case LDBM_SCHEMA_VALUE_STRING:
							fout << "SCH VALUE_STRING\n"; break;
						case LDBM_SCHEMA_VALUE_FLOAT:
							fout << "SCH VALUE_FLOAT\n"; break;
						case LDBM_SCHEMA_VALUE_TYPE:
							fout << "SCH VALUE_TYPE\n"; break;
						case LDBM_SCHEMA_VALUE_FS:
							fout << "SCH VALUE_FS\n"; break;
						case LDBM_SCHEMA_ENTRY:
							fout << "SCH ENTRY\n"; break;
						default:
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
				}
				
				fout << "SYSTEM END\n";
				fout.flush();

				for(mem::dcore_p cp = 0; cp < core.getCorePointer() ; cp++ ) {
					mem::dcell c = core.read(cp);
					switch(mem::DTag(c)) {
						case DT_VAR:
						{
							
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "VAR " << type::Serial(mem::dc2VAR(c).v)->GetName() << '\n';
							break;
						}
						case DT_INT:
						{
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "INT " << mem::dc2INT(c) << '\n';
							break;
						}
						case DT_FLO:
						{
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "FLO " << mem::dc2FLO(c) << '\n';
							break;
						}
						case DT_STG:
						{
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "STG " << mem::dc2STG(c) << '\n';
							break;
						}
						case DT_CHR:
						{
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "CHR";
							for(int i = 0 ; i < mem::charlen; i++) {
								c = c << 8;
								unsigned char ustr = mem::dc2CHR0(c);
								fout << " " << (int) ustr;
							}
							fout << '\n';
							break;
						}
						case DT_PTR: case DT_PTR1: case DT_PTR2: case DT_PTR3:
						{
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "PTR " << mem::dc2PTR(c) << '\n';
							break;
						}
						case DT_STR:
						{
							const type *t = type::Serial(mem::dc2STR(c).v);
							//fout << "[" << std::setw(8) << std::setfill('0') << cp << "] ";
							fout << "STR " << t->GetNFeatures() << " " << t->GetName() << '\n';
							break;
						}
						
						default:
						ABORT("error in LdbmDb<mem>::save");
					}
				}
				fout.flush();
				return true;
#endif // ifndef WITH_ZLIB
			};
			

			type* gettype(const std::string &s) {
				int n = s.length();
				if(n == 0) {
					return (type*)NULL;
				}
				
				bool quoted_module = (s[0] == '\'');
				bool quoted_type = (s[n-1] == '\'');

				int module_spos = quoted_module ? 1 : 0;
				int module_epos = -1;
				int type_spos = -1;
				int type_epos = quoted_type ? n-1 : n;

				if(quoted_module) {
					for(int i = 1 ; i < n-1 ; i++) {
						if( s[i] == '\'' && s[i+1] == '\'' ) { // skip ''
							i++;
							continue;
						}
						if( s[i] == '\'' && s[i+1] == ':' ) { // the end of module
							module_epos = i;
							type_spos = quoted_type ? i+3 : i+2;
							if (type_spos >= type_epos) {
								RUNWARN("cannot interpret " << s << " as a type");
								return (type*)NULL;
							}
							break;
						}
						if( s[i] == '\'' && s[i+1] != ':' ) { // module name is not followed by ':'
							RUNWARN("module name is not followed by module-type separator ':'");
							return (type*)NULL;
						}
					}
				} else {
					for(int i = 0 ; i < n ; i++) {
						if( s[i] == ':' ) { // the end of module
							module_epos = i;
							type_spos = quoted_type ? i+2 : i+1;
							if(type_spos >= type_epos) {
								RUNWARN("cannot interpret " << s << " as a type");
								return (type*)NULL;
							}
							break;
						}
					}
				}
				if( module_epos == -1 || type_spos == -1 ) {
					RUNWARN("cannot interpret " << s << " as a type");
					return (type*)NULL;
				}
				std::string module_name = s.substr(module_spos, module_epos - module_spos);
				std::string type_name = s.substr(type_spos, type_epos - type_spos);
				
				module* md = module::SearchModule(module_name.c_str());
				if (!md) {
					RUNWARN("Module not found " << module_name);
					return (type*)NULL;
				}
				return md->Search(type_name.c_str());
			}
			
			bool load(machine* mach, const char *filename) {
#ifndef WITH_ZLIB
				RUNERR( "Loading database requires zlib, but this binary does not support zlib" );
				return false;
#else // ifndef WITH_ZLIB
				igfstream fin( filename, std::ios::in );

				if ( !fin ) {
					return false;
				}
				
				std::string line;

// clear
				setNewRoot();
				serial_count.v = 0;
				core.clear();

// load system information
				std::list<LdbmSchemaBase*>::iterator sit = getSchema()->begin(), slast = getSchema()->end();
				
				while(std::getline(fin, line) && line != "SYSTEM BEGIN") {}
				while(std::getline(fin, line) && line != "SYSTEM END") {
					if(line == "") continue;
					
					std::string buffer;
					std::stringstream sin(line);
					sin >> buffer;
					if( buffer == "") {
						continue;
					} else if( buffer == "BIT" ) {
						if(!(sin >> buffer)) {
							RUNWARN("fail to load --- not integer in BIT line");
							return false;
						}
#ifdef CELL_64BIT
						if( buffer != "64" ) {
							RUNWARN("fail to load --- different bit-size machine architecture");
							return false;
						}
#else
						if( buffer != "32" ) {
							RUNWARN("fail to load --- different bit-size machine architecture");
							return false;
						}
#endif
					} else if( buffer == "BSP" ) {
						if(! (sin >> base)) {
							RUNWARN("fail to load --- fail to load base pointer");
							return false;
						}
					} else if( buffer == "CORESIZE" ) {
						mem::dcore_p cp;
						if(! (sin >> cp) ) {
							RUNWARN("fail to load core pointer");
							return false;
						}
						core.resize(cp);
					} else  if( buffer == "SCH" ) {
						if(sit == slast) {
							RUNWARN("fail to load --- inconsistent schema");
							return false;
						}
						sin >> buffer;
						if( buffer == "KEY_INTEGER" &&
							(*sit)->getSchemaType() == LDBM_SCHEMA_KEY_INTEGER ) {
							sit++; continue;
						} else if( buffer == "KEY_INTHASH" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_KEY_INTHASH) {
							sit++; continue;
						} else if( buffer == "KEY_FLOAT" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_KEY_FLOAT) {
							sit++; continue;
						} else if( buffer == "KEY_STRING" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_KEY_STRING) {
							sit++; continue;
						} else if( buffer == "KEY_TYPE" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_KEY_TYPE) {
							sit++; continue;
						} else if( buffer == "KEY_FS" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_KEY_FS) {
							sit++; continue;
						} else if( buffer == "VALUE_INTEGER" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_VALUE_INTEGER) {
							sit++; continue;
						} else if( buffer == "VALUE_FLOAT" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_VALUE_FLOAT) {
							sit++; continue;
						} else if( buffer == "VALUE_STRING" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_VALUE_STRING) {
							sit++; continue;
						} else if( buffer == "VALUE_TYPE" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_VALUE_TYPE) {
							sit++; continue;
						} else if( buffer == "VALUE_FS" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_VALUE_FS) {
							sit++; continue;
						} else if( buffer == "ENTRY" &&
								   (*sit)->getSchemaType() == LDBM_SCHEMA_ENTRY) {
							sit++; continue;
						} else {
							RUNWARN("fail to load --- inconsistent schema type");
							return false;
						}
					}
				}

				tokenizer tk( fin );
				mem::dcore_p cp = 0;
				while( tk.readNextToken() ) {
					const std::string buffer = tk.getToken();
					if(buffer == "") {
						continue;
					} else if(buffer == "VAR") {
						tk.readNextToken();
						type * t = gettype(tk.getToken());
						if(t) {
							serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
							core.write(cp++, mem::VAR2dc(mem_sn));
						} else {
							RUNWARN("fail to load --- undefined type " << tk.getToken());
							return false;
						}
					} else if(buffer == "INT") {
						int buffer;
						if(!(tk.nextToken(buffer))) {
							RUNWARN("fail to load --- non integer in INT line ");
							return false;
						}
						core.write(cp++, mem::INT2dc((mint) buffer));
					} else if(buffer == "FLO") {
						float buffer;
						if(!(tk.nextToken(buffer))) {
							RUNWARN("fail to load --- non float in FLO line");
							return false;
						}
						core.write(cp++, mem::FLO2dc((mfloat) buffer));
					} else if(buffer == "STG") {
						int buffer;
						if(!(tk.nextToken(buffer))) {
							RUNWARN("fail to load --- non integer in STG line");
							return false;
						}
						core.write(cp++, mem::STG2dc(buffer));
					} else if(buffer == "CHR") {
						mem::dcell c = 0;
						unsigned int buffer;
						for(int i = 0 ; i < mem::charlen ; i++) {
							if(!(tk.nextToken(buffer))) {
								RUNWARN("fail to load --- non integer in CHR line");
								return false;
							}
							c = c << 8;
							c |= (mem::dcell) buffer;
						}
						core.write(cp++, mem::CHR2dc(c));
					} else if(buffer == "PTR") {
						mem::dcore_p buffer;
						if(!(tk.nextToken(buffer))) {
							RUNWARN("fail to load --- non unsigned integer in PTR line");
							return false;
						}
						core.write(cp++, mem::PTR2dc(buffer));
					} else if(buffer == "STR") {
						int nf;
						if(! (tk.nextToken(nf))) {
							RUNWARN("fail to load --- number of features does not follow STR tag");
							return false;
						}
						tk.readNextToken();
						type * t = gettype(tk.getToken());
						if(t) {
							if(nf != t->GetNFeatures() ) {
								RUNWARN("fail to load --- inconsistent number of features in STR line");
								return false;
							}
							serial<dtype<mem> > mem_sn = (serial<dtype<mem> >) t->GetSerialNo();
							core.write(cp++, mem::STR2dc(mem_sn));
						} else {
							RUNWARN("fail to load --- undefined type" << tk.getToken());
							return false;
						}
						
					} else {
					  //ABORT("error in LdbmDb<mem>::load");
					  RUNERR("error in LdbmDb<mem>::load");
					  return false;
					}
				}

				//std::cout << "load: " << filename << ": cp: " << cp << std::endl;
				//std::cout << "load: " << filename << ": core size: " << core.size() << std::endl;
				//std::cout << "load: " << filename << ": core capacity: " << core.capacity() << std::endl;
				
					// key mappings reconstruction

				serial_count.v++;
				entry_addr.clear();
				entry_addr.push_back(base);
				
				mem::dcore_p dcp = mem::dc2PTR(core.read(base + 2)); // the first entry
				while(dcp != base) {

					code* ip = mach->GetIP();  core_p cp = mach->SetTrailPoint(NULL);
					
					std::vector<FSP> arglist;
					int n = getSchema()->size() - 1;
					for(int i = 0 ; i < n ; i++) {
						arglist.push_back(FSP(mach));
					}
					
					if(! findEntry(mach, dcp, arglist) ) {
						return false;
					}
					
					triple<bool, bool, LIT<mem>* > r = getRoot()->insert(getSchema()->begin(), arglist.begin());
					if(! r.first) {
						return false;
					}
					*(r.third) = LIT<mem>(dcp, serial_count);
					serial_count.v++;
					entry_addr.push_back(dcp);
					dcp = mem::dc2PTR(core.read(dcp+2));
					
					mach->TrailBack(cp);  mach->SetIP(ip);
				}
				return true;
#endif // ifndef WITH_ZLIB
			}
			static bool save(machine* mach, const char *filename, std::vector<FSP>& data) {
				LdbmDb<mem> db;
				std::list<LdbmSchemaBase*> *sc = new std::list<LdbmSchemaBase*>();
				sc->push_back(new LdbmSchema<c_key, c_inthash>(INITIALHASHSIZE));
				sc->push_back(new LdbmSchema<c_value, c_fs>());
				sc->push_back(new LdbmSchema<c_entry, c_dontcare>());
				
				if(! db.open(sc)) {
					std::list<LdbmSchemaBase*>::iterator it = sc->begin(), last = sc->end();
					for(; it != last ; it++) {
						delete (*it);
					}
					delete sc;
					return false;
				}

				std::vector<FSP>::iterator it = data.begin(), last = data.end();
				for(int i = 0; it != last ; it++, i++) {
					std::vector<FSP> entry;
					entry.push_back(FSP(mach, (mint) i));
					entry.push_back(*it);
					db.insert(mach, entry);
				}

				db.save(filename);
				if(! db.close() ) {
					std::cerr << "fail to close" << std::endl;
					return false;
				}
				
				return true;
			}
			static bool load(machine* mach, const char *filename, std::vector<FSP>& data) {
				data.clear();
				
				LdbmDb<mem> db;
				
				std::list<LdbmSchemaBase*> *sc = new std::list<LdbmSchemaBase*>();
				sc->push_back(new LdbmSchema<c_key, c_inthash>(INITIALHASHSIZE));
				sc->push_back(new LdbmSchema<c_value, c_fs>());
				sc->push_back(new LdbmSchema<c_entry, c_dontcare>());
				
				if(! db.open(sc)) {
					std::list<LdbmSchemaBase*>::iterator it = sc->begin(), last = sc->end();
					for(; it != last ; it++) {
						delete (*it);
					}
					delete sc;
					return false;
				}
				
				db.load(mach, filename);
				
				std::vector<std::vector<FSP> > r;
				if(! db.findall(mach, r)) {
					return false;
				}
				
				std::vector<std::vector<FSP> >::iterator it = r.begin(), last = r.end();
				for(; it != last; it++) {
					data.push_back((*it)[1]);
				}
				
				if(! db.close()) {
					std::cerr << "fail to close" << std::endl;
					return false;
				}
				return true;
			}
			static bool save(machine* mach, const char *filename, std::map<int, FSP>& data) {
				LdbmDb<mem> db;
				std::list<LdbmSchemaBase*> *sc = new std::list<LdbmSchemaBase*>();
				sc->push_back(new LdbmSchema<c_key, c_inthash>(INITIALHASHSIZE));
				sc->push_back(new LdbmSchema<c_value, c_fs>());
				sc->push_back(new LdbmSchema<c_entry, c_dontcare>());
				
				if(! db.open(sc)) {
					std::list<LdbmSchemaBase*>::iterator it = sc->begin(), last = sc->end();
					for(; it != last ; it++) {
						delete (*it);
					}
					delete sc;
					return false;
				}

				std::map<int, FSP>::iterator it = data.begin(), last = data.end();
				for(; it != last ; it++) {
					std::vector<FSP> entry;
					entry.push_back(FSP(mach, (mint) it->first));
					entry.push_back(it->second);
					db.insert(mach, entry);
				}

				db.save(filename);
				if(! db.close() ) { std::cerr << "fail to close" << std::endl; return false;}
				
				return true;
			}
			static bool load(machine* mach, const char *filename, std::map<int, FSP>& data) {
				data.clear();
				
				LdbmDb<mem> db;
				
				std::list<LdbmSchemaBase*> *sc = new std::list<LdbmSchemaBase*>();
				sc->push_back(new LdbmSchema<c_key, c_inthash>(INITIALHASHSIZE));
				sc->push_back(new LdbmSchema<c_value, c_fs>());
				sc->push_back(new LdbmSchema<c_entry, c_dontcare>());
				
				if(! db.open(sc)) {
					std::list<LdbmSchemaBase*>::iterator it = sc->begin(), last = sc->end();
					for(; it != last ; it++) {
						delete (*it);
					}
					delete sc;
					return false;
				}
				
				db.load(mach, filename);
				
				std::vector<std::vector<FSP> > r;
				if(! db.findall(mach, r)) {
					return false;
				}
				
				std::vector<std::vector<FSP> >::iterator it = r.begin(), last = r.end();
				for(; it != last; it++) {
					int i = (*it)[0].ReadInteger();
					FSP v = (*it)[1];
					data.insert(std::map<int, FSP>::value_type(std::map<int, FSP>::key_type(i),
															   std::map<int, FSP>::mapped_type(v)));
				}
				
				if(! db.close()) {
					std::cerr << "fail to close" << std::endl;
					return false;
				}
				return true;
			}
		};
	
#ifdef WITH_BDBM
		template <> class LdbmDb<bdb> : public __LdbmDb<bdb> {
		protected:
			LdbmEnvBase *my_envp;
			std::string filename;
			int mode;
			int bdbm_page_size;
			int cache_size;
			int flags;
			DbTypeManager<bdb> *bdb_dtype_manager;
			DbTypeManager<mem> *mem_dtype_manager;
			DbTypeMapper<bdb, mem> *type_mapper;
			DbFeatureMapper<bdb, mem> *feat_mapper;
			Db *dtype_class_dbp;
			Db *dtype_data_dbp;
			Db *db_class_dbp;
			Db *core_class_dbp;
			Db *core_dbp;
			Db *assoc_dbp;
			Db *entry_addr_dbp;

			bool writeEntry(machine *mach, bdb::dcore_p entry_p, std::vector<FSP>& arglist) {
				core_p_mark mark;
				core_p_mark ss;
				core_p_mark nfhash;

				std::list<LdbmSchemaBase*>::iterator sit;
				std::vector<FSP>::iterator ait = arglist.begin(), alast = arglist.end();
			
				sit = getSchema()->begin();
				ait = arglist.begin();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_VALUE_STRING:
						case LDBM_SCHEMA_VALUE_FS:
						{
							DCore<bdb>::markStructureSharing(mach, ait->GetAddress(), mark, ss);
							break;
						}
						case LDBM_SCHEMA_KEY_INTEGER:
						case LDBM_SCHEMA_KEY_INTHASH:
						case LDBM_SCHEMA_KEY_STRING:
						case LDBM_SCHEMA_KEY_FLOAT: 
						case LDBM_SCHEMA_KEY_TYPE:
						case LDBM_SCHEMA_KEY_FS:
						case LDBM_SCHEMA_VALUE_TYPE:
						case LDBM_SCHEMA_VALUE_FLOAT:
						case LDBM_SCHEMA_VALUE_INTEGER:
							break;

						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}

				mark.clear();
				sit = getSchema()->begin();
				ait = arglist.begin();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_VALUE_STRING:
						case LDBM_SCHEMA_VALUE_FS:
						{
							DCore<bdb>::markDefault(mach, ait->GetAddress(), mark, ss, nfhash, (const type*) NULL);
							break;
						}
						case LDBM_SCHEMA_KEY_INTEGER:
						case LDBM_SCHEMA_KEY_INTHASH:
						case LDBM_SCHEMA_KEY_STRING:
						case LDBM_SCHEMA_KEY_FLOAT: 
						case LDBM_SCHEMA_KEY_TYPE:
						case LDBM_SCHEMA_KEY_FS:
						case LDBM_SCHEMA_VALUE_INTEGER:
						case LDBM_SCHEMA_VALUE_FLOAT:
						case LDBM_SCHEMA_VALUE_TYPE:
							break;

						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}

				bdb::lcp2dcp_map mapping;
				sit = getSchema()->begin();
				ait = arglist.begin();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_KEY_INTEGER: case LDBM_SCHEMA_KEY_INTHASH: case LDBM_SCHEMA_VALUE_INTEGER:
						{
							core.write(entry_p + 3 + i, bdb::INT2dc(ait->ReadInteger()));
							break;
						}
						case LDBM_SCHEMA_KEY_STRING:
						{
							core_p_mark key_ss;
							core_p_mark key_nfhash;
							bdb::lcp2dcp_map key_mapping;
							mark.clear();
							DCore<bdb>::markStructureSharing(mach, ait->GetAddress(), mark, key_ss);
							mark.clear();
							DCore<bdb>::markDefault(mach, ait->GetAddress(), mark, key_ss, key_nfhash, (const type*) NULL);
							core.writeFS(mach, ait->GetAddress(),key_mapping, key_ss, key_nfhash, (const type*) NULL);
							core.write(entry_p + 3 + i, bdb::PTR2dc(key_mapping.find(mach->Deref(ait->GetAddress()))->second));
							break;
						}
						case LDBM_SCHEMA_VALUE_STRING:
						{
							core.writeFS(mach, ait->GetAddress(),mapping, ss, nfhash, (const type*) NULL);
							core.write(entry_p + 3 + i, bdb::PTR2dc(mapping.find(mach->Deref(ait->GetAddress()))->second));
							break;
						}
						case LDBM_SCHEMA_KEY_FLOAT: case LDBM_SCHEMA_VALUE_FLOAT:
						{
							core.write(entry_p + 3 + i, bdb::FLO2dc(ait->ReadFloat()));
							break;
						}
						case LDBM_SCHEMA_KEY_TYPE: case LDBM_SCHEMA_VALUE_TYPE:
						{
							core.write(entry_p + 3 + i, bdb::VAR2dc(ait->GetType()->GetSerialNo()));
							break;
						}
						case LDBM_SCHEMA_KEY_FS:
						{
							core_p_mark key_ss;
							core_p_mark key_nfhash;
							bdb::lcp2dcp_map key_mapping;
							mark.clear();
							DCore<bdb>::markStructureSharing(mach, ait->GetAddress(), mark, key_ss);
							mark.clear();
							DCore<bdb>::markDefault(mach, ait->GetAddress(), mark, key_ss, key_nfhash, (const type*) NULL);
							core.writeFS(mach, ait->GetAddress(),key_mapping, key_ss, key_nfhash, (const type*) NULL);
							core.write(entry_p + 3 + i, bdb::PTR2dc(key_mapping.find(mach->Deref(ait->GetAddress()))->second));
							break;
						}
						case LDBM_SCHEMA_VALUE_FS:
						{
							core.writeFS(mach, ait->GetAddress(),mapping, ss, nfhash, (const type*) NULL);
							core.write(entry_p + 3 + i, bdb::PTR2dc(mapping.find(mach->Deref(ait->GetAddress()))->second));
							break;
						}
						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}

				return true;
			};
			bool overwriteEntry(machine *mach, bdb::dcore_p entry_p, std::vector<FSP>& arglist) {
				core_p_mark mark;
				core_p_mark ss;
				core_p_mark nfhash;

				std::list<LdbmSchemaBase*>::iterator sit;
				std::vector<FSP>::iterator ait = arglist.begin(), alast = arglist.end();
			
				sit = getSchema()->begin();
				ait = arglist.begin();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_VALUE_STRING:
						case LDBM_SCHEMA_VALUE_FS:
						{
							DCore<bdb>::markStructureSharing(mach, ait->GetAddress(), mark, ss);
							break;
						}
						case LDBM_SCHEMA_KEY_INTEGER:
						case LDBM_SCHEMA_KEY_INTHASH:
						case LDBM_SCHEMA_KEY_STRING:
						case LDBM_SCHEMA_KEY_FLOAT: 
						case LDBM_SCHEMA_KEY_TYPE:
						case LDBM_SCHEMA_KEY_FS:
						case LDBM_SCHEMA_VALUE_TYPE:
						case LDBM_SCHEMA_VALUE_FLOAT:
						case LDBM_SCHEMA_VALUE_INTEGER:
							break;

						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}

				mark.clear();
				sit = getSchema()->begin();
				ait = arglist.begin();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_VALUE_STRING:
						case LDBM_SCHEMA_VALUE_FS:
						{
							DCore<bdb>::markDefault(mach, ait->GetAddress(), mark, ss, nfhash, (const type*) NULL);
							break;
						}
						case LDBM_SCHEMA_KEY_INTEGER:
						case LDBM_SCHEMA_KEY_INTHASH:
						case LDBM_SCHEMA_KEY_STRING:
						case LDBM_SCHEMA_KEY_FLOAT: 
						case LDBM_SCHEMA_KEY_TYPE:
						case LDBM_SCHEMA_KEY_FS:
						case LDBM_SCHEMA_VALUE_INTEGER:
						case LDBM_SCHEMA_VALUE_FLOAT:
						case LDBM_SCHEMA_VALUE_TYPE:
							break;

						default:
						{
							RUNERR("invalid LDBM_SCHEMA_ENUM");
						}
					}
				}

				bdb::lcp2dcp_map mapping;
				sit = getSchema()->begin();
				ait = arglist.begin();
				for(int i = 0 ; ait != alast ; sit++, ait++, i++) {
					LDBM_SCHEMA_ENUM st = (*sit)->getSchemaType();
					switch(st) {
						case LDBM_SCHEMA_VALUE_INTEGER:
						{
							core.write(entry_p + 3 + i, bdb::INT2dc(ait->ReadInteger()));
							break;
						}
						case LDBM_SCHEMA_VALUE_STRING:
						{
							core.writeFS(mach, ait->GetAddress(),mapping, ss, nfhash, (const type*) NULL);
							core.write(entry_p + 3 + i, bdb::PTR2dc(mapping.find(mach->Deref(ait->GetAddress()))->second));
							break;
						}
						case LDBM_SCHEMA_VALUE_FLOAT:
						{
							core.write(entry_p + 3 + i, bdb::FLO2dc(ait->ReadFloat()));
							break;
						}
						case LDBM_SCHEMA_VALUE_TYPE:
						{
							core.write(entry_p + 3 + i, bdb::VAR2dc(ait->GetType()->GetSerialNo()));
							break;
						}
						case LDBM_SCHEMA_VALUE_FS:
						{
							core.writeFS(mach, ait->GetAddress(),mapping, ss, nfhash, (const type*) NULL);
							core.write(entry_p + 3 + i, bdb::PTR2dc(mapping.find(mach->Deref(ait->GetAddress()))->second));
							break;
						}
						default:
						{
						}
					}
				}

				return true;
			};

		public:
			LdbmDb() : __LdbmDb<bdb>() {};
			~LdbmDb() { if(isOpen()) { close(); } };
			void print_statistics() {}
			void reserve(size_t) {}
			size_t size() { return 0; }
			size_t capacity() { return 0; }
			
			bool find(machine* mach, std::vector<FSP>& arglist) {
				
				if(! sanityCheckKey(arglist) ) {
					return false;
				}

				FSP keyfs(mach);
				if(! Entry2KeyFSList(arglist, keyfs) ) {
					return false;
				}
				std::vector<bdb::dcell> buffer;
				DCore<bdb>::Serialize(mach, type_mapper, feat_mapper, keyfs.GetAddress(), buffer);
			
				LIT<bdb> entry_lit;
				Dbt key((void*) &(*(buffer.begin())), sizeof(bdb::dcell) * buffer.size());
				Dbt val(&entry_lit, sizeof(LIT<bdb>));
				val.set_flags(DB_DBT_USERMEM);
				val.set_ulen(sizeof(LIT<bdb>));
				if( assoc_dbp->get(NULL, &key, &val, 0) != 0) {
					return false;
				}

				bdb::dcore_p dcp = entry_lit.pointer;
				return findEntryValue(mach, dcp, arglist);

			};
			bool find(machine* mach, LIT<ldb> lit, std::vector<FSP>& arglist) {
				int n = getSchema()->size() - 1;
				for(int i = 0 ; i < n ; i++) {
					arglist.push_back(FSP(mach));
				}
				
				mint eid = lit.entry_id;
				serial<bdb> esn(eid);
				bdb::dcore_p dcp;
				Dbt key0((void*) &esn, sizeof(serial<bdb>));
				Dbt val0((void*) &dcp, sizeof(bdb::dcore_p));
				val0.set_flags(DB_DBT_USERMEM);
				val0.set_ulen(sizeof(bdb::dcore_p));
				if( entry_addr_dbp->get(NULL, &key0, &val0, 0) != 0 ) {
					return false;
				}
				if(lit.entry_id != bdb::dc2INT(core.read(dcp))) {
					return false;
				}
				return findEntry(mach, dcp, arglist);
			};
			bool insert(machine* mach, std::vector<FSP>& arglist) {

				if(! sanityCheck(arglist) ) {
					return false;
				}
			
				FSP keyfs(mach);
				if(! Entry2KeyFSList(arglist, keyfs) ) {
					return false;
				}
				std::vector<bdb::dcell> buffer;
				DCore<bdb>::Serialize(mach, type_mapper, feat_mapper, keyfs.GetAddress(), buffer);
			
				LIT<bdb> entry_lit;
				Dbt key((void*) &(*(buffer.begin())), sizeof(bdb::dcell) * buffer.size());
				Dbt val(&entry_lit, sizeof(LIT<bdb>));
				val.set_flags(DB_DBT_USERMEM);
				val.set_ulen(sizeof(LIT<bdb>));

				if( assoc_dbp->get(NULL, &key, &val, 0) != 0) { // new entry

					std::pair<bool, bdb::dcore_p> s = newEntry(serial_count);
					if(!s.first) {
						return false;
					}
				
					entry_lit.entry_id.v = serial_count.v;
					entry_lit.pointer = s.second;
					assoc_dbp->put(NULL, &key, &val, 0);

					bdb::dcore_p dcp = s.second;;
					Dbt key2((void*) &serial_count, sizeof(serial<bdb>));
					Dbt val2((void*) &dcp, sizeof(bdb::dcore_p));
					entry_addr_dbp->put(NULL, &key2, &val2, 0);

						// update serial_count
					serial_count.v++;
					Dbt key1((void*)"serial_count", strlen("serial_count") + 1);
					Dbt val1(&serial_count, sizeof(serial<bdb>));
					db_class_dbp->put(NULL, &key1, &val1, 0);
				
					return writeEntry(mach, entry_lit.pointer, arglist);
				}
				return overwriteEntry(mach, entry_lit.pointer, arglist);
			};
			bool erase(machine* mach, std::vector<FSP>& arglist) {

				if(! sanityCheckKey(arglist) ) {
					return false;
				}

					// serialize key
				FSP keyfs(mach);
				if(! Entry2KeyFSList(arglist, keyfs) ) {
					return false;
				}
				std::vector<bdb::dcell> buffer;
				DCore<bdb>::Serialize(mach, type_mapper, feat_mapper, keyfs.GetAddress(), buffer);

					// get entry
				LIT<bdb> entry_lit;
				Dbt key((void*) &(*(buffer.begin())), sizeof(bdb::dcell) * buffer.size());
				Dbt val(&entry_lit, sizeof(LIT<bdb>));
				val.set_flags(DB_DBT_USERMEM);
				val.set_ulen(sizeof(LIT<bdb>));
				if( assoc_dbp->get(NULL, &key, &val, 0) != 0) {
					return false;
				}

					// delete assoc and entry_addr
				bdb::dcore_p dcp = entry_lit.pointer;
				serial<bdb> entry_sn = entry_lit.entry_id;
				Dbt key2((void*) &entry_sn, sizeof(serial<bdb>));

				if( assoc_dbp->del(NULL, &key, 0) != 0 ) {
					return false;
				}
				if( entry_addr_dbp->del(NULL, &key2, 0) != 0 ) {
					return false;
				}

					// delete entry on the heap
				return eraseEntry(dcp);
			};
			bool clear() {
				u_int32_t assoc_count_p, entry_addr_count_p;
				if( assoc_dbp->truncate(NULL, &assoc_count_p, 0) != 0 ) {
					return false;
				}
				if( entry_addr_dbp->truncate(NULL, &entry_addr_count_p, 0) != 0 ) {
					return false;
				}

				serial_count.v = 0;

				core.clear();
			
				base = core.getCorePointer();
				core.push(bdb::INT2dc(serial_count.v));
				core.push(bdb::PTR2dc(base)); // previous entry
				core.push(bdb::PTR2dc(base)); // next entry

				Dbt key((void*)"base", strlen("base") + 1);
				Dbt val(&base, sizeof(bdb::dcore_p));
				if( db_class_dbp->put(NULL, &key, &val, 0) != 0 ) {
					return false;
				}

					// update entry_addr
				Dbt key2((void*) &serial_count, sizeof(serial<bdb>));
				Dbt val2((void*) &base, sizeof(bdb::dcore_p));
				if( entry_addr_dbp->put(NULL, &key2, &val2, 0) != 0 ) {
					return false;
				}

					// update serial_count
				serial_count.v++;
				Dbt key3((void*)"serial_count", strlen("serial_count") + 1);
				Dbt val3(&serial_count, sizeof(serial<bdb>));
				if( db_class_dbp->put(NULL, &key3, &val3, 0) != 0 ) {
					return false;
				}

					// sync core
				core.sync();

				return true;
			};
			std::pair<bool, LIT<ldb> > next(LIT<ldb> lit) {
				mint eid = lit.entry_id;
				serial<bdb> esn(eid);
				bdb::dcore_p dcp;
				Dbt key0((void*) &esn, sizeof(serial<bdb>));
				Dbt val0((void*) &dcp, sizeof(bdb::dcore_p));
				val0.set_flags(DB_DBT_USERMEM);
				val0.set_ulen(sizeof(bdb::dcore_p));

				if( entry_addr_dbp->get(NULL, &key0, &val0, 0) != 0 ) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}

				if(lit.entry_id != bdb::dc2INT(core.read(dcp))) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}

				dcp = bdb::dc2PTR(core.read(dcp + 2)); // next entry

				if(dcp == base) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}
				eid = bdb::dc2INT(core.read(dcp));
				return std::pair<bool, LIT<ldb> >(true, LIT<ldb>(eid));
			};
			std::pair<bool, LIT<ldb> > prev(LIT<ldb> lit) {
				mint eid = lit.entry_id;
				serial<bdb> esn(eid);
				bdb::dcore_p dcp;
				Dbt key0((void*) &esn, sizeof(serial<bdb>));
				Dbt val0((void*) &dcp, sizeof(bdb::dcore_p));
				val0.set_flags(DB_DBT_USERMEM);
				val0.set_ulen(sizeof(bdb::dcore_p));
				if( entry_addr_dbp->get(NULL, &key0, &val0, 0) != 0 ) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}

				if(lit.entry_id != bdb::dc2INT(core.read(dcp))) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}

				dcp = bdb::dc2PTR(core.read(dcp + 1)); // prev entry

				if(dcp == base) {
					return std::pair<bool, LIT<ldb> >(false, LIT<ldb>());
				}
				eid = bdb::dc2INT(core.read(dcp));
				return std::pair<bool, LIT<ldb> >(true, LIT<ldb>(eid));
			};

			bool open(LdbmEnvBase *my_envp, char* filename, DBTYPE db_type, int mode, int db_flags, int cache_size, std::list<LdbmSchemaBase*> *sc);
			bool close();
			bool sync() {
				if(! core.sync() ) {
					return false;
				}
				return true;
			};
			bool save(const char*) { return false; };
			bool load(machine*, const char*) { return false; };
		};
#else //WITH_BDBM
		template <> class LdbmDb<bdb> : public __LdbmDb<bdb> {
		protected:
		
		public:
			LdbmDb() : __LdbmDb<bdb>() {};
			~LdbmDb() {};
			void print_statistics() {}
			void reserve(size_t) {}
			size_t size() { return 0; }
			size_t capacity() { return 0; }
			bool find(machine*, std::vector<FSP>&) {return false; };
			bool find(machine*, LIT<ldb>, std::vector<FSP>&) {return false; };
			bool insert(machine*, std::vector<FSP>&) {return false; };
			bool erase(machine*, std::vector<FSP>&) {return false; };
			bool clear() {return false; };
			bool findall(machine*, std::vector<std::vector<FSP> >&) {return false; };
			std::pair<bool, LIT<ldb> > first() {return std::pair<bool, LIT<ldb> >(false, LIT<ldb>()); };
			std::pair<bool, LIT<ldb> > last() {return std::pair<bool, LIT<ldb> >(false, LIT<ldb>()); };
			std::pair<bool, LIT<ldb> > next(LIT<ldb>) {return std::pair<bool, LIT<ldb> >(false, LIT<ldb>()); };
			std::pair<bool, LIT<ldb> > prev(LIT<ldb>) {return std::pair<bool, LIT<ldb> >(false, LIT<ldb>()); };
			bool close() {return false; };
			bool open(std::list<LdbmSchemaBase*>*) { return  false; };
//			bool open(LdbmEnvBase *, char*, DBTYPE, int, int, int, std::list<LdbmSchemaBase*>*) { return false;};
			bool sync() { return false; };
			bool save(const char*) { return false; };
			bool load(machine*, const char*) { return false; };
	
		};
#endif //WITH_BDBM


	} // end of namespace lildbm
} // end of namespace lilfes

#endif // __lildbm_db_h
// end of lildbm-db.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.27  2006/05/21 11:31:47  yusuke
 *
 * fix type-punning bug
 *
 * Revision 1.26  2004/09/29 11:58:23  ninomi
 * load時にメモりを無駄に食うばぐをとりました。
 *
 * Revision 1.25  2004/08/24 10:20:15  ninomi
 * DBにstringとして日本語(charで負の数)をいれられないばぐをとりました。
 *
 * Revision 1.24  2004/08/10 03:15:14  ninomi
 * 簡易save,loadにmap<int, FSP>を加えました。
 *
 * Revision 1.23  2004/08/09 15:17:01  ninomi
 * 簡易save, loadを加えました。
 * LdbmDb<mem>::save(machine*, char* filename, vector<FSP>&)でFSPのvectorを
 * ファイルに格納します。
 * LdbmDb<mem>::load(machine*, char* filename, vector<FSP>&)で空のvectorに
 * ファイル内のFSをいれていきます。
 *
 * Revision 1.22  2004/08/09 13:34:57  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.21  2004/07/26 05:34:27  ninomi
 * C++から直接DBを操作しやすくなるように一部修正した
 *
 * Revision 1.20  2004/06/28 05:48:13  yusuke
 *
 * えらーめっせーじは cerr にだすようにした。
 *
 * Revision 1.19  2004/06/14 07:55:52  yusuke
 *
 * こうそくな tokenizer をじっそう。これをつかって db_load をかきかえた。
 *
 * Revision 1.18  2004/06/01 15:24:55  ninomi
 * db_reserve, db_size, db_capacity
 * db_reserve_k, db_size_k, db_capacity_k
 * db_reserve_m, db_size_m, db_capacity_mを加えました。
 *
 * Revision 1.17  2004/05/30 13:42:07  ninomi
 * lildbm終了時のごみをちゃんとdeleteするようにしました
 *
 * Revision 1.16  2004/05/27 12:59:09  ninomi
 * print_statisticsというbuilt-inをくわえました。
 *
 * Revision 1.15  2004/05/27 06:58:58  ninomi
 * db_loadのresizeバージョンです。
 *
 * Revision 1.14  2004/05/26 12:23:43  ninomi
 * bdbのほうのoverwriteも改良しました。
 *
 * Revision 1.12  2004/05/25 13:20:30  yusuke
 *
 * db_load をこうそくか。
 *
 * Revision 1.11  2004/05/25 03:32:13  yusuke
 *
 * zlib をつかえるようにした。
 *
 * Revision 1.10  2004/05/23 07:39:01  yusuke
 *
 * std::endl -> '\n'
 *
 * Revision 1.9  2004/05/22 22:44:00  ninomi
 * lildbmのmem版のcoreへの書き込みの高速化＆メモリ節約化
 * lildbmのmem版のcoreへのsaveのアドレス消去
 *
 * Revision 1.8  2004/04/23 08:28:39  yusuke
 *
 * std:: をつけました。
 *
 * Revision 1.7  2004/04/22 07:48:44  ninomi
 * lildbmにdb_saveとdb_loadが加わりました。
 *
 * Revision 1.6  2004/04/20 10:22:36  ninomi
 * save, load機能追加中
 *
 * Revision 1.5  2004/04/19 08:49:13  ninomi
 * sys/waitのincludeの順番によってコンパイルできない現象のバグをとりました。
 * profile.hの中のlilfes namespace内でsignal.hをincludeしているのが原因でした。
 *
 * Revision 1.4  2004/04/16 10:52:59  ninomi
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/14 12:48:26  ninomi
 * Berkeley DB依存のデータや、lilfesの特定のタイプ依存のデータを
 * lildbm-interface.cppに移しました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
