/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file lexrefine_main.cc
 *  @version Time-stamp: <2009-12-19 11:45:39 yusuke>
 *  Main program of lexrefine
 *
 **********************************************************************/

#include "LexiconTable.h"
#include "TemplateTable.h"
#include "WordCounter.h"
#include "Initializer.h"
#include "liblilfes/builtin.h"
#include <string>
#include <iostream>
#include <set>

using namespace std;
using namespace mayz;
using namespace lilfes;
using namespace up;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

static int word_frequency_threshold = 1;
static int template_frequency_threshold = 0;
static int unknown_word_frequency_threshold = 1;
static int unknown_word_template_frequency_threshold = 0;
//static int expand_template_threshold = 0;
static int verbose_level = 0;

static string rule_file_name;
static string orig_lexicon_file;
static string orig_template_file;
static string new_lexicon_file;
static string new_template_file;

procedure* unknown_word_key = NULL;
procedure* expand_lexical_template = NULL;
procedure* expand_lexicon = NULL;

//////////////////////////////////////////////////////////////////////

void help_message() {
  *error_stream << "Usage: lexrefine [options] lexrule_module orig_lexicon_file orig_template_file new_lexicon_file new_template_file" << endl;
  *error_stream << "  lexrule_module: LiLFeS module in which lexical rules are defined" << endl;
  *error_stream << "  orig_lexicon_file: input lexicon database" << endl;
  *error_stream << "  orig_template_file: input template database" << endl;
  *error_stream << "  new_lexicon_file: output lexicon database" << endl;
  *error_stream << "  new_template_file: output template database" << endl;
  *error_stream << "Options:" << endl;
  *error_stream << "  -wf threshold:\tword frequency threshold (default: " << word_frequency_threshold << ")" << endl;
  *error_stream << "  -tf threshold:\ttemplate frequency threshold (default: " << template_frequency_threshold << ")" << endl;
  *error_stream << "  -uwf threshold:\tfrequency threshold of unknown words (default: " << unknown_word_frequency_threshold << ")" << endl;
  *error_stream << "  -utf threshold:\tfrequency threshold of unknown word templates (default: " << unknown_word_template_frequency_threshold << ")" << endl;
  //  *error_stream << "  -lf\tfrequency threshold of lexemes to be expanded (default: " << expand_template_threshold << ")" << endl;
  *error_stream << "  -v:\tprint verbose messages" << endl;
  *error_stream << "  -vv:\tprint many verbose messages" << endl;
  *error_stream << "  -vvv:\tprint many many verbose messages" << endl;
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
      } else if ( std::strcmp( (*argv), "-vvv" ) == 0 ) {
        verbose_level = 3;
      } else {
        // option with argument
        if ( argv + 1 == end ) {
          *error_stream << *argv << " option requires additional argument" << endl;
          return false;
        }
        if ( std::strcmp( (*argv), "-wf" ) == 0 ) {
          word_frequency_threshold = strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-tf" ) == 0 ) {
          template_frequency_threshold = strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-uwf" ) == 0 ) {
          unknown_word_frequency_threshold = strtol( *(++argv), NULL, 0 );
        } else if ( std::strcmp( (*argv), "-utf" ) == 0 ) {
          unknown_word_template_frequency_threshold = strtol( *(++argv), NULL, 0 );
//          } else if ( std::strcmp( (*argv), "-lf" ) == 0 ) {
//            expand_template_threshold = strtol( *(++argv), NULL, 0 );
        } else {
          *error_stream << "Unknown option: " << *argv << endl;
          return false;
        }
      }
    } else {
      // arguments
      if ( end - argv != 5 ) {
        *error_stream << "lexrefine requires five arguments" << endl;
        return false;
      }
      rule_file_name = argv[ 0 ];
      orig_lexicon_file = argv[ 1 ];
      orig_template_file = argv[ 2 ];
      new_lexicon_file = argv[ 3 ];
      new_template_file = argv[ 4 ];
      return true;
    }
  }
  *error_stream << "lexrefine requires five arguments" << endl;
  return false;
}

//////////////////////////////////////////////////////////////////////

bool apply_unknown_word_key( machine* mach, const std::string& key, std::string& unknown_key ) {
  std::vector< FSP > args( 2 );
  args[ 0 ] = FSP( mach, key.c_str() );
  args[ 1 ] = FSP( mach );  // lexicon key of unknown word
  core_p cutp = mach->GetCutPoint();
  if ( ! call_proc( *mach, unknown_word_key, args ) ) return false;
  mach->DoCut( cutp );
  if ( ! lilfes_to_c< string >().convert( *mach, args[ 1 ], unknown_key ) ) {
    *error_stream << "2nd argument of unknown_word_key/2 must be string" << endl;
    return false;
  }
  return true;
}

bool apply_expand_lexicon( machine* mach, const std::string& orig_key, vector< string >& new_temp_list, LexiconTable* expanded_lexicon, set< string >& new_key_list ) {
  if ( expand_lexicon ) {
    new_key_list.insert( orig_key );
    for ( vector< string >::iterator new_temp_it = new_temp_list.begin();
          new_temp_it != new_temp_list.end();
          ++new_temp_it ) {
      if ( verbose_level > 1 ) {
        *error_stream << "expanding for: " << *new_temp_it << endl;
      }
      vector< FSP > args( 3 );
      args[ 0 ] = FSP( mach, orig_key.c_str() );
      args[ 1 ] = FSP( mach, new_temp_it->c_str() );
      args[ 2 ] = FSP( mach );    // new key
      vector< bool > flags( 3, false );
      flags[ 2 ] = true;
      list< vector< FSP > > results;
      findall_proc( *mach, expand_lexicon, args, flags, results );
      for ( list< vector< FSP > >::iterator result_it = results.begin();
            result_it != results.end();
            ++result_it ) {
        // add new template to new key
        string new_key;
        if ( ! lilfes_to_c< string >().convert( *mach, (*result_it)[ 0 ], new_key ) ) {
          *error_stream << "3rd argument of expand_lexicon/3 must be string" << endl;
          return false;
        }
        expanded_lexicon->add( new_key, *new_temp_it );
        new_key_list.insert( new_key );
//         vector< string > temp_list;
//         expanded_lexicon->lookup( new_key, temp_list );
//         bool already_exist = false;
//         for ( vector< string >::iterator temp_it = temp_list.begin();
//               temp_it != temp_list.end();
//               ++temp_it ) {
//           if ( *new_temp_it == *temp_it ) {
//             already_exist = true;
//             break;
//           }
//         }
//         if ( ! already_exist ) {
//           temp_list.push_back( *new_temp_it );
//           expanded_lexicon->insert( new_key, temp_list );
//           if ( verbose_level > 1 ) {
//             *error_stream << "expanded word: " << new_key << endl;
//           }
//         }
//         if ( new_key_list.insert( new_key ).second ) {
//           int new_key_count = 0;
//           lex_count_db->getCount( new_key, new_key_count );
//           new_key_count += count;
// //           if ( ! lex_count_db->insert( (*result_it)[ 0 ], new_key_count ) ) {
// //             *error_stream << "database access failed: new key count" << endl;
// //             return false;
// //           }
//           lex_count_db->incCount( new_key, new_key_count );
//           if ( verbose_level > 1 ) {
//             *error_stream << "update count: " << new_key_count << endl;
//           }
//         }
      }
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

int main( int argc, char** argv ) {

  *error_stream << "lexrefine (MAYZ " << MAYZ_VERSION << ")" << endl;
  *error_stream << MAYZ_COPYRIGHT << endl;

  lilfes::Initializer::PerformAll();

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

  machine mach;

  try {
    // Load rule file
    *error_stream << "Loading rule file \"" << rule_file_name << "\"... ";
    if ( ! load_module( mach, "lexicon refine module", rule_file_name ) ) {
      *error_stream << "\nRule file \"" << rule_file_name << "\" not found" << endl;
      return 1;
    }
    *error_stream << "done." << endl;

    // Initialization
    *error_stream << "Initializing... ";
    if ( ! mayz::Initializer::initialize( &mach ) ) {
      *error_stream << "initialization failed" << endl;
      return 1;
    }
    unknown_word_key = prepare_proc( module::UserModule(), "unknown_word_key", 2 );
    if ( ! unknown_word_key ) {
      *error_stream << "'unknown_word_key/2' not found" << endl;
      return 1;
    }
    expand_lexical_template = prepare_proc( module::UserModule(), "expand_lexical_template", 5 );
    expand_lexicon = prepare_proc( module::UserModule(), "expand_lexicon", 3 );
    *error_stream << "done." << endl;
  } catch ( exception& e ) {
    *error_stream << "Initialization failed: " << e.what() << endl;
    return 1;
  }

  try {

    // Load original lexicon
    *error_stream << "Loading original lexicon and templates... ";
    LexiconTable orig_lexicon;
    TemplateTable orig_temptable( &mach );
    LexiconTable new_lexicon;
    TemplateTable new_temptable( &mach );
    WordCounter lex_count_db;
    if ( ! orig_lexicon.load( orig_lexicon_file ) ) {
      *error_stream << "Loading lexicon failed" << endl;
      return 1;
    }
    if ( ! orig_temptable.load( orig_template_file, orig_template_file + ".count" ) ) {
      *error_stream << "Loading template database failed" << endl;
      return 1;
    }
    if ( ! lex_count_db.load( orig_lexicon_file + ".count" ) ) {
      *error_stream << "Loading lexicon count database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;

    //////////////////////////////////////////////////////////////////////
    ////  Stage 1: Refine template table
    //////////////////////////////////////////////////////////////////////

    *error_stream << "Refine template table..." << endl;
    int num_processed_templates = 0;
    int num_original_templates = 0;
    int num_expanded_templates = 0;
    FSArray< string, vector< string > > template_mapping( &mach );
    for ( TemplateTable::iterator temp_it = orig_temptable.begin();
          temp_it != orig_temptable.end();
          ++temp_it ) {
      core_p trailp = mach.SetTrailPoint( NULL );
      ++num_processed_templates;
      int count = 0;
      if ( ! orig_temptable.getCount( temp_it->first, count ) ) {
        *error_stream << "Frequency of a template not found" << endl;
        return 1;
      }
      if ( verbose_level > 1 ) {
        *error_stream << "processing template: count=" << count << ", " << temp_it->first << endl;
      }
      if ( count > template_frequency_threshold ) {
        // more than the threshold
        //   -> register to the new table
        ++num_original_templates;
        if ( ! new_temptable.insert( temp_it->first, temp_it->second ) ) {
          *error_stream << "database access error: insert in refining template table" << endl;
          return 1;
        }
        new_temptable.incCount( temp_it->first, count );
        if ( verbose_level > 1 ) *error_stream << "registered to the template table" << endl;
        // expansion by lexical rules
        if ( expand_lexical_template ) {
          //LexTemplate orig_temp_name( temp_it->first );
          vector< FSP > args( 5 );
          args[ 0 ] = FSP( mach, temp_it->first.c_str() );
          args[ 1 ] = temp_it->second;
          args[ 2 ] = FSP( mach, static_cast< mint >( count ) );
          args[ 3 ] = FSP( mach );    // new template name
          args[ 4 ] = FSP( mach );    // new template
          vector< bool > flags( 5, false );
          flags[ 3 ] = true;
          flags[ 4 ] = true;
          list< vector< FSP > > results;
          vector< string > new_templates;
          findall_proc( mach, expand_lexical_template, args, flags, results );
          for ( list< vector< FSP > >::iterator result_it = results.begin();
                result_it != results.end();
                ++result_it ) {
            ++num_expanded_templates;
            FSP new_template( mach );
            if ( ! builtin::canonical_copy( mach, (*result_it)[ 1 ], new_template ) ) {
              *error_stream << "canonical_copy/2 failed.  maybe lilfes's bug" << endl;
              return 1;
            }
            std::string new_temp_name;
            if ( ! lilfes_to_c< string >().convert( mach, (*result_it)[ 0 ], new_temp_name ) ) {
              *error_stream << "4th argument of expand_lexical_template/5 must be string" << endl;
              return 1;
            }
            if ( ! new_temptable.insert( new_temp_name, new_template ) ) {
              *error_stream << "database access error: insert in template expansion" << endl;
              return 1;
            }
            new_temptable.incCount( new_temp_name, count );
            if ( verbose_level > 1 ) {
              *error_stream << "expanded to:" << new_temp_name << endl;
            }
            if ( verbose_level > 2 ) {
              *error_stream << new_template.DisplayAVM() << flush;
            }
            new_templates.push_back( new_temp_name );
          }
          if ( ! template_mapping.insert( temp_it->first, new_templates ) ) {
            *error_stream << "database access error: insert in updating temlate mapping" << endl;
            return 1;
          }
        } // if ( expand_lexical_template )
      } else {
        // less than the threshold
        //   -> removed from the table
        if ( verbose_level > 1 ) *error_stream << "removed from the template table" << endl;
      }
      mach.TrailBack( trailp );
      // profiling message
      if ( verbose_level > 0 || num_processed_templates % 100 == 0 ) {
        *error_stream << num_processed_templates << " templates processed" << endl;
        *error_stream << "  Number of original templates = " << num_original_templates << endl;
        *error_stream << "  Number of expanded templates = " << num_expanded_templates << endl;
      }
    }
    *error_stream << "done." << endl;
    *error_stream << num_processed_templates << " templates processed" << endl;
    *error_stream << "  Number of original templates = " << num_original_templates << endl;
    *error_stream << "  Number of expanded templates = " << num_expanded_templates << endl;

    //////////////////////////////////////////////////////////////////////
    //// Stage 2: Expand lexicon
    //////////////////////////////////////////////////////////////////////

    *error_stream << "Expand lexicon..." << endl;
    int num_processed_words = 0;
    LexiconTable expanded_lexicon;
    for ( LexiconTable::iterator word_it = orig_lexicon.begin();
          word_it != orig_lexicon.end();
          ++word_it ) {
      core_p trailp = mach.SetTrailPoint( NULL );
      ++num_processed_words;
      // get word count
      int count = 0;
      //*error_stream << "a2" << endl;
      if ( ! lex_count_db.getCount( word_it->first, count ) ) {
        *error_stream << "Frequency of words not found: " << word_it->first << endl;
        return 1;
      }
      if ( verbose_level > 1 ) {
        *error_stream << "processing word: word=" << word_it->first << ", count=" << count << endl;
      }
      // apply expand_lexicon
      set< string > new_key_list;
      for ( vector< string >::iterator temp_it = word_it->second.begin();
            temp_it != word_it->second.end();
            ++temp_it ) {
        FSP fs( mach );
        if ( new_temptable.find( *temp_it, fs ) ) {
          // valid template
          if ( verbose_level > 1 ) {
            *error_stream << "expanding template: " << *temp_it << endl;
          }
          expanded_lexicon.add( word_it->first, *temp_it );
          vector< string > new_temp_list;
          if ( template_mapping.find( *temp_it, new_temp_list ) ) {
            // expansion
            if ( ! apply_expand_lexicon( &mach, word_it->first, new_temp_list, &expanded_lexicon, new_key_list ) ) {
              *error_stream << "expand_lexicon failed" << endl;
              return 1;
            }
          }
        } else {
          if ( verbose_level > 1 ) {
            *error_stream << "removed template: " << *temp_it << endl;
          }
        }
      }
      // update count
      for ( set< string >::const_iterator new_key_it = new_key_list.begin();
            new_key_it != new_key_list.end();
            ++new_key_it ) {
        if ( *new_key_it != word_it->first ) {
          lex_count_db.incCount( *new_key_it, count );
          if ( verbose_level > 1 ) {
            *error_stream << "count " << count << " added to " << *new_key_it << endl;
          }
        }
      }
      mach.TrailBack( trailp );
      if ( verbose_level > 0 || num_processed_words % 1000 == 0 ) {
        *error_stream << num_processed_words << " words processed" << endl;
      }
    }
    *error_stream << "done." << endl;
    *error_stream << num_processed_words << " words processed" << endl;

    //////////////////////////////////////////////////////////////////////
    //// Stage 3: Refine lexicon
    //////////////////////////////////////////////////////////////////////

    *error_stream << "Refine lexicon..." << endl;
    num_processed_words = 0;
    int num_original_words = 0;
    for ( LexiconTable::iterator word_it = expanded_lexicon.begin();
          word_it != expanded_lexicon.end();
          ++word_it ) {
      core_p trailp = mach.SetTrailPoint( NULL );
      ++num_processed_words;
      int count = 0;
      //*error_stream << "a2" << endl;
      if ( ! lex_count_db.getCount( word_it->first, count ) ) {
        //*error_stream << "Frequency of words not found" << endl;
        //return 1;
        // maybe unknown word entry
        continue;
      }
      if ( verbose_level > 1 ) {
        *error_stream << "processing word: word=" << word_it->first << ", count=" << count << endl;
      }
      if ( count <= unknown_word_frequency_threshold ) {
        // register as an unknown word
        string unknown_word;
        if ( ! apply_unknown_word_key( &mach, word_it->first, unknown_word ) ) {
          *error_stream << "'unknown_word_key/2' failed for " << word_it->first << endl;
          return 1;
        }
        vector< string > temp_id_list;
        expanded_lexicon.lookup( unknown_word, temp_id_list );
        if ( verbose_level > 1 ) {
          *error_stream << "registered as an unknown word: " << unknown_word << endl;
        }
        //*error_stream << "a9" << endl;
        bool dirty = false;
        for ( vector< string >::iterator temp_it = word_it->second.begin();
              temp_it != word_it->second.end();
              ++temp_it ) {
          int count = 0;
          if ( ! new_temptable.getCount( *temp_it, count ) ) {
            *error_stream << "Frequency of a template not found" << endl;
            return 1;
          }
          if ( count > unknown_word_template_frequency_threshold ) {
            bool already_exist = false;
            for ( vector< string >::iterator temp_it2 = temp_id_list.begin();
                  temp_it2 != temp_id_list.end();
                  ++temp_it2 ) {
              if ( *temp_it == *temp_it2 ) {
                already_exist = true;
                break;
              }
            }
            if ( ! already_exist ) {
              // new template for unknown word
              if ( verbose_level > 1 ) {
                *error_stream << "template added: " << *temp_it << endl;
              }
              temp_id_list.push_back( *temp_it );
              dirty = true;
            } else {
              if ( verbose_level > 1 ) {
                *error_stream << "template already exists: " << *temp_it << endl;
              }
            }
          } else {
            if ( verbose_level > 1 ) {
              *error_stream << "template ignored because freq <= utf: " << *temp_it << endl;
            }
          }
        }
        if ( dirty ) {
          expanded_lexicon.insert( unknown_word, temp_id_list );
        }
        //*error_stream << "a11" << endl;
        //*error_stream << unknown_word.DisplayAVM();
        //*error_stream << "a12" << endl;
      }
      if ( count > word_frequency_threshold ) {
        // frequent word
        ++num_original_words;
        vector< string > temp_id_list;
        //*error_stream << "a4" << endl;
        if ( verbose_level > 1 ) *error_stream << "registered to the lexicon" << endl;
        for ( vector< string >::iterator temp_it = word_it->second.begin();
              temp_it != word_it->second.end();
              ++temp_it ) {
          FSP fs( mach );
          if ( new_temptable.find( *temp_it, fs ) ) {
            // valid template
            if ( verbose_level > 1 ) {
              *error_stream << "template added: " << *temp_it << endl;
            }
            temp_id_list.push_back( *temp_it );
          }
        }
        //*error_stream << "a5" << endl;
        expanded_lexicon.insert( word_it->first, temp_id_list );
        //*error_stream << "a6" << endl;
      } else {
        if ( verbose_level > 1 ) *error_stream << "removed from the lexicon" << endl;
        vector< string > dummy_temp_list;
        expanded_lexicon.insert( word_it->first, dummy_temp_list );
      }
      mach.TrailBack( trailp );
      //*error_stream << "a13" << endl;
      if ( verbose_level > 0 || num_processed_words % 1000 == 0 ) {
        *error_stream << num_processed_words << " words processed" << endl;
        *error_stream << "  Number of original words = " << num_original_words << endl;
      }
    }
    *error_stream << "done." << endl;
    *error_stream << num_processed_words << " words processed" << endl;
    *error_stream << "  Number of original words = " << num_original_words << endl;

    //////////////////////////////////////////////////////////////////////
    ////  Stage 4: Make new lexicon
    //////////////////////////////////////////////////////////////////////

    *error_stream << "Making new lexicon..." << endl;
    for ( LexiconTable::iterator word_it = expanded_lexicon.begin();
          word_it != expanded_lexicon.end();
          ++word_it ) {
      vector< string >& temp_list = word_it->second;
      if ( temp_list.empty() ) continue;  // deleted entry
      new_lexicon.insert( word_it->first, word_it->second );
    }
    *error_stream << "done." << endl;

    // Save new lexicon
    *error_stream << "Output refined lexicon... ";
    new_lexicon.save( new_lexicon_file );
    new_temptable.save( new_template_file );
    *error_stream << "done." << endl;

  } catch ( exception& e ) {
    cerr << "Fatal error: " << e.what() << endl;
    return 1;
  }

  Terminator::PerformAll();

  return 0;
}

