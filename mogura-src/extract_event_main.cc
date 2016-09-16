/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file extract_event_main.cc
 *  @version Time-stamp: <2008-07-14 14:48:51 yusuke>
 *  Main program of extract_event
 *
 **********************************************************************/

#include <string>
#include <iostream>
#include <stdexcept>

#include "ParserBuiltin.h"
#include "Derivbank.h"
#include "EventExtractor.h"

void* dummy = lilfes::BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates
void* dummy2 = lilfes::UP_BUILTIN_PRED_SYMBOLS; // just to avoid warnings

//////////////////////////////////////////////////////////////////////

void help_message() {
    std::cerr
        << "Usage: extract_event grammar_module derivbank_file event_file" << std::endl
        << "  grammar_module: LiLFeS module in which the grammar and event extraction predicates are defined"
            << std::endl
        << "  derivbank_file: derivation database file" << std::endl
        << "  event_file    : output file" << std::endl;
    return;
}

//////////////////////////////////////////////////////////////////////

bool hasSuffix(const std::string &full, const std::string &suffix)
{
    return full.size() >= suffix.size()
        && full.substr(full.size() - suffix.size()) == suffix;
}

std::ostream *openEventFile(const std::string &fname)
{
    if (hasSuffix(fname, ".bz2")) {
        return new lilfes::obfstream(fname.c_str());
    }
    else if (hasSuffix(fname, ".gz")) {
        return new lilfes::ogfstream(fname.c_str());
    }
    else {
        return new std::ofstream(fname.c_str());
    }
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
try {

    std::cerr << "unimaker (MAYZ " << MAYZ_VERSION << ")" << std::endl;
    std::cerr << MAYZ_COPYRIGHT << std::endl;

    // Initialize everything
    lilfes::Initializer::PerformAll();

    // show help message
    if (argc > 1 && std::string(argv[ 1 ]) == "-h") {
        help_message();
        return 0;
    }
    else if (argc != 4) {
        help_message();
        return 1;
    }

    std::string grammar_name(argv[1]);
    std::string derivbank_name(argv[2]);
    std::string eventFileName(argv[3]);

    std::ostream *eventFile = up::openOutputFile(eventFileName);

    // Initialize a machine object
    lilfes::machine mach;

    // Load grammar module
    std::cerr << "Loading grammar module \"" << grammar_name << "\"... ";
    if (! lilfes::load_module(mach, "grammar module", grammar_name)) {
        std::cerr << "grammar module not found" << std::endl;
        return 1;
    }
    if (! lilfes::load_module(mach, "parser module", "mayz/up")) {
        std::cerr << "parser module \"mayz/up\" not found" << std::endl;
        return 1;
    }
    std::cerr << "done." << std::endl;

    // Load derivbank
    std::cerr << "Loading derivbank \"" << derivbank_name << "\"... ";
    mayz::Derivbank derivbank(&mach);
    if (! derivbank.load(derivbank_name)) {
        std::cerr << "derivbank not found" << std::endl;
        return 1;
    }
    std::cerr << "done." << std::endl;

    // Initialize grammar
    std::cerr << "Initializing grammar ...\n";
    mogura::Grammar grammar;
    if (! grammar.init(&mach)) {
        std::cerr << "failed." << std::endl;
        return 1;
    }
    
    // Initialize extractor
    std::cerr << "Initializing event extractor ...\n";
    mogura::EventExtractor extractor(&mach, &grammar);
    std::cerr << "done." << std::endl;

    // Start lilfes command
    std::cerr << "Start making events" << std::endl;

    for ( mayz::Derivbank::iterator deriv_it = derivbank.begin();
          deriv_it != derivbank.end();
          ++deriv_it) {

        lilfes::core_p trailp = mach.SetTrailPoint( NULL );

        int deriv_id = deriv_it->first;
        mayz::Derivation derivation = deriv_it->second;
                
        std::cerr << "No. " << deriv_id << ": ";

        extractor.extractEvent(derivation, *eventFile, std::cerr);

        std::cerr << std::endl;
                
        mach.TrailBack( trailp );
    }

    std::cerr << "done." << std::endl;

    delete eventFile;

    lilfes::Terminator::PerformAll();

    return 0;
} 
catch (std::bad_alloc) {
    std::cerr << "Memory allocation failed, unimaker terminated." << std::endl;
    throw;
}
catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    throw;
}

