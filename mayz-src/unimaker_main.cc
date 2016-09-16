/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file unimaker_main.cc
 *  @version Time-stamp: <2009-12-09 21:37:03 yusuke>
 *  Main program of unimaker
 *
 **********************************************************************/

#include <memory>
#include <cstring>
#include <set>
#include "Supertagger.h"
#include "mayzutil.h"

static std::string super_conf_name;
static std::string lexicon_name; // lexicon in txt format
static std::string lexseq_name;
static std::string uevent_file_name;
static unsigned limit_sentences = 0;
static unsigned num_word_fields = 0;

static std::set<int> target_sentences;

//////////////////////////////////////////////////////////////////////

void help_message(void)
{
    std::cerr
        << "Usage: unimaker [options] super_conf lexicon lexseq num_word_fields uevent" << std::endl
        << "  super_conf      : supertagger configuration file" << std::endl
        << "  lexicon_tbl     : lexicon in text file format" << std::endl
        << "  lexseq          : word/POS/supertag sequence file" << std::endl
        << "  num_word_fields : number of word-data fields in the lexseq file" << std::endl
        << "  uevent          : output file" << std::endl
        << std::endl
        << "Options:" << std::endl
        << "  -n num : limit number of sentences (default: " << limit_sentences << ")" << std::endl
        ;
}

bool analyze_arguments( int argc, char** argv )
{
    char** end = argv + argc;
    ++argv;
    for ( ; argv != end; ++argv ) {
        if ( (*argv)[ 0 ] == '-' ) {
            // option without argument
            if ( argv + 1 == end ) {
                std::cerr << *argv << " option requires additional argument" << std::endl;
                return false;
            }
            if ( std::strcmp( (*argv), "-n" ) == 0 ) {
                limit_sentences = std::strtol( *(++argv), NULL, 0 );
            }
            else if ( std::strcmp( (*argv), "--target-sentence" ) == 0 ) {
                target_sentences.insert( std::strtol( *(++argv), NULL, 0 ) );
            }
            else {
                std::cerr << "Unknown option: " << *argv << std::endl;
                return false;
            }
        }
        else { // arguments
            if ( end - argv != 5 ) {
                std::cerr << "unimaker requires five arguments" << std::endl;
                return false;
            }
            super_conf_name = argv[ 0 ];
            lexicon_name    = argv[ 1 ];
            lexseq_name     = argv[ 2 ];
            num_word_fields = std::atoi(argv[ 3 ]);
            uevent_file_name  = argv[ 4 ];
            return true;
        }
    }
    std::cerr << "unimaker requires four arguments" << std::endl;
    return false;
}

bool readWordLattice(std::istream &ist,
                     unsigned num_word_fields,
                     up::WordLattice &word_lattice,
                     std::vector<std::string> &templates)
{
    word_lattice.clear();
    templates.clear();

    unsigned word_position = 0;
    std::string line;
    while (std::getline(ist, line)) {
        if (line.empty()) {
            return true;
        }

        std::istringstream iss(line);
        up::WordExtent w;
        std::string tmpl;

        w.word_fom = 0;
        w.begin = word_position;
        w.end = word_position + 1;
        ++word_position;

        w.word.resize(num_word_fields);
        for (unsigned i = 0; i < num_word_fields; ++i) {
            if (! (iss >> w.word[i])) {
                throw std::runtime_error("lexseq file format error (wrong number of fields): " + line);
            }
        }

        if (! (iss >> tmpl)) {
            throw std::runtime_error("lexseq file format error (cannot read template name): " + line);
        }

        std::string key;
        while (iss >> key) {
            w.lookup_keys.push_back(key);
        }

        if (w.lookup_keys.empty()) {
            throw std::runtime_error("lexseq file format error (no lookup keys found): " + line);
        }

        word_lattice.push_back(w);
        templates.push_back(tmpl);
    }

    return false; // EOF
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
try {
    std::cerr << "unimaker (MAYZ " << MAYZ_VERSION << ")" << std::endl;
    std::cerr << MAYZ_COPYRIGHT << std::endl;

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

    std::auto_ptr<std::istream> super_conf_file( up::openInputFile(super_conf_name) );
    std::auto_ptr<std::istream> lexicon_file( up::openInputFile(lexicon_name) );
    std::auto_ptr<std::istream> lexseq_file( up::openInputFile(lexseq_name) );
    std::auto_ptr<std::ostream> unfiltered_event_file( up::openOutputFile(uevent_file_name) );

    up::SupertaggerEventExtractor extractor(*lexicon_file, *super_conf_file);

    std::cerr << "Start making uni-lexical events" << std::endl;

    up::WordLattice word_lattice;
    std::vector<std::string> templates;
    unsigned sentence_id = 0;
    while (readWordLattice(*lexseq_file, num_word_fields, word_lattice, templates)) {

        ++sentence_id;
        if (limit_sentences > 0 && sentence_id > limit_sentences) {
            break;
        }

        if (! target_sentences.empty() && target_sentences.find(sentence_id) == target_sentences.end()) {
            continue;
        }

        std::cerr << "No. " << sentence_id << ": ";

        bool is_success = extractor.outputEvents(sentence_id, word_lattice, templates, *unfiltered_event_file);

        std::cerr << ( is_success ? "success" : "fail" ) << std::endl;
    }

    std::cerr << "done." << std::endl;

    return 0;
} 
catch (std::bad_alloc) {
    std::cerr << "Memory allocation failed, unimaker terminated." << std::endl;
    throw;
}
catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    throw;
}

