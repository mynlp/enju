/**
 * @module  = xml
 * @cvs     = $Id: xml.cpp,v 1.8 2011-05-02 10:38:24 matuzaki Exp $
 * @copyright = Copyright (c) 2004, MIYAO Yusuke
 * @copyright = You may distribute this file under the terms of the Artistic License.
 * @desc    = XML
 * Builtin predicates for converting feature structures into XML.
 * @japanese =
 * 素性構造を XML に変換する述語です．
 * @end_japanese
*/

#include "xml.h"
#include "builtin.h"
#include "utility.h"
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>


namespace lilfes {

using std::cerr;
using std::cout;
using std::endl;
using std::list;
using std::map;
using std::ostringstream;
using std::string;
using std::vector;



const string FSXML::DEFAULT_FS_NAMESPACE = "";

void FSXML::outputXMLString( const string& str, ostringstream& ostr ) {
  for ( string::const_iterator it = str.begin();
        it != str.end();
        ++it ) {
    switch (*it) {
    case '&':
      ostr << "&amp;";
      break;
    case '<':
      ostr << "&lt;";
      break;
    case '>':
      ostr << "&gt;";
      break;
    case '\"':
      ostr << "&quot;";
      break;
    default:
      ostr << *it;
    }
  }
  return;
}

/*
void FSXML::makeShareMap( const vector< cell >& vec,
                          map< vector< cell >::const_iterator, int >& share_map ) {
  int id = 0;
  for ( vector< cell >::const_iterator it = vec.begin();
        it != vec.end();
        ++it ) {
    switch ( Tag ( *it ) ) {
    case T_PTR:
    case T_PTR1:
    case T_PTR2:
    case T_PTR3: {
      vector< cell >::const_iterator ptr = vec.begin() + c2PTR( *it );
      if ( ptr < it ) {
        share_map[ ptr ] = (++id);
      }
      break;
    }
    default:
      break;
    }
  }
}
*/

void FSXML::inputInternal( machine* mach, core_p addr, map< core_p, int >& index_map, ostringstream& ostr ) {
  bool is_shared = false;
  addr = mach->Deref( addr );
  map< core_p, int >::iterator index = index_map.find( addr );
  if ( index != index_map.end() ) {
    if ( index->second < 0 ) {
      // already output
      ostr << "<" << prefix << "fs shared_id=\"" << -(index->second) << "\" />";
      return;
    }
    ostr << "<" << prefix << "fs shared_id=\"" << index->second << "\">";
    index->second = -(index->second);
    is_shared = true;
  }
  cell c = mach->ReadHeap( addr );
  switch ( Tag( c ) ) {
  case T_STR:
  case T_FSTR: {
    if ( ! is_shared ) { ostr << "<" << prefix << "fs>"; }
    const type* str = c2STR( c );
    ostr << "<" << prefix << "type>";
    outputXMLString( string( Quote( str->GetPrintName() ) ), ostr );
    ostr << "</" << prefix << "type>";
    for ( int i = 0; i < str->GetNFeatures(); ++i ) {
      ostr << "<" << prefix << "feat edge=\"";
      outputXMLString( string( Quote( str->Feature( i )->GetPrintName(), true ) ), ostr );
      ostr << "\">";
      inputInternal( mach, addr + i + 1, index_map, ostr );
      ostr << "</" << prefix << "feat>";
    }
    if ( ! is_shared ) { ostr << "</" << prefix << "fs>"; }
    break;
  }
  case T_VAR:
  case T_FVAR: {
    const type* var = c2VAR( c );
    ostr << "<" << prefix << "type>";
    outputXMLString( string( Quote(var->GetPrintName()) ), ostr );
    ostr << "</" << prefix << "type>";
    break;
  }
  case T_INT: {
    mint i = c2INT( c );
    ostr << "<" << prefix << "decimal>" << i << "</" << prefix << "decimal>";
    break;
  }
  case T_FLO: {
    mfloat f = c2FLO( c );
    ostr << "<" << prefix << "float>" << f << "</" << prefix << "float>";
    break;
  }
  case T_CHR: {
    mchar ch = c2CHR( c );
    ostr << "<" << prefix << "doublequoted>`";
    outputXMLString( string( 1, ch ), ostr );
    ostr << "\'<" << prefix << "doublequoted>";
    break;
  }
  case T_STG: {
    const char* s = c2STG( c );
    ostr << "<" << prefix << "doublequoted>&quot;";
    outputXMLString( s, ostr );
    ostr << "&quot;</" << prefix << "doublequoted>" ;
    break;
  }
  default: {
    ABORT( "Unknown Tag in FSXML::inputInternalOld" );
  }
  }
  if ( is_shared ) {
    ostr << "</" << prefix << "fs>";
  }
}

/*
void FSXML::inputInternalOld( vector< cell >::const_iterator begin,
                              vector< cell >::const_iterator it,
                              ostringstream& ostr,
                              const map< vector< cell >::const_iterator, int >& share_map ) {
  bool is_shared = false;
  map< vector< cell >::const_iterator, int >::const_iterator share = share_map.find( it );
  if ( share != share_map.end() ) {
    ostr << "($" << share->second << ' ';
    is_shared = true;
  }
  switch ( Tag( *it ) ) {
  case T_PTR:
  case T_PTR1:
  case T_PTR2:
  case T_PTR3: {
    vector< cell >::const_iterator ptr = begin + c2PTR( *it );
    if ( ptr < it ) {
      ostr << '$' << share_map.find( ptr )->second;
    } else {
      inputInternalOld( begin, ptr, ostr, share_map );
    }
    break;
  }
  case T_STR:
  case T_FSTR: {
    if ( ! is_shared ) { ostr << '('; }
    const type* str = c2STR( *it );
    ostr << Quote(str->GetPrintName());
    ++it;
    for ( int i = 0; i < str->GetNFeatures(); ++i ) {
      ostr << ' ' << Quote(str->Feature( i )->GetPrintName(), true) << ' ';
      inputInternalOld( begin, it + i, ostr, share_map );
    }
    if ( ! is_shared ) { ostr << ')'; }
    break;
  }
  case T_VAR:
  case T_FVAR: {
    const type* var = c2VAR( *it );
    ostr << Quote( var->GetPrintName() );
    break;
  }
  case T_INT: {
    mint i = c2INT( *it );
    ostr << i;
    break;
  }
  case T_FLO: {
    mfloat f = c2FLO( *it );
    ostr << f;
    break;
  }
  case T_CHR: {
    mchar c = c2CHR( *it );
    ostr << '`' << c << '\'';
  }
  case T_STG: {
    char* s = c2STG( *it );
    ostr << '"' << s << '"' ;
    break;
  }
  }
  if ( is_shared ) {
    ostr << ')';
  }
}
*/

void FSXML::inputInternalOld( machine* mach, core_p addr, map< core_p, int >& index_map, ostringstream& ostr ) {
  bool is_shared = false;
  addr = mach->Deref( addr );
  map< core_p, int >::iterator index = index_map.find( addr );
  if ( index != index_map.end() ) {
    if ( index->second < 0 ) {
      // already output
      ostr << '$' << -(index->second);
      return;
    }
    ostr << "($" << index->second << ' ';
    index->second = -(index->second);
    is_shared = true;
  }
  cell c = mach->ReadHeap( addr );
  switch ( Tag( c ) ) {
  case T_STR:
  case T_FSTR: {
    if ( ! is_shared ) { ostr << '('; }
    const type* str = c2STR( c );
    outputXMLString( string( Quote(str->GetPrintName()) ), ostr );
    for ( int i = 0; i < str->GetNFeatures(); ++i ) {
      ostr << ' ';
      outputXMLString( string( Quote(str->Feature( i )->GetPrintName(), true) ), ostr );
      ostr << ' ';
      inputInternalOld( mach, addr + i + 1, index_map, ostr );
    }
    if ( ! is_shared ) { ostr << ')'; }
    break;
  }
  case T_VAR:
  case T_FVAR: {
    const type* var = c2VAR( c );
    outputXMLString( string( Quote( var->GetPrintName() ) ), ostr );
    break;
  }
  case T_INT: {
    mint i = c2INT( c );
    ostr << i;
    break;
  }
  case T_FLO: {
    mfloat f = c2FLO( c );
    ostr << f;
    break;
  }
  case T_CHR: {
    mchar ch = c2CHR( c );
    ostr << '`';
    outputXMLString( string( 1, ch ), ostr );
    ostr << '\'';
  }
  case T_STG: {
    const char* s = c2STG( c );
    ostr << "&quot;";
    outputXMLString( s, ostr );
    ostr << "&quot;";
    break;
  }
  default: {
    ABORT( "Unknown Tag in FSXML::inputInternalOld" );
  }
  }
  if ( is_shared ) {
    ostr << ')';
  }
}

FSP FSXML::indexMap( FSP tfs, map< core_p, int >& index_map ) {
  map< core_p, unsigned int > share_map;
  FSP norm = tfs.SharedNodeList( share_map );
  int coindex_id = 0;
  for ( map< core_p, unsigned int >::const_iterator it = share_map.begin();
        it != share_map.end();
        ++it ) {
    //cerr << hex << it->first;
    if ( it->second > 0 ) {
      //cerr << "  shared";
      index_map[ it->first ] = ++coindex_id;
    }
    //cerr << endl;
  }
  return norm;
}

void FSXML::input( FSP tfs, const string& ns, bool old_version ) {
  /*
  vector< cell > vec;
  tfs.Serialize( vec );
  for ( vector< cell >::const_iterator it = vec.begin();
        it != vec.end();
        ++it ) {
    cerr << *it << endl;
  }
  map< vector< cell >::const_iterator, int > share_map;
  makeShareMap( vec, share_map );
  ostringstream ostr;
  inputInternalOld( vec.begin(), vec.begin(), ostr, share_map );
  str = ostr.str();
  */
  setNameSpace( ns );
  map< core_p, int > index_map;
  FSP norm = indexMap( tfs, index_map );
  ostringstream ostr;
  if ( old_version ) {
    // old version
    ostr << "<" << prefix << "fs data=\"";
    inputInternalOld( norm.GetMachine(), norm.GetAddress(), index_map, ostr );
    ostr << "\">";
  } else {
    // new version
    inputInternal( norm.GetMachine(), norm.GetAddress(), index_map, ostr );
  }
  str = ostr.str();
}

namespace builtin {

  /**
   * @predicate	= tfstoxml(+TFS, -XML) 
   * @desc	= Convert typed feature structure into XML format
   * @param	= +TFS/ bot : typed feature structure
   * @param      = -XML/ string : XML representation
   */

  bool tfstoxml( machine& m, FSP arg1, FSP arg2 ) {
    FSXML xml;
    xml.input( arg1 );
    return arg2.Unify( FSP( m, xml.getString().c_str() ) );
  }

  LILFES_BUILTIN_PRED_2( tfstoxml, tfstoxml );

  /**
   * @predicate	= tfstoxml(+TFS, +NS, -XML) 
   * @desc	= Convert typed feature structure into XML format
   * @param	= +TFS/ bot : typed feature structure
   * @param	= +NS/ string : namespace prefix
   * @param      = -XML/ string : XML representation
   */

  bool tfstoxml_3( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
    if ( ! arg2.IsString() ) {
      RUNERR( "arg2 of 'tfstoxml/3' must be string" );
      return false;
    }
    FSXML xml;
    xml.input( arg1, arg2.ReadString() );
    return arg3.Unify( FSP( m, xml.getString().c_str() ) );
  }

  LILFES_BUILTIN_PRED_OVERLOAD_3( tfstoxml_3, tfstoxml_3, tfstoxml );

  /**
   * @predicate	= tfstoxml_list(+TFSList, +NS, -XMLList) 
   * @desc	= Convert typed feature structure into XML format
   * @param	= +TFSList/ list : list of typed feature structures
   * @param	= +NS/ string : namespace prefix
   * @param      = -XML/ list of strings : list of XML representations
   */

  bool tfstoxml_list_3( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
    if ( ! arg2.IsString() ) {
      RUNERR( "arg2 of 'tfstoxml_list/3' must be string" );
      return false;
    }
    FSXML xml;
    xml.setNameSpace( arg2.ReadString() );
    map< core_p, int > index_map;
    FSP norm = xml.indexMap( arg1, index_map );
    vector< FSP > input, output;
    if ( ! lilfes_to_c< vector< FSP > >().convert( m, norm, input ) ) {
      RUNERR( "arg1 of 'tfstoxml_list/3' must be list" );
      return false;
    }
    for ( vector< FSP >::iterator it = input.begin();
          it != input.end();
          ++it ) {
      ostringstream ostr;
      xml.inputInternal( &m, it->GetAddress(), index_map, ostr );
      output.push_back( FSP( m, ostr.str().c_str() ) );
    }
    return c_to_lilfes< vector< FSP > >().convert( m, output, arg3 );
  }

  LILFES_BUILTIN_PRED_3( tfstoxml_list_3, tfstoxml_list );

  /**
   * @predicate	= tfstoxml_old(+TFS, -XML) 
   * @desc	= Convert typed feature structure into XML format (old version)
   * @param	= +TFS/ bot : typed feature structure
   * @param      = -XML/ string : XML representation
   */

  bool tfstoxml_old( machine& m, FSP arg1, FSP arg2 ) {
    FSXML xml;
    xml.input( arg1, FSXML::DEFAULT_FS_NAMESPACE, true );
    return arg2.Unify( FSP( m, xml.getString().c_str() ) );
  }

  LILFES_BUILTIN_PRED_2( tfstoxml_old, tfstoxml_old );

  /**
   * @predicate	= tfstoxml_old(+TFS, +NS, -XML) 
   * @desc	= Convert typed feature structure into XML format (old version)
   * @param	= +TFS/ bot : typed feature structure
   * @param	= +NS/ string : namespace prefix
   * @param      = -XML/ string : XML representation
   */

  bool tfstoxml_old_3( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
    if ( ! arg2.IsString() ) {
      RUNERR( "arg2 of 'tfstoxml/3' must be string" );
      return false;
    }
    FSXML xml;
    xml.input( arg1, arg2.ReadString(), true );
    return arg3.Unify( FSP( m, xml.getString().c_str() ) );
  }

  LILFES_BUILTIN_PRED_OVERLOAD_3( tfstoxml_old_3, tfstoxml_old_3, tfstoxml_old );

  /**
   * @predicate = encode_xml_string(+$InStr, -$OutStr)
   * @desc      = escape special characters
   * @param     = +$InStr/string : input string
   * @param     = -$OutStr/string : output string
   */

  bool encode_xml_string( machine&, FSP arg1, FSP arg2 ) {
    if ( ! arg1.IsString() ) {
      RUNERR( "first argument of encode_xml_string/2 must be string" );
      return false;
    }
    string str( arg1.ReadString() );
    ostringstream ostr;
    FSXML::outputXMLString( str, ostr );
    return arg2.Unify( ostr.str() );
  }

  LILFES_BUILTIN_PRED_2( encode_xml_string, encode_xml_string );

} // namespace builtin

} // namespace lilfes

// end of xml.cpp
