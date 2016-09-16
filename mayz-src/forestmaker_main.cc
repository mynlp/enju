/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file forestmaker_main.cc
 *  @version Time-stamp: <2008-07-14 14:51:45 yusuke>
 *  Main program of forestmaker
 *
 **********************************************************************/

#include "ForestMaker.h"
#include "Derivbank.h"
#include "LexAnalyzerLilfes.h"
#include "SupertaggerExternal.h"
#include <liblilfes/builtin.h>
#include <string>
#include <set>
#include <memory>

using namespace std;
using namespace lilfes;
using namespace up;
using namespace mayz;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

enum file_type {
  _RAW, BZ2, GZ
};

static int DEFAULT_NUM_TEMPLATE_THRESHOLD = 20;
static double DEFAULT_FOM_TEMPLATE_THRESHOLD = 5.5;

static string model_name;
static string grammar_name;
static string derivbank_name;
static string uevent_file_name;
static string reference_file_name;
static string super_lexicon_name;
static string super_model_name;
static string super_param_name;
static string super_command;
static int limit_sentences = 0;
static int verbose_level = 0;

static set< int > target_sentences;

static int num_template_threshold = DEFAULT_NUM_TEMPLATE_THRESHOLD;
static double fom_templates_threshold = DEFAULT_FOM_TEMPLATE_THRESHOLD;

//////////////////////////////////////////////////////////////////////

void help_message() {
  *error_stream << "Usage: forestmaker [options] model_name grammar_module derivbank_file uevent_file [ super_lexicon super_model super_param | super_command ]" << endl;
  *error_stream << "  model_name: name of the model to be made" << endl;
  *error_stream << "  grammar_module: LiLFeS module in which the grammar and event extraction predicates are defined" << endl;
  *error_stream << "  derivbank_file: derivation database file" << endl;
  *error_stream << "  uevent_file: output file" << endl;
  *error_stream << "  super_lexicon: lexicon file" << endl;
  *error_stream << "  super_model:   supertagger model definition file" << endl;
  *error_stream << "  super_param:   supertagger parameter file" << endl;
  *error_stream << "  super_command: external supertagger command-line" << std::endl;
  *error_stream << "Options:" << endl;
  *error_stream << "  -tf value: template thresholding cut-off FOM value (default: " << DEFAULT_FOM_TEMPLATE_THRESHOLD << ")" << std::endl;
  *error_stream << "  -tn value: template thresholding cut-off number (default: " << DEFAULT_NUM_TEMPLATE_THRESHOLD << ")" << std::endl;
  *error_stream << "  -r filename:\treference distribution file" << endl;
  *error_stream << "  -n num:\tlimit number of sentences (default: " << limit_sentences << ")" << endl;
  *error_stream << "  -v:\t\tprint verbose messages" << endl;
  *error_stream << "  -vv:\t\tprint many verbose messages" << endl;
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
      } else {
        // option with argument
        if ( argv + 1 == end ) {
          *error_stream << *argv << " option requires additional argument" << endl;
          return false;
        }
        if ( std::strcmp( (*argv), "-r" ) == 0 ) {
          reference_file_name = *(++argv);
        } else if ( std::strcmp( (*argv), "-n" ) == 0 ) {
          limit_sentences = strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "--target-sentence" ) == 0 ) {
          target_sentences.insert( strtol( *(++argv), NULL, 0 ) );
        } else if ( std::strcmp( (*argv), "-tf" ) == 0 ) {
          fom_templates_threshold = std::atof( *(++argv) );
        } else if ( std::strcmp( (*argv), "-tn" ) == 0 ) {
          num_template_threshold = strtol( *(++argv), NULL, 0 );
        } else {
          *error_stream << "Unknown option: " << *argv << endl;
          return false;
        }
      }
    } else {
      // arguments
      if ( end - argv != 7 && end - argv != 5 ) {
        *error_stream << "forestmaker requires seven or five arguments" << endl;
        return false;
      }
      model_name = argv[ 0 ];
      grammar_name = argv[ 1 ];
      derivbank_name = argv[ 2 ];
      uevent_file_name = argv[ 3 ];
      if ( end - argv == 7 ) {
        super_lexicon_name = argv[ 4 ];
        super_model_name = argv[ 5 ];
        super_param_name = argv[ 6 ];
      }
      else {
        super_command = argv[ 4 ];
      }
      return true;
    }
  }
  *error_stream << "forestmaker requires seven or five arguments" << endl;
  return false;
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {

    *error_stream << "forestmaker (MAYZ " << MAYZ_VERSION << ")" << endl;
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
        file_type uevent_file_type = _RAW;
        if ( uevent_file_name.substr( uevent_file_name.size() - 4, std::string::npos ) == ".bz2" ) {
            uevent_file_type = BZ2;
        } else if ( uevent_file_name.substr( uevent_file_name.size() - 3, std::string::npos ) == ".gz" ) {
            uevent_file_type = GZ;
        }

            // Load grammar module
        *error_stream << "Loading grammar module \"" << grammar_name << "\"... ";
        if ( ! load_module( mach, "grammar module", grammar_name ) ) {
            *error_stream << "grammar module not found" << endl;
            return 1;
        }
        if ( ! load_module( mach, "parser module", "mayz/up" ) ) {
            *error_stream << "parser module \"mayz/up\" not found" << endl;
            return 1;
        }
        *error_stream << "done." << endl;

            // Load derivbank
        *error_stream << "Loading derivbank \"" << derivbank_name << "\"... ";
        Derivbank derivbank( &mach );
        if ( ! derivbank.load( derivbank_name ) ) {
            *error_stream << "derivbank not found" << endl;
            return 1;
        }
        *error_stream << "done." << endl;

		// Initialize grammar
		*error_stream << "Initializing grammar object ...";
		up::Grammar grammar;
		if (! grammar.init( &mach )) {
        	*error_stream << "initialization of a grammar failed." << std::endl;
        	return 1;
		}
        grammar.enableQuickCheck();
        up::Builtin::setGrammar(&grammar);
		*error_stream << "done." << std::endl;

		// Initialize supertagger
		*error_stream << "Initializing supertagger ...";
        up::Supertagger *super = 0;
        if (super_command.empty()) {
		    std::auto_ptr< std::istream > super_lexicon( up::openInputFile( super_lexicon_name ) );
		    std::auto_ptr< std::istream > super_model( up::openInputFile( super_model_name ) );
		    std::auto_ptr< std::istream > super_param( up::openInputFile( super_param_name ) );
		    super = new up::SupertaggerViterbiMorph(*super_lexicon, *super_param, *super_model);
        }
        else {
            std::istringstream iss_command_line(super_command);
            std::vector<std::string> args;
            std::copy(std::istream_iterator<std::string>(iss_command_line),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(args));
            super = new up::SupertaggerExternal(args);
        }
		*error_stream << "done." << std::endl;

        // dummy: should not be used
        up::Builtin::setLexAnalyzer(new up::LexAnalyzerLilfes(&mach, &grammar));

            // Initialize parser
//    *error_stream << "Initializing forest maker...\n";
//    FMGrammar fm_grammar;
//    if ( ! fm_grammar.init( &mach ) ) {
//      *error_stream << "initialization of a grammar failed." << endl;
//      return 1;
//    }
        ForestMaker forest_maker( model_name, num_template_threshold, fom_templates_threshold );
//     forest_maker.setBeamThresholdNum( 0 );
//     forest_maker.setBeamThresholdWidth( 0.0 );
        if( ! forest_maker.init( &mach, &grammar, super ) ) {
            *error_stream << "failed." << endl;
            return 1;
        }
        *error_stream << "done." << endl;
        *error_stream << "Grammar: " << grammar.getName() << " - " << grammar.getVersion() << endl;
        *error_stream << "Limit of sentence length: " << forest_maker.getLimitSentenceLength() << endl;
        *error_stream << "Limit of edge number: " << forest_maker.getLimitEdgeNumber() << endl;
        *error_stream << "Cutoff value of template number: " << num_template_threshold << endl;
        *error_stream << "Cutoff value of template FOM: " << fom_templates_threshold << endl;

            // Start lilfes command
        *error_stream << "Start making feature forests" << endl;
        {
            std::auto_ptr< std::ostream > unfiltered_event_file( NULL );
            if ( uevent_file_type == _RAW ) {
                unfiltered_event_file.reset( new std::ofstream( uevent_file_name.c_str() ) );
            } else if ( uevent_file_type == BZ2 ) {
                unfiltered_event_file.reset( new obfstream( uevent_file_name.c_str() ) );
            } else if ( uevent_file_type == GZ ) {
                unfiltered_event_file.reset( new ogfstream( uevent_file_name.c_str() ) );
            } else {
                *error_stream << "Unknown compression type: " << uevent_file_name << endl;
                return 1;
            }
            if ( ! (*unfiltered_event_file) ) {
                *error_stream << "Cannot open unfiltered event file: " << uevent_file_name << endl;
                return 1;
            }
            std::auto_ptr< std::ostream > reference_file( NULL );
            if ( ! reference_file_name.empty() ) {
                reference_file.reset( new std::ofstream( reference_file_name.c_str() ) );
            }

                /// iterate derivation bank
            for ( Derivbank::iterator deriv_it = derivbank.begin(); deriv_it != derivbank.end(); ++deriv_it ) {
                lilfes::IPTrailStack iptrail(&mach);
          
                int deriv_id = deriv_it->first;
                if ( limit_sentences > 0 && deriv_id > limit_sentences ) break;
                if ( ! target_sentences.empty() && target_sentences.find( deriv_id ) == target_sentences.end() ) continue;
                Derivation derivation = deriv_it->second;

                *error_stream << "No. " << deriv_id << ": ";
          
				up::WordLattice word_vec;
                std::vector<ForestMaker::lex_template_type> lex_template_vec;
                if ( ! forest_maker.makeLeaves( derivation, word_vec, lex_template_vec) ) {
                    *error_stream << "derivation_to_lexical_entry_list/2 failed" << endl;
                    continue;
                }
                int n = forest_maker.analyzeSentenceLength(word_vec);
                if(! forest_maker.checkSentenceLength(n, forest_maker.getLimitSentenceLength())) {
                    *error_stream << "too long sentence (" << n << " words)" << endl;
                    continue;
                }
                if(! forest_maker.checkWordPosition( word_vec ) ) {
                    *error_stream << "word positions are not well ordered in derivation" << endl;
                    continue;
                }
        
                ParseTree parse_tree( mach );
                if ( ! forest_maker.derivationToParseTree( derivation, parse_tree ) ) {
                    *error_stream << "derivation_to_parse_tree/2 failed" << endl;
                    continue;
                }
          
                if ( ! forest_maker.fullParse(word_vec, lex_template_vec ) ) {
                    *error_stream << "parse error: ";
                    switch( forest_maker.getParseStatus() ) {
                        case up::NOT_PARSED_YET:
                            *error_stream << "no parsed yet" << endl;
                            break;
                        case up::SUCCESS:
                            *error_stream << "no successful parse" << endl;
                            break;
                        case up::WORD_ANALYSIS_ERROR:
                            *error_stream << "word analysis error" << endl;
                            break;
                        case up::LEXENT_ANALYSIS_ERROR:
                            *error_stream << "lexical entry analysis error" << endl;
                            break;
                        case up::SETUP_ERROR:
                            *error_stream << "setup error" << endl;
                            break;
                        case up::TOO_LONG:
                            *error_stream << "too long sentence (" << forest_maker.getSentenceLength() << " words)" << endl;
                            break;
                        case up::SEARCH_LIMIT:
                            *error_stream << "limit of search exceeded" << endl;
                            break;
                        case up::EDGE_LIMIT:
                            *error_stream << "edge limit exceeded" << endl;
                            break;
                        case up::TIME_LIMIT:
                            *error_stream << "time limit exceeded" << endl;
                            break;
                        default:
                            *error_stream << "unknown reason" << endl;
                    }
                    continue;
                }

//                if( deriv_id == 33 ) forest_maker.showChart();

				std::vector<up::eserial> root_edges;
                forest_maker.getEdges( 0, forest_maker.getSentenceLength(), root_edges);
                if ( root_edges.empty() ) {
                    *error_stream << "no successful parse (no root edges)" << endl;
                    //forest_maker.dump(std::cerr);
                    continue;
                }

                    /// output event
                *unfiltered_event_file << "event_" << deriv_id << '\n';
                if ( reference_file.get() != NULL ) {
                        // with reference distribution
                    *reference_file << "event_" << deriv_id << '\n';
                    if ( ! forest_maker.outputParseTree( parse_tree, *unfiltered_event_file, *reference_file ) ) {
                        *error_stream << "output parse tree failed" << endl;
                        return 1;
                    }
                    if ( ! forest_maker.outputForest( *unfiltered_event_file, *reference_file ) ) {
                        *error_stream << "output feature forest failed" << endl;
                        return 1;
                    }
                    *reference_file << '\n';
                } else {
                        // without reference distribution
                    if ( ! forest_maker.outputParseTree( parse_tree, *unfiltered_event_file ) ) {
                        *error_stream << "output parse tree failed" << endl;
                        return 1;
                    }
                    if ( ! forest_maker.outputForest( *unfiltered_event_file ) ) {
                        *error_stream << "output feature forest failed" << endl;
                        return 1;
                    }
                }
                *unfiltered_event_file << '\n';
                *error_stream << "success" << endl;
            }
        }
        *error_stream << "done." << endl;

            // Termination process
        if( ! forest_maker.term() ) {
            *error_stream << "parser failed to terminate" << endl;
            return 1;
        }
        Terminator::PerformAll();

        return 0;
    } 
    catch (bad_alloc) {
        cerr << "Memory allocation failed, forestmaker terminated." << endl;
        throw;
    }
    catch (exception &e) {
        cerr << "Fatal error: " << e.what() << endl;
        throw;
    }
}

