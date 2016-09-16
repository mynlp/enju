#ifndef MoguraParser_h__
#define MoguraParser_h__

#include "ParserBasic.h"
#include "Grammar.h"
#include "MoguraGrammar.h"
#include "CfgGrammar.h"

namespace mogura { 

/// pimpl pattern
struct ParserImpl;

class Parser : public up::ParserBasic {
public:
    typedef ParserBasic base_type;

public:
    Parser(bool asSupertagger, unsigned numParse);

    ~Parser(void);

    bool init(lilfes::machine *m, mogura::Grammar *g);

    void getEdges(unsigned begin, unsigned end, std::vector<up::eserial>& edges) const;

    bool parseImpl(void);

    /// 'threshold' is in linear scale (e.g., 0.001, not log(0.001) )
    void setLexThreshold(double threshold);
    void setSeqThreshold(double threshold);

    /// 2009-06-17: to avoid unpredictably large memory consumption
    void setMaxCfgQueueItem(unsigned max);

private:
    ParserImpl *_impl;
    bool _asSupertagger;
    unsigned _numParse;
};

} // namespace mogura

#endif // MoguraParser_h__
