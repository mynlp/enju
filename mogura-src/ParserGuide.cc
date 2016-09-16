#include <set>

#include "ParserGuide.h"
#include "CfgParser.h"
#include "CfgGrammar.h"
#include "ChunkFinder.h"

#ifdef _MSC_VER
#undef max
#endif

//------------------------------------------------------------------------------
namespace mogura {
using cfg::Chart;
using cfg::Edge;
using cfg::EdgeSet;
using cfg::Cell;
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Set of positions in a packed parse forest
//------------------------------------------------------------------------------
class State {
private:

    /// Type of a node in a packed parse forest
	enum Position {
        LEFT  = 0,  /// Left dtr of a binary branch
        RIGHT = 1,  /// Right dtr of a binary branch
        UNARY = 2,  /// Unary dtr
        LEAF  = 3,  /// Leaf node
        ROOT  = 4   /// Root node
    };

    //-------------------------------------------------------------------------
    // A position in a packed parse forest and pointers to its parents
    //-------------------------------------------------------------------------
	struct StackSymbol {
		const Edge *_edge;
		Position _position;
		std::vector<unsigned> _prev;

		StackSymbol(void) {}
		StackSymbol(const Edge *edge, Position position)
			: _edge(edge)
			, _position(position)
		{}

        /// For debug
		void print(std::ostream &ost) const
		{
			ost << "prev=(";
			for (unsigned i = 0; i < _prev.size(); ++i) {
				ost << ' ' << _prev[i];
			}
			ost << " ), edge=" << _edge << ", pos=";

			switch(_position) {
				case LEFT:  ost << "LEFT";  break;
				case RIGHT: ost << "RIGHT"; break;
				case UNARY: ost << "UNARY"; break;
				case LEAF:  ost << "LEAF";  break;
				case ROOT:  ost << "ROOT";  break;
			}
		}
	};

	typedef std::vector<StackSymbol> Heap;

	typedef std::map<const Edge*, unsigned> StateIndex;

	typedef std::set<ActionCode> ActSet;

public:
	State(void)
		: _shift(ACT_SHIFT)
		, _none(ACT_NONE)
	{}

    void clear(void)
    {
        _heap.clear();
        _curr.clear();
        _nextAct.clear();
    }

	void init(const cfg::Grammar &g, const Cell &rootCell)
	{
		_heap.clear();
		_heap.push_back(StackSymbol(0, ROOT));

		_curr.clear();

		StateIndex seen;

		const Cell &c = rootCell;
		for (Cell::const_iterator eset = c.begin(); eset != c.end(); ++eset) {
			if (g._root.find(eset->first) != g._root.end()) {
				followLcPath(&eset->second, 0, _curr, seen);
			}
		}

		_nextAct.clear();
		_nextAct.insert(_shift);
	}

	bool feed(const ActionCode a)
	{
		if (a._type == ACT_OTHER) {
			return ! _nextAct.empty();
		}

		std::set<unsigned> next;
		StateIndex seen;

		_nextAct.clear();

		for (std::set<unsigned>::const_iterator i = _curr.begin(); i != _curr.end(); ++i) {
			if (match(_heap[*i], a)) {
				advance(*i, next, seen, _nextAct);
			}
		}

		next.swap(_curr);

		return ! _nextAct.empty();
	}

	bool accept(const ActionCode a) const
	{
		return _nextAct.find(a) != _nextAct.end();
	}

public:
    /// For debug
	void dump(void)
	{
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "(state)" << std::endl;
		for (unsigned i = 0; i < _heap.size(); ++i) {
			std::cout << "h[" << i << "] = ";
			_heap[i].print(std::cout);
			std::cout << std::endl;
		}
		std::cout << std::endl;
		std::cout << "curr =";
		for (std::set<unsigned>::const_iterator i = _curr.begin();
				i != _curr.end(); ++i) {
			std::cout << ' ' << *i;
		}
        std::cout << "----------------------------------------" << std::endl;
	}

    /// For debug
	void printExpectedActions(
		std::ostream &ost,
		const cfg::Grammar &g
	) const {
		for (ActSet::const_iterator a = _nextAct.begin();
				a != _nextAct.end(); ++a) {

			switch (a->_type) {
				case ACT_NONE:
					ost << " none";
					break;
				case ACT_SHIFT:
					ost << " shift";
					break;
				case ACT_REDUCE1:
					ost << ' ' << g._rule.getSymbol(a->_rule);
					break;
				case ACT_REDUCE2:
					ost << ' ' << g._rule.getSymbol(a->_rule);
					break;
				case ACT_OTHER:
					ost << " other";
					break;
			}
		}
	}

    /// For debug
	void printCurrNode(
		std::ostream &ost,
		const cfg::Grammar &
	) const {
		std::cout << "curr = {";
		for (std::set<unsigned>::const_iterator i = _curr.begin(); i != _curr.end(); ++i) {
			ost << " (";
			_heap[*i].print(ost);
			ost << ")";
		}
		std::cout << " }";
	}

private:
	bool match(const StackSymbol &s, const ActionCode a) const
	{
		return expectedAction(s) == a;
	}

	ActionCode expectedAction(const StackSymbol &s) const
	{
		switch (s._position) {
			case LEFT: return _none; /// error?
			case RIGHT: return ActionCode(ACT_REDUCE2, s._edge->_rule);
			case UNARY: return ActionCode(ACT_REDUCE1, s._edge->_rule);
			case LEAF: return _shift;
			case ROOT: return _none;
		}

		/// Never reached
		assert(false);
		return _none;
	}

    /// Follow left corner path
	void followLcPath(
		const EdgeSet *eset,
		unsigned prevIx,
		std::set<unsigned> &leaves,
		StateIndex &seen
	) {
		for (std::vector<Edge>::const_iterator e = eset->_elems.begin(); e != eset->_elems.end(); ++e) {

			StateIndex::iterator i = seen.find(&*e);
			if (i != seen.end()) {
				_heap[i->second]._prev.push_back(prevIx);
			}
			else {
				if (e->_left == 0) { /// leaf
					unsigned newIx = alloc(&*e, LEAF, prevIx);
					seen[&*e] = newIx;
					leaves.insert(newIx);
				}
				else {
					Position position = (e->_right == 0) ? UNARY : LEFT;
					unsigned newIx = alloc(&*e, position, prevIx);
					seen[&*e] = newIx;

					followLcPath(e->_left, newIx, leaves, seen);
				}
			}
		}
	}

	unsigned alloc(const Edge *e, Position p, unsigned prevIx)
	{
		unsigned newIx = alloc(e, p);
		_heap[newIx]._prev.push_back(prevIx);
		return newIx;
	}

	unsigned alloc(const Edge *e, Position p)
	{
		unsigned newIx = _heap.size();
		_heap.push_back(StackSymbol(e, p));
		return newIx;
	}

	void advance(
		unsigned ix,
		std::set<unsigned> &next,
		StateIndex &seen,
		ActSet &nextAct
	) {
		StackSymbol s = _heap[ix];

		switch (s._position) {
			case LEFT: {

				unsigned newIx = alloc(s._edge, RIGHT);
				_heap[newIx]._prev = s._prev;
				followLcPath(s._edge->_right, newIx, next, seen);
				nextAct.insert(_shift);
				return;
			}

			case RIGHT:
			case UNARY:
			case LEAF: {

				for (std::vector<unsigned>::const_iterator p = s._prev.begin(); p != s._prev.end(); ++p) {

					if (_heap[*p]._position == LEFT) {
						unsigned newIx = alloc(_heap[*p]._edge, RIGHT);
						_heap[newIx]._prev = _heap[*p]._prev;
						followLcPath(_heap[*p]._edge->_right, newIx, next, seen);
						nextAct.insert(_shift);
					}
					else {
						next.insert(*p);
						nextAct.insert(expectedAction(_heap[*p]));
					}
				}

				return;
			}

			case ROOT:
				nextAct.insert(_none);
				return;
		}

		/// Never reached
		assert(false);
		return;
	}

private:
	Heap _heap;
	std::set<unsigned> _curr;
	ActSet _nextAct;

	ActionCode _shift;
	ActionCode _none;
};

////////////////////////////////////////////////////////////////////////////////

struct ParserGuideImpl {
public:
	const cfg::Grammar *_g;
	const cfg::Grammar *_restricted; // maybe null

    const cfg::Grammar *_currentGrammar; // _g or _restricted

	Chart _chart;
	State _state;

public:
    ParserGuideImpl(const cfg::Grammar *g, const cfg::Grammar *restricted)
        : _g(g)
        , _restricted(restricted)
        , _currentGrammar(restricted)
    {}
};

////////////////////////////////////////////////////////////////////////////////

ParserGuide::ParserGuide(const cfg::Grammar *grammar, const cfg::Grammar *restrcted)
	: _impl(new ParserGuideImpl(grammar, restrcted))
{}

ParserGuide::~ParserGuide(void)
{
	delete _impl;
}

bool ParserGuide::init(const std::vector<std::string> &lexitems)
{
    // initialization of the chart will be done in parseCky()
	// unsigned len = lexitems.size();
	// _impl->_chart.clear();
	// _impl->_chart.resize(len, std::vector<Cell>(len + 1));

	if (_impl->_restricted && parseCky(*_impl->_restricted, lexitems, _impl->_chart)) {
        _impl->_currentGrammar = _impl->_restricted;
		_impl->_state.init(*_impl->_restricted, _impl->_chart.front().back());
		return true;
	}
    else if (parseCky(*_impl->_g, lexitems, _impl->_chart)) {
        _impl->_currentGrammar = _impl->_g;
		_impl->_state.init(*_impl->_g, _impl->_chart.front().back());
		return true;
	}
	else {
        _impl->_currentGrammar = _impl->_g; // 2010-12-14: fix for the segv on ".." (matuzaki)
        _impl->_state.clear();
		return false;
	}
}

bool ParserGuide::feed(ActionCode a)
{
	return _impl->_state.feed(a);
}

bool ParserGuide::accept(ActionCode a) const
{
	return _impl->_state.accept(a);
}

namespace cfg {
    void printChart(const Grammar &g, const Chart &chart);
}

/// for debug
void ParserGuide::dump(void)
{
	_impl->_state.dump();

    std::cout << "--------------------------------" << std::endl;
    std::cout << "(chart)" << std::endl;
    printChart(*_impl->_currentGrammar, _impl->_chart);
}

/// for debug
void ParserGuide::printExpectedActions(std::ostream &ost) const
{
	_impl->_state.printExpectedActions(ost, *_impl->_currentGrammar);
	ost << std::endl;
}

struct EdgeSetPosition {
	const EdgeSet *_eset;
	unsigned _begin;

	EdgeSetPosition(void) {}
	EdgeSetPosition(const EdgeSet* eset, unsigned begin)
		: _eset(eset)
		, _begin(begin)
	{}
};

unsigned ParserGuide::getNumChunk(void)
{
	unsigned len = _impl->_chart.size();
	unsigned begin = 0;

	unsigned numChunk = 0;
	while (begin < len) {
		for (unsigned end = len; end > begin; --end) {
			if (! _impl->_chart[begin][end].empty()) {
				++numChunk;
				begin = end;
				break;
			}
		}
	}

	return numChunk;
}

unsigned calcNumParse(
	const EdgeSet *es,
	std::map<const EdgeSet*, unsigned> &memo
) {
	std::map<const EdgeSet*, unsigned>::const_iterator m = memo.find(es);

	if (m == memo.end()) {

		unsigned sum = 0;
		for (std::vector<Edge>::const_iterator e = es->_elems.begin(); e != es->_elems.end(); ++e) {

			if (e->_left == 0) {
				++sum; /// terminal
			}
			else if (e->_right == 0) {
				sum += calcNumParse(e->_left, memo); /// unary
			}
			else {
				sum += calcNumParse(e->_left, memo) * calcNumParse(e->_right, memo);
			}
		}

		memo[es] = sum;

		return sum;
	}
	else {
		return m->second;
	}
}

unsigned ParserGuide::getNumParse(void) const
{
	unsigned n = 0;
	const Cell &root = _impl->_chart.front().back();

	std::map<const EdgeSet*, unsigned> memo;

	for (Cell::const_iterator it = root.begin(); it != root.end(); ++it) {
		if (_impl->_currentGrammar->_root.find(it->first) != _impl->_currentGrammar->_root.end()) {
			n += calcNumParse(&it->second, memo);
		}
	}

	return n;
}

////////////////////////////////////////////////////////////////////////////////

struct ChunkFinder : public ChunkFinderBase<Cell, EdgeSet> {
	const EdgeSet *findBestEdgeSet(const Cell &c) const
	{
		assert(! c.empty());
		return &(c.begin()->second);
	}

	double getEdgeSetScore(const EdgeSet *) const
	{
		return 0;
	}
};

void ParserGuide::forceConnect(void)
{
    std::vector<ChunkFinder::Chunk> chunks;
    ChunkFinder().findBestChunkSeq(_impl->_chart, chunks);

	unsigned len = _impl->_chart.size();

    if (_impl->_currentGrammar->_root.empty()) {
        _impl->_state.clear();
        return;
    }

    unsigned dummySchema = std::numeric_limits<unsigned>::max();
	unsigned anyRoot = *(_impl->_currentGrammar->_root.begin());

    if (chunks.size() == 1) {
        EdgeSet &es = _impl->_chart[0][len][anyRoot];
        es._sym = anyRoot;
        es._elems.push_back(Edge(chunks.back()._es, 0, dummySchema));
    }
    else {
        const EdgeSet *prev = chunks.back()._es;
        for (int i = chunks.size() - 2; i >= 0; --i) {
            EdgeSet &es = _impl->_chart[chunks[i]._start][len][anyRoot];
            es._sym = anyRoot;
            es._elems.push_back(Edge(chunks[i]._es, prev, dummySchema));
    
            prev = &es;
        }
    }

    _impl->_state.init(*_impl->_currentGrammar, _impl->_chart.front().back());

    //std::cerr << "Number of expected chunks = " << chunks.size() << std::endl;
}

//------------------------------------------------------------------------------
} // namespace mogura
//------------------------------------------------------------------------------
