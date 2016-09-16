/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file AmisFilter.cc
 *  @version Time-stamp: <2006-11-20 15:31:43 yusuke>
 *  Filtering infrequent ME features in amis-style event files
 *
 **********************************************************************/

#include "AmisFilter.h"

namespace mayz {

  AmisFeatureCounter::AmisFeatureCounter( AmisModel* m, bool count_negative )
    : model( m ), num_events( 0 ), count_negative(count_negative) {
  }
  AmisFeatureCounter::~AmisFeatureCounter() {
  }

  bool AmisFeatureCounter::importUnfilteredEvents( std::istream& event_file, int limit_events, std::ostream* log_stream ) {
    num_events = 0;
    while ( event_file ) {
      std::string line;
      getline( event_file, line );  // event_name
      if ( ! event_file ) break;  // end of file
      if ( line.empty() ) continue;  // empty line
      ++num_events;
      while ( true ) {
	getline( event_file, line );
        if ( ! event_file ) break;  // end of file
	if ( line.empty() ) break;  // end of the event
	if ( line[ 0 ] == '{' ) continue;  // in case of feature forests
	std::istringstream str( line );
	int count;
	str >> count;
	/// MODIFY by matsuzaki 06/11/18
	/// if ( count <= 0 ) continue;
	if ( ! count_negative && count <= 0 ) continue;
	/// MODIFY-END
	std::string event1;
	while ( str >> event1 ) {
	  //cerr << count << "\t" << event << endl;
          // std::string event;
          // std::string feature_value;
          // AmisModel::extractFeatureValue( event1, event, feature_value );
          // std::string category;
          // AmisFeature id_list;
          // model->registerTokenList( event, id_list, category );
          std::string category;
	  std::vector<std::string> tokens;
          std::string feature_value;
	  model->decode(event1, category, tokens, feature_value);
          AmisFeature id_list;
          model->registerTokenList( tokens, id_list );
          std::map< AmisFeature, int >& count_tb = count_table[ category ];
          const AmisModel::FeatureTable& table = model->getFeatureTable( category );
          std::vector< AmisFeature > feature_list;
          AmisModel::extractFeatures( table.mask_table, id_list, feature_list );
	  //cerr << "hoge1" << endl;
          for ( std::vector< AmisFeature >::const_iterator feature_it = feature_list.begin();
                feature_it != feature_list.end();
                ++feature_it ) {
	    count_tb[ *feature_it ] += 1;  // count;  ***** should be fixed to add the count of positive event
	  }
	}
      }
      if ( limit_events > 0 && num_events >= (size_t)limit_events ) break;
      if ( log_stream && num_events % 1000 == 0 ) {
        *log_stream << "  # events = " << numEvents() << "  # features = " << numFeatures() << std::endl;
      }
    }
    return true;
  }

  bool AmisFeatureCounter::outputCountFile( std::ostream& count_file ) const {
    for ( std::map< std::string, std::map< AmisFeature, int > >::const_iterator table_it = count_table.begin();
          table_it != count_table.end();
          ++table_it ) {
      const std::string& category = table_it->first;
      for ( std::map< AmisFeature, int >::const_iterator feature_it = table_it->second.begin();
            feature_it != table_it->second.end();
            ++feature_it ) {
        count_file << model->featureString( category, feature_it->first ) << '\t' << feature_it->second << '\n';
      }
    }
    return true;
  }

  //////////////////////////////////////////////////////////////////////

  AmisFilter::AmisFilter( AmisModel* m )
    : model( m ), num_events( 0 ) {
  }
  AmisFilter::~AmisFilter() {
  }

  bool AmisFilter::importCountFile( std::istream& count_file, int threshold ) {
    std::string name;
    int count;
    count_file >> name >> count;
    while ( count_file ) {
      if ( count >= threshold ) {
        AmisFeature feature;
        std::string category;
        model->registerTokenList( name, feature, category );
        count_table[ category ][ feature ] = count;
      }
      count_file >> name >> count;
    }
    return true;
  }

  bool AmisFilter::makeAmisData( std::istream& unfiltered_file,
                                 std::ostream& model_file,
                                 std::ostream& event_file,
                                 int limit_events,
                                 std::ostream* log_stream ) {
    for ( std::map< std::string, std::map< AmisFeature, int > >::const_iterator table_it = count_table.begin();
          table_it != count_table.end();
          ++table_it ) {
      for ( std::map< AmisFeature, int >::const_iterator feature = table_it->second.begin();
            feature != table_it->second.end();
            ++feature ) {
        model_file << model->featureString( table_it->first, feature->first ) << "\t1.0\n";
      }
    }
    num_events = 0;
    while ( unfiltered_file ) {
      std::string line;
      getline( unfiltered_file, line );  // event_name
      if ( ! unfiltered_file ) break;  // end of file
      if ( line.empty() ) continue;  // empty line
      ++num_events;
      event_file << line << '\n';
      while ( true ) {
	getline( unfiltered_file, line );
        if ( ! unfiltered_file ) throw AmisFilterException( "event file seems to be truncated" );  // truncated event file
	if ( line.empty() ) break;  // empty line
	std::istringstream str( line );
	std::string event;
	if ( line[ 0 ] == '{' ) {
	  // feature forest model
	  str >> event;
	  event_file << event << ' ';  // '{'
	  str >> event;
	  event_file << event << ' ';  // node name
	} else {
	  int count;
	  str >> count;
	  event_file << count << '\t';
	}
	while ( str >> event ) {
	  if ( event == "{" || event == "(" ) {
	    // start of node in feature forest model
	    event_file << event << ' ';
	    str >> event;  // node name
	    event_file << event << ' ';
	  } else if ( event == "}" || event == ")" ) {
	    // end of node in feature forest model
	    event_file << event << ' ';
	  } else if ( event[ 0 ] == '$' ) {
	    // variable in featore forest model
	    event_file << event << ' ';
	  } else {
	    // other case
            // std::string feature_value;
            // std::string event1;
            // AmisModel::extractFeatureValue( event, event1, feature_value );
            // std::string category;
            // AmisFeature id_list;
            // model->makeTokenList( event1, id_list, category );
	    std::string category;
	    std::vector<std::string> tokens;
	    std::string feature_value;
	    model->decode(event, category, tokens, feature_value);
	    AmisFeature id_list;
	    model->registerTokenList( tokens, id_list );
            const AmisModel::FeatureTable& table = model->getFeatureTable( category );
            std::map< AmisFeature, int >& count_tb = count_table[ category ];
            std::vector< AmisFeature > feature_list;
            AmisModel::extractFeatures( table.mask_table, id_list, feature_list );
            //cerr << "hoge1" << endl;
            for ( std::vector< AmisFeature >::const_iterator feature_it = feature_list.begin();
                  feature_it != feature_list.end();
                  ++feature_it ) {
              if ( count_tb.find( *feature_it ) != count_tb.end() ) {
                event_file << model->featureString( category, *feature_it );
                if ( ! feature_value.empty() ) {
                  event_file << ':' << feature_value;
                }
                event_file << ' ';
              }
            }
	  }
	}
	event_file << '\n';
      } // while
      event_file << '\n';
      if ( limit_events > 0 && num_events >= (size_t)limit_events ) break;
      if ( log_stream && num_events % 1000 == 0 ) {
        *log_stream << "  # events = " << numEvents() << std::endl;
      }
    }
    return true;
  }

}


