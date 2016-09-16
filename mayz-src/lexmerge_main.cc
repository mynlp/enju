/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file lexmerge_main.cc
 *  @version Time-stamp: <2009-12-23 10:09:04 yusuke>
 *  Main program of lexmerge
 *
 **********************************************************************/

#include "LexiconTable.h"
#include "TemplateTable.h"
#include "WordCounter.h"
#include "Initializer.h"
#include "Word.h"
#include <string>
#include <iostream>
#include <set>
#include <map>

using namespace std;
using namespace mayz;
using namespace lilfes;
using namespace up;

void* dummy = BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

static int verbose_level = 0;

static string rule_file_name;
static string lexicon_file_1;
static string template_file_1;
static string lexicon_file_2;
static string template_file_2;
static string new_lexicon_file;
static string new_template_file;

//////////////////////////////////////////////////////////////////////

void help_message() {
  *error_stream << "Usage: lexmerge lexrefine lexicon_file_1 template_file_1 lexicon_file_2 template_file_2 new_lexicon_file new_template_file" << endl;
  *error_stream << "  lexrule_module: LiLFeS module in which lexical rules are defined" << endl;
  *error_stream << "  lexicon_file_1: 1st input lexicon database" << endl;
  *error_stream << "  template_file_1: 1st input template database" << endl;
  *error_stream << "  lexicon_file_1: 2nd input lexicon database" << endl;
  *error_stream << "  template_file_1: 2nd input template database" << endl;
  *error_stream << "  new_lexicon_file: output lexicon database" << endl;
  *error_stream << "  new_template_file: output template database" << endl;
  *error_stream << "Options:" << endl;
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
        *error_stream << "Unknown option: " << *argv << endl;
        return false;
      }
    } else {
      // arguments
      if ( end - argv != 7 ) {
        *error_stream << "lexmerge requires seven arguments" << endl;
        return false;
      }
      rule_file_name = argv[ 0 ];
      lexicon_file_1 = argv[ 1 ];
      template_file_1 = argv[ 2 ];
      lexicon_file_2 = argv[ 3 ];
      template_file_2 = argv[ 4 ];
      new_lexicon_file = argv[ 5 ];
      new_template_file = argv[ 6 ];
      return true;
    }
  }
  *error_stream << "lexmerge requires seven arguments" << endl;
  return 0;
}


//////////////////////////////////////////////////////////////////////

int main( int argc, char** argv ) {

  *error_stream << "lexmerge (MAYZ " << MAYZ_VERSION << ")" << endl;
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

    *error_stream << "done." << endl;
  } catch ( exception& e ) {
    *error_stream << "Initialization failed: " << e.what() << endl;
    return 1;
  }

  try {

    // Load original lexicon files

    *error_stream << "Output lexicon database file: " << new_lexicon_file << endl;
    *error_stream << "Output template database file: " << new_template_file << endl;

    LexiconTable lexicon_1;
    TemplateTable temptable_1( &mach );
    LexiconTable lexicon_2;
    TemplateTable temptable_2( &mach );
    LexiconTable new_lexicon;
    TemplateTable new_temptable( &mach );
    map< string, string > translate_table;
    WordCounter lex_count_db_1;
    WordCounter lex_count_db_2;
    WordCounter new_lex_count_db;

    *error_stream << endl << "Loading original lexicon and templates... " << endl;
    *error_stream << "Loading 1st lexicon database file: " << lexicon_file_1 << " ...";
    if ( ! lexicon_1.load( lexicon_file_1 ) ) {
      *error_stream << "Loading 1st lexicon database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading 1st template database file: " << template_file_1 << " ...";
    if ( ! temptable_1.load( template_file_1, template_file_1 + ".count" ) ) {
      *error_stream << "Loading 1st template database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading 1st lexicon count database file: " << lexicon_file_1 << ".count ...";
    if ( ! lex_count_db_1.load( lexicon_file_1 + ".count" ) ) {
      *error_stream << "Loading 1st lexicon count database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading 2nd lexicon database file: " << lexicon_file_2 << " ...";
    if ( ! lexicon_2.load( lexicon_file_2 ) ) {
      *error_stream << "Loading 2nd lexicon database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading 2nd template database file: " << template_file_2 << " ...";
    if ( ! temptable_2.load( template_file_2, template_file_2 + ".count" ) ) {
      *error_stream << "Loading 2nd template database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;
    *error_stream << "Loading 2nd lexicon count database file: " << lexicon_file_2 << ".count ...";
    if ( ! lex_count_db_2.load( lexicon_file_2 + ".count" ) ) {
      *error_stream << "Loading 2nd lexicon count database failed" << endl;
      return 1;
    }
    *error_stream << "done." << endl;


    //////////////////////////////////////////////////////////////////////
    ////  Stage 1: Merge template table
    //////////////////////////////////////////////////////////////////////

    *error_stream << endl;
    *error_stream << "Merging template database..." << endl;
    *error_stream << "Checking 1st template database and adding the templates to new template database..." << endl;

    int tempCount1 = 0;
    int tempCount2 = 0;
    int tempInserted2 = 0;
    int tempNotInserted2 = 0;
    int num_processed_templates = 0;
    for ( TemplateTable::iterator temp_it_1 = temptable_1.begin();
              temp_it_1 != temptable_1.end();
              ++temp_it_1 ) {
      int count = 0;
      if ( ! temptable_1.getCount( temp_it_1->first, count ) ) {
        *error_stream << "Frequency of a template not found" << endl;
        return 1;
      }
      if ( verbose_level > 0 ) {
        *error_stream << temp_it_1->first << "  ---> Added to new template database" << endl;
      }
      new_temptable.insert( temp_it_1->first, temp_it_1->second );
      new_temptable.incCount( temp_it_1->first, count);
      tempCount1++;
      num_processed_templates++;

      if ( num_processed_templates % 100 == 0 ) {
        if ( verbose_level > 0 ) *error_stream << endl;
        *error_stream << num_processed_templates << " templates processed" << endl;
        if ( verbose_level > 0 ) *error_stream << endl;
      }
    }
    *error_stream << endl << num_processed_templates << " templates processed" << endl;
    *error_stream << "done." << endl << endl;

    *error_stream << "Checking 2nd template database and adding unseen templates to new template database..." << endl;
    for ( TemplateTable::iterator temp_it_2 = temptable_2.begin();
          temp_it_2 != temptable_2.end();
          ++temp_it_2 ) {

      tempCount2++;
      core_p trailp = mach.SetTrailPoint( NULL );
      bool already_exist = false;

      for ( TemplateTable::iterator temp_it_1 = temptable_1.begin();
            temp_it_1 != temptable_1.end();
            ++temp_it_1 ) {
        if ( builtin::equivalent( mach, temp_it_2->second, temp_it_1->second ) ) {
          //this template is already inserted as the same/different name.
          //anyway, insert pair of the template_names into translate_table
          already_exist = true;
          tempNotInserted2++;
          if ( verbose_level > 0 ) {
            *error_stream << temp_it_2->first << "  ---> Already in new template database, not added" << endl;
            //*error_stream << ((LexTemplate)temp_it_1->first).lexemeName().DisplayAVM() << flush;
          }
          translate_table[ temp_it_2->first ] = temp_it_1->first;

          int count = 0;
          if ( ! temptable_2.getCount( temp_it_2->first, count ) ) {
            *error_stream << "Frequency of a template not found" << endl;
            return 1;
          }
          new_temptable.incCount( temp_it_1->first, count);
          break;
        }
      }

      if ( ! already_exist ) {

        tempInserted2++;
        if ( verbose_level > 0 ) {
          *error_stream << temp_it_2->first << "  ---> Added to new template database" << endl;
        }

        //new template, just added
        //insert pair of the same template_names into translate table
        lilfes::FSP temp_template;
        if ( ! temptable_1.find( temp_it_2->first, temp_template ) ) {
          translate_table[ temp_it_2->first ] = temp_it_2->first;
          new_temptable.insert( temp_it_2->first, temp_it_2->second );
          int count = 0;
          if ( ! temptable_2.getCount( temp_it_2->first, count ) ) {
            *error_stream << "Frequency of a template not found" << endl;
            return 1;
          }
          new_temptable.incCount( temp_it_2->first, count);
        }
        else {
          *error_stream << "Template names conflict: " << temp_it_2->first << endl;
          //Names conflict
        }
      }
      mach.TrailBack( trailp );
      num_processed_templates++;

      if ( num_processed_templates % 100 == 0 ) {
        if ( verbose_level > 0 ) *error_stream << endl;
        *error_stream << num_processed_templates << " templates processed" << endl;
        if ( verbose_level > 0 ) *error_stream << endl;
      }
    }

    *error_stream << endl << num_processed_templates << " templates processed" << endl;
    *error_stream << "done." << endl << endl;

    *error_stream << "Merging templates database completed." << endl;
    *error_stream << tempCount2 + tempCount1 << " templates checked." << endl;
    *error_stream << tempInserted2 + tempCount1 << " templates added to new templates database." << endl;
    //*error_stream << tempNotInserted2 << " templates were already in new template database." << endl;

    //////////////////////////////////////////////////////////////////////
    ////  Stage 2: Merge lexicon
    //////////////////////////////////////////////////////////////////////

    *error_stream <<  endl;
    *error_stream << "Merging lexicon database..." << endl;

    *error_stream << "Checking lexical entries for words in 1st lexicon database..." << endl;
    int num_processed_words = 0;
    for ( LexiconTable::iterator word_it_1 = lexicon_1.begin();
          word_it_1 != lexicon_1.end();
          ++word_it_1 ) {

      num_processed_words++;

      if (verbose_level > 0) {
        *error_stream << word_it_1->first << endl;
      }
      ////*error_stream << "Word: " << word_it_1->first.DisplayAVM() << endl;

      core_p trailp = mach.SetTrailPoint( NULL );

      vector< string > temp_id_1_list;

      if (!lexicon_1.lookup( word_it_1->first, temp_id_1_list )) {
        *error_stream << "Error: One template at least must be fit for the word." << endl;
      }

      int count_1 = 0;
      if ( ! lex_count_db_1.getCount( word_it_1->first, count_1 ) ) {
        *error_stream << "Frequency of words not found" << endl;
        return 1;
        // maybe unknown word entry
        //continue;
      }

      vector< string > temp_id_2_list;
      vector< string > temp_id_new_list;

      if ( ! lexicon_2.lookup( word_it_1->first, temp_id_2_list ) ) {
        if (verbose_level > 0) {
          *error_stream << "  ---> Only in 1st lexicon, just added to new lexicon...";
        }
        new_lex_count_db.incCount( word_it_1->first, count_1 );
        new_lexicon.insert( word_it_1->first, temp_id_1_list );

        if (verbose_level > 0) *error_stream << "done." << endl;
      }

      else {
        if (verbose_level > 0) {
          *error_stream << "  ---> In both lexicons, merging the lexicons for the word...";
        }
        //*error_stream << "Now merging the two template-lists for the word..." << endl;
        //update count
        int count_2 = 0;
        if ( ! lex_count_db_2.getCount( word_it_1->first, count_2 ) ) {
          *error_stream << "Frequency of words not found" << endl;
          return 1;
          // maybe unknown word entry
          //continue;
        }
        new_lex_count_db.incCount( word_it_1->first, count_1 + count_2 );

        //Firstly, insert lexical entries for the word in 1st lexicon into new lexicon

        for ( vector< string >::iterator temp_it_1 = temp_id_1_list.begin();
              temp_it_1 != temp_id_1_list.end();
              ++temp_it_1 ) {
          ////*error_stream << ((LexTemplate)(*temp_it_1)).lexemeName().DisplayAVM();
          temp_id_new_list.push_back ( *temp_it_1 );
          //*error_stream << temp_it_1->DisplayAVM();
        }

        //Then, insert unseen lexical entries for the word in 2nd lexicon into new lexicon
        for ( vector< string >::iterator temp_it_2 = temp_id_2_list.begin();
              temp_it_2 != temp_id_2_list.end();
              ++temp_it_2 ) {

          //*error_stream << temp_it_2->DisplayAVM() << endl;
          map< string, string >::const_iterator new_temp = translate_table.find( *temp_it_2 );
          if ( new_temp == translate_table.end() ) {
            *error_stream << "Error: No Template was Found: " << *temp_it_2 << endl;
          }
          else {
            bool already_exist = false;
            for ( vector< string >::iterator temp_it_1 = temp_id_1_list.begin();
                  temp_it_1 != temp_id_1_list.end();
                  ++temp_it_1 ) {
              if ( *temp_it_1 ==  new_temp->second ) {
                //*error_stream << "The template for the word is already in new template list" << endl;
                already_exist = true;
                break;
              }
            }
            if ( !already_exist ) {
              //*error_stream << "New template, added to list" << endl;
              temp_id_new_list.push_back( new_temp->second );
            }
          }
        }

        new_lexicon.insert( word_it_1->first, temp_id_new_list );
        if (verbose_level > 0) *error_stream << "done" << endl;
      }
      mach.TrailBack( trailp );
      if ( num_processed_words % 1000 == 0 ) {
        if (verbose_level > 0) *error_stream << endl;
        *error_stream << num_processed_words << " words processed" << endl;
        if (verbose_level > 0) *error_stream << endl;
      }
    }

    *error_stream << endl;
    *error_stream << num_processed_words << " words processed" << endl;
    *error_stream << "done." << endl << endl;

    //Adding rest words in 2nd lexicon into new lexicon
    *error_stream << "Checking lexical entries for words in 2nd lexicon database..." << endl;
    int num_inserted_words = num_processed_words;

    for ( LexiconTable::iterator word_it_2 = lexicon_2.begin();
          word_it_2 != lexicon_2.end();
          ++word_it_2 ) {
      if (verbose_level > 0) {
        *error_stream << word_it_2->first << endl;
      }
      num_processed_words++;
      core_p trailp = mach.SetTrailPoint( NULL );
      vector< string > temp_id_1_list;
      if ( !lexicon_1.lookup( word_it_2->first, temp_id_1_list) ) {

        if (verbose_level > 0) {
          *error_stream << "  ---> Only in 2nd lexicon, just added to new lexicon...";
        }
        num_inserted_words++;
        vector< string > temp_id_2_list;
        vector< string > temp_id_new_list;
        if ( !lexicon_2.lookup( word_it_2->first, temp_id_2_list) ) {
          *error_stream << "Error: One template at least must be fit for the word." << endl;
          return 1;
        }
        for ( vector< string >::iterator temp_it_2 = temp_id_2_list.begin();
              temp_it_2 != temp_id_2_list.end();
              ++temp_it_2 ) {
          map< string, string >::const_iterator new_temp = translate_table.find( *temp_it_2 );
          if (new_temp != translate_table.end()) {
            //*error_stream << "Mapping: "
            //              << ((LexTemplate)(*temp_it_2)).lexemeName().DisplayAVM()
            //              << " -> "
            //              << new_temp_id.DisplayAVM();
            temp_id_new_list.push_back ( new_temp->second );
          }
          else {
            *error_stream << "No Template was Found (2): " << *temp_it_2 << endl;
          }
        }

        new_lexicon.insert( word_it_2->first, temp_id_new_list );
        int count_2 = 0;
        if ( ! lex_count_db_2.getCount( word_it_2->first, count_2 ) ) {
          *error_stream << "Frequency of words not found" << endl;
          //return 1;
          // maybe unknown word entry
          //continue;
        }
        new_lex_count_db.incCount( word_it_2->first, count_2 );
        //return 1;
        // maybe unknown word entry
        //continue;

        if (verbose_level > 0) *error_stream << "done" << endl;
        if ( num_processed_words % 1000 == 0 ) {
          if (verbose_level > 0) *error_stream << endl;
          *error_stream << num_processed_words << " words processed" << endl;
          if (verbose_level > 0) *error_stream << endl;
        }
      }
      else {
        if (verbose_level > 0) {
          *error_stream << "  ---> Already added to new lexicon" << endl;
        }
      }
      mach.TrailBack( trailp );
    }

    *error_stream << endl << num_processed_words << " words processed" << endl;
    *error_stream << "done." << endl;

    *error_stream << endl;
    *error_stream << "Merging lexicon database was completed." << endl;
    *error_stream << num_processed_words << " words checked" << endl;
    *error_stream << num_inserted_words << " words inserted into new lexicon database" << endl;

    // Save new lexicon
    *error_stream << endl;
    *error_stream << "Output new lexicon and templates database... ";
    new_lexicon.save( new_lexicon_file );
    new_temptable.save( new_template_file, new_template_file + ".count" );
    new_lex_count_db.save( new_lexicon_file + ".count" );
    *error_stream << "done." << endl;

  } catch ( exception& e ) {
    cerr << "Fatal error: " << e.what() << endl;
    return 1;
  }

  Terminator::PerformAll();

  return 0;
}

