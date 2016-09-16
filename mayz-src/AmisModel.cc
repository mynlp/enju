/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file AmisModel.cc
 *  @version Time-stamp: <2006-06-03 19:27:52 yusuke>
 *  Classes for handling ME models
 *
 **********************************************************************/

#include <memory>
#include "AmisModel.h"

using namespace std;
using namespace mayz;

namespace mayz {

  const std::string AmisModel::TOKEN_SEPARATOR( "//" );
  const std::string AmisModel::DONT_CARE( "_" );

  AmisModel::AmisModel() {
  }
  AmisModel::~AmisModel() {
  /*
    for ( std::vector< std::vector< std::string >* >::iterator it = feature_table.begin();
          it != feature_table.end();
          ++it ) {
      delete *it;
    }
  */
  }

  // bool AmisModel::extractFeatures( const std::string& name,
  //                                  std::vector< AmisFeature >& features,
  //                                  std::string& category ) const {
  //   AmisFeature id_list;
  //   makeTokenList( name, id_list, category );
  //   FeatureTableMap::const_iterator table_it = feature_table.find( category );
  //   if ( table_it == feature_table.end() ) return false;  // unknown category
  //   return extractFeatures( table_it->second.mask_table, id_list, features );
  // }

  bool AmisModel::extractFeatures( const std::string& category,
                                   const std::vector< std::string >& token_list,
                                   std::vector< AmisFeature >& features ) const {
    FeatureTableMap::const_iterator table_it = feature_table.find( category );
    if ( table_it == feature_table.end() ) return false;  // unknown category
    AmisFeature id_list( token_list.size() );
    makeAmisFeature( token_list, id_list );
    return extractFeatures( table_it->second.mask_table, id_list, features );
  }

  bool AmisModel::extractFeatures( const std::vector< AmisFeatureMask >& mask_table,
                                   const AmisFeature& id_list,
                                   std::vector< AmisFeature >& features ) {
    for ( std::vector< AmisFeatureMask >::const_iterator mask_it = mask_table.begin();
          mask_it != mask_table.end();
          ++mask_it ) {
      AmisFeature feature( id_list.size() );
      if ( feature.applyMask( id_list, *mask_it ) ) {
        features.push_back( feature );
      }
    }
    return true;
  }

  bool AmisModel::importModel( std::istream& model_file ) {
    std::string name;
    double value;
    std::string category;
    AmisFeature token_list;
    while ( model_file ) {
      model_file >> name >> value;
      double x = log( value );
      if ( x != 0.0 ) {
        registerTokenList( name, token_list, category );
        feature_table[ category ].lambda_table[ token_list ] = x;
      }
    }
    return true;
  }

  AmisHandler::AmisHandler() {
  }
  AmisHandler::~AmisHandler() {
    for ( std::map< std::string, AmisModel* >::iterator it = amis_table.begin();
          it != amis_table.end(); ) {
      if ( it->second != NULL ) {
        delete it->second;
      }
      amis_table.erase(it++);
    }
  }

  void AmisHandler::deleteAmisModel( const std::string& name ) {
    std::map< std::string, AmisModel* >::iterator it = amis_table.find( name );
    if ( it == amis_table.end() || it->second == NULL ) {
      //RUNERR("Amis model is not found: " << name);
      return;
    }
    erase( it );
  }

  AmisModel* AmisHandler::newAmisModel( const std::string& name ) {
    if ( amis_table.find( name ) != amis_table.end() ) {
      //RUNERR("Amis model is already generated: " << name);
      return NULL;
    }
    return amis_table[ name ] = new AmisModel;
  }

  AmisModel* AmisHandler::getAmisModel( const std::string& name ) {
    std::map< std::string, AmisModel* >::iterator it = amis_table.find( name );
    if ( it == amis_table.end() ) {
      //RUNERR("Amis model not found: " << name);
      return NULL;
    }
    return it->second;
  }

}
