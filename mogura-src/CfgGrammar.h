#ifndef CfgGrammar_h__
#define CfgGrammar_h__

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "SymbolTable.h"

namespace mogura {
namespace cfg {

struct MotRule {
	unsigned _mot;
	unsigned _rule;

	MotRule(void) {}
	MotRule(unsigned mot, unsigned rule)
        : _mot(mot), _rule(rule) {}
};

typedef std::vector<MotRule> ParentSet;

class BinaryRuleTable {
public:
    struct Record { // = 12 bytes
        unsigned _left  : 24;
        unsigned _right : 24;
        unsigned _mot   : 24;
        unsigned _rule  : 24;

        Record(void) {}
        Record(unsigned left, unsigned right, unsigned mot, unsigned rule)
            : _left(left)
            , _right(right)
            , _mot(mot)
            , _rule(rule)
        {}

        bool operator<(const Record &r) const
        {
            if (_left  != r._left)  return _left  < r._left;
            if (_right != r._right) return _right < r._right;
            if (_mot   != r._mot)   return _mot   < r._mot;
            return _rule < r._rule;
        }
    };

    typedef std::vector<Record>::const_iterator RecordItr;

public:
    void init(unsigned size = 0)
    {
        _recs.clear();
        _recs.reserve(size);
    }

    void clear(void)
    {
        _recs.clear();
    }

    void put(unsigned left, unsigned right, unsigned mother, unsigned rule)
    {
        if (left >= (1 << 24) || right >= (1 << 24) || mother  >= (1 << 24) || rule >= (1 << 24)) {
            throw std::runtime_error("CFG grammar too large");
        }

        _recs.push_back(Record(left, right, mother, rule));
    }

    void sort(void)
    {
        std::stable_sort(_recs.begin(), _recs.end());
    }

    void find(unsigned left, unsigned right, ParentSet &ps) const
    {
        ps.clear();
        RecordItr it = std::lower_bound(_recs.begin(), _recs.end(), Record(left, right, 0, 0));

        while (it != _recs.end() && it->_right == right && it->_left == left) {
            ps.push_back(MotRule(it->_mot, it->_rule));
            ++it;
        }
    }

    RecordItr begin(void) const { return _recs.begin(); }
    RecordItr end(void) const { return _recs.end(); }

    void writeRawFile(
        const SymbolTable<std::string> &nonterm,
        const SymbolTable<std::string> &rule,
        std::ostream &ost) const;

    void changeRuleId(const std::vector<unsigned> &transTable)
    {
        for (std::vector<Record>::iterator it = _recs.begin(); it != _recs.end(); ++it) {
            it->_rule = transTable[it->_rule];
        }
        sort();
    }

private:
    std::vector<Record> _recs;
};

class Grammar {
public:
    /// Table types
	/// Lexent ID -> Nonterm ID
	typedef std::map<unsigned, unsigned> LexTable;

	/// (Left dtr, Right dtr) -> ParentSet
	// typedef std::map<std::pair<unsigned, unsigned>, ParentSet> BinaryTable;
    typedef BinaryRuleTable BinaryTable;

	/// Unary dtr -> ParentSet
	typedef std::map<unsigned, ParentSet> UnaryTable;

public:
	/// Symbol tables
	SymbolTable<std::string> _nonterm;
	SymbolTable<std::string> _lexent;
	mutable SymbolTable<std::string> _rule; /// rules may be added after initialization

	LexTable _lexToNonterm;
	BinaryTable _binary;
	UnaryTable _unary;
	std::set<unsigned> _root;

public:
    void clear(void)
    {
        _nonterm.clear();
        _lexent.clear();
        _rule.clear();
        _lexToNonterm.clear();
        _binary.clear();
        _unary.clear();
        _root.clear();
    }

public:
    /// Load binary data file
    void load(std::istream &ist);

    /// Write binary data file
    void write(std::ostream &ost);

    /// Load raw data file
    void loadRawFile(std::istream &ist);

    /// Write raw data file
    void writeRawFile(std::ostream &ost);

    ///
    void changeRuleId(const SymbolTable<std::string> &rule);
};

} /// namespace cfg
} /// namespace mogura

#endif // CfgGrammar_h__
