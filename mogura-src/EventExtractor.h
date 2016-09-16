#ifndef EventExtractor_h__
#define EventExtractor_h__

#include <iostream>
#include <liblilfes/structur.h>
#include "MoguraGrammar.h"

//------------------------------------------------------------------------------
namespace mogura {
//------------------------------------------------------------------------------

class EventExtractorImpl;

class EventExtractor {
public:
    EventExtractor(lilfes::machine *mach, mogura::Grammar *grammar);
    ~EventExtractor(void);

    void extractEvent(lilfes::FSP derivation, std::ostream &out, std::ostream &log);

private:
    EventExtractorImpl *_impl;
};

//------------------------------------------------------------------------------
} /// namespace mogura {
//------------------------------------------------------------------------------

#endif // EventExtractor_h__
