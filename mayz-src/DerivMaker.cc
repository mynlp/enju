/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file DerivMaker.cc
 *  @version Time-stamp: <2006-11-07 22:12:46 yusuke>
 *  Making derivation by applying inverse schemas
 *
 **********************************************************************/

#include "DerivMaker.h"
#include "Initializer.h"
#include <liblilfes/utility.h>

#define DERIV_MAKER_INTERFACE_MODULE "mayz/derivmake"
#define TREE_TYPES_MODULE "mayz/treetypes"

#define INVERSE_SCHEMA_BINARY "inverse_schema_binary"
#define INVERSE_SCHEMA_UNARY "inverse_schema_unary"
#define LEXICAL_CONSTRAINTS "lexical_constraints"
#define ROOT_CONSTRAINTS "root_constraints"

#define FAIL_POINT_INVERSE_SCHEMA_BINARY "fail_point_inverse_schema_binary"
#define FAIL_POINT_INVERSE_SCHEMA_UNARY "fail_point_inverse_schema_unary"
#define FAIL_POINT_LEXICAL_CONSTRAINTS "fail_point_lexical_constraints"
#define FAIL_POINT_ROOT_CONSTRAINTS "fail_point_root_constraints"

using namespace std;
using namespace lilfes;

namespace mayz {

  void DerivMaker::initialize() {
    if ( ! load_module( *mach, "derivation maker module", DERIV_MAKER_INTERFACE_MODULE ) ) {
      throw DerivMakerException( "Module \"" DERIV_MAKER_INTERFACE_MODULE "\" not found" );
    }
    if ( ! load_module( *mach, "tree types module", TREE_TYPES_MODULE ) ) {
      throw DerivMakerException( "Module \"" TREE_TYPES_MODULE "\" not found" );
    }

    if ( ! Initializer::initialize( mach ) ) {
      throw DerivMakerException( "initialization failed" );
    }

    inverse_schema_binary = prepare_proc( module::UserModule(), INVERSE_SCHEMA_BINARY, 4 );
    if ( ! inverse_schema_binary ) {
      throw DerivMakerException( "Predicate " INVERSE_SCHEMA_BINARY "/4 not found" );
    }
    inverse_schema_unary = prepare_proc( module::UserModule(), INVERSE_SCHEMA_UNARY, 3 );
    if ( ! inverse_schema_unary ) {
      throw DerivMakerException( "Predicate " INVERSE_SCHEMA_UNARY "/3 not found" );
    }
    lexical_constraints = prepare_proc( module::UserModule(), LEXICAL_CONSTRAINTS, 2 );
    if ( ! lexical_constraints ) {
      throw DerivMakerException( "Predicate " LEXICAL_CONSTRAINTS "/2 not found" );
    }
    root_constraints = prepare_proc( module::UserModule(), ROOT_CONSTRAINTS, 1 );
    if ( ! root_constraints ) {
      throw DerivMakerException( "Predicate " ROOT_CONSTRAINTS "/1 not found" );
    }

    fail_point_inverse_schema_binary = prepare_proc( module::UserModule(), FAIL_POINT_INVERSE_SCHEMA_BINARY, 5 );
    if ( ! fail_point_inverse_schema_binary ) {
      throw DerivMakerException( "Predicate " FAIL_POINT_INVERSE_SCHEMA_BINARY "/5 not found" );
    }
    fail_point_inverse_schema_unary = prepare_proc( module::UserModule(), FAIL_POINT_INVERSE_SCHEMA_UNARY, 4 );
    if ( ! fail_point_inverse_schema_unary ) {
      throw DerivMakerException( "Predicate " FAIL_POINT_INVERSE_SCHEMA_UNARY "/4 not found" );
    }
    fail_point_lexical_constraints = prepare_proc( module::UserModule(), FAIL_POINT_LEXICAL_CONSTRAINTS, 3 );
    if ( ! fail_point_lexical_constraints ) {
      throw DerivMakerException( "Predicate " FAIL_POINT_LEXICAL_CONSTRAINTS "/3 not found" );
    }
    fail_point_root_constraints = prepare_proc( module::UserModule(), FAIL_POINT_ROOT_CONSTRAINTS, 2 );
    if ( ! fail_point_root_constraints ) {
      throw DerivMakerException( "Predicate " FAIL_POINT_ROOT_CONSTRAINTS "/2 not found" );
    }
  }

  bool DerivMaker::convertUnary( FSP schema_name, FSP mother_sign, Tree first_dtr, Derivation deriv ) {
    FSP dtr_sign = first_dtr.nodeSign();
    std::vector< FSP > args( 3 );
    args[ 0 ] = schema_name;
    args[ 1 ] = mother_sign;
    args[ 2 ] = dtr_sign;
    core_p cut_point = mach->GetCutPoint();
    if ( ! call_proc( *mach, inverse_schema_unary, args ) ) {
      string error_message( "'inverse_schema_unary/3' failed at \"" + first_dtr.coveredString() + "\"" );
      args.push_back( FSP( mach ) );
      if ( call_proc( *mach, fail_point_inverse_schema_unary, args ) && args[ 3 ].IsString() ) {
        error_message += "\n";
        error_message += args[ 3 ].ReadString();
      }
      throw DerivMakerFailure( error_message );
    }
    mach->DoCut( cut_point );
    Derivation dtr_deriv = FSP( *mach );
    if ( convert_( first_dtr, dtr_deriv ) ) {
      if ( deriv.derivSchema().Unify( schema_name ) &&
           deriv.derivSign().Unify( mother_sign ) &&
           deriv.derivDtrs().Follow( hd ).Unify( dtr_deriv ) &&
           deriv.derivDtrs().Follow( tl ).Coerce( nil ) ) {
        return true;
      } else {
        string covered_string = first_dtr.coveredString();
        throw DerivMakerFailure( "derivation cannot be unified with input (unary daughter) at \"" + covered_string + "\"" );
      }
    } else {
      // empty node
      return false;
    }
  }

  bool DerivMaker::convertBinary( FSP schema_name, FSP mother_sign, Tree first_dtr, Tree second_dtr, Derivation deriv ) {
    FSP left_sign = first_dtr.nodeSign();
    FSP right_sign = second_dtr.nodeSign();
    std::vector< FSP > args( 4 );
    args[ 0 ] = schema_name;
    args[ 1 ] = mother_sign;
    args[ 2 ] = left_sign;
    args[ 3 ] = right_sign;
    core_p cut_point = mach->GetCutPoint();
    if ( ! call_proc( *mach, inverse_schema_binary, args ) ) {
      string error_message( "'inverse_schema_binary/4' failed at \"" + first_dtr.coveredString() + "\" + \"" + second_dtr.coveredString() + "\"" );
      args.push_back( FSP( mach ) );
      if ( call_proc( *mach, fail_point_inverse_schema_binary, args ) && args[ 4 ].IsString() ) {
        error_message += "\n";
        error_message += args[ 4 ].ReadString();
      }
      throw DerivMakerFailure( error_message );
    }
    mach->DoCut( cut_point );
    Derivation left_deriv = FSP( *mach );
    Derivation right_deriv = FSP( *mach );
    FSP dtrs = deriv.derivDtrs();
    bool ret = convert_( first_dtr, left_deriv );
    if ( ret ) {
      if ( ! dtrs.Follow( hd ).Unify( left_deriv ) ) {
        string covered_string1 = first_dtr.coveredString();
        string covered_string2 = second_dtr.coveredString();
        throw DerivMakerFailure( "derivation cannot be unified with input (left daughter) at \"" + covered_string1 + "\" + \"" + covered_string2 + "\"" );
      }
      dtrs = dtrs.Follow( tl );
    }
    if ( convert_( second_dtr, right_deriv ) ) {
      if ( ! dtrs.Follow( hd ).Unify( right_deriv ) ) {
        string covered_string1 = first_dtr.coveredString();
        string covered_string2 = second_dtr.coveredString();
        throw DerivMakerFailure( "derivation cannot be unified with input (right daughter) at \"" + covered_string1 + "\" + \"" + covered_string2 + "\"" );
      }
      dtrs = dtrs.Follow( tl );
      ret = true;
    }
    if ( ret ) {
      if ( deriv.derivSchema().Unify( schema_name ) &&
           deriv.derivSign().Unify( mother_sign ) &&
           dtrs.Coerce( nil ) ) {
        return true;
      }
      string covered_string1 = first_dtr.coveredString();
      string covered_string2 = second_dtr.coveredString();
      throw DerivMakerFailure( "derivation cannot be unified with input (binary daughters) at \"" + covered_string1 + "\" + \"" + covered_string2 + "\"" );
    }
    return false;
  }

  bool DerivMaker::convert_( Tree tree, Derivation deriv ) {
    FSP mother_sign = tree.nodeSign();
    //cerr << mother_sign.DisplayAVM();
    //cerr << "c1" << endl;
    FSP dtrs = tree.treeDtrs();
    //cerr << "c2" << endl;
    if ( dtrs.IsValid() ) {
      //cerr << "c3" << endl;
      // internal node
      //cerr << "internal node" << endl;
      if ( dtrs.GetType() != cons ) {
        throw DerivMakerException( "Empty daughters found in nonterminal node" );
      }
      FSP schema_name = tree.schemaName();
      //cerr << schema_name.DisplayAVM();
      FSP first_dtr = dtrs.Follow( hd );
      dtrs = dtrs.Follow( tl );
      if ( dtrs.GetType() == nil ) {
        // unary
        //cerr << "unary" << endl;
        return convertUnary( schema_name, mother_sign, first_dtr, deriv );
      }
      FSP second_dtr = dtrs.Follow( hd );
      dtrs = dtrs.Follow( tl );
      if ( dtrs.GetType() != nil ) {
        // more than two daughters
        //cerr << "error" << endl;
        string covered_string = tree.coveredString();
        throw DerivMakerException( "More than two daughters found in nonterminal node at \"" + covered_string + "\"" );
      }
      // binary
      //cerr << "binary" << endl;
      return convertBinary( schema_name, mother_sign, first_dtr, second_dtr, deriv );
    } else {
      // terminal node
      //cerr << "terminal" << endl;
      // lexical constraints are coerced after all schemas are applied
      FSP word = tree.word();
      if ( word.IsValid() ) {
        // word
        tmp_terminals.push_back( make_pair( word, mother_sign ) );
        if ( deriv.derivSign().Unify( mother_sign ) &&
             deriv.termWord().Unify( word ) ) {
          return true;
        } else {
          string covered_string = tree.coveredString();
          throw DerivMakerFailure( "derivation cannot be unified with input (terminal) at \"" + covered_string + "\"" );
        }
      } else {
        // empty
        return false;
      }
    }
  }

  void DerivMaker::convertTerminal( FSP word, FSP sign ) {
    std::vector< FSP > args( 2 );
    args[ 0 ] = word;
    args[ 1 ] = sign;
    core_p cut_point = mach->GetCutPoint();
    if ( ! call_proc( *mach, lexical_constraints, args ) ) {
      string error_message( "'lexical_constraints/2' failed at \"" + Word( word ).input() + "\"" );
      args.push_back( FSP( mach ) );
      if ( call_proc( *mach, fail_point_root_constraints, args ) && args[ 2 ].IsString() ) {
        error_message += "\n";
        error_message += args[ 2 ].ReadString();
      }
      throw DerivMakerFailure( error_message );
    }
    mach->DoCut( cut_point );
  }

  void DerivMaker::convert( Tree tree, Derivation deriv ) {
    ++num_all;
    tmp_terminals.resize( 0 );
    FSP root_sign = tree.nodeSign();
    std::vector< FSP > args( 1 );
    args[ 0 ] = root_sign;
    core_p cut_point = mach->GetCutPoint();
    if ( ! call_proc( *mach, root_constraints, args ) ) {
      string error_message( "'root_constraints/1' failed" );
      args.push_back( FSP( mach ) );
      if ( call_proc( *mach, fail_point_root_constraints, args ) && args[ 1 ].IsString() ) {
        error_message += "\n";
        error_message += args[ 1 ].ReadString();
      }
      throw DerivMakerFailure( error_message );
    }
    mach->DoCut( cut_point );
    if ( convert_( tree, deriv ) ) {
      // Coerce lexical constraints
      for ( std::vector< std::pair< FSP, FSP > >::iterator term_it = tmp_terminals.begin();
            term_it != tmp_terminals.end();
            ++term_it ) {
        convertTerminal( term_it->first, term_it->second );
        //cerr << term_it->first.DisplayAVM();
        //cerr << term_it->second.DisplayAVM();
      }
      ++num_success;
      return;
    } else {
      // root node is empty
      throw DerivMakerFailure( "whole tree is empty" );
    }
  }

  void DerivMaker::showStatistics( std::ostream& os, const std::string& indent ) const {
    os << indent << "# successes = " << num_success << endl;
    os << indent << "# failures = " << num_all - num_success << endl;
  }

}

