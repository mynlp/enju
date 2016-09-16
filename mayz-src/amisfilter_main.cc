/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file amisfilter_main.cc
 *  @version Time-stamp: <2008-07-14 14:51:26 yusuke>
 *  Main program of amisfilter
 *
 **********************************************************************/

#include "AmisFilter.h"

#include <liblilfes/lilfes.h>
#include <liblilfes/builtin.h>
#include <liblilfes/utility.h>
#include <liblilfes/bfstream.h>
#include <liblilfes/gfstream.h>
#include <liblilfes/rl_stream.h>
//#include <liblilfes/option.h>

#include <iterator>
#include <iostream>
#include <sstream>
#include <memory>
#include <cstring>
#include <stdexcept>

#ifdef _MSC_VER
#include <iterator>
#endif

using namespace std;
using namespace mayz;
using namespace lilfes;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

string model_name;
string mask_module_name;
string uevent_file_name;
string count_file_name;
string model_file_name;
string event_file_name;
static int threshold = 1;
static int limit_events = 0;
static int verbose_level = 0;
static bool count_negative = false;
static bool flat_mask_file = false;

enum file_type {
  RAW, BZ2, GZ
};

static const char* file_type_name[sizeof(file_type)] = { "raw", "bz2", "gz" };

static file_type uevent_file_type = RAW;
static file_type count_file_type = RAW;
static file_type model_file_type = RAW;
static file_type event_file_type = RAW;

//////////////////////////////////////////////////////////////////////

void help_message() {
  *error_stream << "Usage: amisfilter [options] model_name mask_module uevent_file count_file model_file event_file" << endl;
  *error_stream << "  model_name: name of the model to be made" << endl;
  *error_stream << "  mask_module: LiLFeS module or flat file in which feature masks are defined" << endl;
  *error_stream << "  uevent_file: unfiltered event file" << endl;
  *error_stream << "  count_file: output file of feature counts" << endl;
  *error_stream << "  model_file: amis-style model file" << endl;
  *error_stream << "  event_file: amis-style event file" << endl;
  *error_stream << "Options:" << endl;
  *error_stream << "  -t threshold: feature threshold (default: " << threshold << ")" << endl;
  *error_stream << "  -n num:       limit number of events (default: " << limit_events << ")" << endl;
  *error_stream << "  -fm:          use flat file format mask file" << std::endl;
  *error_stream << "  -v:           print verbose messages" << endl;
  *error_stream << "  -vv:          print many verbose messages" << endl;
  *error_stream << "  -N:           count features in both positive and negative examples in thresholding" << endl;
  return;
}

bool analyze_arguments( int argc, char** argv ) {
  char** end = argv + argc;
  ++argv;
  for ( ; argv != end; ++argv ) {
    if ( (*argv)[ 0 ] == '-' ) {
      // option without argument
      if ( std::strcmp( (*argv), "-v" ) == 0 ) {
        verbose_level = 1;
      } else if ( std::strcmp( (*argv), "-vv" ) == 0 ) {
        verbose_level = 2;
      } else if ( std::strcmp( (*argv), "-N" ) == 0 ) {
        count_negative = true;
      } else if ( std::strcmp( (*argv), "-fm" ) == 0 ) {
        flat_mask_file = true;
      } else {
        // option with argument
        if ( argv + 1 == end ) {
          *error_stream << *argv << " option requires additional argument" << endl;
          return false;
        }
        if ( std::strcmp( (*argv), "-t" ) == 0 ) {
          threshold = strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-n" ) == 0 ) {
          limit_events = strtol( *(++argv), NULL, 0 );
        } else {
          *error_stream << "Unknown option: " << *argv << endl;
          return false;
        }
      }
    } else {
      // arguments
      if ( end - argv != 6 ) {
        *error_stream << "amisfilter requires six arguments" << endl;
        return false;
      }
      model_name = argv[ 0 ];
      mask_module_name = argv[ 1 ];
      uevent_file_name = argv[ 2 ];
      count_file_name = argv[ 3 ];
      model_file_name = argv[ 4 ];
      event_file_name = argv[ 5 ];
      return true;
    }
  }
  *error_stream << "amisfilter requires six arguments" << endl;
  return false;
}

//////////////////////////////////////////////////////////////////////

auto_ptr< istream > new_input_stream( const string& file_name, file_type ftype ) {
  switch( ftype ) {
  case RAW:
    return auto_ptr< istream >( new ifstream( file_name.c_str() ) );
  case BZ2:
    return auto_ptr< istream >( new ibfstream( file_name.c_str() ) );
  case GZ:
    return auto_ptr< istream >( new igfstream( file_name.c_str() ) );
  }
  return auto_ptr< istream >( NULL );
}

auto_ptr< ostream > new_output_stream( const string& file_name, file_type ftype ) {
  switch( ftype ) {
  case RAW:
    return auto_ptr< ostream >( new ofstream( file_name.c_str() ) );
  case BZ2:
    return auto_ptr< ostream >( new obfstream( file_name.c_str() ) );
  case GZ:
    return auto_ptr< ostream >( new ogfstream( file_name.c_str() ) );
  }
  return auto_ptr< ostream >( NULL );
}

//////////////////////////////////////////////////////////////////////

// Set feature masks to amis model

bool set_feature_masks( machine& mach, AmisModel& amis_model, const string& mask_module_name, const string& model_name ) {
  // Load feature mask
  *error_stream << "Loading feature mask module \"" << mask_module_name << "\"... ";
  module* mask_module = module::new_module( mach, "mask module", mask_module_name.c_str(), module::NMT_ENSURE_INITIAL, module::UserModule() );
  if ( ! mask_module ) {
    *error_stream << "Feature mask module \"" << mask_module_name << "\" not found" << endl;
    return false;
  }
  mach.parse();
  mask_module->Finish();
  *error_stream << "done." << endl;

  // Initialize feature mask
  *error_stream << "Initialize feature masks for \"" << model_name << "\"... ";
  procedure* mask_proc = prepare_proc( module::UserModule(), "feature_mask", 3 );
  if ( mask_proc == NULL ) {
    *error_stream << "Cannot find feature_mask/3" << endl;
    return false;
  }
  vector< FSP > args( 3 );
  vector< bool > output_flags( 3 );
  args[ 0 ] = FSP( mach, model_name.c_str() );  output_flags[ 0 ] = false;  // model name
  args[ 1 ] = FSP( mach );                      output_flags[ 1 ] = true;   // category name
  args[ 2 ] = FSP( mach );                      output_flags[ 2 ] = true;   // mask
  list< vector< FSP > > results;
  findall_proc( mach, mask_proc, args, output_flags, results );

  for ( list< vector< FSP > >::const_iterator result_it = results.begin();
        result_it != results.end();
        ++result_it ) {
    if ( ! (*result_it)[ 0 ].IsString() ) {
      *error_stream << "2nd argument of 'feature_mask/3' must be string" << endl;
      return false;
    }
    string category_name( (*result_it)[ 0 ].ReadString() );
    vector< int > mask;
    if ( ! list_to_vector< int >( mach, (*result_it)[ 1 ], mask ) ) {
      *error_stream << "3rd argument of 'feature_mask/3' must be string" << endl;
      return false;
    }
    if ( ! amis_model.addFeatureMask( category_name, mask ) ) {
      *error_stream << "Mask has different length: " << category_name << " has " << mask.size() << "bits" << endl;
      return false;
    }
  }
  *error_stream << "done." << endl;
  return true;
}

bool set_feature_masks_from_flat_file( AmisModel& amis_model, const string& mask_module_name, const string& model_name ) {
  // Load feature mask
  *error_stream << "Loading feature masks from \"" << mask_module_name << "\"... ";
  std::ifstream mask_file(mask_module_name.c_str());
  if (! mask_file) {
    *error_stream << "Feature mask module \"" << mask_module_name << "\" not found" << endl;
    return false;
  }
  std::string line;
  while (std::getline(mask_file, line)) {
      if (line.empty() || line[0] == '#') {
          continue;
      }
      std::istringstream iss(line);
      std::string tag;
      if (! (iss >> tag)) {
          std::cerr << "Mask file format error" << std::endl;
          return false;
      }

      if (tag != "m") {
          continue;
      }

      std::string name;
      std::string category;
      if (! (iss >> name >> category)) {
          std::cerr << "Mask file format error: " << line << std::endl;
          return false;
      }

      if (name != model_name) {
          continue;
      }

      std::vector<int> mask;
      std::copy(std::istream_iterator<int>(iss),
                std::istream_iterator<int>(),
                std::back_inserter(mask));
      
      if ( ! amis_model.addFeatureMask( category, mask ) ) {
        *error_stream << "Mask has different length: " << category << " has " << mask.size() << "bits" << endl;
        return false;
      }
  }
  *error_stream << "done." << endl;
  return true;
}

//////////////////////////////////////////////////////////////////////

// Make count file

bool make_count_file( AmisModel& amis_model, const string& uevent_file_name, const string& count_file_name ) {
  *error_stream << "Making count file \"" << count_file_name << "\"..." << endl;
  // Open unfiltered event file
  auto_ptr< istream > unfiltered_event_file = new_input_stream( uevent_file_name, uevent_file_type );
  if ( ! *unfiltered_event_file ) {
    *error_stream << "Cannot open unfiltered event file: " << uevent_file_name << endl;
    return false;
  }
  // Open count file
  auto_ptr< ostream > count_file = new_output_stream( count_file_name, count_file_type );
  if ( ! *count_file ) {
    *error_stream << "Cannot open count file: " << count_file_name << endl;
    return false;
  }
  // Make count file
  /// MODIFY by matsuzaki 06/11/18
  /// AmisFeatureCounter feature_counter( &amis_model );
  AmisFeatureCounter feature_counter( &amis_model, count_negative );
  /// MODIFY-end
  if ( ! feature_counter.importUnfilteredEvents( *unfiltered_event_file, limit_events, &*error_stream ) ) {
    *error_stream << "Cannot import unfiltered event file: " << uevent_file_name << endl;
    return false;
  }
  *error_stream << "# events = " << feature_counter.numEvents() << "  # features = " << feature_counter.numFeatures() << endl;
  if ( ! feature_counter.outputCountFile( *count_file ) ) {
    *error_stream << "Cannot output to a count file: " << count_file_name << endl;
    return false;
  }
  *error_stream << "done." << endl;
  return true;
}

//////////////////////////////////////////////////////////////////////

// Make model and event files

bool make_amis_files( AmisModel& amis_model, const string& uevent_file_name,
                      const string& count_file_name, const string& model_file_name, const string& event_file_name ) {
  // Open unfiltered event file
  auto_ptr< istream > unfiltered_event_file = new_input_stream( uevent_file_name, uevent_file_type );
  if ( ! *unfiltered_event_file ) {
    *error_stream << "Cannot open unfiltered event file: " << uevent_file_name << endl;
    return false;
  }
  // Open count file
  auto_ptr< istream > count_file = new_input_stream( count_file_name, count_file_type );
  if ( ! *count_file ) {
    *error_stream << "Cannot open count file: " << count_file_name << endl;
    return false;
  }
  // Open model file
  auto_ptr< ostream > model_file = new_output_stream( model_file_name, model_file_type );
  if ( ! *model_file ) {
    *error_stream << "Cannot open model file: " << model_file_name << endl;
    return false;
  }
  // Open event file
  auto_ptr< ostream > event_file = new_output_stream( event_file_name, event_file_type );
  if ( ! *event_file ) {
    *error_stream << "Cannot open event file: " << event_file_name << endl;
    return false;
  }

  // Make model, event files
  *error_stream << "Import a count file... ";
  AmisFilter amis_filter( &amis_model );
  if ( ! amis_filter.importCountFile( *count_file, threshold ) ) {
    *error_stream << "Cannot import a count file: " << count_file_name << endl;
    return false;
  }
  *error_stream << "done." << endl;
  *error_stream << "# features = " << amis_filter.numFeatures() << endl;
  *error_stream << "Making model and event files..." << endl;
  if ( ! amis_filter.makeAmisData( *unfiltered_event_file, *model_file, *event_file, limit_events, &*error_stream ) ) {
    *error_stream << "Cannot make amis data" << endl;
    return false;
  }
  *error_stream << "# events = " << amis_filter.numEvents() << endl;
  *error_stream << "done." << endl;
  return true;
}

//////////////////////////////////////////////////////////////////////

// Main routine

int main(int argc, char **argv)
{

  *error_stream << "amisfilter (MAYZ " << MAYZ_VERSION << ")" << endl;
  *error_stream << MAYZ_COPYRIGHT << endl;

  try {
    // Initialize everything
    Initializer::PerformAll();

    // show help message
    if ( argc > 1 && std::strcmp( argv[ 1 ], "-h" ) == 0 ) {
      help_message();
      return 0;
    }
    // command-line arguments
    if ( ! analyze_arguments( argc, argv ) ) {
      help_message();
      return 1;
    }

    // Initialize a machine object
    machine mach;

    // file type prediction
    if ( uevent_file_name.substr( uevent_file_name.size() - 4, string::npos ) == ".bz2" ) {
      uevent_file_type = BZ2;
    } else if ( uevent_file_name.substr( uevent_file_name.size() - 3, string::npos ) == ".gz" ) {
      uevent_file_type = GZ;
    }
    if ( count_file_name.substr( count_file_name.size() - 4, string::npos ) == ".bz2" ) {
      count_file_type = BZ2;
    } else if ( count_file_name.substr( count_file_name.size() - 3, string::npos ) == ".gz" ) {
      count_file_type = GZ;
    }
    if ( model_file_name.substr( model_file_name.size() - 4, string::npos ) == ".bz2" ) {
      model_file_type = BZ2;
    } else if ( model_file_name.substr( model_file_name.size() - 3, string::npos ) == ".gz" ) {
      model_file_type = GZ;
    }
    if ( event_file_name.substr( event_file_name.size() - 4, string::npos ) == ".bz2" ) {
      event_file_type = BZ2;
    } else if ( event_file_name.substr( event_file_name.size() - 3, string::npos ) == ".gz" ) {
      event_file_type = GZ;
    }

    // Startup message
    *error_stream << "Input event file: " << uevent_file_name << " (File type: " << file_type_name[ uevent_file_type ] << ')' << endl;
    *error_stream << "Model name: " << model_name << endl;
    *error_stream << "Output count file: " << count_file_name << " (File type: " << file_type_name[ count_file_type ] << ')' << endl;
    *error_stream << "Output model file: " << model_file_name << " (File type: " << file_type_name[ model_file_type ] << ')' << endl;
    *error_stream << "Output event file: " << event_file_name << " (File type: " << file_type_name[ event_file_type ] << ')' << endl;
    *error_stream << "Feature count threshold = " << threshold << endl;
    *error_stream << "Event number limit = " << limit_events << endl;

    AmisModel amis_model;

    // Initialize feature masks
    if (flat_mask_file) {
      if ( ! set_feature_masks_from_flat_file( amis_model, mask_module_name, model_name ) ) {
        return 1;
      }
    }
    else {
      if ( ! set_feature_masks( mach, amis_model, mask_module_name, model_name ) ) {
        return 1;
      }
    }

    // Make count file
    if ( ! make_count_file( amis_model, uevent_file_name, count_file_name ) ) {
      return 1;
    }

    // Make model & event files
    if ( ! make_amis_files( amis_model, uevent_file_name, count_file_name, model_file_name, event_file_name ) ) {
      return 1;
    }

    // Start interactive mode
//     lilfes::lexer_t lexer(&lilfes::lilfes_in, "stdin");
//     mach.parse(lexer);

    // Termination process
    Terminator::PerformAll();
    return 0;
  } catch ( bad_alloc ) {
    cerr << "Memory allocation failed, LiLFeS terminated." << endl;
    throw;
  } catch (exception &e) {
    cerr << "Fatal error: " << e.what() << endl;
    throw;
  }
}

