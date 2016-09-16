/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file train_main.cc
 *  @version Time-stamp: <2008-07-14 14:49:04 yusuke>
 *  Main program of train
 *
 **********************************************************************/

#include <sstream>
#include <algorithm>
#include <ctime>
#include "ProgressBar.h"
#include "MultVpModel.h"
#include "mayzutil.h"

using namespace mogura;

void* dummy = lilfes::BUILTIN_PRED_SYMBOLS;  // to use lilfes builtin predicates

struct Event {
    //  int _sntId;
    std::string _key;
    std::vector<std::string> _options;
    std::vector<std::string> _features;
};

std::istream &operator>>(std::istream &ist, Event &e)
{
    std::string line;
    if (! std::getline(ist, line)) {
        return ist;
    }

    std::istringstream iss(line);

    // if (! (iss >> e._sntId)) {
    //    throw std::runtime_error("Event file format error: no sentence ID");
    // }

    if (! (iss >> e._key)) {
        throw std::runtime_error("Event file format error: no key field");
    }

    e._options.clear();
    std::string opt;
    while (true) {
        if (! (iss >> opt)) {
            throw std::runtime_error("Event file format error: no terminator");
        }

        if (opt == "/") {
            break;
        }

        e._options.push_back(opt);
    }

    e._features.clear();
    std::copy(
        std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        std::back_inserter(e._features));

    return ist;
}

void readEvents(
    std::istream &ist,
    std::vector<Event> &events
) {
    events.clear();

    std::cerr << "Reading samples" << std::endl;
    ProgressBar pb(10000, 100000, std::cerr, "samples");

    Event e;
    while (ist >> e) {
        events.push_back(e);
        ++pb;
    }

    pb.done();
}

//////////////////////////////////////////////////////////////////////

void usage(void)
{
    std::cerr << "Usage: train <polynomial-order> <num-loop> <event-file> <output-file>" << std::endl;
    std::cerr << "  <polynomial-order> : 1, 2, or 3" << std::endl;
    exit(1);
}

int main(int argc, char **argv)
try {
    std::time_t beginTime = std::time(0);

    if (argc != 5) {
        usage();
    }

    unsigned order = std::atoi(argv[1]);
    unsigned numLoop = std::atoi(argv[2]);
    std::string eventFileName = argv[3];
    std::string outFileName = argv[4];

    if (order < 1 || 3 < order) {
        usage();
        exit(1);
    }

    std::cerr << "# order   : " << order << std::endl;
    std::cerr << "# num-loop: " << numLoop << std::endl;
    std::cerr << "# event-file : " << eventFileName << std::endl;
    std::cerr << "# output-file: " << outFileName << std::endl;

    std::istream *eventFile = up::openInputFile(eventFileName);
    std::ostream *outFile = up::openOutputFile(outFileName);

    std::vector<Event> events;
    readEvents(*eventFile, events);
    delete eventFile;

    std::cerr << "Begin training" << std::endl;

    VpTrainer trainer;
    for (std::vector<Event>::const_iterator e = events.begin(); e != events.end(); ++e) {
        trainer.addTrainingSample(e->_key, e->_options, e->_features);
    }
    events.clear();


    switch (order) {
        case 1:
            trainer.train<Linear>(numLoop, *outFile, std::cerr);
            break;
        case 2:
            trainer.train<Poly2nd>(numLoop, *outFile, std::cerr);
            break;
        case 3:
            trainer.train<Poly3rd>(numLoop, *outFile, std::cerr);
            break;
        default:
            assert(false);
    }

    delete outFile;

    std::time_t endTime = std::time(0);

    std::cerr << "End training (" << (endTime - beginTime) << " sec)" << std::endl;

    return 0;
}
catch (std::runtime_error &e)
{
    std::cerr << e.what() << std::endl;
    exit(1);
}
catch (...)
{
    std::cerr << "Unknown exception" << std::endl;
    exit(1);
}
