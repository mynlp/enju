/**********************************************************************
 *
 *  Copyright (c) 2005-2006, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file mogura.cc
 *  Main program of mogura
 *
 **********************************************************************/

#include <liblilfes/lilfes.h>
#include "MoguraDriver.h"

using namespace lilfes;

int main(int argc, char** argv)
try {
    // Initialize LiLFeS
    Initializer::PerformAll();

    mogura::Driver driver;

    // Show startup message
    if (! driver.startupMessage()) {
        return 1;
    }

    // Show help message
    if (argc > 1 && std::string(argv[1]) == "-h") {
        if (! driver.helpMessage()) {
            return 1;
        }
        return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "-hh") {
        if (! driver.heavyHelpMessage()) {
            return 1;
        }
        return 0;
    }

    // Analyze command-line arguments
    if (! driver.analyzeArguments(argc, argv)) {
        driver.helpMessage();
        return 1;
    }

    // Initialization
    if (! driver.initialize()) {
        driver.finalize();
        Terminator::PerformAll();
        return 1;
    }

    // Main routine
    if (! driver.run()) {
        driver.finalize();
        Terminator::PerformAll();
        return 1;
    }

    // Finalization
    if (! driver.finalize()) {
        Terminator::PerformAll();
        return 1;
    }

    // Terminate LiLFeS
    Terminator::PerformAll();

    return 0;
}
catch (std::bad_alloc) {
    std::cerr << "Memory allocation failed. Program terminated." << std::endl;
    throw;
}
catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    throw;
}


