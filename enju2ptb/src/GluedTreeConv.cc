#include <stack>
#include <map>
#include <vector>
#include <limits>
#include <cmath>
#include <set>
#include <ctime>
#include <functional>
#include <fstream>
#include <algorithm>

//#include "FileStream.h"
#include "PennTree.h"
#include "SymbolTable.h"
#include "Subtree.h"

////////////////////////////////////////////////////////////////////////////////
/// Constants
////////////////////////////////////////////////////////////////////////////////
const int NONE = -1;

////////////////////////////////////////////////////////////////////////////////
/// Util
////////////////////////////////////////////////////////////////////////////////
typedef std::pair<int, std::pair<int, int> > Tuple3;

template<class T1, class T2, class T3>
inline
std::pair<T1, std::pair<T2, T3> > make_tuple(T1 v1, T2 v2, T3 v3)
{
	return std::make_pair(v1, std::make_pair(v2, v3));
}

////////////////////////////////////////////////////////////////////////////////
/// Simple structres
////////////////////////////////////////////////////////////////////////////////
struct Symbol {
	int _hpsgLabel;
	int _cfgLabel;
	int _address;

	Tuple3 asTuple(void) const
	{
		return make_tuple(_hpsgLabel, _cfgLabel, _address);
	}

	Symbol(void) {}

	Symbol(int hpsgLabel, int cfgLabel, int address)
		: _hpsgLabel(hpsgLabel)
		, _cfgLabel(cfgLabel)
		, _address(address)
	{}

	bool operator==(const Symbol &s) const
	{
		return asTuple() == s.asTuple();
	}

	bool operator!=(const Symbol &s) const
	{
		return ! (*this == s);
	}
};

inline
bool operator<(const Symbol &s1, const Symbol &s2)
{
	return s1.asTuple() < s2.asTuple();
}

struct PairedRule {
	Symbol _parent;
	Symbol _left;
	Symbol _right;
	double _score;
	double _rootMarginal;
	int _cfgSubtreeIx;
};

struct CfgRule {
	int _parent;
	int _left;
	int _right;
	double _score;
};

enum RuleType {
	CFG_RULE = 0, PAIRED_RULE = 1
};

struct RuleId {
	RuleType _type;
	int _id;

	RuleId(void) {}
	RuleId(RuleType type, int id) : _type(type), _id(id) {}

	std::pair<RuleType, int> asPair(void) const
	{
		return std::make_pair(_type, _id);
	}
};

inline
bool operator<(const RuleId &id1, const RuleId &id2)
{
	return id1.asPair() < id2.asPair();
}

typedef std::vector<RuleId> RuleVec;
typedef std::vector<RuleId>::const_iterator RuleVecItr;

/// Unary: _right == NONE
struct PairedRuleKey {
	Symbol _parent;
	int _left;
	int _right;

	PairedRuleKey(void) {}

	PairedRuleKey(const Symbol &parent, int left, int right)
		: _parent(parent)
		, _left(left)
		, _right(right)
	{}

	std::pair<Symbol, std::pair<int, int> > asTuple(void) const
	{
		return make_tuple(_parent, _left, _right);
	}

	bool operator==(const PairedRuleKey &rk) const
	{
		return asTuple() == rk.asTuple();
	}

	bool operator!=(const PairedRuleKey &rk) const
	{
		return ! (*this == rk);
	}
};

inline
bool operator<(const PairedRuleKey &k1, const PairedRuleKey &k2)
{
	return k1.asTuple() < k2.asTuple();
}

#if 0
struct RuleKeyWeak {
	int _parent;
	int _left;
	int _right;

	RuleKeyWeak(void) {}
	RuleKeyWeak(int parent, int left, int right)
		: _parent(parent)
		, _left(left)
		, _right(right)
	{}

	bool operator==(const RuleKeyWeak &rk) const
	{
		return _parent == rk._parent
			&& _left == rk._left
			&& _right == rk._right;
	}

	bool operator!=(const RuleKeyWeak &rk) const
	{
		return ! (*this == rk);
	}
};

inline
bool operator<(const RuleKeyWeak &rk1, const RuleKeyWeak &rk2)
{
	if (rk1._parent != rk2._parent) {
		return rk1._parent < rk2._parent;
	}
	else if (rk1._left != rk2._left) {
		return rk1._left < rk2._left;
	}
	else {
		return rk1._right < rk2._right;
	}
}
#endif

typedef Tuple3 SourceCfgRuleKey; /// (parentCfgSym, leftCfgSym, rightCfgSym)
typedef int TargetCfgRuleKey; /// parentCfgSym

////////////////////////////////////////////////////////////////////////////////
/// Grammar class
////////////////////////////////////////////////////////////////////////////////
class Grammar {
private:

	typedef std::map<SourceCfgRuleKey, RuleVec> SourceCfgRuleIndex;
	// typedef std::map<TargetCfgRuleKey, RuleVec> TargetCfgRuleIndex;
	typedef std::map<PairedRuleKey, RuleVec> PairedRuleIndex;
	// typedef std::map<RuleKeyWeak, std::vector<RuleId> > RuleIndexWeak;

private:
	bool readLine(
		std::istream &ist,
		SubtreeNode &cTree,
		SubtreeNode &hTree,
		int &numerator,
		int &denominator,
		bool inverted
	) {
		SubtreeNode *t1 = &cTree;
		SubtreeNode *t2 = &hTree;

		if (inverted) {
			std::swap(t1, t2);
		}

		if (! ::read(ist, *t1)) {
			return false;
		}

		if (! ::read(ist, *t2)) {
			throw std::runtime_error("grammar file format error");
		}

		if (! (ist >> numerator >> denominator)) {
			throw std::runtime_error("grammar file format error");
		}

		return true;
	}

	int getAddress(const SubtreeNode &t)
	{
		if (t.isLeaf()) {
			return NONE;
		}
		else {
			return _numUsedAddress++;
		}
	}

	unsigned makeRule(
		const SubtreeNode &hTree,
		Symbol rootSymbol,
		int cfgSubtreeIx,
		double score,
		double rootMarginal,
		unsigned yieldIx,
		const std::vector<std::string> &yields
	) {
		const SubtreeNode &left = hTree._dtrs.front();
		Symbol leftSymbol(
			_hpsgSymbols.getID(left._label),
			(left.isLeaf() ? _cfgSymbols.getID(yields[yieldIx++]) : NONE),
			getAddress(left));

		if (! left.isLeaf()) {
			yieldIx = makeRule(
				left, leftSymbol, NONE, 0, 0, yieldIx, yields);
		}

		Symbol rightSymbol;
		SubtreeNode::const_iterator itr = ++(hTree._dtrs.begin());
		if (itr == hTree._dtrs.end()) { /// unary
			rightSymbol._hpsgLabel = NONE;
			rightSymbol._cfgLabel = NONE;
			rightSymbol._address = NONE;
		} 
		else { /// binary
			const SubtreeNode &right = *itr;
			rightSymbol._hpsgLabel = _hpsgSymbols.getID(right._label);
			rightSymbol._cfgLabel
				= (right.isLeaf() ?
					_cfgSymbols.getID(yields[yieldIx++]) : NONE);
			rightSymbol._address = getAddress(right);

			if (! right.isLeaf()) {
				yieldIx = makeRule(
					right, rightSymbol, NONE, 0, 0, yieldIx, yields);
			}
		}

		PairedRule rule;
		rule._parent = rootSymbol;
		rule._left = leftSymbol;
		rule._right = rightSymbol;
		rule._score = score;
		rule._rootMarginal = rootMarginal;
		rule._cfgSubtreeIx = cfgSubtreeIx;

		int ruleIx = _rules.size();
		_rules.push_back(rule);

		PairedRuleKey ruleKey;
		ruleKey._parent = rootSymbol;
		ruleKey._left = leftSymbol._hpsgLabel;
		ruleKey._right = rightSymbol._hpsgLabel;

		_ruleIndex[ruleKey].push_back(RuleId(PAIRED_RULE, ruleIx));

		#if 0
		RuleKeyWeak ruleKeyWeak;
		ruleKeyWeak._parent = rootSymbol._hpsgLabel;
		ruleKeyWeak._left = leftSymbol._hpsgLabel;
		ruleKeyWeak._right = rightSymbol._hpsgLabel;

		_ruleIndexWeak[ruleKeyWeak].push_back(ruleIx);
		#endif

		return yieldIx;
	}

	void countCfgRule(
		SymbolTable<std::string> &symbolDict,
		const SubtreeNode &n,
		int num,
		std::map<Tuple3, int> &count
	) {
		if (n.isLeaf()) {
			return;
		}

		SubtreeNode::const_iterator d = n._dtrs.begin();
		const SubtreeNode *left = &(*d++);
		const SubtreeNode *right = (d == n._dtrs.end()) ? 0 : &(*d++);
		if (d != n._dtrs.end()) {
            std::ostringstream oss;
			oss << "ERROR:" << __FILE__ << __LINE__ << std::endl;
			oss << "subtree: " << n << std::endl;
            throw std::runtime_error(oss.str());
		}

		int parentSym = symbolDict.getID(n._label);
		int leftSym = symbolDict.getID(left->_label);
		int rightSym = (right != 0) ? symbolDict.getID(right->_label) : NONE;

		count[make_tuple(parentSym, leftSym, rightSym)] += num;

		countCfgRule(symbolDict, *left, num, count);

		if (right) {
			countCfgRule(symbolDict, *right, num, count);
		}
	}

	void getCfgSymbols(const SubtreeNode &n)
	{
		(void) _cfgSymbols.getID(n._label);
		for (SubtreeNode::const_iterator d = n._dtrs.begin();
				d != n._dtrs.end(); ++d) {
			getCfgSymbols(*d);
		}
	}

	void makeCfgRuleArray(
		const std::map<Tuple3, int> &ruleCnt,
		std::vector<CfgRule> &array
	) {
		array.clear();

		std::map<int, int> rootCnt;
		for (std::map<Tuple3, int>::const_iterator r = ruleCnt.begin();
				r != ruleCnt.end(); ++r) {
			rootCnt[r->first.first] += r->second;
		}

		for (std::map<Tuple3, int>::const_iterator r = ruleCnt.begin();
				r != ruleCnt.end(); ++r) {

			CfgRule rule;
			rule._parent = r->first.first;
			rule._left = r->first.second.first;
			rule._right = r->first.second.second;
			rule._score = std::log((double) r->second / rootCnt[rule._parent]);

			array.push_back(rule);
		}
	}

public:
	Grammar(std::istream &ist, bool inverted)
	{
		_numUsedAddress = 0;

		SubtreeNode cTree;
		SubtreeNode hTree;
		int numerator;
		int denominator;

		(void) _cfgSymbols.getID("UNK");
		(void) _hpsgSymbols.getID("UNK");

		std::map<Tuple3, int> sourceCfgCount;
		// std::map<Tuple3, int> targetCfgCount;
		std::map<int, std::map<int, int> > substPairSet;
		std::map<int, int> substCount;

		while (readLine(ist, cTree, hTree, numerator, denominator, inverted)) {

			getCfgSymbols(cTree);

			int cfgSubtreeIx = _cfgSubtrees.getID(cTree);

			Symbol rootSymbol(
				_hpsgSymbols.getID(hTree._label),
				_cfgSymbols.getID(cTree._label),
				NONE);

			/// this sum should match the denominator, but we don't check it
			substPairSet[rootSymbol._hpsgLabel][rootSymbol._cfgLabel]
				+= numerator;
			substCount[rootSymbol._hpsgLabel] += numerator;

			double score = std::log((double) numerator / denominator);
			double rootMarginal = std::log((double) denominator);

			std::vector<std::string> yield;
			cTree.getYield(yield);

			makeRule(
				hTree, rootSymbol, cfgSubtreeIx,
				score, rootMarginal, 0, yield);

			countCfgRule(_hpsgSymbols, hTree, numerator, sourceCfgCount);
			// countCfgRule(_cfgSymbols, cTree, numerator, targetCfgCount);
		}

		typedef std::map<int, std::map<int, int> >::const_iterator
			SubstPairSetItr;

		for (SubstPairSetItr it = substPairSet.begin();
				it != substPairSet.end(); ++it) {

			std::vector<int> &symVec = _substPair[it->first];
			// std::vector<std::pair<int, double> > &wsymVec
			//	= _weightedSubstPair[it->first];

			assert(symVec.empty());
			// assert(_weightedSubstPair.empty());

			for (std::map<int, int>::const_iterator it2 = it->second.begin();
					it2 != it->second.end(); ++it2) {

				symVec.push_back(it2->first);
				// wsymVec.push_back(
				//	std::make_pair(it2->first),
				//	std::log((double) it2->second / substCount[it->first]));
				_symRewriteScore[std::make_pair(it->first, it2->first)]
					= std::log((double) it2->second / substCount[it->first]);
			}
		}

		makeCfgRuleArray(sourceCfgCount, _sourceCfg);
		for (unsigned i = 0; i < _sourceCfg.size(); ++i) {
			RuleId id(CFG_RULE, i);
			const CfgRule &r = _sourceCfg[i];
			Tuple3 key = make_tuple(r._parent, r._left, r._right);
			_sourceCfgIndex[key].push_back(id);
		}

		#if 0
		makeCfgRuleArray(targetCfgCount, _targetCfg);
		for (unsigned i = 0; i < _targetCfg.size(); ++i) {

			RuleId id(CFG_RULE, i);
			_targetCfgIndex[_targetCfg[i]._parent].push_back(id);
		}
		#endif
	}

	const PairedRule &pairedRule(int ix) const { return _rules[ix]; }
	const CfgRule &sourceCfgRule(int ix) const { return _sourceCfg[ix]; }
	// const CfgRule &targetCfgRule(int ix) const { return _targetCfg[ix]; }

	const RuleVec &findRules(const Symbol &parent, int left, int right) const
	{
		PairedRuleIndex::const_iterator it
			= _ruleIndex.find(PairedRuleKey(parent, left, right));

		if (it == _ruleIndex.end()) {
			return _emptySet;
		}
		else {
			return it->second;
		}
	}

	#if 0
	const RuleVec &findTargetCfgRules(int parent) const
	{
		TargetCfgRuleIndex::const_iterator it = _targetCfgIndex.find(parent);
		if (it == _targetCfgIndex.end()) {
			return _emptySet;
		}
		else {
			return it->second;
		}
	}
	#endif

	const RuleVec &findSourceCfgRule(int parent, int left, int right) const
	{
		SourceCfgRuleIndex::const_iterator it
			= _sourceCfgIndex.find(make_tuple(parent, left, right));

		if (it == _sourceCfgIndex.end()) {
			return _emptySet;
		}
		else {
			return it->second;
		}
	}

#if 0
	const std::vector<int> &findRulesWeak(
		int parent,
		int left,
		int right
	) {
		RuleIndexWeak::const_iterator it
			= _ruleIndexWeak.find(RuleKeyWeak(parent, left, right));

		if (it == _ruleIndexWeak.end()) {
			return _emptySet;
		}
		else {
			return it->second;
		}
	}
#endif

	int getHpsgSymbolIx(const std::string &sym) const
	{
		return _hpsgSymbols.getID(sym, -1);
	}

	std::string getHpsgSymbol(int ix) const
	{
		return _hpsgSymbols.getSymbol(ix);
	}

	int getCfgSymbolIx(const std::string &sym) const
	{
		return _cfgSymbols.getID(sym, -1);
	}

	std::string getCfgSymbol(int ix) const
	{
		return _cfgSymbols.getSymbol(ix);
	}

	void getCfgSubtree(int ix, SubtreeNode &st) const
	{
		assert(ix >= 0);
		st = _cfgSubtrees.getSymbol(ix);
	}

	#if 0
	typedef std::vector<std::pair<int, double> > WeightedSyms;

	const WeightedSyms &getWeightedSubstitutionPair(int hpsgSymbolIx) const
	{
		static std::map<int, WeightedSyms> defaults;

		std::map<int, WeightedSyms>::const_iterator it
			= _weightedSubstPair.find(hpsgSymbolIx);

		if (it != _weightedSubstPair.end()) {
			return it->second;
		}
		else {
			it = defaults.find(hpsgSymbolIx);
			if (it != defaults.end()) {
				return it->second;
			}
			else {
				WeightedSyms &v = defaults[hpsgSymbolIx];
				assert(v.empty());

				std::string hpsgSym = _hpsgSymbols.getSymbol(hpsgSymbolIx);
				int id = _cfgSymbols.getID(hpsgSym);
				if (id >= 0) {
					v.push_back(std::make_pair(id, 0));
				}
				else {
					/// TODO: better handling
					v.push_back(std::make_pair(getUnkCfgLabel(), 0));
				}

				return v;
			}
		}
	}
	#endif

	const std::vector<int> &getSubstitutionPair(int hpsgSymbolIx) const
	{
		static std::map<int, std::vector<int> > defaults;

		std::map<int, std::vector<int> >::const_iterator it
			= _substPair.find(hpsgSymbolIx);
		if (it != _substPair.end()) {
			return it->second;
		}
		else {
			it = defaults.find(hpsgSymbolIx);
			if (it == defaults.end()) {
				std::string hpsgSym = _hpsgSymbols.getSymbol(hpsgSymbolIx);
				int cfgSymId = _cfgSymbols.getID(hpsgSym, -1);
				if (cfgSymId >= 0) {
					defaults[hpsgSymbolIx].push_back(cfgSymId);
				}
				else {
					defaults[hpsgSymbolIx].push_back(getUnkCfgLabel());
				}
			}

			return defaults[hpsgSymbolIx];
		}
	}

	double getSymbolRewriteScore(int hpsgSym, int cfgSym) const
	{
		std::map<std::pair<int, int>, double>::const_iterator it
			= _symRewriteScore.find(std::make_pair(hpsgSym, cfgSym));

		if (it != _symRewriteScore.end()) {
			return it->second;
		}
		else {
			return 0; /// TODO: better handling
		}
	}

	int getUnkCfgLabel(void) const { return _cfgSymbols.getID("UNK"); }
	int getUnkHpsgLabel(void) const { return _hpsgSymbols.getID("UNK"); }

private:
	int _numUsedAddress;

	std::vector<PairedRule> _rules;
	PairedRuleIndex _ruleIndex;
	// RuleIndexWeak _ruleIndexWeak;

	std::vector<CfgRule> _sourceCfg;
	// std::vector<CfgRule> _targetCfg;

	SourceCfgRuleIndex _sourceCfgIndex;
	// TargetCfgRuleIndex _targetCfgIndex;

	std::map<int, std::vector<int> > _substPair;
	std::map<std::pair<int, int>, double> _symRewriteScore;
	// std::map<int, std::vector<std::pair<int, double> > > _weightedSubstPair;

	SymbolTable<std::string> _hpsgSymbols;
	SymbolTable<std::string> _cfgSymbols;
	SymbolTable<SubtreeNode> _cfgSubtrees;

	const std::vector<RuleId> _emptySet;
	const std::vector<int> _emptyIntVec;
};

////////////////////////////////////////////////////////////////////////////////
/// Edge/EdgeSet
////////////////////////////////////////////////////////////////////////////////
struct EdgeSet;

/// Unary: _right == 0
struct Edge {
	RuleId _ruleIx; /// it may be a cfg rule or paired trees
	EdgeSet *_left;
	EdgeSet *_right;
};

struct Score {
	unsigned _numGap;
	double _gapScore; /// sum log-prob of the pcfg rules for the gaps
	double _pairScore; 

	Score(void) {}
	Score(unsigned numGap, double gapScore, double pairScore)
		: _numGap(numGap)
		, _gapScore(gapScore)
		, _pairScore(pairScore)
	{}

	Score &operator+=(const Score &s)
	{
		_numGap += s._numGap;
		_gapScore += s._gapScore;
		_pairScore += s._pairScore;

		return *this;
	}

	static Score getMinScore(void)
	{
		return Score(
			std::numeric_limits<unsigned>::max(),
			-std::numeric_limits<double>::infinity(),
			-std::numeric_limits<double>::infinity());
	}
};

inline
bool operator<(const Score &s1, const Score &s2)
{
	if (s1._numGap != s2._numGap) {
		return s1._numGap > s2._numGap;  /// fewer gaps are better
	}
	else if (s1._gapScore != s2._gapScore) {
		/// prefer common cfg rules to fill the gaps
		return s1._gapScore < s2._gapScore;
	}
	else {
		return s1._pairScore < s2._pairScore;
	}
}

#if 0
enum Status {
	YET,
	GAPPED,
	COMPLETE
};
#endif

struct EdgeSet {
	Symbol _parent;
	std::vector<Edge> _edges;
	int _bestEdgeIx;
	Score _bestScore;
	// double _bestScore;
	// unsigned _numGap;
	// Status _status;

	EdgeSet(void) {}

	#if 0
	double bestScoreAsGapDtr(const Grammar &g) const
	{
		double s = _bestScore;

		if (_bestEdge < (int) _edges.size()) {
			const Edge &e = _edges[_bestEdge];
			if (e._ruleIx >= 0) {
				const Rule &r = g.rule(e._ruleIx);

				s += r._rootMarginal;
			}
		}

		return s;
	}
	#endif
};

typedef std::map<Symbol, EdgeSet> EdgeSetMap;
typedef std::map<const PennTree*, EdgeSetMap> NodeMap;

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////

bool isValidBranching(
	const std::string &motSym,
	const std::string &leftSym,
	const std::string &rightSym
) {
	
	if (motSym[0] == '*') {
		
		if (leftSym[0] == '*' && motSym != leftSym) {
			return false;
		}

		if (rightSym[0] == '*' && motSym != rightSym) {
			return false;
		}

		/// *A -> B C (where A != B and A != C)
		if (leftSym[0] != '*' && rightSym[0] != '*'
				&& motSym.substr(1) != leftSym
				&& motSym.substr(1) != rightSym) {
			return false;
		}

		/// we could check for the case "*A -> *A *A"
		/// but allow it now
	}
	else {
		if (leftSym[0] == '*' && motSym != leftSym.substr(1)) {
			return false;
		}

		if (rightSym[0] == '*' && motSym != rightSym.substr(1)) {
			return false;
		}

		/// we could add more check for the case like "A -> *A *A"
		/// but allow it now
	}

	return true;
}

bool isValidBranching(
	const std::string &motSym,
	const std::string &dtrSym
) {
	if (motSym[0] == '*') {
		return motSym == dtrSym /// *X -> *X ; we might inhibit this
			|| motSym.substr(1) == dtrSym; /// *X -> X ; same as above
	}
	else {
		if (dtrSym[0] == '*') {
			return dtrSym.substr(1) == motSym; /// X -> *X ; save as above
		}
	}

	return true;
}

void startSubTree(
	const Grammar &g,
	int motHpsgSym,
	int leftHpsgSym,
	int rightHpsgSym,
	EdgeSetMap &motEsetMap,
	EdgeSetMap *leftEsetMap,
	EdgeSetMap *rightEsetMap,
	unsigned backOffLvl
) {
	assert(backOffLvl > 0);

	if (motHpsgSym < 0) { /// PENDING: what to do with this case?
		return;
	}

	#if 0
	const std::vector<int> &cfgLabels = g.getSubstitutionPair(motHpsgSym);

	std::string leftHpsgSymStr(g.getHpsgSymbol(leftHpsgSym));
	std::string rightHpsgSymStr(
		(rightHpsgSym < 0)
			? std::string("NONE")
			: g.getHpsgSymbol(rightHpsgSym));

	for (std::vector<int>::const_iterator it = cfgLabels.begin();
			it != cfgLabels.end(); ++it) {

		Symbol s(motHpsgSym, *it, NONE);
		EdgeSet &motEset = motEsetMap[s];

		/// Prepare for the paired-substitutions
		motEset._parent = s; /// it may already exist

		/// Start or continue gapping cfg re-writing
		#if 0
		const RuleVec &rs = g.findTargetCfgRules(*it);

		for (RuleVecItr rItr = rs.begin(); rItr != rs.end(); ++rItr) {
			
			const CfgRule &r = g.targetCfgRule(rItr->_id);

			if ((r._right == NONE && rightEsetMap)
					|| (r._right != NONE && rightEsetMap == 0)) {
				continue; /// unary/binary mismatch
			}

			if (leftIsPreTerm && g.getCfgSymbol(r._left) != leftHpsgSymStr) {
				continue; /// leaf mismatch
			}

			if (rightIsPreTerm && g.getCfgSymbol(r._right) != rightHpsgSymStr) {
				continue; /// leaf mismatch
			}

			motEset._edges.push_back(Edge());
			Edge &e = motEset._edges.back();

			e._ruleIx = *rItr;

			Symbol leftSym(leftHpsgSym, r._left, NONE);
			e._left = &((*leftEsetMap)[leftSym]);
			e._left->_parent = leftSym;

			if (rightEsetMap) {
				Symbol rightSym(rightHpsgSym, r._right, NONE);
				e._right = &((*rightEsetMap)[rightSym]);
				e._right->_parent = rightSym;
			}
		}
		#endif
	}
	#endif

	typedef std::vector<int> IdVec;
	typedef IdVec::const_iterator IdVecItr;

	const IdVec &motCfgSyms = g.getSubstitutionPair(motHpsgSym);

	for (IdVecItr m = motCfgSyms.begin(); m != motCfgSyms.end(); ++m) {

		Symbol motSym(motHpsgSym, *m, NONE);
		EdgeSet &es = motEsetMap[motSym];
		es._parent = motSym;

		// std::string motSymStr = g.getCfgSymbol(*m);

		/// Start cfg rule

		const IdVec &leftCfgSyms = g.getSubstitutionPair(leftHpsgSym);

		if (rightEsetMap == 0) { /// unary
			
			for (IdVecItr d = leftCfgSyms.begin();
					d != leftCfgSyms.end(); ++d) {

				/// Don't allow invalid use of pseudo nodes
				/// --> This make the conversion accuracy (*PRECISION*) worse.
				///    TODO: check why
				// if (backOffLvl <= 1) {
				//	std::string dtrSymStr = g.getCfgSymbol(*d);
				//	if (! isValidBranching(motSymStr, dtrSymStr)) {
				//		continue;
				//	}
				// }

				es._edges.push_back(Edge());
				Edge &e = es._edges.back();

				Symbol leftSym(leftHpsgSym, *d, NONE);
				e._ruleIx._type = CFG_RULE; /// TODO: don't abuse the fields
				e._left = &((*leftEsetMap)[leftSym]);
				e._left->_parent = leftSym;
				e._right = 0;
			}
		}
		else { /// binary
			
			const IdVec &rightCfgSyms = g.getSubstitutionPair(rightHpsgSym);

			for (IdVecItr d1 = leftCfgSyms.begin();
					d1 != leftCfgSyms.end(); ++d1) {

				// std::string leftSymStr = g.getCfgSymbol(*d1);

				Symbol leftSym(leftHpsgSym, *d1, NONE);

				for (IdVecItr d2 = rightCfgSyms.begin();
						d2 != rightCfgSyms.end(); ++d2) {

					Symbol rightSym(rightHpsgSym, *d2, NONE);

					// if (backOffLvl <= 1) {
					//	std::string rightSymStr = g.getCfgSymbol(*d2);
					//	if (! isValidBranching(
					//				motSymStr, leftSymStr, rightSymStr)) {
					//		continue;
					//	}
					// }

					es._edges.push_back(Edge());
					Edge &e = es._edges.back();

					e._ruleIx._type = CFG_RULE; /// TODO: don't abuse the fields
					e._left = &((*leftEsetMap)[leftSym]);
					e._left->_parent = leftSym;

					e._right = &((*rightEsetMap)[rightSym]);
					e._right->_parent = rightSym;
				}
			}
		}
	}
}

void addEdges(
	const Grammar &g,
	int leftHpsgSym,
	int rightHpsgSym,
	EdgeSetMap &motEset,
	EdgeSetMap *leftEset,
	EdgeSetMap *rightEset
) {
	for (EdgeSetMap::iterator it = motEset.begin();
			it != motEset.end(); ++it) {

		const RuleVec &rules
			= g.findRules(it->first, leftHpsgSym, rightHpsgSym);

		for (RuleVecItr rItr = rules.begin(); rItr != rules.end(); ++rItr) {

			const PairedRule &r = g.pairedRule(rItr->_id);

			it->second._edges.push_back(Edge());
			Edge &e = it->second._edges.back();

			e._ruleIx = *rItr;

			e._left = &((*leftEset)[r._left]);
			e._right = (rightEset != 0) ? &((*rightEset)[r._right]) : 0;

			/// Redundant
			e._left->_parent = r._left;
			if (e._right) {
				e._right->_parent = r._right;
			}
		}
	}
}

bool makeLatticeLoop(
	const Grammar &g,
	const PennTree *t,
	EdgeSetMap &motEset,
	NodeMap &ns,
	unsigned backOffLvl
) {
	if (t->isPreTerminal()) {

		if (motEset.size() > 1) {
            throw std::runtime_error("Grammar error: there seems to be unmatched leaf nodes in the tree pairs");
		}

		return ! motEset.empty();
	}

	PennTree::const_iterator d = t->begin();
	assert(d != t->end());

	int motHpsgSym = g.getHpsgSymbolIx(t->getString());

	PennTree *leftDtr = *(d++);
	PennTree *rightDtr = (d == t->end()) ? 0 : *(d++);

	assert(d == t->end());

	EdgeSetMap *leftEset = &(ns[leftDtr]);
	EdgeSetMap *rightEset = (rightDtr == 0) ? 0 : &(ns[rightDtr]);

	int leftHpsgSym = g.getHpsgSymbolIx(leftDtr->getString());
	if (leftHpsgSym < 0) {

		//std::cerr << "UNKNOWN hpsg symbol: " << leftDtr->getString()
		//		<< std::endl;

		return false;
	}

	int rightHpsgSym = (rightDtr == 0) ? NONE : g.getHpsgSymbolIx(rightDtr->getString());
	if (rightDtr != 0 && rightHpsgSym < 0) {

		// std::cerr << "UNKNOWN hpsg symbol: " << rightDtr->getString()
		//		<< std::endl;

		return false;
	}

	if (backOffLvl > 0) {
		startSubTree(g, motHpsgSym, leftHpsgSym, rightHpsgSym, motEset, leftEset, rightEset, backOffLvl);
	}

	addEdges(g, leftHpsgSym, rightHpsgSym, motEset, leftEset, rightEset);

	if (motEset.empty()) {
		return false;
	}

	if (leftEset->empty()) {
        return false;
	}

	if (! makeLatticeLoop(g, leftDtr, *leftEset, ns, backOffLvl)) {
		return false;
	}

	if (rightDtr) {
		assert(rightEset != 0);
		if (! makeLatticeLoop(g, rightDtr, *rightEset, ns, backOffLvl)) {
			return false;
		}
	}

	return true;
}

bool makeLattice(
	const Grammar &g,
	const PennTree *t,
	NodeMap &ns,
	unsigned backOffLvl
) {
	EdgeSetMap &rootEset = ns[t];

	int topIx = g.getCfgSymbolIx("TOP");
	assert(topIx >= 0);

	int hpsgRootSymIx = g.getHpsgSymbolIx(t->getString());

	if (hpsgRootSymIx < 0) {
		//std::cerr << "UNKNOWN hpsg symbol: " << t->getString() << std::endl;
		return false;
	}

	Symbol rootSym(hpsgRootSymIx, topIx, NONE);
	EdgeSet &root = rootEset[rootSym];
	root._parent = rootSym;

	return makeLatticeLoop(g, t, rootEset, ns, backOffLvl);
}

#if 0
EdgeSet *findBestGapDtrEdgeSet(const Grammar &g, EdgeSetMap &esetMap)
{
	EdgeSet *best = 0;
	double bestScore = -std::numeric_limits<double>::infinity();

	for (EdgeSetMap::iterator it = esetMap.begin(); it != esetMap.end(); ++it) {

		if (it->first._address != NONE) {
			continue;
		}

		if (it->second._status == YET) {
			continue;
		}

		if (it->first._cfgLabel >= 0) {
			std::string cfgLabel = g.getCfgSymbol(it->first._cfgLabel);
			if (cfgLabel[0] == '*') {
				continue;
			}

			/// Messy
			static int topIx = g.getCfgSymbolIx("TOP");
			if (it->first._cfgLabel == topIx) {
				continue;
			}
		}

		if (best == 0) {
			best = &(it->second);
			bestScore = best->bestScoreAsGapDtr(g);
			continue;
		}

		if (best->_status == GAPPED && it->second._status == COMPLETE) {
			best = &(it->second);
			bestScore = best->bestScoreAsGapDtr(g);
			continue;
		}

		if (best->_status == COMPLETE && it->second._status == GAPPED) {
			continue;
		}

		assert(best->_status == it->second._status);

		if (best->_status == GAPPED) {
			if (best->_numGap > it->second._numGap
					|| (best->_numGap == it->second._numGap
						&& bestScore < it->second.bestScoreAsGapDtr(g))) {
				best = &(it->second);
				bestScore = best->bestScoreAsGapDtr(g);
				continue;
			}
		}
		else {
			assert(best->_status == COMPLETE);
			if (bestScore < it->second.bestScoreAsGapDtr(g)) {
				best = &(it->second);
				bestScore = best->bestScoreAsGapDtr(g);
				continue;
			}
		}
	}

	assert(best != 0);

	return best;
}

std::string statusName(Status s)
{
	switch (s) {
		case YET: return "yet";
		case COMPLETE: return "complete";
		case GAPPED: return "gapped";
	}
	assert(false);
	return "???";
}
#endif

void printBranch(const Grammar &g, Symbol mot, const Edge &e)
{
	std::cerr << g.getHpsgSymbol(mot._hpsgLabel)
		<< " -> "
		<< g.getHpsgSymbol(e._left->_parent._hpsgLabel);

	if (e._left->_parent._cfgLabel >= 0) {
		std::cerr << "(" << g.getCfgSymbol(e._left->_parent._cfgLabel >= 0)
			<< ")";
	}

	// std::cerr << "[" << statusName(e._left->_status) << "]";

	if (e._right) {
		std::cerr << ' ' << g.getHpsgSymbol(e._right->_parent._hpsgLabel);
		if (e._right->_parent._cfgLabel >= 0) {
			std::cerr << "("
				<< g.getCfgSymbol(e._right->_parent._cfgLabel >= 0) << ")";
		}
		// std::cerr << "[" << statusName(e._right->_status) << "]";
	}

	std::cerr << std::endl;
}

bool findViterbiDerivLoop(
	const Grammar &g,
	const PennTree *t, 
	NodeMap &ns
) {
	EdgeSetMap &eset = ns[t];

	if (t->isPreTerminal()) {
		
		/// this function should not be called when
		/// the lattice is not connected
		assert(! eset.empty());

		/// Pos tags in both trees should match
		assert(eset.size() == 1);

		EdgeSet &es = eset.begin()->second;
		assert(es._parent == eset.begin()->first);
		assert(es._edges.empty());

		es._bestEdgeIx = 0; /// but, don't access es._edges[0]!
		es._bestScore = Score(0, 0, 0);

		return true;
	}

	//assert(! eset.empty());

	for (PennTree::const_iterator d = t->begin(); d != t->end(); ++d) {
		if (! findViterbiDerivLoop(g, *d, ns)) {
			return false;
		}
	}

	bool connected = false;

	for (EdgeSetMap::iterator it = eset.begin(); it != eset.end(); ++it) {
		
		EdgeSet &es = it->second;
		assert(it->first == es._parent);

		es._bestEdgeIx = -1;
		es._bestScore = Score::getMinScore();

		for (unsigned i = 0; i < es._edges.size(); ++i) {

			const Edge &e = es._edges[i];

			if (e._left->_bestEdgeIx < 0
					|| (e._right && e._right->_bestEdgeIx < 0)) {
				/// This edge does not reach the leaves
				continue;
			}

			Score s = e._left->_bestScore;
			if (e._right) {
				s += e._right->_bestScore;
			}

			if (e._ruleIx._type == CFG_RULE) {

				int motHpsgSym = g.getHpsgSymbolIx(t->getString());

				const RuleVec &sourceCfgRule
					= g.findSourceCfgRule(
						motHpsgSym,
						e._left->_parent._hpsgLabel,
						(e._right) ? e._right->_parent._hpsgLabel : NONE);

				/// PENDING: Will this work as we want?
				///        -> maybe. There is no way to make a tree without
				///        -> using a cfg rule starting here.
				///        -> thus giving 0 to the unknown hpsg-branching
				///        -> should not do harm to the result (really?)
				double sourceCfgRuleScore
					= (sourceCfgRule.empty())
					? 0
					: g.sourceCfgRule(sourceCfgRule[0]._id)._score;

				double symRewriteScore
					= g.getSymbolRewriteScore(
						e._left->_parent._hpsgLabel,
						e._left->_parent._cfgLabel);

				if (e._right) {
					symRewriteScore
						+= g.getSymbolRewriteScore(
							e._right->_parent._hpsgLabel,
							e._right->_parent._cfgLabel);
				}

				s._numGap += 1;
				s._gapScore += symRewriteScore + sourceCfgRuleScore;
			}
			else {
				const PairedRule &r = g.pairedRule(e._ruleIx._id);
				s._pairScore += r._score;
			}

			if (es._bestScore < s) {
				es._bestScore = s;
				es._bestEdgeIx = i;
			}
		}

		connected = (connected || (es._bestEdgeIx >= 0));
	}

	return connected;
}

const EdgeSet *findRootEdgeSet(
	const Grammar &g,
	const NodeMap &ns,
	const PennTree &hTree
) {
	int topIx = g.getCfgSymbolIx("TOP");
	int hpsgRootIx = g.getHpsgSymbolIx(hTree.getString());

	assert(topIx >= 0);
	assert(hpsgRootIx >= 0);

	NodeMap::const_iterator rootNode = ns.find(&hTree);
	assert(rootNode != ns.end());

	const EdgeSetMap &rootEset = rootNode->second;

	Symbol rootSymbol(hpsgRootIx, topIx, NONE);
	EdgeSetMap::const_iterator it = rootEset.find(rootSymbol);

	assert(it != rootEset.end());

	return &(it->second);
}

void extractTerminal(const PennTree *t, std::vector<std::string> *ws)
{
	if (t->isLeaf()) {
		ws->push_back(t->getString());
	}
	else {
		std::for_each(t->begin(), t->end(), std::bind2nd(
			ptr_fun(&extractTerminal), ws));
	}
}

void extractPreTerminal(const PennTree *t, std::vector<const PennTree*> *ps)
{
	if (t->isPreTerminal()) {
		ps->push_back(t);
	}
	else {
		std::for_each(t->begin(), t->end(), std::bind2nd(
			ptr_fun(&extractPreTerminal), ps));
	}
}

unsigned addTerminal(PennTree &t, std::vector<std::string> &ws, unsigned ix)
{
	if (t.isLeaf()) {
		t.addDtr(new PennTree(ws[ix]));
		return ix + 1;
	}
	else {
		for (PennTree::iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
			ix = addTerminal(**d, ws, ix);
		}
		return ix;
	}
}

void extendTree(
	PennTree &t,
	std::stack<PennTree*> &stack,
	const SubtreeNode &st
) {
	t.setString(st._label);

	if (st._dtrs.empty()) {
		stack.push(&t);
	}
	else {
		for (SubtreeNode::const_iterator stDtr = st._dtrs.begin();
				stDtr != st._dtrs.end(); ++stDtr) {

			PennTree *dtr = new PennTree();
			t.addDtr(dtr);
			extendTree(*dtr, stack, *stDtr);
		}
	}
}

#if 0
void createCfgTreeLoop(
	const Grammar &g,
	const EdgeSet &es,
	std::stack<PennTree*> &stack
) {
	if (es._edges.empty()) { /// leaf (i.e., the level of POSs)
		stack.pop();
		return;
	}

	assert(es._bestEdge >= 0 && es._bestEdge < (int) es._edges.size());

	const Edge &e = es._edges[es._bestEdge];

	assert(e._ruleIx >= 0);
	const Rule &r = g.rule(e._ruleIx);

	if (es._parent._cfgLabel == NONE) { /// inside a cfg-subtree

		assert(es._parent._address != NONE);
		assert(r._cfgSubtreeIx == NONE);
	}
	else { /// Substitution points

		assert(es._parent._address == NONE);
		assert(r._cfgSubtreeIx != NONE);

		PennTree *t = stack.top();
		stack.pop();

		SubtreeNode st;
		g.getCfgSubtree(r._cfgSubtreeIx, st);
		std::stack<PennTree*> revStack;
		extendTree(*t, revStack, st);

		while (! revStack.empty()) {
			stack.push(revStack.top());
			revStack.pop();
		}
	}

	createCfgTreeLoop(g, *e._left, stack);
	if (e._right != 0) {
		createCfgTreeLoop(g, *e._right, stack);
	}
}
#endif

void createCfgTreeLoop(
	const Grammar &g,
	const EdgeSet &es,
	std::stack<PennTree*> &stack
) {
	if (es._edges.empty()) { /// leaf (i.e., the level of POSs)
		PennTree *t = stack.top();
		stack.pop();

		assert(es._parent._cfgLabel != NONE);

		t->setString(g.getCfgSymbol(es._parent._cfgLabel));

		assert(t->getString() != "");

		return;
	}

	assert(es._bestEdgeIx >= 0 && es._bestEdgeIx < (int) es._edges.size());

	const Edge &e = es._edges[es._bestEdgeIx];

	switch (e._ruleIx._type) {
		case CFG_RULE: {

			PennTree *t = stack.top();
			stack.pop();

			t->setString(g.getCfgSymbol(es._parent._cfgLabel));

			if (e._right) {
				PennTree *right = new PennTree();
				t->addDtr(right);
				stack.push(right);
			}

			PennTree *left = new PennTree();
			t->insertDtr(t->begin(), left);
			stack.push(left);

			break;
		}
		case PAIRED_RULE:
			if (es._parent._cfgLabel == NONE) {
				/// inside a cfg-subtree
				assert(es._parent._address != NONE);
				assert(g.pairedRule(e._ruleIx._id)._cfgSubtreeIx == NONE);
			}
			else { /// Substitution points

				PennTree *t = stack.top();
				stack.pop();

				SubtreeNode st;

				const PairedRule &r = g.pairedRule(e._ruleIx._id);

				assert(es._parent._address == NONE);
				assert(r._cfgSubtreeIx != NONE);

				g.getCfgSubtree(r._cfgSubtreeIx, st);

				std::stack<PennTree*> revStack;
				extendTree(*t, revStack, st);

				while (! revStack.empty()) {
					stack.push(revStack.top());
					revStack.pop();
				}
			}

			break;
	}

	createCfgTreeLoop(g, *e._left, stack);
	if (e._right != 0) {
		createCfgTreeLoop(g, *e._right, stack);
	}
}

bool createCfgTree(
	const Grammar &g,
	const PennTree &hTree,
	const NodeMap &ns,
	PennTree &cTree
) {
	const EdgeSet *eset = findRootEdgeSet(g, ns, hTree);
	assert(eset != 0 && eset->_bestEdgeIx >= 0);

	std::stack<PennTree*> stack;
	stack.push(&cTree);
	createCfgTreeLoop(g, *eset, stack);

	assert(stack.empty());

	std::vector<std::string> ws;
	extractTerminal(&hTree, &ws);
	addTerminal(cTree, ws, 0);

	return true;
}

bool isConnected(const Grammar &g, const PennTree *root, const NodeMap &ns)
{
	const EdgeSet *eset = findRootEdgeSet(g, ns, *root);

	return (eset != 0 && eset->_bestEdgeIx >= 0);
}

void removePseudoNodeItr(
	const PennTree &t,
	std::vector<PennTree*> &dtrs
) {
	if (t.isLeaf()) {
		dtrs.push_back(t.clone());
	}
	else {
		std::string label = t.getString();
		if (label[0] == '*') {
			for (PennTree::const_iterator d = t.begin(); d != t.end(); ++d) {
				removePseudoNodeItr(**d, dtrs);
			}
		}
		else {
			PennTree *newNode = new PennTree(t.getString());

			std::vector<PennTree*> dd;
			for (PennTree::const_iterator d = t.begin(); d != t.end(); ++d) {
				removePseudoNodeItr(**d, dd);
			}

			for (std::vector<PennTree*>::iterator it = dd.begin();
					it != dd.end(); ++it) {
				newNode->addDtr(*it);
			}

			dtrs.push_back(newNode);
		}
	}
}

void removePseudoNode(
	const PennTree &t1,
	PennTree &t2
) {

	// std::cerr << t1 << std::endl;

	std::vector<PennTree*> dtrs;
	removePseudoNodeItr(t1, dtrs);

	assert(dtrs.size() == 1);

	t2.clear();
	t2.setString(t1.getString());
	t2.raiseDtr(t2.dtrBegin(), *dtrs[0]);

	delete dtrs[0];

	// std::cerr << t2 << std::endl;

}

void makeDefaultTree(
	const PennTree &hTree,
	PennTree &cTree
) {
	std::vector<const PennTree*> preterms;
	extractPreTerminal(&hTree, &preterms);

	cTree.setString("TOP");
	for (std::vector<const PennTree*>::const_iterator d = preterms.begin();
			d != preterms.end(); ++d) {
		cTree.addDtr((*d)->clone());
	}
}

void convTree(const Grammar &g, const PennTree &hTree, PennTree &cTree)
{
	NodeMap nodes;
	bool ok = false;

	/// don't allow gap first
	unsigned backOffLvl = 0;
	for ( ; backOffLvl < 2; ++backOffLvl) {

		nodes.clear();

		if (! makeLattice(g, &hTree, nodes, backOffLvl)) {
			//std::cerr << "fail (case " << backOffLvl << ":1)" << std::endl;
			continue;
		}

		if (! findViterbiDerivLoop(g, &hTree, nodes)) {
			//std::cerr << "fail (case " << backOffLvl << ":2)" << std::endl;
			continue;
		}

		if (! isConnected(g, &hTree, nodes)) {
			//std::cerr << "fail (case " << backOffLvl << ":3)" << std::endl;
			continue;
		}

		ok = true;
		break;
	}

	if (ok) {

		// if (backOffLvl == 0) {
		//	std::cerr << "covered= yes" << std::endl;
		// }
		// else {
		//	std::cerr << "covered= backoff" << backOffLvl << std::endl;
		// }

		PennTree cTree1;
		createCfgTree(g, hTree, nodes, cTree1);

		cTree.clear();
		removePseudoNode(cTree1, cTree);
	}
	else {
		// std::cerr << "covered= no (default_conv)" << std::endl;
		makeDefaultTree(hTree, cTree);
	}
}

int main(int argc, char **argv)
try {
	if (argc != 2 && argc != 3) {
		std::cerr << "Usage: " << argv[0] << " [-i] <grammar-file>"
			<< std::endl;
		exit(1);
	}

	bool inverted = false;
	std::string grammarFileName;

	if (argc == 3) {
		if (argv[1] == std::string("-i")) {
			inverted = true;
			grammarFileName = argv[2];
		}
		else {
			std::cerr << "Unknown option: " << argv[1] << std::endl;
		}
	}
	else {
		grammarFileName = argv[1];
	}

	std::cerr << "# grammar-file= " << grammarFileName << std::endl;
	// std::cerr << "# inverted= " << ((inverted) ? "yes" : "no") << std::endl;

	//------------------
	// grammar loading
	//------------------
	std::cerr << "Loading grammar [" << grammarFileName << "] ..";

	// InputFileStream grammarFile(grammarFileName);
	std::ifstream grammarFile(grammarFileName.c_str());
	if (! grammarFile) {
		std::cerr << " Cannot open " << grammarFileName << std::endl;
        return 1;
	}
	Grammar g(grammarFile, inverted);

	std::cerr << "done" << std::endl;

	//-----------------
	// main loop
	//------------------
	//std::time_t startTime = std::time(0);

	//unsigned ix = 0;
	PennTree t;
	std::string line;
	while (std::getline(std::cin, line)) {
		std::istringstream iss(line);
		if (iss >> t) {
			//std::cerr << "---------------" << std::endl;
			//std::cerr << "sntNo= " << ix++ << std::endl;

			if (t.getString() == "error") {
				t.setString("TOP");
				std::cout << t << std::endl;
			}
            else if (t.getString() == "fragmental") {
                PennTree ct("TOP");
                for (PennTree::const_iterator d = t.dtrBegin(); d != t.dtrEnd(); ++d) {
                    PennTree *dtr = new PennTree();
                    convTree(g, **d, *dtr);
                    ct.addDtr(dtr);
                }
                std::cout << t << std::endl;
            }
			else {
				PennTree ct;
				convTree(g, t, ct);
	
				std::cout << ct << std::endl;
			}
		}
		else { /// ignore non-tree lines
			std::cout << line << std::endl;
		}
	}

	//std::cerr << "---------------" << std::endl;

	//std::time_t endTime = std::time(0);

	//std::cerr << ix << " trees are converted in "
	//	<< (endTime - startTime) << " sec" << std::endl;

	return 0;
}
catch (std::runtime_error &e)
{
	std::cerr << "runtime_error: " << e.what() << std::endl;
	std::exit(1);
}
catch (std::logic_error &e)
{
	std::cerr << "runtime_error: " << e.what() << std::endl;
	std::exit(1);
}
catch (std::bad_alloc &e)
{
	std::cerr << "bad alloc" << e.what() << std::endl;
	std::exit(1);
}
catch (...) {
	std::cerr << "unknown exception" << std::endl;
	std::exit(1);
}
