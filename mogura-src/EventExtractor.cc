#include "ParserBasic.h"
#include "EventExtractor.h"
#include "ParserFeature.h"
#include "MoguraGrammar.h"
#include "CfgGrammar.h"
#include "ParserAction.h"
#include "ParserState.h"
//#include "ParserGuide.h"

//------------------------------------------------------------------------------
namespace mogura {
//------------------------------------------------------------------------------

class EventExtractorImpl : up::ParserBasic {
public:
    EventExtractorImpl(lilfes::machine *mach, mogura::Grammar *grammar)
        : _mach(mach)
        , _grammar(grammar)
        , _fset(0)
        // , _guide(0)
        , _extract_terminals(0)
        , _extract_action_sequence(0)
    {
        up::Builtin::setGrammar(grammar);
        up::ParserBasic::init(mach, grammar);

        _fset = new FeatureExtractor(*grammar);

        std::string cfgGrammarFileName;
        if (! grammar->getCfgGrammarFile(cfgGrammarFileName)) {
            throw std::runtime_error("Cannot get the file-name of CFG grammar");
        }

        lilfes::module *module = lilfes::module::SearchModule("mogura:train");
        if (! module) {
            throw std::runtime_error("module not found: mogra:train");
        }

        bool ok = up::init_proc(module, _extract_terminals, "extract_terminals", 2)
               && up::init_proc(module, _extract_action_sequence, "extract_action_sequence", 2)
               ;

        if (! ok) {
            throw std::runtime_error("predicate initialization error");
        }

        std::ifstream cfgGrammarFile(cfgGrammarFileName.c_str());
        if (! cfgGrammarFile) {
            throw std::runtime_error("Cannot open " + cfgGrammarFileName);
        }

        std::cerr << "Loading CFG grammar from " << cfgGrammarFileName << " .. ";
        _cfgGrammar.load(cfgGrammarFile);
        std::cerr << "done" << std::endl;

        if (! _actionList.init(*grammar, _cfgGrammar)) {
            throw std::runtime_error("Cannot get schema names");
        }

        // _guide = new ParserGuide(_cfgGrammar);
    }

    ~EventExtractorImpl(void)
    {
        delete _fset;
        // delete _guide;
    }

    bool getTerminals(lilfes::FSP derivation, std::vector<LexEnt> &ts)
    {
        lilfes::FSP tsFsp(_mach);
        if (! up::call_proc(_mach, _extract_terminals, derivation, tsFsp)) {
            return false;
        }

        typedef std::vector<std::pair<lilfes::FSP, lilfes::FSP> > PairVec;
        PairVec pairs;
        if (! lilfes::list_to_vector(*_mach, tsFsp, pairs)) {
            throw std::runtime_error("extract_terminals output type error");
        }

        ts.clear();
        ts.resize(pairs.size());

        for (unsigned i = 0; i < ts.size(); ++i) {
            if (! pairs[i].second.IsString()) {
                throw std::runtime_error("lex_template must be a string");
            }
            _grammar->makeVecWord(pairs[i].first, ts[i]._word);
            ts[i]._lexTemplate = pairs[i].second.ReadString();
            ts[i]._word_id = i;
            ts[i]._begin = i;
            ts[i]._end = i + 1;
            ts[i]._fom = 0;
        }

        return true;
    }

    void getActionSequence(lilfes::FSP derivation, std::vector<const ParserAction*> &actions)
    {
        lilfes::FSP actionsFsp(_mach);
        if (! up::call_proc(_mach, _extract_action_sequence, derivation, actionsFsp)) {
            throw std::runtime_error("extract_action_sequence failed");
        }

        std::vector<lilfes::FSP> actionNames;
        if (! lilfes::list_to_vector(*_mach, actionsFsp, actionNames)) {
            throw std::runtime_error("extract_action_sequence output type error");
        }

        actions.clear();
        actions.resize(actionNames.size(), 0);

        for (unsigned i = 0; i < actionNames.size(); ++i) {
            if (! actionNames[i].IsString()) {
                throw std::runtime_error("extract_action_sequence' output includes non-string");
            }

            std::string name = actionNames[i].ReadString();
            const ParserAction *a = _actionList.find(name);
            if (a == 0) {
                throw std::runtime_error("unknown action name: " + name);
            }

            actions[i] = a;
        }
    }

    void getPossibleActions(const ParserState &state, std::vector<const ParserAction*> &options)
    {
        options.clear();
        for (ParserActionList::const_iterator a = _actionList.begin(); a != _actionList.end(); ++a) {
            // if (_guide->accept((*a)->getCode()) && (*a)->applicable(grammar, state)) {
            if ((*a)->applicable(*_grammar, state)) {
                options.push_back(*a);
            }
        }
    }

    Grammar &getGrammarRef(void) { return *_grammar; }

    lilfes::machine *getMachine(void) { return _mach; }

    // do what happens in ParserBasic::parseLexEntLattice
    void initParserState(const std::vector<LexEnt> &terminals)
    {
        brackets.clear();
        sentence_length = terminals.size();

        lexent_lattice.clear();
        lexent_lattice.resize(sentence_length);

        word_chart.clear();
        word_chart.resize(sentence_length, std::vector<std::vector<unsigned> >(sentence_length + 1));

        for (unsigned i = 0; i < terminals.size(); ++i) {
            assert(i == terminals[i]._word_id);
            lexent_lattice[i].word = terminals[i]._word;
            lexent_lattice[i].tmpls.push_back(up::LexTemplateFom(terminals[i]._lexTemplate, 0));
            word_chart[i][i+1].push_back(i);
        }
    }

private:
    // dummy: never called
    void getEdges(unsigned, unsigned, std::vector<up::eserial>&) const {}
    bool parseImpl(void) { return false; }

public:
    lilfes::machine *_mach;
    mogura::Grammar *_grammar;
    cfg::Grammar _cfgGrammar;
    FeatureExtractor *_fset;
    ParserActionList _actionList;
    // ParserGuide *_guide;

    lilfes::procedure *_extract_terminals;
    lilfes::procedure *_extract_action_sequence;
};

void printEvent(
    std::ostream &out,
    const std::string &key,
    const ParserAction *correctAction,
    const std::vector<const ParserAction*> &options,
    const std::vector<std::string> &fv
) {
    out << key << ' ' << correctAction->asString();
    for (std::vector<const ParserAction*>::const_iterator a = options.begin(); a != options.end(); ++a) {
        if (*a != correctAction) {
            out << ' ' << (*a)->asString();
        }
    }
    out << ' ' << '/';

    for (std::vector<std::string>::const_iterator f = fv.begin(); f != fv.end(); ++f) {
        out << ' ' << *f;
    }

    out << std::endl;
}

//bool initGuide(const ParserState &state, ParserGuide &guide)
//{
//    std::vector<std::string> ts(state._input.size());
//    for (unsigned i = 0; i < ts.size(); ++i) {
//        ts[i] = state._input[i]._lexTemplate;
//    }
//
//    return guide.init(ts);
//}

EventExtractor::EventExtractor(lilfes::machine *mach, mogura::Grammar *grammar)
    : _impl(new EventExtractorImpl(mach, grammar))
{}

EventExtractor::~EventExtractor(void)
{
    delete _impl;
}

void EventExtractor::extractEvent(lilfes::FSP derivation, std::ostream &out, std::ostream &log)
{
    lilfes::IPTrailStack iptrail(_impl->getMachine());

    mogura::Grammar &grammar = _impl->getGrammarRef();

    std::vector<LexEnt> terminals;
    if (! _impl->getTerminals(derivation, terminals)) {
        log << "out-of-grammar lexent" << std::endl;
        return;
    }
    _impl->initParserState(terminals);

    std::vector<const ParserAction*> actions;
    _impl->getActionSequence(derivation, actions);

    ParserState state;
    state._input = terminals;
    if (! state.init(grammar)) {
        log << "lex lookup failed" << std::endl;
        return;
    }

    // if (! initGuide(state, *_impl->_guide)) {
    //    log << "initialization of parser guide failed" << std::endl;
    //    return;
    // }

    for (std::vector<const ParserAction*>::const_iterator a = actions.begin(); a != actions.end(); ++a) {
        // 2009-09-07: use all actions as (negative) examples
        std::vector<const ParserAction*> options(_impl->_actionList.begin(), _impl->_actionList.end());
        // std::vector<const ParserAction*> options;
        // _impl->getPossibleActions(state, options);

        if (std::find(options.begin(), options.end(), *a) == options.end()) {
            log << "specified action \'" << (*a)->asString() << "\' is not applicable" << std::endl;
            return;
        }

        std::vector<std::string> fv;
        _impl->_fset->extract(state, fv);

        if (options.size() > 1) {
            printEvent(out, state.getKey(grammar), *a, options, fv);
        }

        if (! (*a)->apply(grammar, state)) {
            // 2009-09-03: action a is not necessarily valid
            log << "specified action \'" << (*a)->asString() << "\' is not applicable" << std::endl;
            return;
            // throw std::logic_error("application of " + (*a)->asString() + " was failed");
        }
    }

    if (! state.isComplete(grammar)) {
        log << "warn: derivation does not a complete tree" << std::endl;
    }
}

//------------------------------------------------------------------------------
} /// namespace mogura {
//------------------------------------------------------------------------------
