/*
 *
 * $Id: lildbm-dbtree.cpp,v 1.6 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "lildbm-dbtree.h"
#include "lildbm-schema.h"
#include "lildbm-core.h"

#include <list>
#include <utility>
#include <vector>

namespace lilfes {

using std::list;
using std::pair;
using std::vector;

    namespace lildbm {

        template <class DataT> 
        triple<bool, bool, LIT<mem>* >
        __DbTree<c_value, DataT>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            if(next) { schema++; entry++; return next->insert(schema, entry); }
            schema++;
            next = (*schema)->newDbTree();
            entry++;
            return next->insert(schema, entry);
        };

        template class __DbTree<c_value, c_integer>;
        template class __DbTree<c_value, c_float>;
        template class __DbTree<c_value, c_type>;
        template class __DbTree<c_value, c_fs>;
        template class __DbTree<c_value, c_string>;
        
            // DbTree<c_root>
        pair<bool, LIT<mem> > DbTree<c_root, c_dontcare>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            if(! next) return pair<bool, LIT<mem> >(false, LIT<mem>());
            return next->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_root, c_dontcare>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            if(next) return next->insert(schema, entry);
            next = (*schema)->newDbTree();
            return next->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_root, c_dontcare>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            if(! next) return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            triple<bool, bool, LIT<mem> > r = next->erase(schema, entry);
            if(r.second) { delete next; next = (DbTreeBase*) 0; } // is it ok?
            return r;
        };
    
            // DbTree<c_entry>
        pair<bool, LIT<mem> > DbTree<c_entry, c_dontcare>::find(list<LdbmSchemaBase*>::iterator, vector<FSP>::iterator) {
            return pair<bool, LIT<mem> >(true, it);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_entry, c_dontcare>::insert(list<LdbmSchemaBase*>::iterator, vector<FSP>::iterator) {
            if(allocated) { // already allocated -- overwriting
                return triple<bool, bool, LIT<mem>* >(true, false, &it);
            } // not allocated
            allocated = true;
            return triple<bool, bool, LIT<mem>* >(true, true, &it);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_entry, c_dontcare>::erase(list<LdbmSchemaBase*>::iterator, vector<FSP>::iterator) {
            return triple<bool, bool, LIT<mem> >(true, true, it);
        };
            // DbTree<c_key, c_integer>
        pair<bool, LIT<mem> > DbTree<c_key, c_integer>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mint idx = e.ReadInteger();
            if( idx - base < 0 || idx - base >= ((mint) next.size()))
                return pair<bool, LIT<mem> >(false, LIT<mem>());
            
            DbTreeBase* n = next[idx-base];
            if(!n) return pair<bool, LIT<mem> >(false, LIT<mem>());
            schema++; entry++;
            return (next[idx - base])->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_key, c_integer>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mint idx = e.ReadInteger();
            if( idx - base < 0 || idx - base >= ((mint) next.size()))
                return triple<bool, bool, LIT<mem>* >(false, true, (LIT<mem>*)0);
            DbTreeBase* n = next[idx-base];
            schema++;
            if(!n) { refc++; next[idx-base] = (*schema)->newDbTree(); }
            entry++;
            return next[idx-base]->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_key, c_integer>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mint idx = e.ReadInteger();
            if( idx - base < 0 || idx - base >= ((mint) next.size()))
                return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            DbTreeBase* n = next[idx - base];
            if(!n) return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            schema++; entry++;
            triple<bool, bool, LIT<mem> > r = next[idx-base]->erase(schema, entry);
            if(r.second) {
                delete next[idx-base];
                next[idx-base] = (DbTreeBase*) 0;
                refc--;
                if(refc > 0) return triple<bool, bool, LIT<mem> >(r.first, false, r.third);
                return r;
            }
            return r;
        };
    
            // DbTree<c_key, c_inthash>
        pair<bool, LIT<mem> > DbTree<c_key, c_inthash>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mint idx = e.ReadInteger();
            dbtree_hash::iterator it = next.find(idx);
            if(it == next.end())
                return pair<bool, LIT<mem> >(false, LIT<mem>());
            schema++; entry++;
            return it->second->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_key, c_inthash>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mint idx = e.ReadInteger();
            dbtree_hash::iterator it = next.find(idx);
            if(it != next.end()) {
                schema++; entry++;
                return it->second->insert(schema, entry);
            }
            schema++;
            DbTreeBase* newdbtree = (*schema)->newDbTree();
            next.insert(dbtree_hash::value_type(dbtree_hash::key_type(idx),
                                                dbtree_hash::mapped_type(newdbtree)));
            entry++;
            return newdbtree->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_key, c_inthash>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mint idx = e.ReadInteger();
            dbtree_hash::iterator it = next.find(idx);
            if(it == next.end())
                return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            schema++; entry++;
            triple<bool, bool, LIT<mem> > r = it->second->erase(schema, entry);
            if(r.second) {
                delete (it->second);
                next.erase(idx);
                if(next.size() > 0) return triple<bool, bool, LIT<mem> >(r.first, false, r.third);
                return r;
            }
            return r;
        };
            // DbTree<c_key, c_string>
        pair<bool, LIT<mem> > DbTree<c_key, c_string>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            vector<mem::dcell> idx;
            DCore<mem>::Serialize(e.GetMachine(), e.GetAddress(), idx);
            dbtree_hash::iterator it = next.find(&idx);
            if(it == next.end())
                return pair<bool, LIT<mem> >(false, LIT<mem>());
            schema++; entry++;
            return it->second->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_key, c_string>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            vector<mem::dcell>* idx = new vector<mem::dcell>();
            DCore<mem>::Serialize(e.GetMachine(), e.GetAddress(), *idx);
            dbtree_hash::iterator it = next.find(idx);
            if(it != next.end()) {
                delete idx;
                schema++; entry++;
                return it->second->insert(schema, entry);
            }
            schema++;
            DbTreeBase* newdbtree = (*schema)->newDbTree();
            next.insert(dbtree_hash::value_type(dbtree_hash::key_type(idx),
                                                dbtree_hash::mapped_type(newdbtree)));
            entry++;
            return newdbtree->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_key, c_string>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            vector<mem::dcell> idx;
            DCore<mem>::Serialize(e.GetMachine(), e.GetAddress(), idx);
            dbtree_hash::iterator it = next.find(&idx);
            if(it == next.end())
                return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            schema++; entry++;
            triple<bool, bool, LIT<mem> > r = it->second->erase(schema, entry);
            if(r.second) {
                vector<mem::dcell>* backup = it->first;
                delete (it->second);
                next.erase(&idx);
                delete backup;
                if(next.size() > 0) return triple<bool, bool, LIT<mem> >(r.first, false, r.third);
                return r;
            }
            return r;
        };
            // DbTree<c_key, c_float>
        pair<bool, LIT<mem> > DbTree<c_key, c_float>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mfloat idx = e.ReadFloat();
            dbtree_hash::iterator it = next.find(idx);
            if(it == next.end())
                return pair<bool, LIT<mem> >(false, LIT<mem>());
            schema++; entry++;
            return it->second->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_key, c_float>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mfloat idx = e.ReadFloat();
            dbtree_hash::iterator it = next.find(idx);
            if(it != next.end()) {
                schema++; entry++;
                return it->second->insert(schema, entry);
            }
            schema++;
            DbTreeBase* newdbtree = (*schema)->newDbTree();
            next.insert(dbtree_hash::value_type(dbtree_hash::key_type(idx),
                                                dbtree_hash::mapped_type(newdbtree)));
            entry++;
            return newdbtree->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_key, c_float>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            mfloat idx = e.ReadFloat();
            dbtree_hash::iterator it = next.find(idx);
            if(it == next.end())
                return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            schema++; entry++;
            triple<bool, bool, LIT<mem> > r = it->second->erase(schema, entry);
            if(r.second) {
                delete (it->second);
                next.erase(idx);
                if(next.size() > 0) return triple<bool, bool, LIT<mem> >(r.first, false, r.third);
                return r;
            }
            return r;
        };
            // DbTree<c_key, c_type>
        pair<bool, LIT<mem> > DbTree<c_key, c_type>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            tserial idx = e.GetType()->GetSerialNo();
            dbtree_hash::iterator it = next.find(idx);
            if(it == next.end())
                return pair<bool, LIT<mem> >(false, LIT<mem>());
            schema++; entry++;
            return it->second->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_key, c_type>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            tserial idx = e.GetType()->GetSerialNo();
            dbtree_hash::iterator it = next.find(idx);
            if(it != next.end()) {
                schema++; entry++;
                return it->second->insert(schema, entry);
            }
            schema++;
            DbTreeBase* newdbtree = (*schema)->newDbTree();
            next.insert(dbtree_hash::value_type(dbtree_hash::key_type(idx),
                                                dbtree_hash::mapped_type(newdbtree)));
            entry++;
            return newdbtree->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_key, c_type>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            tserial idx = e.GetType()->GetSerialNo();
            dbtree_hash::iterator it = next.find(idx);
            if(it == next.end())
                return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            schema++; entry++;
            triple<bool, bool, LIT<mem> > r = it->second->erase(schema, entry);
            if(r.second) {
                delete (it->second);
                next.erase(idx);
                if(next.size() > 0) return triple<bool, bool, LIT<mem> >(r.first, false, r.third);
                return r;
            }
            return r;
        };

            // DbTree<c_key, c_fs>
        pair<bool, LIT<mem> > DbTree<c_key, c_fs>::find(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            vector<mem::dcell> idx;
            DCore<mem>::Serialize(e.GetMachine(), e.GetAddress(), idx);
            dbtree_hash::iterator it = next.find(&idx);
            if(it == next.end()) {
                return pair<bool, LIT<mem> >(false, LIT<mem>());
            }
            schema++; entry++;
            return it->second->find(schema, entry);
        };
        triple<bool, bool, LIT<mem>* > DbTree<c_key, c_fs>::insert(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            vector<mem::dcell>* idx = new vector<mem::dcell>();
            DCore<mem>::Serialize(e.GetMachine(), e.GetAddress(), *idx);
            dbtree_hash::iterator it = next.find(idx);
            if(it != next.end()) {
                delete idx;
                schema++; entry++;
                return it->second->insert(schema, entry);
            }
            schema++;
            DbTreeBase* newdbtree = (*schema)->newDbTree();
            next.insert(dbtree_hash::value_type(dbtree_hash::key_type(idx),
                                                dbtree_hash::mapped_type(newdbtree)));
            entry++;
            return newdbtree->insert(schema, entry);
        };
        triple<bool, bool, LIT<mem> > DbTree<c_key, c_fs>::erase(list<LdbmSchemaBase*>::iterator schema, vector<FSP>::iterator entry) {
            FSP e = *entry;
            vector<mem::dcell> idx;
            DCore<mem>::Serialize(e.GetMachine(), e.GetAddress(), idx);
            dbtree_hash::iterator it = next.find(&idx);
            if(it == next.end())
                return triple<bool, bool, LIT<mem> >(false, false, LIT<mem>());
            schema++; entry++;
            triple<bool, bool, LIT<mem> > r = it->second->erase(schema, entry);
            if(r.second) {
                vector<mem::dcell>* backup = it->first;
                delete (it->second);
                next.erase(&idx);
                delete backup;
                if(next.size() > 0) return triple<bool, bool, LIT<mem> >(r.first, false, r.third);
                return r;
            }
            return r;
        };
    } // end of namespace lildbm
} // end of namespace lilfes

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2005/05/18 10:29:34  ninomi
 * gcc3.4でもコンパイルできるようにしました。
 *
 * Revision 1.4  2004/08/09 13:35:06  ninomi
 * lildbm-dbのIFを変えました。pred型のFSPの入出力ではなくてvector<FSP>の入出力
 * にしました。db内部でpred型に依存する部分がなくなりました。
 *
 * Revision 1.3  2004/05/27 16:14:43  ninomi
 * DbTreeのdestructの時にkey_string, key_fsのキーが削除されていなかったの
 * で、するようにしました。
 *
 * Revision 1.2  2004/04/14 04:53:04  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
