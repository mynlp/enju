//////////////////////////////////////////////////////////////////////
////
////  Copyright (c) 1999, MAKINO Takaki and MIYAO Yusuke
////  You may distribute under the terms of the Artistic License.
////
////  Name: stream.h
////  Author: MAKINO Takaki (mak@is.s.u-tokyo.ac.jp)
////          MIYAO Yusuke (yusuke@is.s.u-tokyo.ac.jp)
////  Time-stamp: <1999-11-22 15:21:22 yusuke>
////  Purpose: Implementation of streams
////  $Id: xml.h,v 1.6 2006-11-06 04:32:23 yusuke Exp $
////
//////////////////////////////////////////////////////////////////////

#ifndef __xml_h
#define __xml_h

#include "lconfig.h"
#include "cell.h"
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace lilfes {

class FSXML {
private:
  std::string prefix;
  std::string str;

protected:
  /*
  void makeShareMap( const std::vector< cell >& vec,
                     std::map< std::vector< cell >::const_iterator, int >& share_map );
  */
  /*
  void inputInternal( std::vector< cell >::const_iterator begin,
                      std::vector< cell >::const_iterator it,
                      std::ostringstream& istr,
                      const std::map< std::vector< cell >::const_iterator, int >& share_map );
  void inputInternalOld( std::vector< cell >::const_iterator begin,
                         std::vector< cell >::const_iterator it,
                         std::ostringstream& istr,
                         const std::map< std::vector< cell >::const_iterator, int >& share_map );
  */
public:

  FSXML() {}
  virtual ~FSXML() {}

  void setNameSpace( const std::string& ns ) {
    if ( ns.empty() ) {
      prefix = ns;
    } else {
      prefix = ns + ":";
    }
  }

  void inputInternal( machine* mach, core_p addr, std::map< core_p, int >& index_map, std::ostringstream& ostr );
  void inputInternalOld( machine* mach, core_p addr, std::map< core_p, int >& index_map, std::ostringstream& ostr );
  FSP indexMap( FSP tfs, std::map< core_p, int >& index_map );

  virtual void input( const FSP tfs, const std::string& ns = DEFAULT_FS_NAMESPACE, bool old_version = false );
  //void output( FSP tfs );

  const std::string& getString() {
    return str;
  }

  static void outputXMLString( const std::string& str, std::ostringstream& ostr );

  static const std::string DEFAULT_FS_NAMESPACE;
};

namespace builtin {

  bool tfstoxml( machine&, FSP, FSP );
  bool encode_xml_string( machine&, FSP, FSP );

} // namespace builtin

} // namespace lilfes

#endif // __xml_h
// end of __xml.h
