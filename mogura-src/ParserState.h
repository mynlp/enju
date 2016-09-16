#ifndef ParserState_h__
#define ParserState_h__

#include "Extent.h"
#include "MoguraGrammar.h"
#include "Schema.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace mogura {

struct LexEnt {
    unsigned _word_id; // index in lexent_lattice
    unsigned _begin;
    unsigned _end;
    std::vector<std::string> _word;
    std::string _lexTemplate;
    double _fom;

    LexEnt(void) {}
    LexEnt(unsigned word_id, unsigned begin, unsigned end,
           const std::vector<std::string> &word, const std::string &lexTemplate, double fom)
        : _word_id(word_id)
        , _begin(begin)
        , _end(end)
        , _word(word)
        , _lexTemplate(lexTemplate)
        , _fom(fom)
    {}

    // for debug
    std::string asString(void) const
    {
        std::ostringstream oss;
        std::copy(_word.begin(), _word.end(), std::ostream_iterator<std::string>(oss, "/"));
        oss << _lexTemplate << "@[" << _begin << "," << _end << "):" << _fom;
        return oss.str();
    }
};

struct ParseTree {
    enum Direction { LEFT, RIGHT };

    lilfes::FSP _sign;
    std::string _signLabel;
    const Schema *_schema;
    const LexEnt *_lexHead;
    ParseTree *_leftDtr;
    ParseTree *_rightDtr;
    unsigned _numDep;
    ParseTree *_leftRecentDep;
    ParseTree *_rightRecentDep;

    const LexEnt *_leftBound;
    const LexEnt *_rightBound;

    double _fom;

    bool initLex(Grammar &g, const LexEnt *lex)
    {
        _schema = 0;
        _lexHead = lex;
        _leftDtr = 0;
        _rightDtr = 0;
        _numDep = 0;
        _leftRecentDep = 0;
        _rightRecentDep = 0;

        _leftBound = lex;
        _rightBound = lex;

        _fom = lex->_fom;

        lilfes::FSP wordFSP = g.makeWordFSP(lex->_word, lex->_begin, lex->_end);
        lilfes::FSP lexid = g.makeLexicalEntryFSP(wordFSP, lex->_lexTemplate);

        _sign = lilfes::FSP(g.getMachine());
        if (! g.lexicalEntrySign(lexid, _sign)) {
            return false;
        }

        if (! g.getSignLabel(_sign, _signLabel)) {
            return false;
        }

        return true;
    }

    bool initUnary(Grammar &g, const Schema *schema, ParseTree *dtr, lilfes::FSP sign)
    {
        _sign = sign;
        _schema = schema;
        _lexHead = dtr->_lexHead;
        _leftDtr = dtr;
        _rightDtr = 0;
        _numDep = dtr->_numDep;
        _leftRecentDep = dtr->_leftRecentDep;
        _rightRecentDep = dtr->_rightRecentDep;

        _leftBound = dtr->_leftBound;
        _rightBound = dtr->_rightBound;

        _fom = dtr->_fom;

        if (! g.getSignLabel(_sign, _signLabel)) {
            return false;
        }

        return true;
    }

    bool initBinary(
        Grammar &g,
        const Schema *schema,
        ParseTree *left,
        ParseTree *right,
        lilfes::FSP sign,
        Direction headDir
    ) {
        _sign = sign;
        _schema = schema;
        _lexHead = left->_lexHead;
        _leftDtr = left;
        _rightDtr = right;

        _leftBound = left->_leftBound;
        _rightBound = right->_rightBound;

        if (headDir == LEFT) {
            _numDep = left->_numDep + 1;
            _leftRecentDep = left->_leftRecentDep;
            _rightRecentDep = right;
        }
        else {
            _numDep = right->_numDep + 1;
            _leftRecentDep = left;
            _rightRecentDep = right->_rightRecentDep;
        }

        _fom = left->_fom + right->_fom;

        if (! g.getSignLabel(_sign, _signLabel)) {
            return false;
        }

        return true;
    }

    ~ParseTree(void)
    {
        delete _leftDtr;
        delete _rightDtr;
    }

    const ParseTree *getDtr(Direction dir) const
    {
        return dir == LEFT ? _leftDtr : _rightDtr;
    }

    const ParseTree *getRecentDep(Direction dir) const
    {
        return dir == LEFT ? _leftRecentDep : _rightRecentDep;
    }

    // for debug
    std::string print(void) const
    {
        if (_leftDtr == 0 && _rightDtr == 0) {
            //return _lexHead->_word.getInput() + "/" + _lexHead->_lexTemplateName;
            return _lexHead->asString();
        }
        else if (_rightDtr == 0) {
            return "(" + _schema->getName() + " " + _leftDtr->print() + ")";
        }
        else {
            return "(" + _schema->getName() + " " + _leftDtr->print() + " " + _rightDtr->print() + ")";
        }
    }
    std::string printDeriv(void) const
    {
        if (_leftDtr == 0 && _rightDtr == 0) {
            return "[\"" + _lexHead->_lexTemplate + "\"]";
        }
        else if (_rightDtr == 0) {
            return "[" + _schema->getName() + ", " + _leftDtr->printDeriv() + "]";
        }
        else {
            return "[" + _schema->getName() + ", " + _leftDtr->printDeriv() + ", " + _rightDtr->printDeriv() + "]";
        }
    }
};

class Stack {
public:
    void push(ParseTree *t) { _v.push_back(t); }

    ParseTree *pop(void)
    {
        if (_v.empty()) {
            return 0;
        }
        else {
            ParseTree *t = _v.back();
            _v.pop_back();
            return t;
        }
    }

    const ParseTree *at(unsigned ix) const
    {
        return (ix >= _v.size()) ? 0 : _v[_v.size() - ix - 1];
    }

    ParseTree *at(unsigned ix)
    {
        return (ix >= _v.size()) ? 0 : _v[_v.size() - ix - 1];
    }

    void clear(void)
    {
        while (! empty()) {
            delete pop();
        }
    }

    bool empty(void) const { return _v.empty(); }

    unsigned size(void) const { return _v.size(); }

private:
    std::vector<ParseTree *> _v;
};

struct ParserState {
public:
    enum { MAX_UNARY_CHAIN_LEN = 5 };

public:
    unsigned _unaryChainLen;
    std::vector<LexEnt> _input;
    Stack _stack;
    Stack _words;

public:
    ~ParserState(void)
    {
        clear();
    }

    void clear(void)
    {
        _input.clear();
        _stack.clear();
        _words.clear();
        _unaryChainLen = 0;
    }

    bool init(Grammar &g)
    {
        _stack.clear();
        _words.clear();
        _unaryChainLen = 0;

        for (std::vector<LexEnt>::reverse_iterator it = _input.rbegin(); it != _input.rend(); ++it) {
            ParseTree *t = new ParseTree();
            if (! t->initLex(g, &*it)) {
                delete t;
                return false;
            }
            _words.push(t);
        }

        return true;
    }

    bool shift(void)
    {
        if (_words.empty()) {
            return false;
        }
        else {
            _unaryChainLen = 0;
            _stack.push(_words.pop());
            return true;
        }
    }

    bool reduceUnary(Grammar &g, const Schema *s)
    {
        if (_stack.empty() || _unaryChainLen > MAX_UNARY_CHAIN_LEN) {
            return false;
        }

        ParseTree *dtr = _stack.at(0);
        lilfes::FSP mot(g.getMachine());

        if (! g.applyIdSchemaUnary(s->getLilfesType(), dtr->_sign, mot)) {
            return false;
        }

        ParseTree *t = new ParseTree();
        if (! t->initUnary(g, s, dtr, mot)) { /// this should not happen
            delete t;
            return false;
        }

        ++_unaryChainLen;
        _stack.pop();
        _stack.push(t);

        return true;
    }

    bool reduceBinary(Grammar &g, const Schema *s)
    {
        if (_stack.size() < 2) {
            return false;
        }

        ParseTree *right = _stack.at(0);
        ParseTree *left = _stack.at(1);
        lilfes::FSP mot(g.getMachine());

        if (! g.applyIdSchemaBinary(s->getLilfesType(), left->_sign, right->_sign, mot)) {
            return false;
        }

        ParseTree *t = new ParseTree();
        ParseTree::Direction headDir = (s->getType() == Schema::LEFT_HEAD) ? ParseTree::LEFT : ParseTree::RIGHT;
        if (! t->initBinary(g, s, left, right, mot, headDir)) {
            delete t;
            return false;
        }

        _unaryChainLen = 0;
        _stack.pop();
        _stack.pop();
        _stack.push(t);

        return true;
    }

    bool isComplete(Grammar &g) const
    {
        lilfes::IPTrailStack iptrail(g.getMachine());

        return _words.empty()
            && _stack.size() == 1
            && g.rootSign(_stack.at(0)->_sign);
    }

    std::string getKey(Grammar &g) const
    {
        if (_stack.empty()) {
            return "~none~";
        }
        else {
            // return _stack.at(0)->_lexHead->_word.getPos();
            return _stack.at(0)->_lexHead->_word[ g.getModelKeyIndex() ];
        }
    }
};

} // namespace mogura {

#endif // ParserState_h__
