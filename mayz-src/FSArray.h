/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file FSArray.h
 *  @version Time-stamp: <2005-06-21 19:06:52 yusuke>
 *  Wrapper for accessing lildbm as an array
 *
 **********************************************************************/

#ifndef MAYZ_FSARRAY_H

#define MAYZ_FSARRAY_H

#include "mconfig.h"
#include <liblilfes/lildbm.h>
#include <liblilfes/utility.h>
#include <vector>
#include <string>

namespace mayz {

  template < class T > class FSArrayTraits { };
  template <> class FSArrayTraits< int > {
  public:
    typedef lilfes::lildbm::c_inthash lildbm_key_type;
    typedef lilfes::lildbm::c_integer lildbm_value_type;
  };
  template <> class FSArrayTraits< float > {
  public:
    typedef lilfes::lildbm::c_float lildbm_key_type;
    typedef lilfes::lildbm::c_float lildbm_value_type;
  };
  template <> class FSArrayTraits< std::string > {
  public:
    typedef lilfes::lildbm::c_string lildbm_key_type;
    typedef lilfes::lildbm::c_string lildbm_value_type;
  };
  template <> class FSArrayTraits< lilfes::FSP > {
  public:
    typedef lilfes::lildbm::c_fs lildbm_key_type;
    typedef lilfes::lildbm::c_fs lildbm_value_type;
  };
  template <> class FSArrayTraits< std::vector< lilfes::cell > > {
  public:
    typedef lilfes::lildbm::c_fs lildbm_key_type;
    typedef lilfes::lildbm::c_fs lildbm_value_type;
  };
  template < class T > class FSArrayTraits< std::vector< T > > {
  public:
    typedef lilfes::lildbm::c_fs lildbm_key_type;
    typedef lilfes::lildbm::c_fs lildbm_value_type;
  };
  template < class T1, class T2 > class FSArrayTraits< std::pair< T1, T2 > > {
  public:
    typedef lilfes::lildbm::c_fs lildbm_key_type;
    typedef lilfes::lildbm::c_fs lildbm_value_type;
  };

  template < class Key, class Value,
    class LdbmKey = typename FSArrayTraits< Key >::lildbm_key_type,
    class LdbmValue = typename FSArrayTraits< Value >::lildbm_value_type >
  class FSArray {
  private:
    lilfes::machine* mach;
    lilfes::lildbm::LdbmDb<lilfes::lildbm::mem> db;

  public:

    class FSArrayIter {
    private:
      FSArray* array;
      lilfes::lildbm::LIT<lilfes::lildbm::ldb> ldbm_iter;
      std::pair< Key, Value >* entry;

    protected:
      FSArrayIter()
        : array( NULL ), ldbm_iter( 0 ), entry( NULL ) {
      }
      explicit FSArrayIter( FSArray* a )
        : array( a ), ldbm_iter( 0 ), entry( NULL ) {
        std::pair< bool, lilfes::lildbm::LIT<lilfes::lildbm::ldb> > ret = array->db.first();
        if ( ret.first ) {
          ldbm_iter = ret.second;
        } else {
          array = NULL;
        }
      }
      FSArrayIter( FSArray* a, lilfes::lildbm::LIT<lilfes::lildbm::ldb> iter )
        : array( a ), ldbm_iter( iter ), entry( NULL ) {
      }

    public:
      FSArrayIter( const FSArrayIter& iter )
        : array( iter.array ), ldbm_iter( iter.ldbm_iter ), entry( NULL ) {
      }
      ~FSArrayIter() {
        delete entry;
      }

      static FSArrayIter firstIter( FSArray* a ) {
        return FSArrayIter( a );
      }
      static FSArrayIter lastIter( FSArray* ) {
        return FSArrayIter();
      }

      const FSArray* getArray() const { return array; }
      const lilfes::lildbm::LIT<lilfes::lildbm::ldb>& getLdbmIter() const { return ldbm_iter; }

      FSArrayIter& operator=( const FSArrayIter& iter ) {
        delete entry;
        array = iter.array;
        ldbm_iter = iter.ldbm_iter;
        entry = NULL;
        return *this;
      }

      bool ensureEntry() {
        if ( ! entry ) {
          entry = new std::pair< Key, Value >;
          std::vector< lilfes::FSP > arglist;
          if ( ! array->db.find( array->mach, ldbm_iter, arglist ) ) return false;
          return lilfes::lilfes_to_c< Key >().convert( *(array->mach), arglist[ 0 ], entry->first )
            && lilfes::lilfes_to_c< Value >().convert( *(array->mach), arglist[ 1 ], entry->second );
        } else {
          return true;
        }
      }

      std::pair< Key, Value > operator*() {
        ensureEntry();
        return *entry;
      }
      std::pair< Key, Value >* operator->() {
        ensureEntry();
        return entry;
      }

      FSArrayIter operator++() {
        std::pair< bool, lilfes::lildbm::LIT<lilfes::lildbm::ldb> > ret = array->db.next( ldbm_iter );
        if ( ret.first ) {
          *this = FSArrayIter( array, ret.second );
        } else {
          *this = FSArrayIter();
        }
        return *this;
      }

      bool operator==( const FSArrayIter& iter ) {
        return array == iter.array
          && ldbm_iter.entry_id == iter.ldbm_iter.entry_id;
      }
      bool operator!=( const FSArrayIter& iter ) {
        return ! ( *this == iter );
      }
    };

  public:
    typedef FSArrayIter iterator;

    iterator begin() {
      return iterator::firstIter( this );
    }
    iterator end() {
      return iterator::lastIter( this );
    }

  public:
    FSArray( lilfes::machine* m ) : mach( m ) {
      std::list<lilfes::lildbm::LdbmSchemaBase*> *sc = new std::list<lilfes::lildbm::LdbmSchemaBase*>();
      sc->push_back(new lilfes::lildbm::LdbmSchema<lilfes::lildbm::c_key, LdbmKey >(INITIALHASHSIZE));
      sc->push_back(new lilfes::lildbm::LdbmSchema<lilfes::lildbm::c_value, LdbmValue >());
      sc->push_back(new lilfes::lildbm::LdbmSchema<lilfes::lildbm::c_entry, lilfes::lildbm::c_dontcare>());
      if(! db.open(sc)) {
        std::list<lilfes::lildbm::LdbmSchemaBase*>::iterator it = sc->begin(), last = sc->end();
        for(; it != last ; it++) 
          delete (*it);
        delete sc;
      }
    }
    ~FSArray() {}

    lilfes::machine* getMachine() { return mach; }
    lilfes::lildbm::LdbmDb<lilfes::lildbm::mem> getDB() { return db; }

    bool find( const Key& key, Value& value ) {
      std::vector< lilfes::FSP > arglist( 2 );
      arglist[ 0 ] = lilfes::FSP( mach );
      arglist[ 1 ] = lilfes::FSP( mach );
      return lilfes::c_to_lilfes< Key >().convert( *mach, key, arglist[ 0 ] )
        && db.find( mach, arglist )
        && lilfes::lilfes_to_c< Value >().convert( *mach, arglist[ 1 ], value );
    }

    bool erase( const Key& key ) {
      std::vector< lilfes:: FSP > arglist( 2 );
      arglist[ 0 ] = lilfes::FSP( mach );
      return lilfes::c_to_lilfes< Key >().convert( *mach, key, arglist[ 0 ] )
      && db.erase( mach, arglist );
    }

    bool insert( const Key& key, const Value& value ) {
      std::vector< lilfes::FSP > arglist( 2 );
      arglist[ 0 ] = lilfes::FSP( mach );
      arglist[ 1 ] = lilfes::FSP( mach );
      return lilfes::c_to_lilfes< Key >().convert( *mach, key, arglist[ 0 ] )
      && lilfes::c_to_lilfes< Value >().convert( *mach, value, arglist[ 1 ] )
      && db.insert( mach, arglist );
    }

    bool save( const std::string& filename ) {
      return db.save( filename.c_str() );
    }
    bool load( const std::string& filename ) {
      return db.load( mach, filename.c_str() );
    }

  };

}

#endif // MAYZ_FSARRAY_H
