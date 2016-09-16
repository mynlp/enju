#ifndef CfgParser_h__
#define CfgParser_h__

#include <vector>
#include <map>
#include <string>

#include "CfgGrammar.h"

namespace mogura {
namespace cfg {

//------------------------------------------------------------------------------
// Data structures
//------------------------------------------------------------------------------
struct Edge;

struct EdgeSet {
	unsigned _sym;
	std::vector<Edge> _elems;
};

struct Edge {
	const EdgeSet *_left;
	const EdgeSet *_right;

	/// rule ID for internal node; lexent ID for leaf node
	unsigned _rule;

	Edge(const EdgeSet *left, const EdgeSet *right, unsigned rule)
		: _left(left)
		, _right(right)
		, _rule(rule)
	{}
};

typedef std::map<unsigned, EdgeSet> Cell;

typedef std::vector<std::vector<Cell> > Chart;

//------------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------------

bool parseCky(
	const Grammar &g,
	const std::vector<std::string> &items);

bool parseCky(
	const Grammar &g,
	const std::vector<std::string> &items,
	Chart &chart);

bool parseCky(
	const Grammar &g,
	const std::vector<std::vector<std::string> > &items,
	Chart &chart);

} // namespace cfg
} // namespace mogura

#endif // CfgParser_h__
