#include "Extent.h"

#include "MoguraParser.h"
#include "ParserAction.h"
#include "ParserGuide.h"
#include "SeqEnumerator.h"
#include "ParserFeature.h"
#include "MultVpModel.h"

#include <functional>

namespace mogura {

////////////////////////////////////////////////////////////////////////////////
// Parser core
////////////////////////////////////////////////////////////////////////////////
typedef std::vector<std::vector<std::vector<up::eserial> > > Chart;

struct ParserImpl {
public:
    ParserImpl(
        Grammar *grammar,
        const std::string &cfgFileName,
        const std::string &restrictedCfgFileName, /// maybe empty
        const std::string &modelFileName,
        double lexThreshold,  /// supertags with prob less than P(t_0) x lexThreshold will not be used
        double seqThreshold,  /// supertag seq. with prob less than P(s_0) x seqThreshold will not be used
        unsigned maxCfgQueueItem /// maximum number of queue pop in the seqeuence enumerator
    )
        : _grammar(grammar)
        , _cfgGrammar(0)
        , _restrictedCfgGrammar(0)
        , _enumerator(0)
        , _guide(0)
        , _fset(*grammar)
    {
        std::istream *cfgFile = up::openInputFile(cfgFileName);
        std::istream *restrictedCfgFile = (restrictedCfgFileName.empty()) ? 0 : up::openInputFile(restrictedCfgFileName);
        std::istream *modelFile = up::openInputFile(modelFileName);

        std::cerr << "  Loading CFG grammar from " << cfgFileName << " .. ";
        _cfgGrammar = new cfg::Grammar();
        _cfgGrammar->load(*cfgFile);
        std::cerr << "done" << std::endl;

        if (! restrictedCfgFileName.empty()) {
            std::cerr << "  Loading restricted CFG grammar from " << restrictedCfgFileName << " .. ";
            _restrictedCfgGrammar = new cfg::Grammar();
            _restrictedCfgGrammar->load(*restrictedCfgFile);
            _restrictedCfgGrammar->changeRuleId(_cfgGrammar->_rule);
            std::cerr << "done" << std::endl;
        }

        _enumerator = new cfg::SeqEnumerator(_cfgGrammar, _restrictedCfgGrammar, lexThreshold, seqThreshold, maxCfgQueueItem);
        _guide = new ParserGuide(_cfgGrammar, _restrictedCfgGrammar);

        std::cerr << "  Loading model file from " << modelFileName << " .. ";
        _model.load(*modelFile);
        std::cerr << "done" << std::endl;

        delete modelFile;
        delete cfgFile;
        delete restrictedCfgFile; /// maybe = 0

        if (! _actionList.init(*_grammar, *_cfgGrammar)) {
            throw std::runtime_error("Cannot get schema names");
        }
    }

    ~ParserImpl(void)
    {
        delete _guide;
        delete _enumerator;
        delete _cfgGrammar;
        delete _restrictedCfgGrammar;
    }

    bool parse(ParserState &state);

    bool makeChunks(const up::LexEntLattice &lattice, ParserState &state);

    void rankAction(const ParserState &state, std::vector<const ParserAction*> &as) const
    {
        if (as.size() < 2) {
            return;
        }

        std::vector<std::string> fv;
        _fset.extract(state, fv);

        std::vector<double> scores;
        _model.getScore(state.getKey(*_grammar), fv, scores);

        typedef std::pair<double, const ParserAction*> ScoreAction;
        std::vector<ScoreAction> sas(as.size());
        for (unsigned i = 0; i < as.size(); ++i) {
            int ix = _model.getClassId(as[i]->asString());
            double s = (0 <= ix && ix < (int) scores.size()) ? scores[ix] : 0; /// default should be -infinity?
            sas[i] = ScoreAction(s, as[i]);
        }

        std::stable_sort(sas.begin(), sas.end(), std::greater<ScoreAction>());

        for (unsigned i = 0; i < sas.size(); ++i) {
            as[i] = sas[i].second;
        }
    }

public:
    Grammar *_grammar;
    cfg::Grammar *_cfgGrammar;
    cfg::Grammar *_restrictedCfgGrammar;
    cfg::SeqEnumerator *_enumerator;
    ParserGuide *_guide;
    ParserActionList _actionList;
    FeatureExtractor _fset;
    MultVpModel _model;

    Chart _chart;
};

bool initState(
    Grammar &grammar,
    const up::LexEntLattice &lattice,
    const cfg::SeqEnumerator::TemplateSeq &seq,
    ParserState &state
) {
    up::ScopeProf prof("mogura::initState");

    state.clear();
    state._input.resize(seq.size());
    for (unsigned i = 0; i < seq.size(); ++i) {
        const up::LexEntExtent &ex = lattice[ seq[i]._extentIx ];
        const up::LexTemplateFom &tmpl = ex.tmpls[ seq[i]._templateIx ];
        state._input[i] = LexEnt(seq[i]._extentIx, ex.begin, ex.end, ex.word, tmpl.tmpl, tmpl.fom);
    }

    return state.init(grammar);
}


void shiftEager(std::vector<const ParserAction*> &as)
{
    for (unsigned i = 0; i < as.size(); ++i) {
        if (as[i]->asString() == "shift") {
            std::swap(as[0], as[i]);
            return;
        }
    }
}

const ParserAction *applyAction(
    const std::vector<const ParserAction*> &as,
    Grammar &g,
    ParserState &state
) {
    for (std::vector<const ParserAction*>::const_iterator a = as.begin(); a != as.end(); ++a) {
        if ((*a)->apply(g, state)) {
            return *a;
        }
    }
    return 0;
}

void filterAction(
    const ParserActionList &actions,
    const ParserGuide &guide,
    std::vector<const ParserAction*> &as
) {
    for (ParserActionList::const_iterator a = actions.begin(); a != actions.end(); ++a) {
        if (guide.accept((*a)->getCode())) {
            as.push_back(*a);
        }
    }
}

bool initGuide(const ParserState &state, ParserGuide &guide)
{
    up::ScopeProf prof("mogura::initGuide");

    std::vector<std::string> ts(state._input.size());
    for (unsigned i = 0; i < ts.size(); ++i) {
        ts[i] = state._input[i]._lexTemplate;
    }

    return guide.init(ts);
}

bool ParserImpl::parse(ParserState &state)
{
    (void) initGuide(state, *_guide);

    while (! state.isComplete(*_grammar)) {
        
        std::vector<const ParserAction*> as;
        filterAction(_actionList, *_guide, as);

        if (as.size() > 1) {
            rankAction(state, as);
            // shiftEager(as);
        }

        const ParserAction *applied = applyAction(as, *_grammar, state);
        if (applied) {
            _guide->feed(applied->getCode());
        }
        else {
            return false;
        }
    }

    return true;
}

bool ParserImpl::makeChunks(const up::LexEntLattice &lattice, ParserState &state)
{
    up::ScopeProf prof("mogura::makeChunks");

    cfg::SeqEnumerator::TemplateSeq seq;
    if (! _enumerator->findBestFailSeq(seq)) {
        /// No lexical item for a word => lex analysis error
        return false;
    }

    if (! initState(*_grammar, lattice, seq, state)) {
        return false;
    }

    bool followGuide = true;
    if (! initGuide(state, *_guide)) {
        _guide->forceConnect();
    }

    while (! state.isComplete(*_grammar)) {
        
        std::vector<const ParserAction*> as;
        if (followGuide) {
            filterAction(_actionList, *_guide, as);
            if (as.empty()) {
                followGuide = false;
                continue; /// retry without the guide
            }
        }
        else {
            as.insert(as.end(), _actionList.begin(), _actionList.end());
        }

        if (as.size() > 1) {
            rankAction(state, as);
            // shiftEager(as);
        }

        const ParserAction *applied = applyAction(as, *_grammar, state);
        if (applied) {
            if (followGuide) {
                _guide->feed(applied->getCode());
            }
        }
        else {
            if (followGuide) {
                followGuide = false;
                continue;
            }
            else {
                while (! state._words.empty()) {
                    state.shift();
                }
                return true;
            }
        }
    }

    assert(state._words.empty());

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// make parse results
//////////////////////////////////////////////////////////////////////////////

std::pair<unsigned, unsigned> /// (edge-idx, right-position)
storeTree(
    lilfes::machine *mach,
    Grammar &grammar,
    const ParseTree *tree,
    Chart &chart,
    std::vector<up::Edge> &es,
    std::vector<up::Link> &ls,
    unsigned leftPosition,
    bool isPartial
) {
    up::lserial linkIx = (up::lserial) -1;
    unsigned rightPosition = 0;

    if (tree->_leftDtr) { /// non-terminal
        std::pair<unsigned, unsigned> dtr1
            = storeTree(mach, grammar, tree->_leftDtr, chart, es, ls, leftPosition, isPartial);

        if (tree->_rightDtr == 0) { /// unary
            rightPosition = dtr1.second;
            linkIx = ls.size();
            ls.push_back(up::Link(linkIx, dtr1.first));
        }
        else { /// binary
            std::pair<unsigned, unsigned> dtr2
                = storeTree(mach, grammar, tree->_rightDtr, chart, es, ls, dtr1.second, isPartial);
            rightPosition = dtr2.second;
            linkIx = ls.size();
            ls.push_back(up::Link(linkIx, dtr1.first, dtr2.first));
        }

        lilfes::FSP schema(mach);
        schema.Coerce(tree->_schema->getLilfesType());
        schema.Serialize(ls[linkIx].getAppliedSchema());
    }
    else { /// terminal
        assert(tree->_rightDtr == 0);

        const LexEnt *lex = tree->_lexHead;
        rightPosition = lex->_end;
        linkIx = ls.size();
        ls.push_back(up::Link(linkIx, lex->_fom));

        lilfes::FSP wordFSP = grammar.makeWordFSP(lex->_word, lex->_begin, lex->_end);
        lilfes::FSP lexid = grammar.makeLexicalEntryFSP(wordFSP, lex->_lexTemplate);
        lexid.Serialize(ls.back().getLexName());
        ls.back().setWordId(lex->_word_id);
    }

    lilfes::FSP(mach).Serialize(ls[linkIx].getSignPlus()); /// sign_plus = bot

    up::eserial edgeIx = es.size();
    es.push_back(up::Edge(edgeIx, std::make_pair(leftPosition, rightPosition), up::Fs()));
    lilfes::FSP sign = tree->_sign;
    sign.Serialize(es.back().getSign());
    es.back().addLink(linkIx);
    if (isPartial) {
        es.back().setFom(-1.0 / (rightPosition - leftPosition)); /// give hint to enju:partial
    }
    else {
        es.back().setFom(tree->_fom);
    }

    chart[leftPosition][rightPosition].push_back(edgeIx);

    return std::make_pair(edgeIx, rightPosition);
}

void storeResult(
    lilfes::machine *mach,
    Grammar &grammar,
    const ParserState &state,
    Chart &chart,
    std::vector<up::Edge> &es,
    std::vector<up::Link> &ls,
    bool isPartial  /// we cannot assume state._stack.size() == 1 <==> isPartial == false
                    /// since root condition may not be satisfied by state._stack[0]
) {
    up::ScopeProf prof("mogura::storeResult");

    unsigned right = 0;
    for (int i = state._stack.size() - 1; i >= 0; --i) {
        std::pair<unsigned, unsigned> p = storeTree(mach, grammar, state._stack.at(i), chart, es, ls, right, isPartial);
        right = p.second;
    }
}

//////////////////////////////////////////////////////////////////////////////
// supertagging + CFG-filtering
//////////////////////////////////////////////////////////////////////////////
struct ViterbiSearchNode {
    unsigned _begin;
    unsigned _extentIx;
    unsigned _templateIx;
    double _fom;

    double _maxForwardFom;
    int _backPtr;
};

struct LessFom {
    template<class ElemT>
    bool operator()(const ElemT &e1, const ElemT &e2) const
    {
        return e1.fom < e2.fom;
    }
};

struct LessMaxFowardFom {
    bool operator()(const ViterbiSearchNode &n1, const ViterbiSearchNode &n2) const
    {
        return n1._maxForwardFom < n2._maxForwardFom;
    }
};

bool viterbiSearch(const up::LexEntLattice &lattice, cfg::SeqEnumerator::TemplateSeq &seq)
{
    seq.clear();
    if (lattice.empty()) {
        return true;
    }

    std::vector<std::vector<ViterbiSearchNode> > searchLattice;
    for (unsigned i = 0; i < lattice.size(); ++i) {
        const up::LexEntExtent &ex = lattice[i];

        if (ex.end <= ex.begin) {
            std::cerr << "lexent-lattice includes invalid extent: "
                << "[" << ex.begin << ", " << ex.end << ")" << std::endl;
            return false;
        }

        if (searchLattice.size() <= ex.end) {
            searchLattice.resize(ex.end + 1);
        }
        searchLattice[ex.end].push_back(ViterbiSearchNode());
        ViterbiSearchNode &node = searchLattice[ ex.end ].back();

        std::vector<up::LexTemplateFom>::const_iterator max
            = std::max_element(ex.tmpls.begin(), ex.tmpls.end(), LessFom());

        node._begin = ex.begin;
        node._extentIx = i;
        node._templateIx = std::distance(ex.tmpls.begin(), max);
        node._fom = max->fom;

        node._maxForwardFom = -std::numeric_limits<double>::infinity();
        node._backPtr = -1;
    }

    for (unsigned i = 0; i < searchLattice.size(); ++i) {
        for (unsigned j = 0; j < searchLattice[i].size(); ++j) {
            ViterbiSearchNode &node = searchLattice[i][j];

            if (node._begin == 0) {
                node._maxForwardFom = node._fom;
            }
            else {
                const std::vector<ViterbiSearchNode> &prev = searchLattice[ node._begin ];
                for (unsigned k = 0; k < prev.size(); ++k) {
                    if (prev[k]._maxForwardFom > node._maxForwardFom) {
                        node._backPtr = k;
                    }
                }
                if (node._backPtr != -1) { // connected
                    node._maxForwardFom += node._fom;
                }
            }
        }
    }

    // find best last template
    std::vector<ViterbiSearchNode>::const_iterator ptr
        = std::max_element(searchLattice.back().begin(), searchLattice.back().end(), LessMaxFowardFom());

    while (true) {
        seq.push_back(cfg::SeqEnumerator::LeafPtr(ptr->_extentIx, ptr->_templateIx));
        if (ptr->_backPtr >= 0) {
            ptr = searchLattice[ ptr->_begin ].begin() + ptr->_backPtr;
        }
        else {
            break;
        }
    }

    if (ptr->_begin != 0) {
        seq.clear();
        std::cerr << "lexent-lattice is not connected" << std::endl;
        return false;
    }

    // reverse
    cfg::SeqEnumerator::TemplateSeq(seq.rbegin(), seq.rend()).swap(seq);

    return true;
}

bool storeSupertags(
    lilfes::machine *mach,
    Grammar &grammar,
    const up::LexEntLattice &lattice,
    std::vector<up::Edge> &edges,
    std::vector<up::Link> &links,
    ParserImpl *impl
) {
    lilfes::IPTrailStack iptrail(mach);

    cfg::SeqEnumerator::TemplateSeq seq;
    bool sequenceExists = impl->_enumerator->getFirst(lattice, seq);

    if (! sequenceExists) { /// fall-back: select highest-scored lexents
        if (! viterbiSearch(lattice, seq)) {
            return false; // lattice not connected
        }
    }

    ParserState state;
    if (! initState(grammar, lattice, seq, state)) {
        return false;
    }

    unsigned right = 0;
    for (unsigned i = 0; i < state._words.size(); ++i) {
        std::pair<unsigned, unsigned> p
            = storeTree(mach, grammar, state._words.at(i), impl->_chart, edges, links, right, true);
        right = p.second;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// Public interface
//////////////////////////////////////////////////////////////////////////////

bool Parser::parseImpl(void)
{
    lilfes::IPTrailStack iptrail(mach);

    if (! checkSentenceLength()) { // too long
        return false;
    }

    if (sentence_length == 0) { // no successful parse for "!" etc.: following enju's convention
        return true;
    }

    _impl->_chart.clear();
    _impl->_chart.resize(sentence_length, std::vector<std::vector<up::eserial> >(sentence_length + 1));

    edge_vec.clear();
    link_vec.clear();

    if (_asSupertagger) {
        if (storeSupertags(mach, *_impl->_grammar, lexent_lattice, edge_vec, link_vec, _impl)) {
            parse_status = up::EDGE_LIMIT; /// need a new state?
            return true; /// should be false?
        }
        else {
            parse_status = up::LEXENT_ANALYSIS_ERROR;
            return false;
        }
    }

    const unsigned NUM_MAX_FAIL = 500; /// TODO: make this value configurable
    unsigned numFail = 0;
    unsigned numSuccess = 0;

    cfg::SeqEnumerator::TemplateSeq seq;
    bool sequenceExists = _impl->_enumerator->getFirst(lexent_lattice, seq);

    while (sequenceExists) {
        
        lilfes::IPTrailStack iptrail2(mach);

        ParserState state;
        if (! initState(*_impl->_grammar, lexent_lattice, seq, state)) {
            if (numSuccess > 0) {
                parse_status = up::SUCCESS;
                return true;
            }
            else {
                parse_status = up::LEXENT_ANALYSIS_ERROR;
                return false;
            }
        }

        if (_impl->parse(state)) {
            parse_status = up::SUCCESS;
            ++numSuccess;
            storeResult(mach, *_impl->_grammar, state, _impl->_chart, edge_vec, link_vec, false);
            if (numSuccess >= _numParse) {
                return true;
            }
        }

        if (++numFail > NUM_MAX_FAIL) {
            break;
        }

        sequenceExists = _impl->_enumerator->getNext(seq);
    }

    if (numSuccess > 0) {
        parse_status = up::SUCCESS;
        return true;
    }

    ParserState state;
    if (_impl->makeChunks(lexent_lattice, state)) {
        storeResult(mach, *_impl->_grammar, state, _impl->_chart, edge_vec, link_vec, true);
        parse_status = up::SEARCH_LIMIT;
    }
    else {
        parse_status = up::LEXENT_ANALYSIS_ERROR;
    }

    return false;
}

void Parser::getEdges(unsigned left, unsigned right, std::vector<up::eserial>& edges) const
{
    edges.clear();
    if (left < _impl->_chart.size() && right < _impl->_chart[left].size()) {
        edges = _impl->_chart[left][right];
    }
}

Parser::Parser(bool asSupertagger, unsigned numParse)
    : _impl(0)
    , _asSupertagger(asSupertagger)
    , _numParse(numParse)
{}

Parser::~Parser(void)
{
    delete _impl;
}

bool Parser::init(lilfes::machine *m, Grammar *g)
{
    up::ScopeProf prof("mogura::Parser::init");

    if (! base_type::init(m, g)) {
        return false;
    }

    std::string cfgGrammarFileName;
    if (! g->getCfgGrammarFile(cfgGrammarFileName)) {
        std::cerr << "Cannot get the file-name of CFG grammar" << std::endl;
        return false;
    }

    std::string restrictedCfgGrammarFileName;
    g->getRestrictedCfgGrammarFile(restrictedCfgGrammarFileName); /// fail -> restrictedCfgGrammar won't be used

    std::string modelFileName;
    if (! g->getModelFile(modelFileName)) {
        std::cerr << "Cannot get model-file name" << std::endl;
        return false;
    }

    if (_impl) {
        delete _impl;
    }

    try {
        _impl = new ParserImpl(g,
                               cfgGrammarFileName,
                               restrictedCfgGrammarFileName,
                               modelFileName,
                               0, 0,
                               std::numeric_limits<unsigned>::max());
    }
    catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        delete _impl;
        _impl = 0;
        return false;
    }

    return true;
}

void Parser::setLexThreshold(double threshold)
{
    //_impl->_lexThreshold = std::log(threshold);
    _impl->_enumerator->setLexThreshold(threshold);
}

void Parser::setSeqThreshold(double threshold)
{
    _impl->_enumerator->setSeqThreshold(threshold);
}

void Parser::setMaxCfgQueueItem(unsigned max)
{
    _impl->_enumerator->setMaxQueueItem(max);
}

} // namespace mogura {
