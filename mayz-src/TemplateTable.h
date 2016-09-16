/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file TemplateTable.h
 *  @version Time-stamp: <2009-12-09 00:56:47 yusuke>
 *  Mappings from template names to signs
 *
 **********************************************************************/

#ifndef MAYZ_TEMPLATE_TABLE_H

#define MAYZ_TEMPLATE_TABLE_H

#include "mconfig.h"
#include "FSArray.h"
#include "LexEntry.h"
#include <liblilfes/builtin.h>
#include <string>
#include <vector>
#include <map>

namespace mayz {

  class TemplateTable {
  private:
    lilfes::machine* mach;
    FSArray< std::string, lilfes::FSP > template_db;
    std::map< std::string, int > count_db;

  public:
    typedef FSArray< std::string, lilfes::FSP >::iterator iterator;
    iterator begin() { return template_db.begin(); }
    iterator end() { return template_db.end(); }

  public:
    TemplateTable( lilfes::machine* m ) : mach( m ), template_db( mach ), count_db() {
    }
    virtual ~TemplateTable() {}

    void incCount( const std::string& name, int i = 1 ) {
//       int count = 0;
//       if ( ! count_db.find( name, count ) ) {
//         count = 0;
//       }
//       count += i;
//       return count_db.insert( name, count );
      count_db[ name ] += i;
    }
    bool getCount( const std::string& name, int& count ) {
      std::map< std::string, int >::const_iterator it = count_db.find( name );
      if ( it == count_db.end() ) return false;
      count = it->second;
      return true;
    }
//     virtual bool newTemplate( const LexTemplate& name, lilfes::FSP temp ) {
//       int count = 0;
//       if ( count_db.find( name, count ) ) {
//         return count_db.insert( name, ++count );
//       } else {
//         return count_db.insert( name, 1 )
//           && template_db.insert( name, temp );
//       }
//     }
    virtual bool insert( const std::string& name, lilfes::FSP temp ) {
      return template_db.insert( name, temp );
    }
    virtual bool find( const std::string& name, lilfes::FSP temp ) {
      lilfes::FSP t;
      return template_db.find( name, t ) && temp.Unify( t );
    }

    virtual bool load( const std::string& lexfile, const std::string& countfile = "" );
    virtual bool save( const std::string& lexfile, const std::string& countfile = "" );
  };

}

#endif // MAYZ_TEMPLATE_TABLE_H
