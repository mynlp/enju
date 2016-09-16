/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file TreeConverter.h
 *  @version Time-stamp: <2005-06-21 19:01:08 yusuke>
 *  Converting trees using pattern rules
 *
 **********************************************************************/

#ifndef MAYZ_TREE_CONVERTER_H

#define MAYZ_TREE_CONVERTER_H

#include "mconfig.h"
#include <exception>
#include <vector>
#include <string>
#include <liblilfes/builtin.h>

namespace mayz {

  class TreeConverterException : public std::exception {
  private:
    std::string message;
  public:
    TreeConverterException( const char* m ) : message( m ) {}
    TreeConverterException( const std::string& m ) : message( m ) {}
    ~TreeConverterException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

//    class TreeConvertRule {
//    private:
//      std::string name;
//      lilfes::procedure* pred;
//    public:
//      TreeConvertRule( const std::string& n, lilfes::procedure* p )
//        : name( n ), pred( p ) {}
//      virtual ~TreeConvertRule() {}

//      const std::string& getName() const { return name; }

//      bool convertNode( lilfes::machine* mach, lilfes::FSP in, lilfes::FSP out );
//      virtual bool convert( lilfes::machine* mach, lilfes::FSP in, lilfes::FSP out ) = 0;
//    };

//    class TreeConvertRuleRootOnly : public TreeConvertRule {
//    public:
//      TreeConvertRuleRootOnly( const std::string& n, lilfes::procedure* p )
//        : TreeConvertRule( n, p ) {}
//      ~TreeConvertRuleRootOnly() {}

//      bool convert( lilfes::machine* mach, lilfes::FSP in, lilfes::FSP out );
//    };

//    class TreeConvertRuleBottomUp : public TreeConvertRule {
//    public:
//      TreeConvertRuleBottomUp( const std::string& n, lilfes::procedure* p )
//        : TreeConvertRule( n, p ) {}
//      ~TreeConvertRuleBottomUp() {}

//      bool convert( lilfes::machine* mach, lilfes::FSP in, lilfes::FSP out );
//    };

//    class TreeConvertRuleTopDown : public TreeConvertRule {
//    public:
//      TreeConvertRuleTopDown( const std::string& n, lilfes::procedure* p )
//        : TreeConvertRule( n, p ) {}
//      ~TreeConvertRuleTopDown() {}

//      bool convert( lilfes::machine* mach, lilfes::FSP in, lilfes::FSP out );
//    };

//    class TreeConverter {
//    private:
//      lilfes::machine* mach;
//      std::vector< TreeConvertRule* > rules;

//    public:
//      TreeConverter( lilfes::machine* m ) : mach( m ) {}
//      virtual ~TreeConverter() {
//        while ( ! rules.empty() ) {
//          delete rules.back();
//          rules.pop_back();
//        }
//      }

//      virtual void initialize();

//      virtual bool convert( lilfes::FSP in, lilfes::FSP out );

//      virtual void showStatistics( std::ostream& os ) const;

//    };

  class TreeConverter {
  private:
    lilfes::machine* mach;
    lilfes::procedure* convert_proc;

    size_t num_all;
    size_t num_success;
    size_t num_fail;

    int verbose_level;

  public:
    TreeConverter( lilfes::machine* m )
      : mach( m ),
        convert_proc( NULL ),
        num_all( 0 ),
        num_success( 0 ),
        num_fail( 0 ),
        verbose_level( 0 ) {
    }
    virtual ~TreeConverter() {}

    virtual void initialize();
    virtual bool convert( lilfes::FSP in, lilfes::FSP& out );
    virtual void showStatistics( std::ostream& os ) const;

    void setVerboseLevel( int l = 0 ) {
      verbose_level = l;
    }
    int getVerboseLevel() const {
      return verbose_level;
    }
  };

}

#endif // MAYZ_TREE_CONVERTER_H
