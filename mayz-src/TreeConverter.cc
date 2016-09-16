/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file TreeConverter.cc
 *  @version Time-stamp: <2005-06-21 19:00:57 yusuke>
 *  Converting trees using pattern rules
 *
 **********************************************************************/

#include "TreeConverter.h"
#include "Initializer.h"
#include <liblilfes/utility.h>

namespace mayz {

  using namespace lilfes;
  using namespace std;

//    bool TreeConvertRule::convertNode( machine* mach, lilfes::FSP in, lilfes::FSP out ) {
//      std::vector< FSP > args( 2 );
//      args[ 0 ] = in;
//      args[ 1 ] = out;
//      return call_proc( *mach, pred, args );
//    }

//    bool TreeConvertRuleRootOnly::convert( machine* mach, lilfes::FSP in, lilfes::FSP out ) {
//      return convertNode( mach, in, out );
//    }

//    bool TreeConvertRuleBottomUp::convert( machine* mach, lilfes::FSP in, lilfes::FSP out ) {
//      FSP dtrs = in.Follow( TreeConverter::tree_dtrs_feature );
//      if ( dtrs.IsValid() ) {
//        FSP out_dtrs( mach );
//        out_dtrs.Follow( TreeConverter::tree_dtrs_feature );
//        while ( dtrs.GetType() == cons ) {
//          FSP dtr = dtrs.Follow( hd );
//          FSP out_dtr( mach );
//          if ( convert( mach, dtr, out_dtr ) ) {
//            out_dtrs.Follow( hd ).Unify( out_dtr );
//          } else {
//            out_dtrs.Follow( hd ).Unify( dtr );
//          }
//          dtrs = dtrs.Follow( tl );
//        }
//        out_dtrs.Coerce( nil );
//        FSP new_in( mach );
//        new_in.Follow( TreeConverter::tree_node_feature ).Unify( in.Follow( TreeConverter::tree_node_feature ) );
//        new_in.Follow( TreeConverter::tree_dtrs_feature ).Unify( out_dtrs );
//        return convertNode( mach, new_in, out );
//      } else {
//        return convertNode( mach, in, out );
//      }
//    }

//    bool TreeConvertRuleTopDown::convert( machine* mach, lilfes::FSP in, lilfes::FSP out ) {
//      return convertNode( mach, in, out );
//    }

//    void TreeConverter::initialize() {
//      if ( ! load_module( *mach, "tree conversion module", TREE_CONVERTER_INTERFACE_MODULE ) ) {
//        throw TreeConverterException( "Module \"" TREE_CONVERTER_INTERFACE_MODULE "\" not found" );
//      }
//      tree_node_feature = feature::Search( module::UserModule()->Search( TREE_NODE_FEATURE ) );
//      tree_dtrs_feature = feature::Search( module::UserModule()->Search( TREE_DTRS_FEATURE ) );
//      if ( tree_dtrs_feature == NULL ) {
//        throw TreeConverterException( "Feature TREE_DTRS\\ is not defined (maybe necessary modules are not loaded)" );
//      }
//      procedure* proc = prepare_proc( module::UserModule(), TREE_CONVERSION_RULE, 3 );
//      if ( proc == NULL ) {
//        throw TreeConverterException( TREE_CONVERSION_RULE " is not defined (maybe necessary modules are not loaded)" );
//      }
//      std::vector< FSP > args( 3 );
//      args[ 0 ] = FSP( mach ); // rule name
//      args[ 1 ] = FSP( mach ); // predicate
//      args[ 2 ] = FSP( mach ); // direction
//      std::vector< bool > flags( 3 );
//      flags[ 0 ] = true;
//      flags[ 1 ] = true;
//      flags[ 2 ] = true;
//      std::list< std::vector< FSP > > results;
//      findall_proc( *mach, proc, args, flags, results );
//      for ( std::list< std::vector< FSP > >::const_iterator it = results.begin();
//            it != results.end();
//            ++it ) {
//        if ( ! (*it)[ 0 ].IsString() || ! (*it)[ 2 ].IsString() ) {
//          throw TreeConverterException( "1st and 3rd arguments of " TREE_CONVERSION_RULE " must be string" );
//        }
//        std::string rule_name = (*it)[ 0 ].ReadString();
//        procedure* rule_proc = prepare_proc( (*it)[ 1 ].GetType(), 2 );
//        if ( rule_proc == NULL ) {
//          std::string m( "Cannot find tree conversion predicate: " );
//          m += (*it)[ 1 ].GetType()->GetSimpleName();
//          throw TreeConverterException( m );
//        }
//        std::string rule_dir = (*it)[ 2 ].ReadString();
//        if ( rule_dir == "root_only" ) {
//          rules.push_back( new TreeConvertRuleRootOnly( rule_name, rule_proc ) );
//        } else if ( rule_dir == "bottom_up" ) {
//          rules.push_back( new TreeConvertRuleBottomUp( rule_name, rule_proc ) );
//        } else if ( rule_dir == "top_down" ) {
//          rules.push_back( new TreeConvertRuleTopDown( rule_name, rule_proc ) );
//        } else {
//          throw TreeConverterException( "Unknown rule direction: " + rule_dir );
//        }
//      }
//    }

//    bool TreeConverter::convert( FSP in, FSP out ) {
//      FSP tmp_in = in;
//      FSP tmp_out;
//      for ( std::vector< TreeConvertRule* >::const_iterator rule = rules.begin();
//            rule != rules.end();
//            ++rule ) {
//        tmp_out = FSP( mach );
//        if ( ! (*rule)->convert( mach, tmp_in, tmp_out ) ) {
//          tmp_out = tmp_in;
//        }
//        tmp_in = tmp_out;
//      }
//      out.Unify( tmp_out );
//      return true;
//    }

//    void TreeConverter::showStatistics( std::ostream& os ) const {
//    }

#define TREE_CONVERSION_MODULE "mayz/treetrans"
#define TREE_CONVERSION_PROC "transform_tree"

  void TreeConverter::initialize() {
    if ( ! load_module( *mach, "tree conversion module", TREE_CONVERSION_MODULE ) ) {
      throw TreeConverterException( "tree conversion module not found: " TREE_CONVERSION_MODULE );
    }
    if ( ! Initializer::initialize( mach ) ) {
      throw TreeConverterException( "initialization failed" );
    }
    convert_proc = prepare_proc( module::UserModule(), TREE_CONVERSION_PROC, 2 );
    if ( convert_proc == NULL ) {
      throw TreeConverterException( "Cannot find tree conversion predicate: " TREE_CONVERSION_PROC "/2" );
    }
  }

  bool TreeConverter::convert( lilfes::FSP in, lilfes::FSP& out ) {
    ++num_all;
    std::vector< FSP > args( 2 );
    args[ 0 ] = in;
    args[ 1 ] = FSP( mach );
    if ( call_proc( *mach, convert_proc, args ) ) {
      ++num_success;
      out = args[ 1 ];
      return true;
    } else {
      ++num_fail;
      return false;
    }
  }

  void TreeConverter::showStatistics( ostream& os ) const {
    os << "Statistics of tree transformation" << endl;
    //os << "  # trees = " << num_all << endl;
    os << "  # successes = " << num_success << endl;
    os << "  # failures = " << num_fail << endl;
  }

}

// end of TreeConverter.cc
