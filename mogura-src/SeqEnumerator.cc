#include <queue>
#include <stack>
#include <cmath>
#include <map>
#include <limits>

#include "SeqEnumerator.h"
#include "ChunkFinder.h"
#include "mayzutil.h"

#ifdef _MSC_VER
#undef max
#endif

//------------------------------------------------------------------------------
namespace mogura {
namespace cfg {
namespace seq_enum {
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// type, struct defs
//------------------------------------------------------------------------------
typedef SeqEnumerator::TemplateSeq TemplateSeq;
typedef SeqEnumerator::LeafPtr LeafPtr;

struct ScoredTmplSeq {
    TemplateSeq _seq;
    double _score;

    ScoredTmplSeq(void) {}
    ScoredTmplSeq(const TemplateSeq &seq, double score)
        : _seq(seq)
        , _score(score)
    {}
};

struct EdgeSet;

struct Edge { 
public:
    Edge(void) {}

    Edge(EdgeSet *left, EdgeSet *right, double score)
    {
        _left  = left;
        _leOrRight._right = right;
        _score = score;
    }

    //Edge(unsigned lexIndex, double score)
    //{
    //    _left = 0;
    //    _leOrRight._leIx = lexIndex;
    //    _score = score;
    //}
    Edge(const LeafPtr &leafPtr, double score)
    {
        _left = 0;
        _leOrRight._leafPtr._extentIx = leafPtr._extentIx;
        _leOrRight._leafPtr._templateIx = leafPtr._templateIx;
        _score = score;
    }

    EdgeSet *getLeft(void) const { return _left; }
    EdgeSet *getRight(void) const { return _leOrRight._right; }
    bool isUnary(void) const { return _leOrRight._right == 0; }
    bool isBinary(void) const { return _leOrRight._right != 0; }

    bool isTerminal(void) const { return _left == 0; }
    //unsigned getLexIndex(void) const { return _leOrRight._leIx; }
    LeafPtr getLeafPtr(void) const { return LeafPtr(_leOrRight._leafPtr._extentIx, _leOrRight._leafPtr._templateIx); }

    double getScore(void) const { return _score; }

private:
    struct LeafPtrData {
        unsigned _extentIx;
        unsigned _templateIx;
    };

    EdgeSet *_left;
    union {
        EdgeSet *_right;
        //unsigned _leIx;
        LeafPtrData _leafPtr;
    } _leOrRight;
    double _score;
};

struct EnumQueueElem {
    double _score;

    /// Resize of EdgeSet._es is forbidden while enumeration.
    /// Note that EdgeSet::getNthSeq(n=0, s) does not touch the
    /// EnumQueue and hence the parsing can be continued after the retrieval
    /// of the first tmpl sequence.
    Edge *_e;

    unsigned _leftIx;
    unsigned _rightIx;

    EnumQueueElem(void) {}
    EnumQueueElem(double s, Edge *e, unsigned leftIx, unsigned rightIx)
        : _score(s)
        , _e(e)
        , _leftIx(leftIx)
        , _rightIx(rightIx)
    {}
};

inline
bool operator<(const EnumQueueElem &e1, const EnumQueueElem &e2)
{
    return e1._score < e2._score;
}

struct CompEnumQueueElem {
    bool operator()(const EnumQueueElem &e1, const EnumQueueElem &e2) const
    {
        return e1 < e2;
    }
};

struct Enumerator {
    std::vector<ScoredTmplSeq> _ordered;
    std::priority_queue<EnumQueueElem, std::vector<EnumQueueElem>, CompEnumQueueElem> _queue;
    bool _queueIsFilled;

    Enumerator(void) : _queueIsFilled(false) {}
};

struct EdgeSet {
    unsigned _sym;
    std::vector<Edge> _es;

    EdgeSet(void) : _enum(0) {}
    ~EdgeSet(void) { delete _enum; }

    bool getNthSeq(unsigned n, TemplateSeq &s)
    {
        if (search(n)) {
            s = _enum->_ordered[n]._seq;
            return true;
        }
        else {
            return false;
        }
    }

    bool getNthScore(unsigned n, double &score)
    {
        if (search(n)) {
            score = _enum->_ordered[n]._score;
            return true;
        }
        else {
            return false;
        }
    }

private:

    bool search(unsigned n)
    {
        if (_enum == 0) {
            _enum = new Enumerator();
            setFirst();
        }

        if (n > 0 && ! _enum->_queueIsFilled) {
            fillQueue();
        }

        while (_enum->_ordered.size() <= n) {
            
            if (_enum->_queue.empty()) {
                return false;
            }

            EnumQueueElem qe = _enum->_queue.top();
            _enum->_queue.pop();

            TemplateSeq seq;
            makeSeq(*qe._e, qe._leftIx, qe._rightIx, seq);

            if (! isSeen(seq, qe._score)) {
                _enum->_ordered.push_back(ScoredTmplSeq(seq, qe._score));
            }

            pushSuccessors(qe);
        }

        return true;
    }

private:

    /// Condition: _es is sorted by best score of the edge and thus links 
    ///    do not form a loop.
    void setFirst(void)
    {
        assert(! _es.empty());

        Edge &e0 = _es.front();

        TemplateSeq s;
        makeSeq(e0, 0, 0, s);

        _enum->_ordered.push_back(ScoredTmplSeq(s, e0.getScore()));
    }

    /// Note: putting _es.front() with leftIx = rightIx = 0 is redundant,
    /// but it makes the code simple.
    void fillQueue(void)
    {
        _enum->_queueIsFilled = true;

        typedef std::vector<Edge>::iterator EdgeItr;
        for (EdgeItr e = _es.begin(); e != _es.end(); ++e) {

            double score;

            if (e->isTerminal()) {
                score = e->getScore();
            }
            else {
                double lscore;
                bool ok = e->getLeft()->getNthScore(0, lscore);
                assert(ok);

                double rscore = 0;
                if (e->isBinary()) {
                    ok = e->getRight()->getNthScore(0, rscore);
                    assert(ok);
                }

                score = lscore + rscore;
            }

            _enum->_queue.push(EnumQueueElem(score, &*e, 0, 0));
        }
    }

    void makeSeq(Edge &e, unsigned leftIx, unsigned rightIx, TemplateSeq &s)
    {
        if (e.isTerminal()) {
            s = TemplateSeq(1, e.getLeafPtr());
        }
        else {

            bool ok = e.getLeft()->getNthSeq(leftIx, s);
            assert(ok);

            if (e.isBinary()) {
                TemplateSeq t;
                ok = e.getRight()->getNthSeq(rightIx, t);
                assert(ok);

                s.insert(s.end(), t.begin(), t.end());
            }
        }
    }

    bool isSeen(const TemplateSeq &s, double score) const
    {
        static const double small = 1e-5;

        double limit = score + small;

        typedef std::vector<ScoredTmplSeq>::const_reverse_iterator SItr;
        SItr rend = _enum->_ordered.rend();
        for (SItr it = _enum->_ordered.rbegin(); it != rend && it->_score <= limit; ++it) {
            if (it->_seq == s) {
               return true;
            }
        }

        return false;
    }
    
    void pushSuccessors(const EnumQueueElem &qe)
    {
        if (qe._e->isTerminal()) {
            return; /// terminal has no successors
        }
        else if (qe._e->isUnary()) {
            double score;
            unsigned nextIx = qe._leftIx + 1;
            if (qe._e->getLeft()->getNthScore(qe._leftIx + 1, score)) {
                _enum->_queue.push(EnumQueueElem(score, qe._e, nextIx, 0));
            }
        }
        else { /// binary
            double lscore;
            double rscore;

            if (qe._e->getLeft()->getNthScore(qe._leftIx + 1, lscore)) {
                
                bool ok = qe._e->getRight()->getNthScore(qe._rightIx, rscore);
                assert(ok);

                _enum->_queue.push(EnumQueueElem(
                    lscore + rscore, qe._e, qe._leftIx + 1, qe._rightIx));
            }

            if (qe._leftIx == 0 && qe._e->getRight()->getNthScore(qe._rightIx + 1, rscore)) {

                bool ok = qe._e->getLeft()->getNthScore(qe._leftIx, lscore);
                assert(ok);

                _enum->_queue.push(EnumQueueElem(lscore + rscore, qe._e, qe._leftIx, qe._rightIx + 1));
            }
        }
    }

private:
    Enumerator *_enum;
};

typedef std::map<unsigned, EdgeSet> Cell;

typedef std::vector<std::vector<Cell> > Chart;

/// Memo: a possible alternative method is storing all QueueItems in
/// the chart and putting pointers to them in the queue
struct QueueElem {
    unsigned _sym;
    unsigned _start;
    unsigned _end;
    Edge _edge;

    QueueElem(void) {}
    QueueElem(unsigned sym, unsigned start, unsigned end, const Edge &e)
        : _sym(sym)
        , _start(start)
        , _end(end)
        , _edge(e)
    {}
};

//------------------------------------------------------------------------------
// Subroutines for debug
//------------------------------------------------------------------------------
void dump(const QueueElem &e)
{
    std::cerr
        << "score=" << e._edge.getScore()
        << ", left=" << e._edge.getLeft()
        << ", right=" << e._edge.getRight()
        << ", leafPtr=" << e._edge.getLeafPtr()._extentIx << ":"
                        << e._edge.getLeafPtr()._templateIx
        << ", sym=" << e._sym 
        << ", start=" << e._start
        << ", end=" << e._end
        << std::endl;
}

void dump(QueueElem &e, const Grammar &g)
{
    std::cerr
        << "score=" << e._edge.getScore()
        << ", left=" << e._edge.getLeft()
        << ", right=" << e._edge.getRight()
        << ", leafPtr=" << e._edge.getLeafPtr()._extentIx << ":"
                        << e._edge.getLeafPtr()._templateIx
        << ", sym=" << e._sym << "(" << g._nonterm.getSymbol(e._sym) << ")"
        << ", start=" << e._start
        << ", end=" << e._end
        << std::endl;
}

//------------------------------------------------------------------------------
// Agenda
//------------------------------------------------------------------------------
inline
bool operator<(const QueueElem &e1, const QueueElem &e2)
{
    return e1._edge.getScore() < e2._edge.getScore();
}

struct CompQueueElem {

    bool operator()(const QueueElem &e1, const QueueElem &e2) const
    {
        return e1 < e2;
    }
};

//#define ITER_DEEP 1
#ifdef ITER_DEEP
// Iterative deepening version
// ~5% reduction of cfg parsing time with delta=-6
class Queue {
public:
    Queue(void) : _limit(0), _delta(-6) {}

    void push(const QueueElem &e)
    {
        if (e._edge.getScore() >= _limit) {
            _queue.push(e);
        }
        else {
            _wait.push_back(e);
        }
    }

    const QueueElem &top(void)
    {
        if (_queue.empty()) {
            promote();
        }
        return _queue.top();
    }

    void pop(void)
    {
        if (_queue.empty()) {
            promote();
        }
        _queue.pop();
    }

    bool empty(void) const { return _queue.empty() && _wait.empty(); }

    void clear(void)
    {
        while (! _queue.empty()) {
            _queue.pop();
        }
        _wait.clear();
        _limit = 0;
    }

private:
    void promote(void)
    {
        _limit += _delta;
        std::vector<QueueElem> nexts;
        for (std::vector<QueueElem>::const_iterator e = _wait.begin(); e != _wait.end(); ++e) {
            if (e->_edge.getScore() >= _limit) {
                _queue.push(*e);
            }
            else {
                nexts.push_back(*e);
            }
        }
        _wait.swap(nexts);
    }

private:
    double _limit;
    double _delta;
    std::priority_queue<QueueElem, std::vector<QueueElem>, CompQueueElem> _queue;
    std::vector<QueueElem> _wait;
};

#else

/// Simple best-first version
class Queue {
public:
    Queue(void) {}

    void push(const QueueElem &e) { _queue.push(e); }
    const QueueElem &top(void) { return _queue.top(); }
    void pop(void) { _queue.pop(); }
    bool empty(void) const { return _queue.empty(); }

    void clear(void)
    {
        while (! _queue.empty()) {
            _queue.pop();
        }
    }

private:
    std::priority_queue<QueueElem, std::vector<QueueElem>, CompQueueElem> _queue;
};
#endif


//------------------------------------------------------------------------------
// routines for best-first parsing
//------------------------------------------------------------------------------

struct LessFom {
    bool operator()(const up::LexTemplateFom &t1, const up::LexTemplateFom &t2) const
    {
        return t1.fom < t2.fom;
    }
};

enum LatticeType {
    LT_NOT_CONNECTED,
    LT_LINEAR_CHAIN,
    LT_TRUE_LATTICE
};

// lattice may not be connected. we don't check it here
LatticeType checkLatticeType(const up::LexEntLattice &lattice)
{
    std::vector<unsigned> numCovered;
    for (up::LexEntLattice::const_iterator it = lattice.begin(); it != lattice.end(); ++it) {
        if (it->end <= it->begin) {
            continue;
        }
        if (numCovered.size() < it->end) {
            numCovered.resize(it->end, false);
        }
        for (unsigned i = it->begin; i < it->end; ++i) {
            ++numCovered[i];
        }
    }

    LatticeType type = LT_LINEAR_CHAIN;
    for (std::vector<unsigned>::const_iterator c = numCovered.begin(); c != numCovered.end(); ++c) {
        if (*c == 0) {
            return LT_NOT_CONNECTED;
        }
        else if (*c > 1) {
            type = LT_TRUE_LATTICE;
        }
    }
    return type;
}

unsigned getSentecneLength(const up::LexEntLattice &lattice)
{
    unsigned last = 0;
    for (up::LexEntLattice::const_iterator it = lattice.begin(); it != lattice.end(); ++it) {
        last = std::max(last, it->end);
    }
    return last;
}

bool initQueue(
    const Grammar &g,
    const up::LexEntLattice &input,
    double lexBeta, // = log(threshold) < 0
    Queue &queue,
    double &baseScore
) {
    LatticeType latticeType = checkLatticeType(input);
    if (latticeType == LT_NOT_CONNECTED) {
        return false;
    }

    bool useScoreShift = (latticeType == LT_LINEAR_CHAIN);

    baseScore = 0;

    for (unsigned i = 0; i < input.size(); ++i) {

        if (input[i].tmpls.empty()) {
            continue;
        }

        std::vector<up::LexTemplateFom>::const_iterator max
            = std::max_element(input[i].tmpls.begin(), input[i].tmpls.end(), LessFom());

        if (useScoreShift) {
            baseScore += max->fom;
        }

        for (unsigned j = 0; j < input[i].tmpls.size(); ++j) {

            const up::LexTemplateFom &tmpl = input[i].tmpls[j];

            if (tmpl.fom < max->fom + lexBeta) {
                continue;
            }

            int lexId = g._lexent.getID(tmpl.tmpl, -1);
            if (lexId < 0) {
                /// unknown lex item
                continue;
            }

            Grammar::LexTable::const_iterator it = g._lexToNonterm.find(lexId);
            if (it == g._lexToNonterm.end()) {
                /// unreachable lex item
                continue;
            }

            double score = useScoreShift ? (tmpl.fom - max->fom) : tmpl.fom;
            queue.push(QueueElem(it->second, input[i].begin, input[i].end, Edge(LeafPtr(i, j), score)));
        }
    }

    return true;
}

inline
void applyBinary(
    const Grammar &g,
    EdgeSet *left,
    EdgeSet *right,
    unsigned start,
    unsigned end,
    Queue &queue
) {
    // Grammar::BinaryTable::const_iterator it = g._binary.find(std::make_pair(left->_sym, right->_sym));
    //
    // if (it == g._binary.end()) {
    //    return;
    // }
    ParentSet ms;
    g._binary.find(left->_sym, right->_sym, ms);
    if (ms.empty()) {
        return;
    }

    double score = left->_es.front().getScore() + right->_es.front().getScore();

    QueueElem e;
    e._start = start;
    e._end = end;
    e._edge = Edge(left, right, score);

    // const ParentSet &ms = it->second;
    for (ParentSet::const_iterator m = ms.begin(); m != ms.end(); ++m) {
        e._sym = m->_mot;
        queue.push(e);
    }
}

inline
void applyUnary(
    const Grammar &g,
    EdgeSet *dtr,
    unsigned start,
    unsigned end,
    Queue &queue
) {
    Grammar::UnaryTable::const_iterator it = g._unary.find(dtr->_sym);

    if (it == g._unary.end()) {
        return;
    }

    assert(! dtr->_es.empty());

    QueueElem e;
    e._start = start;
    e._end = end;
    e._edge = Edge(dtr, 0, dtr->_es.front().getScore());

    const ParentSet &ms = it->second;
    for (ParentSet::const_iterator m = ms.begin(); m != ms.end(); ++m) {
        e._sym = m->_mot;
        queue.push(e);
    }
}

static const double SMALL = 1e-5;

void addItem(
    const Grammar &g,
    Chart &chart,
    const QueueElem &qe,
    Queue &queue,
    unsigned startSymbol
) {
    EdgeSet &eset = chart[qe._start][qe._end][qe._sym];

    if (! eset._es.empty()) { /// this edge set is already found.
        // assert(eset._es.back().getScore() + SMALL >= qe._edge.getScore());
        eset._es.push_back(qe._edge);
        return;
    }

    eset._sym = qe._sym;
    eset._es.push_back(qe._edge);

    /// binary, as left dtr
    for (unsigned end = qe._end + 1; end <= chart.size(); ++end) {
        Cell &c = chart[qe._end][end];
        for (Cell::iterator right = c.begin(); right != c.end(); ++right) {
            applyBinary(g, &eset, &(right->second), qe._start, end, queue);
        }
    }

    /// binary, as right dtr
    for (unsigned start = 0; start < qe._start; ++start) {
        Cell &c = chart[start][qe._start];
        for (Cell::iterator left = c.begin(); left != c.end(); ++left) {
            applyBinary(g, &(left->second), &eset, start, qe._end, queue);
        }
    }

    applyUnary(g, &eset, qe._start, qe._end, queue);

    if (g._root.find(qe._sym) != g._root.end()) {
        QueueElem e;
        e._start = qe._start;
        e._end = qe._end;
        e._edge = Edge(&eset, 0, eset._es.front().getScore());
        e._sym = startSymbol;
        queue.push(e);
    }
}

//------------------------------------------------------------------------------
// Class implementation
//------------------------------------------------------------------------------

/// Find parsable lex entry sequences that have scores within
/// (score-of-top) + beta
/// Possible alternative: use beta * input-size
class SeqEnumeratorImpl {
public:

    SeqEnumeratorImpl(
        const Grammar *grammar,           /// != 0
        const Grammar *restrictedGrammar, /// maybe = 0
        double lexThreshold,
        double seqThreshold,
        unsigned maxQueueElem
    )
        : _grammar(grammar)
        , _restrictedGrammar(restrictedGrammar)
        , _chartIsFilled(false)
        , _lexAnalysisError(false)
    {
        _startSymbol = std::numeric_limits<unsigned>::max();
        // _beta = std::log(1e-2); /// this should be configurable
        setLexThreshold(lexThreshold);
        setSeqThreshold(seqThreshold);
        setMaxQueueItem(maxQueueElem);
    }

    bool getFirst(
        const up::LexEntLattice &input,
        TemplateSeq &seq
    ) {
        unsigned length = getSentecneLength(input);

        _useRestricted = true;
        if (_restrictedGrammar && getFirstImpl(_restrictedGrammar, input, length, seq)) {
            return true;
        }

        _useRestricted = false;
        if (getFirstImpl(_grammar, input, length, seq)) {
            return true;
        }

        return false;
    }

    bool getNext(TemplateSeq &seq)
    {
        if (_useRestricted) {
            assert(_restrictedGrammar != 0);
            return getNextImpl(_restrictedGrammar, seq);
        }
        else {
            return getNextImpl(_grammar, seq);
        }
    }

    struct ChunkFinder : public ChunkFinderBase<Cell, EdgeSet> {
        const EdgeSet *findBestEdgeSet(const Cell &c) const
        {
            assert(! c.empty());

            const EdgeSet *e = &(c.begin()->second);
            double maxScore = getEdgeSetScore(e);

            for (Cell::const_iterator it = c.begin(); it != c.end(); ++it) {
                double score = getEdgeSetScore(&(it->second));
                if (score > maxScore) {
                    maxScore = score;
                    e = &(it->second);
                }
            }

            return e;
        }

        virtual double getEdgeSetScore(const EdgeSet *e) const
        {
            return e->_es.front().getScore();
        }
    };

    bool findBestFailSeq(TemplateSeq &seq)
    {
        if (_lexAnalysisError) {
            return false;
        }

        // Make the fragments connected
        while (! _queue.empty()) {
            if (_queue.top()._edge.isTerminal()) {
                QueueElem qe = _queue.top();
                EdgeSet &eset = _chart[qe._start][qe._end][qe._sym];
                if (eset._es.empty()) {
                    eset._es.push_back(qe._edge);
                }
            }
            _queue.pop();
        }

        std::vector<ChunkFinder::Chunk> chunks;
        ChunkFinder().findBestChunkSeq(_chart, chunks);

        seq.clear();
        for (std::vector<ChunkFinder::Chunk>::const_iterator it = chunks.begin(); it != chunks.end(); ++it) {
            appendBestSeq(seq, it->_es);
        }

        //assert(seq.size() == _chart.size());

        return true;
    }

    void setSeqThreshold(double threshold)
    {
        _seqBeta = std::log(threshold);
    }

    void setLexThreshold(double threshold)
    {
        _lexBeta = std::log(threshold);
    }

    void setMaxQueueItem(unsigned max)
    {
        _maxQueueElem = max;
    }

private:

    void clear(void)
    {
        _chart.clear();
        _queue.clear();
    }

    bool isRoot(unsigned len, const QueueElem &e)
    {
        return e._start == 0
            && e._end == len
            && e._sym == _startSymbol;
    }

    void appendBestSeq(TemplateSeq &s, const EdgeSet *es) const
    {
        assert(! es->_es.empty());

        const Edge &e = es->_es.front();

        if (e.isTerminal()) {
            s.push_back(e.getLeafPtr());
        }
        else if (e.isUnary()) {
            appendBestSeq(s, e.getLeft());
        }
        else { /// binary
            appendBestSeq(s, e.getLeft());
            appendBestSeq(s, e.getRight());
        }
    }

    bool getFirstImpl(
        const Grammar *grammar,
        const up::LexEntLattice &input,
        unsigned length,
        TemplateSeq &seq
    ) {
        clear();

        _numQueueElem = 0;
        _chartIsFilled = false;
        _lexAnalysisError = false;

        if (! initQueue(*grammar, input, _lexBeta, _queue, _baseScore)) {
            _lexAnalysisError = true;
            return false;
        }

        _chart.resize(length, std::vector<Cell>(length + 1));

        bool found = false;

        while (! _queue.empty()) {
            QueueElem e = _queue.top();
            _queue.pop();

            addItem(*grammar, _chart, e, _queue, _startSymbol);

            if (isRoot(length, e)) {
                found = true;
                _limit = e._edge.getScore() + _seqBeta;
                break;
            }

            if (++_numQueueElem > _maxQueueElem) {
                return false;
            }
        }

        if (found) {
            EdgeSet &root = (_chart.front().back())[_startSymbol];
            bool ok = root.getNthSeq(0, seq);
            assert(ok);
            _seqIx = 1;
            return true;
        }
        else {
            return false;
        }
    }

    bool getNextImpl(const Grammar *grammar, TemplateSeq &seq)
    {
        if (_lexAnalysisError) {
            return false;
        }

        if (! _chartIsFilled) {
            _chartIsFilled = true;

            while (! _queue.empty()) {
                QueueElem e = _queue.top();
                _queue.pop();

                if (e._edge.getScore() < _limit) {
                    break;
                }

                addItem(*grammar, _chart, e, _queue, _startSymbol);

                if (++_numQueueElem > _maxQueueElem) {
                    break; // not return: new sequences may be found
                }
            }
        }

        EdgeSet &root = (_chart.front().back())[_startSymbol];

        double score;
        if (! root.getNthScore(_seqIx, score) || score < _limit) {
            return false;
        }

        bool ok = root.getNthSeq(_seqIx, seq);
        assert(ok);
        _seqIx++;

        return true;
    }

private:
    const Grammar *_grammar;
    const Grammar *_restrictedGrammar;

    bool _useRestricted;

    unsigned _startSymbol;
    Chart _chart;
    Queue _queue;
    double _baseScore;
    unsigned _seqIx;
    double _lexBeta;
    double _seqBeta;
    double _limit;
    bool _chartIsFilled;
    bool _lexAnalysisError;

    // 2009-06-17: to avoid unpredictably large memory consumption
    unsigned _maxQueueElem;
    unsigned _numQueueElem;
};

//------------------------------------------------------------------------------
// public interface
//------------------------------------------------------------------------------
//
/// rg = 0 -> without thomazo's optimization
SeqEnumerator::SeqEnumerator(
    const Grammar *g,
    const Grammar *rg,
    double lexThreshold,
    double seqThreshold,
    unsigned maxQueueElem)
    : _impl(new SeqEnumeratorImpl(g, rg, lexThreshold, seqThreshold, maxQueueElem))
{}

SeqEnumerator::~SeqEnumerator(void)
{
    delete _impl;
}

bool SeqEnumerator::getFirst(const up::LexEntLattice &input, TemplateSeq &seq)
{
    up::ScopeProf prof("mogura::SeqEnumerator::getFirst");
    return _impl->getFirst(input, seq);
}

bool SeqEnumerator::getNext(TemplateSeq &seq)
{
    up::ScopeProf prof("mogura::SeqEnumerator::getNext");
    return _impl->getNext(seq);
}

bool SeqEnumerator::findBestFailSeq(TemplateSeq &seq) 
{
    up::ScopeProf prof("mogura::SeqEnumerator::findBestFailSeq");
    return _impl->findBestFailSeq(seq);
}

void SeqEnumerator::setSeqThreshold(double threshold)
{
    _impl->setSeqThreshold(threshold);
}

void SeqEnumerator::setLexThreshold(double threshold)
{
    _impl->setLexThreshold(threshold);
}

void SeqEnumerator::setMaxQueueItem(unsigned max)
{
    _impl->setMaxQueueItem(max);
}

////////////////////////////////////////////////////////////////////////////////
} // namespace seq_enum
} // namespace cfg
} // namespace mogura
////////////////////////////////////////////////////////////////////////////////
