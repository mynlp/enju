#include "BitStream.h"
#include "CfgGrammar.h"
#include "diffEncode.h"
#include "SymbolTable.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <vector>

//------------------------------------------------------------------------------
namespace mogura {
namespace cfg {
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Data structures
//------------------------------------------------------------------------------

enum SchemaType {
    ST_DTR01,
    ST_DTR10,
    ST_UNARY
};

struct UnaryRule {
    unsigned _schema;
    unsigned _mot;
    unsigned _dtr;

    UnaryRule(void) {}
    UnaryRule(unsigned schema, unsigned mot, unsigned dtr)
        : _schema(schema)
        , _mot(mot)
        , _dtr(dtr)
    {}

    bool operator<(const UnaryRule &r) const {
        if (_schema != r._schema) {
            return _schema < r._schema;
        }
        else if (_mot != r._mot) {
            return _mot < r._mot;
        }
        else {
            return _dtr < r._dtr;
        }
    }
};

struct BinaryRule {
    unsigned _schema;
    unsigned _mot;
    unsigned _dtr0;
    unsigned _dtr1;

    BinaryRule(void) {}
    BinaryRule(unsigned schema, unsigned mot, unsigned dtr0, unsigned dtr1)
        : _schema(schema)
        , _mot(mot)
        , _dtr0(dtr0)
        , _dtr1(dtr1)
    {}

    bool operator<(const BinaryRule &r) const {
        if (_schema != r._schema) {
            return _schema < r._schema;
        }
        else if (_mot != r._mot) {
            return _mot < r._mot;
        }
        else if (_dtr0 != r._dtr0) {
            return _dtr0 < r._dtr0;
        }
        else {
            return _dtr1 < r._dtr1;
        }
    }
};

/// Layout
/// [0] Header                                  : text
/// [1] NontermSymbols: (nonterm-symbol '\n')*  : text
/// [2] LexEntSymbols:  (lexent-symbol  '\n')*  : text
/// [3] Schemata:       (schema-symbol '\t' schema-type '\n') : text
/// [4] LexRules:       (lexent-ID nonter-ID)*  : integer array
/// [5] RootSymbols:    (nonterm-ID)*           : integer array
/// [6] BinaryTable:    (binary-rule-rep)*      : bit array
/// [7] UnaryTable:     (unary-rule-rep)*       : bit array
///

struct Header {
    unsigned _numNonterm;
    unsigned _numLexEnt;
    unsigned _numSchema;
    unsigned _numLexRule;
    unsigned _numRootSym;
    unsigned _numBinary;
    unsigned _numUnary;

    void write(std::ostream &ost) const
    {
        ost << _numNonterm << ' '
            << _numLexEnt  << ' '
            << _numSchema  << ' '
            << _numLexRule << ' '
            << _numRootSym << ' '
            << _numBinary  << ' '
            << _numUnary   << std::endl;
    }

    void read(std::istream &ist)
    {
        std::string line;
        if (! std::getline(ist, line)) {
            throw std::runtime_error("No header line");
        }

        std::istringstream iss(line);
        if (! (iss >> _numNonterm
                   >> _numLexEnt
                   >> _numSchema
                   >> _numLexRule
                   >> _numRootSym
                   >> _numBinary
                   >> _numUnary)
        ) {
            throw std::runtime_error("Header format error");
        }
    }
};

//------------------------------------------------------------------------------
// Read/write a integer
//------------------------------------------------------------------------------
inline
void writeNumber(unsigned n, OutputBitStream &obs)
{
    while (n >= 128) {
        obs << 0;
        for (unsigned i = 0; i < 7; ++i) {
            obs << (n & 1);
            n >>= 1;
        }
    }

    obs << 1;
    for (unsigned i = 0; i < 7; ++i) {
        obs << (n & 1);
        n >>= 1;
    }
}

inline
unsigned readNumber(InputBitStream &ibs)
{
    unsigned n = 0;
    unsigned ix = 0;
    unsigned headBit = 0;

    while (true) {
        if (! (ibs >> headBit)) {
            throw std::runtime_error("input terminated in a number");
        }

        for (unsigned i = 0; i < 7; ++i) {
            unsigned bit;
            if (! (ibs >> bit)) {
                throw std::runtime_error("input terminated in a number");
            }
            n |= (bit << ix++);
        }

        if (headBit == 1) {
            break;
        }
    }

    return n;
}

//------------------------------------------------------------------------------
// Writer and subroutines
//------------------------------------------------------------------------------
void renameSymbols(
    const SymbolTable<std::string> &orig,
    SymbolTable<unsigned> &rename,
    std::vector<std::string> &syms
) {
    syms.clear();
    /// Don't clear 'rename' here. It may already contain some mappings.

    for (SymbolTable<std::string>::const_iterator it = orig.begin(); it != orig.end(); ++it) {
        unsigned id = rename.getID(it->second);
        if (syms.size() <= id) {
            syms.resize(id + 1);
        }
        syms[id] = it->first;
    }
}

void makeSymbolVec(const SymbolTable<std::string> &symtab, std::vector<std::string> &v)
{
    v.clear();
    v.resize(symtab.getNumSymbol());
    for (SymbolTable<std::string>::const_iterator it = symtab.begin(); it != symtab.end(); ++it) {
        assert(it->second < (int) v.size());
        v[it->second] = it->first;
    }
}

void writeLexRules(
    const Grammar::LexTable &lexToNonterm,
    const SymbolTable<unsigned> &nontermRename,
    std::ostream &ost
) {
    typedef Grammar::LexTable::const_iterator LexRuleItr;
    std::vector<unsigned> v(lexToNonterm.size() * 2);
    unsigned ix = 0;
    for (LexRuleItr it = lexToNonterm.begin(); it != lexToNonterm.end(); ++it, ix += 2) {
        v[ix] = it->first;
        v[ix + 1] = nontermRename.getID(it->second);
    }
    ost.write(reinterpret_cast<const char*>(&(v[0])), sizeof(unsigned) * v.size());
}

void writeRootSymbols(
    const std::set<unsigned> &root,
    const SymbolTable<unsigned> &nontermRename,
    std::ostream &ost
) {
    std::vector<unsigned> v;
    for (std::set<unsigned>::const_iterator it = root.begin(); it != root.end(); ++it) {
        v.push_back(nontermRename.getID(*it));
    }
    ost.write(reinterpret_cast<const char*>(&(v[0])), sizeof(unsigned) * v.size());
}

void writeBinaryRules(
    const std::vector<BinaryRule> &binary,
    OutputBitStream &out
) {
    BinaryRule prev(0, 0, 0, 0);

    for (std::vector<BinaryRule>::const_iterator r = binary.begin(); r != binary.end(); ++r) {

        const BinaryRule &curr = *r;

        if (prev._schema != curr._schema) { /// type 4
            out << 1 << 1 << 1 << 1 << 0;
            assert(prev._schema < curr._schema);
            writeNumber(curr._schema - prev._schema - 1, out);
            writeNumber(curr._mot, out);
            writeNumber(curr._dtr0, out);
            writeNumber(curr._dtr1, out);
        }
        else if (prev._mot != curr._mot) { /// type 3
            out << 1 << 1 << 1 << 0;
            assert(prev._mot < curr._mot);
            writeNumber(curr._mot - prev._mot - 1, out);
            writeNumber(curr._dtr0, out);
            writeNumber(curr._dtr1, out);
        }
        else if (prev._dtr0 != curr._dtr0) { /// type 2
            out << 1 << 1 << 0;
            assert(prev._dtr0 < curr._dtr0);
            writeNumber(curr._dtr0 - prev._dtr0 - 1, out);
            writeNumber(curr._dtr1, out);
        }
        else if (prev._dtr1 != curr._dtr1) {
            if (prev._dtr1 + 1 == curr._dtr1) { /// (0 0 0 1)-pattern -> type 0
                out << 0;
            }
            else { /// type 1
                out << 1 << 0;
                assert(prev._dtr1 < curr._dtr1);
                writeNumber(curr._dtr1 - prev._dtr1 - 1, out);
            }
        }
        else {
            throw std::runtime_error("same binary rules");
        }

        prev = curr;
    }
}

void writeUnaryRule(
    const std::vector<UnaryRule> &unary,
    OutputBitStream &out
) {
    UnaryRule prev(0, 0, 0);

    for (std::vector<UnaryRule>::const_iterator r = unary.begin(); r != unary.end(); ++r) {

        const UnaryRule &curr = *r;

        if (prev._schema != curr._schema) { /// type 2
            out << 1 << 1 << 0;
            assert(prev._schema < curr._schema);
            writeNumber(curr._schema - prev._schema - 1, out);
            writeNumber(curr._mot, out);
            writeNumber(curr._dtr, out);
        }
        else if (prev._mot != curr._mot) { /// type 1
            out << 1 << 0;
            assert(prev._mot < curr._mot);
            writeNumber(curr._mot - prev._mot - 1, out);
            writeNumber(curr._dtr, out);
        }
        else if (prev._dtr != curr._dtr) { /// type 0
            out << 0;
            assert(prev._dtr < curr._dtr);
            writeNumber(curr._dtr - prev._dtr - 1, out);
        }
        else {
            throw std::runtime_error("same unary rules");
        }

        prev = curr;
    }
}

void makeRuleTables(
    const Grammar &grammar,
    SymbolTable<unsigned> &nontermRename,
    std::vector<BinaryRule> &binary,
    std::vector<UnaryRule> &unary,
    std::vector<SchemaType> &schemaType
) {
    nontermRename.clear();
    binary.clear();
    unary.clear();

    typedef std::set<std::pair<unsigned, unsigned> > PairSet;
    std::vector<std::vector<PairSet> > motDtrPairs(grammar._rule.getNumSymbol(), std::vector<PairSet>(2));

    std::cerr << "Collecting rules .. ";
#if 0
    for (Grammar::BinaryTable::const_iterator r = grammar._binary.begin(); r != grammar._binary.end(); ++r) {
        for (ParentSet::const_iterator p = r->second.begin(); p != r->second.end(); ++p) {
            std::pair<unsigned, unsigned> dtrs = r->first;
            /// initially, (schema, mot, left-dtr, right-dtr)
            binary.push_back(BinaryRule(p->_rule, p->_mot, dtrs.first, dtrs.second));
            
            motDtrPairs[p->_rule][0].insert(std::make_pair(p->_mot, dtrs.first)); // (mot, dtr0)
            motDtrPairs[p->_rule][1].insert(std::make_pair(p->_mot, dtrs.second)); // (mot, dtr1)
        }
    }
#endif
    for (Grammar::BinaryTable::RecordItr r = grammar._binary.begin(); r != grammar._binary.end(); ++r) {
        // std::pair<unsigned, unsigned> dtrs = r->first;
        std::pair<unsigned, unsigned> dtrs = std::make_pair(r->_left, r->_right);
        /// initially, (schema, mot, left-dtr, right-dtr)
        binary.push_back(BinaryRule(r->_rule, r->_mot, dtrs.first, dtrs.second));

        motDtrPairs[r->_rule][0].insert(std::make_pair(r->_mot, dtrs.first)); // (mot, dtr0)
        motDtrPairs[r->_rule][1].insert(std::make_pair(r->_mot, dtrs.second)); // (mot, dtr1)
    }

    for (Grammar::UnaryTable::const_iterator r = grammar._unary.begin(); r != grammar._unary.end(); ++r) {
        for (ParentSet::const_iterator p = r->second.begin(); p != r->second.end(); ++p) {
            unary.push_back(UnaryRule(p->_rule, p->_mot, r->first));
        }
    }
    std::cerr << "done" << std::endl;

    schemaType.clear();
    schemaType.resize(grammar._rule.getNumSymbol());
    for (unsigned i = 0; i < schemaType.size(); ++i) {
        if (motDtrPairs[i][0].empty() && motDtrPairs[i][1].empty()) {
            schemaType[i] = ST_UNARY;
        }
        else {
            schemaType[i] = (motDtrPairs[i][0].size() < motDtrPairs[i][1].size()) ? ST_DTR01 : ST_DTR10;
        }
    }

    std::cerr << "Reorder dtrs .. ";
    for (std::vector<BinaryRule>::iterator r = binary.begin(); r != binary.end(); ++r) {
        if (schemaType[r->_schema] == ST_DTR10) {
            std::swap(r->_dtr0, r->_dtr1);
        }
    }
    std::cerr << "done" << std::endl;

    std::cerr << "sort-1 .. ";
    std::sort(binary.begin(), binary.end());
    std::sort(unary.begin(), unary.end());
    std::cerr << "done" << std::endl;

    std::cerr << "Renaming .. ";
    for (std::vector<BinaryRule>::iterator r = binary.begin(); r != binary.end(); ++r) {
        r->_dtr1 = nontermRename.getID(r->_dtr1);
    }
    for (std::vector<BinaryRule>::iterator r = binary.begin(); r != binary.end(); ++r) {
        r->_dtr0 = nontermRename.getID(r->_dtr0);
    }
    for (std::vector<BinaryRule>::iterator r = binary.begin(); r != binary.end(); ++r) {
        r->_mot = nontermRename.getID(r->_mot);
    }
    for (std::vector<UnaryRule>::iterator r = unary.begin(); r != unary.end(); ++r) {
        r->_dtr = nontermRename.getID(r->_dtr);
    }
    for (std::vector<UnaryRule>::iterator r = unary.begin(); r != unary.end(); ++r) {
        r->_mot = nontermRename.getID(r->_mot);
    }
    std::cerr << "done" << std::endl;

    std::cerr << "sort-2 .. ";
    std::sort(binary.begin(), binary.end());
    std::sort(unary.begin(), unary.end());
    std::cerr << "done" << std::endl;
}

void writeDiffEncodedCfgFile(const Grammar &grammar, std::ostream &ost)
{
    SymbolTable<unsigned> nontermRename;
    std::vector<BinaryRule> binary;
    std::vector<UnaryRule> unary;
    std::vector<SchemaType> schemaType;
    makeRuleTables(grammar, nontermRename, binary, unary, schemaType);


    /// Symbol vectors
    std::vector<std::string> nonterms;
    renameSymbols(grammar._nonterm, nontermRename, nonterms); /// Collect redundant symbols
    assert(nonterms.size() == nontermRename.getNumSymbol());

    std::vector<std::string> lexents;
    makeSymbolVec(grammar._lexent, lexents);

    std::vector<std::string> schema;
    makeSymbolVec(grammar._rule, schema);
    schemaType.resize(schema.size(), ST_UNARY);


    Header header;
    header._numNonterm = nonterms.size();
    header._numLexEnt = lexents.size();
    header._numSchema = schema.size();
    header._numLexRule = grammar._lexToNonterm.size();
    header._numRootSym = grammar._root.size();
    header._numBinary = binary.size();
    header._numUnary = unary.size();


    std::cerr << "Writing header .. ";
    header.write(ost);
    std::cerr << "done" << std::endl;

    std::cerr << "Wrting nonterm symbols .. ";
    std::copy(nonterms.begin(), nonterms.end(), std::ostream_iterator<std::string>(ost, "\n"));
    std::cerr << "done" << std::endl;

    std::cerr << "Writing lexent symbols .. ";
    std::copy(lexents.begin(), lexents.end(), std::ostream_iterator<std::string>(ost, "\n"));
    std::cerr << "done" << std::endl;

    std::cerr << "Writing schema list .. ";
    for (unsigned i = 0; i < schema.size(); ++i) {
        ost << schema[i] << '\t';
        switch (schemaType[i]) {
            case ST_DTR01: ost << "DTR01"; break;
            case ST_DTR10: ost << "DTR10"; break;
            case ST_UNARY: ost << "UNARY"; break;
        }
        ost << std::endl;
    }
    std::cerr << "done";

    std::cerr << "Writing lex rules .. ";
    writeLexRules(grammar._lexToNonterm, nontermRename, ost);
    std::cerr << "done" << std::endl;

    std::cerr << "Writing root symbols .. ";
    writeRootSymbols(grammar._root, nontermRename, ost);
    std::cerr << "done" << std::endl;

    OutputBitStream obs(ost);

    std::cerr << "Writing binary rules .. ";
    writeBinaryRules(binary, obs);
    std::cerr << "done" << std::endl;

    std::cerr << "Writing unary rules .. ";
    writeUnaryRule(unary, obs);
    std::cerr << "done" << std::endl;

    std::cerr << "finished" << std::endl;
}


//------------------------------------------------------------------------------
// Reader and subroutines
//------------------------------------------------------------------------------
inline
unsigned readDiffType(InputBitStream &ibs)
{
    unsigned ret = 0;
    unsigned bit = 0;
    while (true) {
        if (! (ibs >> bit)) {
            throw std::runtime_error("input terminated in diff type code");
        }
        if (bit == 0) {
            break;
        }
        ++ret;
    }
    return ret;
}

BinaryRule readBinaryRule(InputBitStream &ibs, const BinaryRule &prev)
{
    BinaryRule rule;

    unsigned diffType = readDiffType(ibs);
    switch (diffType) {
        case 4:
            rule._schema = prev._schema + readNumber(ibs) + 1;
            rule._mot    = readNumber(ibs);
            rule._dtr0   = readNumber(ibs);
            rule._dtr1   = readNumber(ibs);
            break;
        case 3:
            rule._schema = prev._schema;
            rule._mot    = prev._mot + readNumber(ibs) + 1;
            rule._dtr0   = readNumber(ibs);
            rule._dtr1   = readNumber(ibs);
            break;
        case 2:
            rule._schema = prev._schema;
            rule._mot    = prev._mot;
            rule._dtr0   = prev._dtr0 + readNumber(ibs) + 1;
            rule._dtr1   = readNumber(ibs);
            break;
        case 1:
            rule._schema = prev._schema;
            rule._mot    = prev._mot;
            rule._dtr0   = prev._dtr0;
            rule._dtr1   = prev._dtr1 + readNumber(ibs) + 1;
            break;
        case 0:
            rule._schema = prev._schema;
            rule._mot    = prev._mot;
            rule._dtr0   = prev._dtr0;
            rule._dtr1   = prev._dtr1 + 1;
            break;
        default:
            throw std::runtime_error("invalid diff type (binary rule)");
    }

    return rule;
}

UnaryRule readUnaryRule(InputBitStream &ibs, const UnaryRule &prev)
{
    UnaryRule rule;

    unsigned diffType = readDiffType(ibs);
    switch (diffType) {
        case 2:
            rule._schema = prev._schema + readNumber(ibs) + 1;
            rule._mot    = readNumber(ibs);
            rule._dtr    = readNumber(ibs);
            break;
        case 1:
            rule._schema = prev._schema;
            rule._mot    = prev._mot + readNumber(ibs) + 1;
            rule._dtr    = readNumber(ibs);
            break;
        case 0:
            rule._schema = prev._schema;
            rule._mot    = prev._mot;
            rule._dtr    = prev._dtr + readNumber(ibs) + 1;
            break;
        default:
            throw std::runtime_error("invalid diff type (unary rule)");
    }

    return rule;
}

void loadDiffEncodedCfgFile(std::istream &ist, Grammar &grammar)
{
    grammar.clear();

    Header header;
    header.read(ist);

    /// NontermSymbols
    for (unsigned i = 0; i < header._numNonterm; ++i) {
        std::string symbol;
        if (! std::getline(ist, symbol)) {
            throw std::runtime_error("format error (nonterm)");
        }
        grammar._nonterm.setID(symbol, i);
    }

    /// LexEntSymbols
    for (unsigned i = 0; i < header._numLexEnt; ++i) {
        std::string symbol;
        if (! std::getline(ist, symbol)) {
            throw std::runtime_error("format error (lexent)");
        }
        grammar._lexent.setID(symbol, i);
    }

    /// Schemata
    std::vector<SchemaType> schemaType(header._numSchema);
    for (unsigned i = 0; i < header._numSchema; ++i) {
        std::string line;
        if (! std::getline(ist, line)) {
            throw std::runtime_error("too few schemata");
        }

        std::istringstream iss(line);
        std::string schema;
        std::string type;
        if (! (iss >> schema >> type)) {
            throw std::runtime_error("format error (schema): " + line);
        }

        grammar._rule.setID(schema, i);

        if (type == "DTR01") {
            schemaType[i] = ST_DTR01;
        }
        else if (type == "DTR10") {
            schemaType[i] = ST_DTR10;
        }
        else if (type == "UNARY") {
            schemaType[i] = ST_UNARY;
        }
        else {
            throw std::runtime_error("unknown schema type: " + type);
        }
    }

    /// LexRules
    std::vector<unsigned> lexToNonterm(header._numLexRule * 2);
    if (! ist.read(reinterpret_cast<char*>(&(lexToNonterm[0])), lexToNonterm.size() * sizeof(unsigned))) {
        throw std::runtime_error("too few lex rules");
    }

    for (unsigned i = 0; i < lexToNonterm.size(); i += 2) {
        unsigned lexent = lexToNonterm[i];
        unsigned nonterm = lexToNonterm[i + 1];
        grammar._lexToNonterm[lexent] = nonterm;
    }

    /// RootSymbols
    std::vector<unsigned> root(header._numRootSym);
    if (! ist.read(reinterpret_cast<char*>(&(root[0])), root.size() * sizeof(unsigned))) {
        throw std::runtime_error("too few root symbols");
    }
    grammar._root.insert(root.begin(), root.end());

    InputBitStream ibs(ist);

    /// BinaryTable
    grammar._binary.init(header._numBinary);
    BinaryRule prevBinary(0, 0, 0, 0);
    for (unsigned i = 0; i < header._numBinary; ++i) {
        BinaryRule rule = readBinaryRule(ibs, prevBinary);
        if (rule._schema >= schemaType.size()) {
            throw std::runtime_error("too large schema ID (binary rule)");
        }

        std::pair<unsigned, unsigned> dtrs = (schemaType[rule._schema] == ST_DTR10)
                                           ? std::make_pair(rule._dtr1, rule._dtr0)
                                           : std::make_pair(rule._dtr0, rule._dtr1);

        // grammar._binary[dtrs].push_back(MotRule(rule._mot, rule._schema));
        grammar._binary.put(dtrs.first, dtrs.second, rule._mot, rule._schema);

        prevBinary = rule;
    }
    grammar._binary.sort();

    /// UnaryTable
    UnaryRule prevUnary(0, 0, 0);
    for (unsigned i = 0; i < header._numUnary; ++i) {
        UnaryRule rule = readUnaryRule(ibs, prevUnary);
        grammar._unary[rule._dtr].push_back(MotRule(rule._mot, rule._schema));
        prevUnary = rule;
    }
}

//------------------------------------------------------------------------------
} /// namespace cfg {
} /// namespace mogura {
//------------------------------------------------------------------------------
