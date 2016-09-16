/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file AmisFilter.h
 *  @version Time-stamp: <2005-06-21 19:08:00 yusuke>
 *  Filtering infrequent ME features in amis-style event files
 *
 **********************************************************************/

#ifndef MAYZ_AMIS_FILTER_H

#define MAYZ_AMIS_FILTER_H

#include "mconfig.h"
#include "AmisModel.h"

namespace mayz {

  class AmisFilterException : public std::exception {
  private:
    std::string message;
  public:
    AmisFilterException( const char* m ) : message( m ) {}
    AmisFilterException( const std::string& m ) : message( m ) {}
    virtual ~AmisFilterException() throw () {}
    const char* what() const throw () { return message.c_str(); }
  };

  //////////////////////////////////////////////////////////////////////

  class AmisFeatureCounter {
  private:
    std::map< std::string, std::map< AmisFeature, int > > count_table;
    AmisModel* model;
    size_t num_events;

	bool count_negative; /// ADD by matsuzaki 06/11/18

  public:
    // AmisFeatureCounter( AmisModel* m );
    AmisFeatureCounter( AmisModel* m, bool count_negative );
    virtual ~AmisFeatureCounter();

    size_t numFeatures() const {
      size_t num = 0;
      for ( std::map< std::string, std::map< AmisFeature, int > >::const_iterator it = count_table.begin();
            it != count_table.end();
            ++it )
        num += it->second.size();
      return num;
    }
    size_t numEvents() const { return num_events; }

    virtual bool importUnfilteredEvents( std::istream& event_file, int limit_events = 0, std::ostream* log_stream = NULL );

    virtual bool outputCountFile( std::ostream& count_file ) const;
  };

//////////////////////////////////////////////////////////////////////

  class AmisFilter {
  private:
    std::map< std::string, std::map< AmisFeature, int > > count_table;
    AmisModel* model;
    size_t num_events;

  public:
    AmisFilter( AmisModel* m );
    virtual ~AmisFilter();

    size_t numFeatures() const {
      size_t num = 0;
      for ( std::map< std::string, std::map< AmisFeature, int > >::const_iterator it = count_table.begin();
            it != count_table.end();
            ++it )
        num += it->second.size();
      return num;
    }
    size_t numEvents() const { return num_events; }

    virtual bool importCountFile( std::istream& count_file, int threshold );

    virtual bool makeAmisData( std::istream& unfiltered_file,
                               std::ostream& model_file,
                               std::ostream& event_file,
                               int limit_events = 0,
                               std::ostream* log_stream = NULL );
  };

} // namespace mayz

#endif // MAYZ_AMIS_FILTER_H
