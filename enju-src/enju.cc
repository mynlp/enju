/**********************************************************************
 *
 *  Copyright (c) 2005-2006, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file enju.cc
 *  Main program of enju
 *
 **********************************************************************/

#include "EnjuDriver.h"
#include <liblilfes/lilfes.h>
#include <cstring>

using namespace lilfes;

int main( int argc, char** argv ) {
  try {
    // Check time limit
//#define ENJU_TIME_LIMIT 1263772800  // 2010/01/15
#ifdef ENJU_TIME_LIMIT
    time_t t = time(NULL);
    if ( t <= ENJU_TIME_LIMIT ) {
      //std::cerr << "OK" << std::endl;
    } else {
      std::cerr << "Time limit expired" << std::endl;
      exit(1);
    }
#endif // ENJU_TIME_LIMIT

    // Initialize LiLFeS
    Initializer::PerformAll();

    enju::EnjuDriver enju_driver;

    // Show startup message
    if ( ! enju_driver.startupMessage() ) {
      return 1;
    }

    // Show help message
    if ( argc > 1 && std::strcmp( argv[ 1 ], "-h" ) == 0 ) {
      if ( ! enju_driver.helpMessage() ) {
        return 1;
      }
      return 0;
    }
    if ( argc > 1 && std::strcmp( argv[ 1 ], "-hh" ) == 0 ) {
      if ( ! enju_driver.heavyHelpMessage() ) {
        return 1;
      }
      return 0;
    }

    // Analyze command-line arguments
    if ( ! enju_driver.analyzeArguments( argc, argv ) ) {
      enju_driver.helpMessage();
      return 1;
    }

    // Initialization
    if ( ! enju_driver.initialize() ) {
      enju_driver.finalize();
      Terminator::PerformAll();
      return 1;
    }

    // Main routine
    if ( ! enju_driver.run() ) {
      enju_driver.finalize();
      Terminator::PerformAll();
      return 1;
    }

    // Finalization
    if ( ! enju_driver.finalize() ) {
      Terminator::PerformAll();
      return 1;
    }

    // Terminate LiLFeS
    Terminator::PerformAll();
  }
  catch (std::bad_alloc) {
    std::cerr << "Memory allocation failed. Program terminated." << std::endl;
    throw;
  }
  catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    throw;
  }

  return 0;
}

