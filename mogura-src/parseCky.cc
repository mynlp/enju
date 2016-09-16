#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "CfgParser.h"
#include "CfgGrammar.h"

//------------------------------------------------------------------------------
namespace mogura {
namespace cfg {
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Rule application
////////////////////////////////////////////////////////////////////////////////

void applyUnary(const Grammar::UnaryTable &urules, Cell &c)
{
	std::set<EdgeSet*> wait;

	for (Cell::iterator it = c.begin(); it != c.end(); ++it) {
		wait.insert(&(it->second));
	}

	while (! wait.empty()) {

		std::set<EdgeSet*>::iterator es = wait.begin();

		Grammar::UnaryTable::const_iterator ms = urules.find((*es)->_sym);
		if (ms != urules.end()) {
			for (std::vector<MotRule>::const_iterator m = ms->second.begin(); m != ms->second.end(); ++m) {

				Cell::iterator it = c.find(m->_mot);
				if (it == c.end()) {

					EdgeSet &mes = c[m->_mot];
					mes._sym = m->_mot;
					mes._elems.push_back(Edge(*es, 0, m->_rule));

					wait.insert(&mes);
				}
				else {
					it->second._elems.push_back(Edge(*es, 0, m->_rule));
				}
			}
		}

		wait.erase(es);
	}
}

void applyBinary(
	const Grammar::BinaryTable &brules,
	const Cell &leftCell,
	const Cell &rightCell,
	Cell &motCell
) {

	for (Cell::const_iterator left = leftCell.begin(); left != leftCell.end(); ++left) {
		for (Cell::const_iterator right = rightCell.begin(); right != rightCell.end(); ++right) {

			// Grammar::BinaryTable::const_iterator ms = brules.find(std::make_pair(left->first, right->first));
            //
			// if (ms == brules.end()) {
			//	continue;
			// }
            ParentSet ms;
            brules.find(left->first, right->first, ms);
            if (ms.empty()) {
                continue;
            }

			// for (std::vector<MotRule>::const_iterator m = ms->second.begin(); m != ms->second.end(); ++m) {
			for (std::vector<MotRule>::const_iterator m = ms.begin(); m != ms.end(); ++m) {

				Cell::iterator it = motCell.find(m->_mot);
				EdgeSet *es = 0;
				if (it == motCell.end()) {
					EdgeSet &newEs = motCell[m->_mot];
					newEs._sym = m->_mot;
					es = &newEs;
				}
				else {
					es = &(it->second);
				}

				es->_elems.push_back(
					Edge(&(left->second), &(right->second), m->_rule));
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////////
bool initCell(
	Cell &c,
	const Grammar &g,
	const std::string &lexEntry
) {
	
	int lexEntryId = g._lexent.getID(lexEntry, -1);
	if (lexEntryId < 0) {
		std::cerr << "cky: unknown lexical entry (" << lexEntry << ")" << std::endl;
		return false;
	}

	Grammar::LexTable::const_iterator it = g._lexToNonterm.find(lexEntryId);

	if (it == g._lexToNonterm.end()) {
		std::cerr << "cky: unknown lexical entry (" << lexEntry << ")" << std::endl;
		return false;
	}

	EdgeSet &es = c[it->second];
	es._sym = it->second;
	es._elems.push_back(Edge(0, 0, lexEntryId));

	applyUnary(g._unary, c);

	return true;
}

bool initChart(
	Chart &chart,
	const Grammar &g,
	const std::vector<std::string> &items
) {
	chart.clear();
	chart.resize(items.size(), std::vector<Cell>(items.size() + 1));

	for (unsigned i = 0; i < items.size(); ++i) {
		
		if (! initCell(chart[i][i + 1], g, items[i])) {
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization for ambiguous lexent assignments
////////////////////////////////////////////////////////////////////////////////
bool initCell(
	Cell &c,
	const Grammar &g,
	const std::vector<std::string> &lexEntry
) {
	for (std::vector<std::string>::const_iterator lex = lexEntry.begin(); lex != lexEntry.end(); ++lex) {

		int lexEntryId = g._lexent.getID(*lex, -1);
		if (lexEntryId < 0) {
			/// Warning may be printed here
			continue;
		}

		Grammar::LexTable::const_iterator it = g._lexToNonterm.find(lexEntryId);

		if (it == g._lexToNonterm.end()) {
			continue;
		}

		EdgeSet &es = c[it->second];

		if (es._elems.empty()) {
			es._sym = it->second;
			es._elems.push_back(Edge(0, 0, lexEntryId));
		}
	}

	if (c.empty()) {
		return false;
	}

	applyUnary(g._unary, c);

	return true;
}

bool initChart(
	Chart &chart,
	const Grammar &g,
	const std::vector<std::vector<std::string> > &items
) {
	chart.clear();
	chart.resize(items.size(), std::vector<Cell>(items.size() + 1));

	for (unsigned i = 0; i < items.size(); ++i) {
		
		if (! initCell(chart[i][i + 1], g, items[i])) {
			return false;
		}
	}

	return true;
}

bool checkRootCondition(
	const std::set<unsigned> &root,
	const Cell &c
) {
	if (root.size() < c.size()) {
		for (std::set<unsigned>::const_iterator r = root.begin(); r != root.end(); ++r) {
			if (c.find(*r) != c.end()) {
				return true;
			}
		}
	}
	else {
		for (Cell::const_iterator r = c.begin(); r != c.end(); ++r) {
			if (root.find(r->first) != root.end()) {
				return true;
			}
		}
	}

	return false;
}

/// for debug
void printChart(const Grammar &g, const Chart &chart)
{
	for (unsigned s = 0; s < chart.size(); ++s) {
		for (unsigned e = s + 1; e < chart[s].size(); ++e) {
			std::cerr << "c[" << s << "][" << e << "] =";

			const Cell &c = chart[s][e];
			for (Cell::const_iterator it = c.begin(); it != c.end(); ++it) {
				std::cerr << ' ' << g._nonterm.getSymbol(it->first);
				std::cerr << '(' << &(*it) << ')';
				std::cerr << "->{";
				const std::vector<Edge> &es = it->second._elems;
				for (std::vector<Edge>::const_iterator e = es.begin();
						e != es.end(); ++e) {
					std::cerr << " (" << &(*e) << ' ';
					if (e->_left) {
						std::cerr << g._nonterm.getSymbol(e->_left->_sym);
					}
					if (e->_right) {
						std::cerr << ' '
							<< g._nonterm.getSymbol(e->_right->_sym);
					}
					if (e->_left || e->_right) {
						std::cerr << ' '
							<< g._rule.getSymbol(e->_rule);
					}
					std::cerr << ")";
				}
				std::cerr << " }";
			}

			std::cerr << std::endl;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void parseMain(
	const Grammar &g,
	unsigned n,
	Chart &chart
) {
	for (unsigned len = 2; len <= n; ++len) {
		for (unsigned b = 0, e = len; e <= n; ++b, ++e) {
			for (unsigned s = b + 1; s < e; ++s) {
				applyBinary(g._binary, chart[b][s], chart[s][e], chart[b][e]); /// left, right, mother
			}
			applyUnary(g._unary, chart[b][e]);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// exported functions
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Single lexent assignments
//------------------------------------------------------------------------------
bool parseCky(
	const Grammar &g,
	const std::vector<std::string> &items,
	Chart &chart
) {
	if (! initChart(chart, g, items)) {
		return false;
	}

	parseMain(g, items.size(), chart);

	return checkRootCondition(g._root, chart[0][items.size()]);
}

//------------------------------------------------------------------------------
// Single lexent assignments
//------------------------------------------------------------------------------
bool parseCky(const Grammar &g, const std::vector<std::string> &items)
{
	Chart chart;
	return parseCky(g, items, chart);
}

//------------------------------------------------------------------------------
// Ambiguous lexent assignments
//------------------------------------------------------------------------------
bool parseCky(
	const Grammar &g,
	const std::vector<std::vector<std::string> > &items,
	Chart &chart
) {
	if (! initChart(chart, g, items)) {
		std::cerr << "unknown lexitem ";
		return false;
	}

	parseMain(g, items.size(), chart);

	return checkRootCondition(g._root, chart[0][items.size()]);
}

//------------------------------------------------------------------------------
} // namespace cfg
} // namespace mogura
