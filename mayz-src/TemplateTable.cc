/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file TemplateTable.cc
 *  @version Time-stamp: <2009-12-09 01:00:26 yusuke>
 *  Mappings from template names to signs
 *
 **********************************************************************/

#include "TemplateTable.h"

namespace mayz {

  using namespace lilfes;
  using namespace std;

  bool TemplateTable::load( const std::string& lexfile, const std::string& countfile ) {
    if ( ! template_db.load( lexfile ) ) return false;
    std::ifstream file( countfile.c_str() );
    if ( ! file ) return false;
    std::string name;
    int count;
    while ( file >> name >> count ) {
      count_db[ name ] = count;
    }
    return true;
  }
  bool TemplateTable::save( const std::string& lexfile, const std::string& countfile ) {
    if ( ! template_db.save( lexfile ) ) return false;
    std::ofstream file( countfile.c_str() );
    if ( ! file ) return false;
    for ( map< string, int >::iterator it = count_db.begin(); it != count_db.end(); ++it ) {
      file << it->first << '\t' << it->second << std::endl;
    }
    return true;
  }

}

