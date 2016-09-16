/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file AmisModel.h
 *  @version Time-stamp: <2008-12-09 20:56:14 yusuke>
 *  Classes for handling ME models
 *
 **********************************************************************/

#ifndef MAYZ_AMIS_MODEL_H

#define MAYZ_AMIS_MODEL_H

#include <liblilfes/lconfig.h>
#include "mconfig.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace mayz {

  class AmisFeatureMask {
  public:
    typedef std::vector< bool >::const_iterator const_iterator;

  private:
    std::vector< bool > mask_list;

  public:
    const_iterator begin() const {
      return mask_list.begin();
    }
    const_iterator end() const {
      return mask_list.end();
    }

  public:
    AmisFeatureMask( const std::vector< int >& v ) {
      for ( std::vector< int >::const_iterator it = v.begin();
            it != v.end();
            ++it ) {
        if ( *it ) {
          mask_list.push_back( true );
        } else {
          mask_list.push_back( false );
        }
      }
    }
    ~AmisFeatureMask() {
    }

    size_t size() const {
      return mask_list.size();
    }
  };

//////////////////////////////////////////////////////////////////////

  class AmisFeature {
  public:
    typedef int TokenID;
    //static const TokenID INVALID_TOKEN_ID = -2;
    //static const TokenID DONT_CARE_ID = -1;
    enum {
        INVALID_TOKEN_ID = -2,
        DONT_CARE_ID = -1
    };

  private:
    std::vector< TokenID > token_list;

  public:
    typedef std::vector< TokenID >::iterator iterator;
    typedef std::vector< TokenID >::const_iterator const_iterator;

    iterator begin() { return token_list.begin(); }
    const_iterator begin() const { return token_list.begin(); }
    iterator end() { return token_list.end(); }
    const_iterator end() const { return token_list.end(); }

  public:
    AmisFeature() : token_list() {}
    AmisFeature( size_t s ) : token_list( s ) {}
    ~AmisFeature() {}

    void push_back( TokenID id ) { token_list.push_back( id ); }

    template<class IteratorT>
    void append(IteratorT begin, IteratorT end) { token_list.insert(token_list.end(), begin, end); }

    void resize( size_t s ) { token_list.resize( s ); }

    size_t size() const { return token_list.size(); }

    bool applyMask( const AmisFeature& id_list,
                    const AmisFeatureMask& mask ) {
      //feature.token_list.resize( id_list.size() );
      std::vector< TokenID >::iterator it = token_list.begin();
      AmisFeatureMask::const_iterator mask_it = mask.begin();
      for ( std::vector< TokenID >::const_iterator id_it = id_list.begin();
            id_it != id_list.end();
            ++id_it, ++it, ++mask_it ) {
        if ( *mask_it ) {
          if ( *id_it == INVALID_TOKEN_ID ) return false;
          *it = *id_it;
        } else {
          *it = DONT_CARE_ID;
        }
      }
      return true;
    }

    bool operator<( const AmisFeature& f ) const {
      std::vector< TokenID >::const_iterator f_it = f.token_list.begin();
      for ( std::vector< TokenID >::const_iterator it = token_list.begin();
            it != token_list.end();
            ++it, ++f_it ) {
        if ( *it < *f_it ) return true;
        if ( *it > *f_it ) return false;
      }
      return false;
    }

    bool operator==( const AmisFeature& f ) const {
      std::vector< TokenID >::const_iterator f_it = f.token_list.begin();
      for ( std::vector< TokenID >::const_iterator it = token_list.begin();
            it != token_list.end();
            ++it, ++f_it ) {
        if ( *it != *f_it ) return false;
      }
      return true;
    }
  };

//////////////////////////////////////////////////////////////////////

  class AmisModel {
  public:
    class AmisStringHashFun {
    public:
      size_t operator()( const std::string& f ) const {
        size_t x = 0;
        for ( std::string::const_iterator it = f.begin();
              it != f.end();
              ++it ) {
          x = x * 37 + static_cast< size_t >( *it );
        }
        return x;
      }
    };

    class AmisFeatureHashFun {
    public:
      size_t operator()( const AmisFeature& f ) const {
        size_t x = 0;
        for ( AmisFeature::const_iterator it = f.begin();
              it != f.end();
              ++it ) {
          x = x * 37 + *it + 2;
        }
        return x;
      }
    };

    typedef _HASHMAP< std::string, AmisFeature::TokenID, AmisStringHashFun > TokenTable;
    typedef _HASHMAP< AmisFeature, double, AmisFeatureHashFun > LambdaTable;

    struct FeatureTable {
      std::vector< AmisFeatureMask > mask_table;
      LambdaTable lambda_table;
    };

    typedef std::map< std::string, FeatureTable > FeatureTableMap;

  public:
    static const std::string TOKEN_SEPARATOR;
    static const std::string DONT_CARE;

  private:
    TokenTable token_table;
    std::vector< std::string > token_name_table;
    FeatureTableMap feature_table;

  protected:
    // static void split( const std::string& str,
    //                    const std::string& separator,
    //                    std::vector< std::string >& tokens ) {
    //   tokens.resize( 0 );
    //   int current_pos = 0;
    //   for ( ; ; ) {
    //     std::string::size_type pos = str.find( separator, current_pos );
    //     if ( pos == std::string::npos ) {
    //       tokens.push_back( str.substr( current_pos ) );
    //       break;
    //     }
    //     tokens.push_back( str.substr( current_pos, pos - current_pos ) );
    //     current_pos = pos + separator.length();
    //   }
    // }
    // static void join( const std::vector< std::string >& tokens,
    //                   const std::string& separator,
    //                   std::string& ret ) {
    //   std::vector< std::string >::const_iterator it = tokens.begin();
    //   ret = *it;
    //   ++it;
    //   for ( ; it != tokens.end(); ++it ) {
    //     ret += separator + *it;
    //   }
    // }

    // convert string to amis-escaped string
    static std::string str2amis(std::string str) {
      // \ -> \\ conversion
      std::string::size_type p = str.find("\\");
      while (p != std::string::npos) {
	str.replace(p, 1, "\\\\");
	p = str.find("\\", p + 2);
      }
      return str;
    }

    // convert amis-escaped string to string
    static std::string amis2str(std::string str) {
      // \? -> ?
      std::string::size_type p = str.find("\\");
      while (p != std::string::npos) {
	str.erase(p, 1);
	p = str.find("\\", p + 1);
      }
      return str;
    }

  public:
    static void encode( const std::string& category,
                        const std::vector< std::string >& tokens,
                        std::string& name ) {
      //join( tokens, AmisModel::TOKEN_SEPARATOR, name );
      std::vector< std::string >::const_iterator it = tokens.begin();
      name = str2amis(*it);
      ++it;
      for ( ; it != tokens.end(); ++it ) {
        name += AmisModel::TOKEN_SEPARATOR + str2amis(*it);
      }
      name += AmisModel::TOKEN_SEPARATOR + category;
    }
    static void encode( const std::string& category,
                        const std::vector< std::string >& tokens,
                        const std::string& feature_value,
                        std::string& name ) {
      encode( category, tokens, name );
      if ( ! feature_value.empty() ) {
        name += ':' + feature_value;
      }
    }
    static void encode( const std::string& category,
                        const std::vector< std::string >& tokens,
                        const double& feature_value,
                        std::string& name ) {
      std::ostringstream ostr;
      ostr << feature_value;
      encode( category, tokens, ostr.str(), name );
    }
    static void decode( const std::string& str,
                        std::string& category,
                        std::vector< std::string >& tokens ) {
      //split( name, AmisModel::TOKEN_SEPARATOR, tokens );
      tokens.resize( 0 );
      int current_pos = 0;
      for ( ; ; ) {
        std::string::size_type pos = str.find( AmisModel::TOKEN_SEPARATOR, current_pos );
        if ( pos == std::string::npos ) {
          tokens.push_back( amis2str( str.substr( current_pos ) ) );
          break;
        }
        tokens.push_back( amis2str( str.substr( current_pos, pos - current_pos ) ) );
        current_pos = pos + AmisModel::TOKEN_SEPARATOR.length();
      }
      category = tokens.back();
      tokens.pop_back();
    }
    static void decode( const std::string& name,
                        std::string& category,
                        std::vector< std::string >& tokens,
                        std::string& feature_value ) {
      std::string::size_type colon_pos = name.rfind( ':' );
      std::string feature_name;
      if ( colon_pos != std::string::npos ) {
        feature_value = name.substr( colon_pos + 1 );
        feature_name = name.substr( 0, colon_pos );
      } else {
        feature_name = name;
      }
      decode( feature_name, category, tokens );
    }

  public:
    AmisModel();
    virtual ~AmisModel();

  public:
    // static void extractFeatureValue( const std::string& name,
    //                                  std::string& feature,
    //                                  std::string& value ) {
    //   std::string::size_type colon_pos = name.rfind( ':' );
    //   if ( colon_pos != std::string::npos ) {
    //     feature = name.substr( 0, colon_pos );
    //     value = name.substr( colon_pos + 1 );
    //   } else {
    //     feature = name;
    //   }
    // }

    AmisFeature::TokenID getTokenID(const std::string &token) const {
      if ( token == DONT_CARE ) {
        return AmisFeature::DONT_CARE_ID;
      } else {
        TokenTable::const_iterator id = token_table.find( token );
        return ( id == token_table.end() ) ? AmisFeature::INVALID_TOKEN_ID : id->second;
      }
    }

    void makeAmisFeature( const std::vector< std::string >& token_list,
                        AmisFeature& id_list ) const {
      id_list.resize( token_list.size() );
      std::vector< std::string >::const_iterator token_it = token_list.begin();
      for ( AmisFeature::iterator id_it = id_list.begin();
            id_it != id_list.end();
            ++id_it, ++token_it ) {
        if ( *token_it == DONT_CARE ) {
          *id_it = AmisFeature::DONT_CARE_ID;
        } else {
          TokenTable::const_iterator id = token_table.find( *token_it );
          if ( id == token_table.end() ) {
            *id_it = AmisFeature::INVALID_TOKEN_ID;
          } else {
            *id_it = id->second;
          }
        }
      }
      return;
    }

    void makeAmisFeature( const std::string& name,
                        AmisFeature& id_list,
                        std::string& category ) const {
      // id_list.resize( 0 );
      // std::string::size_type pos = 0;
      // std::string token;
      // while ( true ) {
      //   std::string::size_type next_pos = name.find( TOKEN_SEPARATOR, pos );
      //   if ( next_pos == std::string::npos ) {
      //     category = name.substr( pos );
      //     return;
      //   }
      //   token = name.substr( pos, next_pos - pos );
      //   if ( token == DONT_CARE ) {
      //     id_list.push_back( AmisFeature::DONT_CARE_ID );
      //   } else {
      //     TokenTable::const_iterator id = token_table.find( token );
      //     if ( id == token_table.end() ) {
      //       id_list.push_back( AmisFeature::INVALID_TOKEN_ID );
      //     } else {
      //       id_list.push_back( id->second );
      //     }
      //   }
      //   pos = next_pos + TOKEN_SEPARATOR.size();
      // }
      std::vector<std::string> tokens;
      decode(name, category, tokens);
      makeAmisFeature(tokens, id_list);
    }

    void registerTokenList( const std::vector< std::string >& token_list,
                            AmisFeature& id_list ) {
      id_list.resize( token_list.size() );
      std::vector< std::string >::const_iterator token_it = token_list.begin();
      for ( AmisFeature::iterator id_it = id_list.begin();
            id_it != id_list.end();
            ++id_it, ++token_it ) {
        if ( *token_it == DONT_CARE ) {
          *id_it = AmisFeature::DONT_CARE_ID;
        } else {
          TokenTable::const_iterator id = token_table.find( *token_it );
          if ( id == token_table.end() ) {
            AmisFeature::TokenID new_id = token_name_table.size();
            token_table[ *token_it ] = new_id;
            token_name_table.push_back( *token_it );
            *id_it = new_id;
          } else {
            *id_it = id->second; 
         }
        }
      }
      return;
    }

    void registerTokenList( const std::string& name,
                            AmisFeature& id_list,
                            std::string& category ) {
      // id_list.resize( 0 );
      // std::string::size_type pos = 0;
      // while ( true ) {
      //   std::string::size_type next_pos = name.find( TOKEN_SEPARATOR, pos );
      //   if ( next_pos == std::string::npos ) {
      //     category = name.substr( pos );
      //     return;
      //   }
      //   std::string token = name.substr( pos, next_pos - pos );
      //   if ( token == DONT_CARE ) {
      //     id_list.push_back( AmisFeature::DONT_CARE_ID );
      //   } else {
      //     TokenTable::const_iterator id = token_table.find( token );
      //     if ( id == token_table.end() ) {
      //       AmisFeature::TokenID new_id = token_name_table.size();
      //       token_table[ token ] = new_id;
      //       token_name_table.push_back( token );
      //       id_list.push_back( new_id );
      //     } else {
      //       id_list.push_back( id->second );
      //     }
      //   }
      //   pos = next_pos + TOKEN_SEPARATOR.size();
      // }
      std::vector<std::string> tokens;
      decode(name, category, tokens);
      registerTokenList(tokens, id_list);
    }

  public:
    bool addFeatureMask( const std::string& category, const AmisFeatureMask& m ) {
      feature_table[ category ].mask_table.push_back( m );
      return true;
    }

    FeatureTable& getFeatureTable( const std::string& category ) {
      return feature_table[ category ];
    }

    std::vector< AmisFeatureMask >& getFeatureMasks( const std::string& category ) {
      return feature_table[ category ].mask_table;
    }

    // virtual bool extractFeatures( const std::string& name,
    //                               std::vector< AmisFeature >& features,
    //                               std::string& category ) const;
    virtual bool extractFeatures( const std::string& category,
                                  const std::vector< std::string >& token_list,
                                  std::vector< AmisFeature >& features ) const;
    static bool extractFeatures( const std::vector< AmisFeatureMask >& mask_table,
                                 const AmisFeature& id_list,
                                 std::vector< AmisFeature >& features );

  public:
    virtual bool importModel( std::istream& model_file );

    // bool featureWeight( const std::string& name1, double& weight ) const {
    //   // std::string name1;
    //   // std::string feature_value_str;
    //   // extractFeatureValue( name, name1, feature_value_str );
    //   // AmisFeature feature;
    //   // std::string category;
    //   // makeTokenList( name1, feature, category );
    //   std::string category;
    //   std::vector<std::string> tokens;
    //   std::string feature_value_str;
    //   decode(name1, category, tokens, feature_value_str);
    //   AmisFeature feature;
    //   makeAmisFeature(tokens, feature, category);
    //   FeatureTableMap::const_iterator table_it = feature_table.find( category );
    //   if ( table_it == feature_table.end() ) return false;  // unknown category
    //   double feature_value = feature_value_str.empty() ? 1.0 : strtod( feature_value_str.c_str(), NULL );
    //   LambdaTable::const_iterator lambda_it = table_it->second.lambda_table.find( feature );
    //   if ( lambda_it == table_it->second.lambda_table.end() ) return false;  // unknown feature
    //   weight = lambda_it->second * feature_value;
    //   return true;
    // }

    // double eventWeight( const std::string& name1 ) const {
    //   // std::string name1;
    //   // std::string feature_value_str;
    //   // extractFeatureValue( name, name1, feature_value_str );
    //   // AmisFeature id_list;
    //   // std::string category;
    //   // makeTokenList( name1, id_list, category );
    //   std::string category;
    //   std::vector<std::string> tokens;
    //   std::string feature_value_str;
    //   decode(name1, category, tokens, feature_value_str);
    //   AmisFeature id_list;
    //   makeAmisFeature(tokens, id_list, category);
    //   double feature_value = feature_value_str.empty() ? 1.0 : strtod( feature_value_str.c_str(), NULL );
    //   return eventWeight( category, id_list, feature_value );
    // }
    double eventWeight( const std::string& category, const std::vector< std::string >& token_list, double feature_value = 1.0 ) const {
      AmisFeature id_list( token_list.size() );
      makeAmisFeature( token_list, id_list );
      return eventWeight( category, id_list, feature_value );
    }

    double eventWeight( const std::string& category, const AmisFeature& id_list, double feature_value = 1.0 ) const {
      FeatureTableMap::const_iterator table_it = feature_table.find( category );
      if ( table_it == feature_table.end() ) return 0.0;  // unknown category
      AmisFeature feature( id_list.size() );
      double ret = 0.0;
      for ( std::vector< AmisFeatureMask >::const_iterator mask_it = table_it->second.mask_table.begin();
            mask_it != table_it->second.mask_table.end();
            ++mask_it ) {
        if ( feature.applyMask( id_list, *mask_it ) ) {
          LambdaTable::const_iterator lambda_it = table_it->second.lambda_table.find( feature );
          if ( lambda_it != table_it->second.lambda_table.end() ) {
            //std::cerr << featureString( category, feature ) << "\t\t" << lambda_it->second * feature_value << std::endl;
            ret += lambda_it->second * feature_value;
          }
        }
      }
      return ret;
    }

    std::string featureString( const std::string& category, const AmisFeature& feature ) const {
      std::string ret;
      return featureString( category, token_name_table, feature );
    }

    static std::string featureString( const std::string& category, const std::vector< std::string >& token_name_table, const AmisFeature& feature ) {
      std::string ret;
      for ( AmisFeature::const_iterator it = feature.begin();
            it != feature.end();
            ++it ) {
        if ( *it == AmisFeature::DONT_CARE_ID ) {
          ret += DONT_CARE;
        } else {
          ret += str2amis(token_name_table[ *it ]);
        }
        ret += TOKEN_SEPARATOR;
      }
      ret += category;
      return ret;
    }
  };

//////////////////////////////////////////////////////////////////////

  class AmisHandler {
  private:
    std::map< std::string, AmisModel* > amis_table;

  protected:
    void erase( std::map< std::string, AmisModel* >::iterator it ) {
      delete it->second;
      it->second = NULL;
      amis_table.erase( it );
    }

  public:
    AmisHandler();
    virtual ~AmisHandler();

    virtual void deleteAmisModel( const std::string& name );
    virtual AmisModel* newAmisModel( const std::string& name );
    virtual AmisModel* getAmisModel( const std::string& name );

  };

}

#endif // MAYZ_AMIS_MODEL_H
