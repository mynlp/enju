#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <stack>

#include "PennTree.h"
#include "Subtree.h"
#include "ProgressBar.h"
#include "Config.h"

////////////////////////////////////////////////////////////////////////////////
/// Typedefs
////////////////////////////////////////////////////////////////////////////////
typedef std::map<std::pair<SubtreeNode, SubtreeNode>, unsigned> TreePairCnt;
typedef std::map<std::pair<std::string, std::string>, unsigned> RootPairCnt;

typedef std::vector<PennTree*> Treebank;
typedef std::vector<std::pair<PennTree*, PennTree*> > PairedTreebank;

typedef std::vector<std::pair<const PennTree*, const PennTree*> > NodeMapping;

typedef std::vector<std::vector<const PennTree*> > ChartLine;
typedef std::vector<ChartLine> Chart;

////////////////////////////////////////////////////////////////////////////////
/// I/O
////////////////////////////////////////////////////////////////////////////////
void readTrees(
	std::istream &ist, 
	Treebank &trees
) {
	trees.clear();

	PennTree t;
	while (ist >> t) {
		trees.push_back(t.clone());
	}
}

void readTrees(
	std::vector<std::string> &files,
	Treebank &trees
) {
	trees.clear();

	for (std::vector<std::string>::const_iterator f = files.begin(); f != files.end(); ++f) {

		std::ifstream file(f->c_str());
		if (! file) {
            std::runtime_error("Cannot open " + *f);
		}

		PennTree t;
		while (file >> t) {
			trees.push_back(t.clone());
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/// Utils
////////////////////////////////////////////////////////////////////////////////
void extractTerminal(const PennTree &t, std::vector<std::string> &ws)
{
	if (t.isLeaf()) {
		ws.push_back(t.getString());
	}
	else {
		for (PennTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
			extractTerminal(**d, ws);
		}
	}
}

void extractPreTerminal(const PennTree &t, std::vector<std::string> &pos)
{
	if (t.isPreTerminal()) {
		pos.push_back(t.getString());
	}
	else {
		for (PennTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
			extractPreTerminal(**d, pos);
		}
	}
}

void removePeriod(PennTree &t)
{
	PennTree::iterator d = t.dtrBegin();
	while (d != t.dtrEnd()) {
		if ((*d)->isPreTerminal()) {
			if ((*d)->getString() == ".") {
				t.eraseDtr(d++);
			}
			else {
				++d;
			}
		}
		else {
			removePeriod(**d);

			if ((*d)->isLeaf()) { /// When all the dtrs of *d were "."
				/// ADD 2007/05/22
				t.eraseDtr(d++);
			}
			else {
				++d;
			}
		}
	}
}

void removePeriod(Treebank &trees)
{
	for (Treebank::iterator t = trees.begin(); t != trees.end(); ++t) {
		assert(! (*t)->isPreTerminal());
		removePeriod(**t);
	}
}

bool endWithSlash(const std::string &s)
{
	return *s.rbegin() == '/';
}

/// Workaround for mayz bug
void removeBadWords(std::vector<std::string> &ws)
{
	std::vector<std::string> ret;

	for (std::vector<std::string>::const_iterator w = ws.begin(); w != ws.end(); ++w) {
		if (! endWithSlash(*w)) {
			ret.push_back(*w);
		}
	}

	ws.swap(ret);
}

void match(
	const Treebank &cfgTrees,
	const Treebank &hpsgTrees,
	PairedTreebank &matchedTrees
) {
	Treebank::const_iterator hItr = hpsgTrees.begin();
	Treebank::const_iterator hEnd = hpsgTrees.end();
	Treebank::const_iterator cItr = cfgTrees.begin();
	Treebank::const_iterator cEnd = cfgTrees.end();

	matchedTrees.clear();

	for ( ; hItr != hEnd; ++hItr) {

		std::vector<std::string> term1;
		extractTerminal(**hItr, term1);
		//extractPreTerminal(**hItr, term1);
		
		bool found = false;
		bool skip = false;

		for (Treebank::const_iterator c = cItr; c != cEnd; ++c) {

			std::vector<std::string> term2;
			extractTerminal(**c, term2);
			//extractPreTerminal(**c, term2);

			if (term1 == term2) {
				matchedTrees.push_back(std::make_pair(*c, *hItr));
				found = true;
                cItr = ++c;
                break;
			}
			else { /// Workaround for mayz bug
				removeBadWords(term2);
				if (term1 == term2) {
					skip = true;
                    break;
				}
			}
		}

		if (! found && ! skip) {
			std::cerr << "WARN: No cfg tree was found for the following hpsg tree:" << std::endl;
            std::cerr << **hItr << std::endl;
		}
	}
}

unsigned putNode(
	const PennTree *t,
	Chart &chart,
	unsigned begin
) {
	unsigned end = begin;
	for (PennTree::const_iterator d = t->dtrBegin(); d != t->dtrEnd(); ++d) {
		end = putNode(*d, chart, end);
	}

	if (begin == end) { /// a leaf node
		// chart[begin][begin + 1] = t; // do not put words
		return (begin + 1);
	}
#if 0
	else if ((begin + 1) == end) { // a preterminal or its unary ancestors

		if (chart[begin][end] == 0) { // preterminal
			chart[begin][end] = t;
		}
		return end;
	}
	else { /// internal or root node; overwrite the unary dtrs if exist
		chart[begin][end] = t;
		return end;
	}
#endif
	else {
		chart[begin][end].push_back(t);

		return end;
	}
}

unsigned getLength(const PennTree *t)
{
	if (t->isPreTerminal()) {
		return 1;
	}

	unsigned len = 0;
	for (PennTree::const_iterator d = t->dtrBegin(); d != t->dtrEnd(); ++d) {
		len += getLength(*d);
	}
	return len;
}

void getMapping(
	const Chart &chart1,
	const Chart &chart2,
	NodeMapping &m
) {
	for (unsigned i = 0; i < chart1.size(); ++i) {
		for (unsigned j = i + 1; j < chart1[i].size(); ++j) {

			#if 0
			const PennTree *n1 = chart1[i][j];
			const PennTree *n2 = chart2[i][j];

			if (n1 != 0 && n2 != 0) {
				m.push_back(std::make_pair(n1, n2));
			}
			#endif

			const std::vector<const PennTree*> &c1 = chart1[i][j];
			const std::vector<const PennTree*> &c2 = chart2[i][j];

			if (c1.size() == 0 || c2.size() == 0) {
				continue;
			}

			if (c1.size() > 1 && c2.size() > 1) {

				/// an unary chain pair exists
				m.push_back(std::make_pair(c1.back(), c2.back()));
				m.push_back(std::make_pair(c1.front(), c2.front()));
			}
			else {

				if (j == i + 1) { /// leaf node
					/// put only the POS node pair
					m.push_back(std::make_pair(c1.front(), c2.front()));
				}
				else { /// internal node
					/// put only the parent node pair
					m.push_back(std::make_pair(c1.back(), c2.back()));
				}
			}
		}
	}
}

void getSubtree(
	const PennTree *r,
	const std::set<const PennTree*> &cutPoints,
	SubtreeNode &s,
	bool isRoot
) {
	s._label = r->getString();
	s._dtrs.clear();

	if (! isRoot && cutPoints.find(r) != cutPoints.end()) {
		return;
	}

	for (PennTree::const_iterator d = r->dtrBegin(); d != r->dtrEnd(); ++d) {
		s._dtrs.push_back(SubtreeNode());
		getSubtree(*d, cutPoints, s._dtrs.back(), false);
	}
}

void collectPairs(
	const PennTree *t1,
	const PennTree *t2,
	TreePairCnt &treeCnt,
	RootPairCnt &rootCnt
) {
	unsigned length = getLength(t1);

	Chart chart1(length, ChartLine(length + 1));
	putNode(t1, chart1, 0);

	Chart chart2(length, ChartLine(length + 1));
	putNode(t2, chart2, 0);

	/// For debug: begin
	assert(length == 1 || t1 == chart1.front().back().back());
	assert(length == 1 || t2 == chart2.front().back().back());

	for (unsigned i = 0; i < chart1.size(); ++i) {
		assert(! chart1[i][i + 1].empty()
			&& chart1[i][i + 1].front()->isPreTerminal());
		assert(! chart2[i][i + 1].empty()
			&& chart2[i][i + 1].front()->isPreTerminal());
	}
	/// For debug: end

	NodeMapping mapping;
	getMapping(chart1, chart2, mapping);

	if (length == 1) {
		if (chart1.front().back().size() == 1
				|| chart2.front().back().size() == 1) { /// singleton case
			return;
		}

		// mapping.push_back(std::make_pair(t1, t2));
	}

	std::set<const PennTree*> cut1;
	std::set<const PennTree*> cut2;
	for (NodeMapping::const_iterator it = mapping.begin();
			it != mapping.end(); ++it) {
		cut1.insert(it->first);
		cut2.insert(it->second);
	}

	for (NodeMapping::const_iterator it = mapping.begin();
			it != mapping.end(); ++it) {

		if (it->first->isPreTerminal()) {
			assert(it->second->isPreTerminal());
			continue;
		}

		SubtreeNode n1;
		SubtreeNode n2;
		getSubtree(it->first, cut1, n1, true);
		getSubtree(it->second, cut2, n2, true);

		++rootCnt[std::make_pair(n1._label, n2._label)];
		++treeCnt[std::make_pair(n1, n2)];
	}
}

struct Range : public std::pair<int, int> {
public:
	Range(int a, int b) : std::pair<int, int>(a, b) {}

	template<class PairT>
	Range(const PairT &p) : std::pair<int, int>(p.first, p.second) {}

	Range(void) {}

	void write(std::ostream &ost) const
	{
		ost << '[' << first << ',' << second << ']';
	}

	bool include(const Range &r) const
	{
		return first <= r.first && r.second <= second;
	}
};

typedef PennNode<Range> RangeTree;

void addRange(const PennTree &t, RangeTree &rt, unsigned begin = 0)
{
	rt.setString(t.getString());

	if (t.isLeaf()) {
		rt.setAnnotation(std::make_pair(begin, begin + 1));
		return;
	}

	for (PennTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
		RangeTree *rd = new RangeTree();
		addRange(**d, *rd, begin);
		begin = rd->getAnnotation().second;
		rt.addDtr(rd);
	}

	unsigned b = (*rt.dtrBegin())->getAnnotation().first;
	unsigned e = (*rt.dtrRevBegin())->getAnnotation().second;

	rt.setAnnotation(std::make_pair(b, e));
}

void removeRange(const RangeTree &rt, PennTree &t)
{
	t.setString(rt.getString());

	for (RangeTree::const_iterator rd = rt.dtrBegin();
			rd != rt.dtrEnd(); ++rd) {
		PennTree *d = new PennTree();
		removeRange(**rd, *d);
		t.addDtr(d);
	}
}

unsigned getNumDtr(const RangeTree &t)
{
	return std::distance(t.dtrBegin(), t.dtrEnd());
}

std::string getPseudoLabel(const RangeTree &t)
{
	std::string s = t.getString();
	if (s[0] == '*') {
		return s;
	}
	else {
		return "*" + s;
	}
}

std::string getPseudoLabel(const RangeTree &t, const std::string &type)
{
	std::string s = t.getString();
	if (s[0] == '*') {
		std::string::size_type colon = s.find(':');
		assert(colon != std::string::npos);
		return (s.substr(0, colon) + ':' + type);
	}
	else {
		return (std::string("*") + s + ':' + type);
	}
}

const RangeTree *findMatchNode(
	const std::pair<int, int> &r,
	const RangeTree &t
) {
	if (getNumDtr(t) == 1) {
		return findMatchNode(r, **t.dtrBegin());
	}

	Range nodeRange = t.getAnnotation();

	if (nodeRange == r) {
		return &t;
	}

	for (RangeTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
		Range dtrRange = (*d)->getAnnotation();
		if (dtrRange.first <= r.first && r.second <= dtrRange.second) {
			return findMatchNode(r, **d);
		}
	}

	return 0;
}

void binarize(
	const RangeTree &cfgTree,
	const RangeTree &hpsgTree,
	PennTree &binCfgTree);

void binarizeLoop(
	const RangeTree::const_iterator cfgTreeDtrBegin,
	const RangeTree::const_iterator cfgTreeDtrEnd,
	const RangeTree &hpsgTree,
	PennTree &binCfgTreeRoot
) { 
	for (RangeTree::const_iterator d = cfgTreeDtrBegin;
			d != cfgTreeDtrEnd; ++d) {

		PennTree *newDtr = new PennTree();
		binCfgTreeRoot.addDtr(newDtr);
		binarize(**d, hpsgTree, *newDtr);
	}
}

void collectRange(const RangeTree &t, std::set<Range> &rs)
{
	if (t.isPreTerminal()) {
		return;
	}

	rs.insert(t.getAnnotation());

	for (RangeTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
		collectRange(**d, rs);
	}
}

void addBracket(RangeTree &t, const Range &r)
{
	Range nodeRange = t.getAnnotation();

	if (r == nodeRange) {
		return;
	}

	if (! nodeRange.include(r)) { /// crossing or disjoint bracket
		return;
	}

	RangeTree::iterator b = t.dtrEnd();
	RangeTree::iterator e = t.dtrEnd();

	for (RangeTree::iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {

		Range drange = (*d)->getAnnotation();

		if (drange.include(r)) {
			addBracket(**d, r);
			return;
		}

		if (drange.first == r.first) {
			b = d;
		}

		if (drange.second == r.second) {
			e = d;
		}
	}

	if (b == t.dtrEnd() || e == t.dtrEnd()) { /// no matching sisters
		return;
	}

	assert(b != e);

	RangeTree *open = new RangeTree("(");
	open->setAnnotation(Range(r.first, r.first));

	RangeTree *close = new RangeTree(")");
	close->setAnnotation(Range(r.second, r.second));

	t.insertDtr(b, open);
	t.insertDtr(++e, close);
}

void makeBinTree(const RangeTree &t, PennTree &bt)
{
	bt.setString(t.getString());

	if (t.isLeaf()) {
		return;
	}

	std::stack<PennTree*> stack;

	for (RangeTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {

		if ((*d)->getString() == "(") {
			stack.push(0);
		}
		else if ((*d)->getString() == ")") {

			PennTree *newDtr = new PennTree(getPseudoLabel(t));

			assert(! stack.empty());

			PennTree *dd = stack.top();
			stack.pop();

			assert(dd);

			while (dd != 0) {
				newDtr->pushFrontDtr(dd);
				assert(! stack.empty());
				dd = stack.top();
				stack.pop();
			}

			stack.push(newDtr);
		}
		else {
			PennTree *newDtr = new PennTree();
			makeBinTree(**d, *newDtr);
			stack.push(newDtr);
		}
	}

	while (! stack.empty()) {
		assert(stack.top() != 0);
		bt.pushFrontDtr(stack.top());
		stack.pop();
	}
}

void binarize(
	const RangeTree &cfgTree,
	const RangeTree &hpsgTree,
	PennTree &binCfgTree
) {
	std::set<Range> rs;
	collectRange(hpsgTree, rs);

	RangeTree *btree = cfgTree.clone();

	for (std::set<Range>::const_iterator r = rs.begin(); r != rs.end(); ++r) {
		addBracket(*btree, *r);
	}

	makeBinTree(*btree, binCfgTree);

	delete btree;
}

void parentAnnot(
	const PennTree &t,
	const std::string &parent,
	PennTree &u
) {
	if (t.isLeaf()) {
		u.setString(t.getString());
		return;
	}

	std::string parentMark = "^" + t.getString();

	for (PennTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {

		PennTree *newDtr = new PennTree();

		parentAnnot(**d, parentMark, *newDtr);

		u.addDtr(newDtr);
	}

	u.setString(t.getString() + parent);
}

void collectPairs(
	const PairedTreebank &trees,
	TreePairCnt &treeCnt,
	RootPairCnt &rootCnt
) {
	ProgressBar pb(1000, 10000, std::cerr, "trees");
	for (unsigned i = 0; i < trees.size(); ++i, ++pb) {

		const PennTree *cfgTree = trees[i].first;
		const PennTree *hpsgTree = trees[i].second;

		RangeTree cfgTreeR;
		RangeTree hpsgTreeR;

		addRange(*cfgTree, cfgTreeR);
		addRange(*hpsgTree, hpsgTreeR);

		PennTree binCfgTree;
		binarize(cfgTreeR, hpsgTreeR, binCfgTree);

		collectPairs(&binCfgTree, hpsgTree, treeCnt, rootCnt);
	}

	pb.done();
}

void printPairs(
	std::ostream &ost,
	const TreePairCnt &tcnt,
	const RootPairCnt &rcnt
) {
	for (TreePairCnt::const_iterator tp = tcnt.begin();
			tp != tcnt.end(); ++tp) {

		const SubtreeNode &t1 = tp->first.first;
		const SubtreeNode &t2 = tp->first.second;

		RootPairCnt::const_iterator rp
			= rcnt.find(std::make_pair(t1._label, t2._label));

		assert(rp != rcnt.end());

		ost << t1 << '\t' << t2
			<< '\t' << tp->second << '\t' << rp->second << std::endl;
	}
}

bool markBaseNp(PennTree *t)
{
	if (t->isPreTerminal()) {
		return false;
	}

	bool hasNpDescendant = false;
	for (PennTree::iterator d = t->dtrBegin(); d != t->dtrEnd(); ++d) {
		if (markBaseNp(*d)) {
			hasNpDescendant = true;
		}
	}

	if (hasNpDescendant) {
		return true;
	}
	else{
		if (t->getString() == "NP") {
			t->setString("NPB");
		}
		return true;
	}

	return false;
}

void renamePercent(PennTree *t)
{
	if (t->isPreTerminal()) {
		if ((*t->dtrBegin())->getString() == "%") {
			t->setString("%");
		}
	}
	else {
		std::for_each(t->dtrBegin(), t->dtrEnd(), renamePercent);
	}
}

void markRbNot(PennTree *t)
{
	if (t->isPreTerminal()) {
		std::string pos = t->getString();
		std::string word = (*t->dtrBegin())->getString();

		if (pos == "RB" && (word == "not" || word == "Not")) {
			t->setString("RB-not");
		}
	}
	else {
		std::for_each(t->dtrBegin(), t->dtrEnd(), markRbNot);
	}
}

class Config : public ConfigWithHelp {
public:
	Config(int argc, char **argv)
	{
		define("cfg_trees", &_cfgTrees);
		define("hpsg_trees", &_hpsgTrees);
		define("config_file", &_configFile, 'c');

		define("cfg_tree", &_cfgTree, 'C');
		define("hpsg_tree", &_hpsgTree, 'H');

		readOption(argc, argv);

		if (! _configFile.empty()) {
			readConfFile(_configFile);
		}

		if (! _cfgTree.empty()) {
			_cfgTrees.insert(_cfgTrees.begin(), _cfgTree);
		}

		if (! _hpsgTree.empty()) {
			_hpsgTrees.insert(_hpsgTrees.begin(), _hpsgTree);
		}
	}

	std::vector<std::string> _cfgTrees;
	std::vector<std::string> _hpsgTrees;
	std::string _configFile;

	std::string _cfgTree;
	std::string _hpsgTree;
};

int main(int argc, char **argv)
{
	Config config(argc, argv);
	config.write(std::cerr, "# ");

	Treebank fullTrees;
	readTrees(config._cfgTrees, fullTrees);

	Treebank exTrees;
	readTrees(config._hpsgTrees, exTrees);

	std::cerr << "size of full: " << fullTrees.size() << std::endl;
	std::cerr << "size of ex: " << exTrees.size() << std::endl;

	if (fullTrees.size() < exTrees.size()) {
		std::cerr << "full forest is too small" << std::endl;
		std::exit(1);
	}

	std::cerr << "Removal of periods .. ";
	removePeriod(fullTrees);
	std::cerr << "done" << std::endl;

	/// TMP-begin
	/// !! DON'T FORGET TO MARK BASE NP BEFORE USING extractTreePair !!
	// std::cerr << "Marking base NP .. ";
	// std::for_each(fullTrees.begin(), fullTrees.end(), markBaseNp);
	// std::cerr << "done" << std::endl;

	// std::cerr << "Marking (RB not) .. ";
	// std::for_each(fullTrees.begin(), fullTrees.end(), markRbNot);
	// std::cerr << "done" << std::endl;

	// std::cerr << "Re-tagging '%' .. ";
	// std::for_each(fullTrees.begin(), fullTrees.end(), renamePercent);
	// std::cerr << "done" << std::endl;
	/// TMP-end

	std::cerr << "Checking matching tree pairs .. ";
	PairedTreebank matchedTrees;
	match(fullTrees, exTrees, matchedTrees);
	std::cerr << "done" << std::endl;

	TreePairCnt treeCnt;
	RootPairCnt rootCnt;

	std::cerr << "Collecting subtree pairs" << std::endl; 
	collectPairs(matchedTrees, treeCnt, rootCnt);

	printPairs(std::cout, treeCnt, rootCnt);

	return 0;
}
