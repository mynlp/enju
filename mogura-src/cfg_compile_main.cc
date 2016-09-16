#include "Derivbank.h"
#include "FSArray.h"
#include "Fs.h"
#include "Grammar.h"
#include "hashdef.h"
#include "ParseTree.h"
#include "Restrictor.h"

#include <ctime>
#include <limits>
#include <liblilfes/utility.h>

typedef mayz::FSArray<std::string, lilfes::FSP> TemplateDB;

struct Global {
    lilfes::machine *machine;
    up::Grammar *grammar;

    std::vector<lilfes::FSP> unary;
    std::vector<lilfes::FSP> binary;

    lilfes::procedure *max_level;
    lilfes::procedure *schema_list;
    lilfes::procedure *sign_to_string;
    lilfes::procedure *fill_runtime_features;
    lilfes::procedure *derivation_to_parse_tree;

    std::vector<up::Fs> maxSymSet; // from derivank; optional
} g;


// in ll/src/builtinfs.cc
namespace lilfes {
namespace builtin {
bool subsume(lilfes::machine &mach, FSP arg1, FSP arg2);
bool unifiable(lilfes::machine &mach, FSP arg1, FSP arg2);
}
}

////////////////////////////////////////////////////////////////////////////////
// util for logging
////////////////////////////////////////////////////////////////////////////////
std::string getTime(void)
{
    std::time_t time;
    std::time(&time);
    std::string s = std::ctime(&time);
    s.resize(s.size()-1); // remove trailing '\n'
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// Sign <-> symbol-ID mapping
////////////////////////////////////////////////////////////////////////////////
struct FsHasher {
public:
    std::size_t operator()(const up::Fs &v) const
    {
        std::size_t n = 0;
        for (up::Fs::const_iterator c = v.begin(); c != v.end(); ++c) {
            n = n * 37 + *c;
        }
        return n;
    }
};

class SignTable {
public:
    bool known(const up::Fs &fs) const
    {
        return _table.find(fs) != _table.end();
    }

    std::pair<unsigned, bool> getId(const up::Fs &fs)
    {
        std::pair<HashTable::iterator, bool> it = _table.insert(std::make_pair(fs, (unsigned) _table.size()));
        if (it.second) {
            _array.resize(it.first->second + 1);
            _array[ it.first->second ] = fs;
        }
        return std::make_pair(it.first->second, it.second);
    }

    const up::Fs &getSign(unsigned id) const
    {
        return _array[id];
    }

    unsigned size(void) const { return _table.size(); }

    void swap(SignTable &st)
    {
        st._table.swap(_table);
        st._array.swap(_array);
    }

private:
    typedef _HASHMAP<up::Fs, unsigned, FsHasher> HashTable;
    HashTable _table;
    std::vector<up::Fs> _array;
};

////////////////////////////////////////////////////////////////////////////////
// Set of signs
////////////////////////////////////////////////////////////////////////////////
typedef _HASHSET<up::Fs, FsHasher> SignSet;

////////////////////////////////////////////////////////////////////////////////
// CFG rule
////////////////////////////////////////////////////////////////////////////////
struct Rule {
    unsigned _schema;
    unsigned _left;
    unsigned _right; // std::numeric_limits<unsigned>::max(): unary, otherwise: binary
    unsigned _mother;

    Rule(unsigned schema, unsigned left, unsigned right, unsigned mother)
        : _schema(schema)
        , _left(left)
        , _right(right)
        , _mother(mother) {}

    Rule(unsigned schema, unsigned unary, unsigned mother)
        : _schema(schema)
        , _left(unary)
        , _right(std::numeric_limits<unsigned>::max())
        , _mother(mother) {}

    bool isUnary(void) const { return _right == std::numeric_limits<unsigned>::max(); }
};

////////////////////////////////////////////////////////////////////////////////
// Symbol set at each level
////////////////////////////////////////////////////////////////////////////////
struct SymbolSet {
    std::vector<unsigned> tmplSymbols;
    SignTable symbols;
    std::vector<Rule> rules;
    std::vector<std::vector<unsigned> > subsumed;

    void swap(SymbolSet &ss)
    {
        tmplSymbols.swap(ss.tmplSymbols);
        symbols.swap(ss.symbols);
        rules.swap(ss.rules);
        subsumed.swap(ss.subsumed);
    }
};

////////////////////////////////////////////////////////////////////////////////
// restriction
////////////////////////////////////////////////////////////////////////////////
void restrict(unsigned level, up::Fs in, up::Fs &out)
{
    lilfes::IPTrailStack iptrail(g.machine);
    lilfes::FSP inFSP(*g.machine, in);
    lilfes::FSP outFSP(*g.machine);
    Restrictor::getInstance()->restrict(*g.machine, level, inFSP, outFSP);
    outFSP.Serialize(out);
}

void restrict(unsigned level, lilfes::FSP in, up::Fs &out)
{
    lilfes::IPTrailStack iptrail(g.machine);
    lilfes::FSP outFSP(*g.machine);
    Restrictor::getInstance()->restrict(*g.machine, level, in, outFSP);
    outFSP.Serialize(out);
}

////////////////////////////////////////////////////////////////////////////////
// base level: any sign -> bot
////////////////////////////////////////////////////////////////////////////////
void baseLevel(TemplateDB &templates, SymbolSet &ss)
{
    lilfes::FSP bot(g.machine);
    up::Fs botFs;
    bot.Serialize(botFs);
    (void) ss.symbols.getId(botFs); // always 0

    // all unary schemata
    for (unsigned s = 0; s < g.unary.size(); ++s) {
        ss.rules.push_back(Rule(s, 0, 0));
    }

    // all binary schemata
    for (unsigned s = 0; s < g.binary.size(); ++s) {
        ss.rules.push_back(Rule(s, 0, 0, 0));
    }

    // count number of templates
    unsigned n = 0;
    for (TemplateDB::iterator it = templates.begin(); it != templates.end(); ++it) {
        ++n;
    }
    ss.tmplSymbols.clear();
    ss.tmplSymbols.resize(n, 0);

    if (! g.maxSymSet.empty()) {
        ss.subsumed.resize(1);
        ss.subsumed[0].resize(g.maxSymSet.size());
        for (unsigned i = 0; i < ss.subsumed[0].size(); ++i) {
            ss.subsumed[0][i] = i; // all derivank signs are subsumed by bot
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Schema application and restriction
////////////////////////////////////////////////////////////////////////////////
#if 0
void applyUnary(
    lilfes::machine &mach,
    unsigned level,
    up::Grammar &grammar,
    const Rule &rule,
    std::vector<lilfes::FSP> &unary,
    unsigned dtr,
    lilfes::FSP dtrFSP,
    SignTable &currSymbols,
    std::vector<Rule> &currRules,
    std::vector<std::vector<unsigned> > &successors,
    std::vector<unsigned> &precursor
) {
    lilfes::IPTrailStack iptrail(&mach);

    //lilfes::FSP dtrFSP(mach, currSymbols.getSign(dtr));
    //if (! up::call_proc(&mach, fill_runtime_features, dtrFSP)) {
    //    throw std::runtime_error("fill_runtime_features failed");
    //}

    lilfes::FSP motherFSP(mach);
    lilfes::FSP dcpFSP(mach);

    bool ok = grammar.idSchemaUnary(unary[rule._schema], dtrFSP, motherFSP, dcpFSP)
            && lilfes::builtin::lilfes_call(mach, dcpFSP)
            && mach.GetIP() != NULL
            && mach.Execute(mach.GetIP());

    if (! ok) {
        return;
    }

    up::Fs motherFs;
    Restrictor::getInstance()->restrict(mach, level, motherFSP, motherFs);

    std::pair<unsigned, bool> motherId = currSymbols.getId(motherFs);

    currRules.push_back(Rule(rule._schema, dtr, motherId.first));

    if (motherId.second) { // new symbol
        successors[ rule._mother ].push_back(motherId.first);
        if (precursor.size() <= motherId.first) {
            precursor.resize(motherId.first + 1, std::numeric_limits<unsigned>::max());
        }
        precursor[ motherId.first ] = rule._mother;
    }
}

void applyBinaryImpl(
    lilfes::machine &mach,
    unsigned level,
    up::Grammar &grammar,
    const Rule &rule,
    std::vector<lilfes::FSP> &binary,
    unsigned ldtr,
    lilfes::FSP ldtrFSP,
    unsigned rdtr,
    lilfes::FSP rdtrFSP,
    SignTable &currSymbols,
    std::vector<Rule> &currRules,
    std::vector<std::vector<unsigned> > &successors,
    std::vector<unsigned> &precursor
) {
    lilfes::IPTrailStack iptrail(&mach);

    lilfes::FSP motherFSP(mach);
    lilfes::FSP dcpFSP(mach);

    bool ok = grammar.idSchemaBinary(binary[rule._schema], ldtrFSP, rdtrFSP, motherFSP, dcpFSP)
            && lilfes::builtin::lilfes_call(mach, dcpFSP)
            && mach.GetIP() != NULL
            && mach.Execute(mach.GetIP());
    if (! ok) {
        return;
    }

    up::Fs motherFs;
    Restrictor::getInstance()->restrict(mach, level, motherFSP, motherFs);

    std::pair<unsigned, bool> motherId = currSymbols.getId(motherFs);

    currRules.push_back(Rule(rule._schema, ldtr, rdtr, motherId.first));

    if (motherId.second) { // new symbol
        successors[ rule._mother ].push_back(motherId.first);
        if (precursor.size() <= motherId.first) {
            precursor.resize(motherId.first + 1, std::numeric_limits<unsigned>::max());
        }
        precursor[ motherId.first ] = rule._mother;
    }
}

void applyBinary(
    lilfes::machine &mach,
    unsigned level,
    up::Grammar &grammar,
    const Rule &rule,
    std::vector<lilfes::FSP> &binary,
    unsigned ldtr,
    unsigned rdtr,
    lilfes::FSP rdtrFSP,
    SignTable &currSymbols,
    std::vector<Rule> &currRules,
    std::vector<std::vector<unsigned> > &successors,
    std::vector<unsigned> &precursor
) {
    lilfes::IPTrailStack iptrail(&mach);

    lilfes::FSP ldtrFSP(mach, currSymbols.getSign(ldtr));
    if (! up::call_proc(&mach, fill_runtime_features, ldtrFSP)) {
        throw std::runtime_error("fill_runtime_features failed");
    }
    applyBinaryImpl(mach, level, grammar, rule, binary, ldtr, ldtrFSP, rdtr, rdtrFSP, currSymbols, currRules, successors, precursor);
}

void applyBinary(
    lilfes::machine &mach,
    unsigned level,
    up::Grammar &grammar,
    const Rule &rule,
    std::vector<lilfes::FSP> &binary,
    unsigned ldtr,
    lilfes::FSP ldtrFSP,
    unsigned rdtr,
    SignTable &currSymbols,
    std::vector<Rule> &currRules,
    std::vector<std::vector<unsigned> > &successors,
    std::vector<unsigned> &precursor
) {
    lilfes::IPTrailStack iptrail(&mach);

    lilfes::FSP rdtrFSP(mach, currSymbols.getSign(rdtr));
    if (! up::call_proc(&mach, fill_runtime_features, rdtrFSP)) {
        throw std::runtime_error("fill_runtime_features failed");
    }
    applyBinaryImpl(mach, level, grammar, rule, binary, ldtr, ldtrFSP, rdtr, rdtrFSP, currSymbols, currRules, successors, precursor);
}
#endif

bool checkSubsumption(const up::Fs &fs, unsigned precursor, const SymbolSet &prev, std::vector<unsigned> &subsumed)
{
    subsumed.clear();

    assert(! prev.subsumed[ precursor ].empty());
    const std::vector<unsigned> &prevSub = prev.subsumed[ precursor ];
    for (std::vector<unsigned>::const_iterator s = prevSub.begin(); s != prevSub.end(); ++s) {
        lilfes::IPTrailStack iptrail(g.machine);

        ////TMP
        //if (lilfes::builtin::subsume(*g.machine, lilfes::FSP(g.machine, fs), lilfes::FSP(g.machine, g.maxSymSet[*s]))) {
        if (lilfes::builtin::unifiable(*g.machine, lilfes::FSP(g.machine, fs), lilfes::FSP(g.machine, g.maxSymSet[*s]))) {
            subsumed.push_back(*s);
        }
    }

    return ! subsumed.empty();
}

void applyUnary(
    unsigned level,
    const Rule &rule,
    unsigned dtr,
    const SymbolSet &prev,
    SymbolSet &curr,
    std::vector<std::vector<unsigned> > &successors,
    std::vector<unsigned> &precursor
) {
    lilfes::IPTrailStack iptrail(g.machine);

    lilfes::FSP dtrFSP(*g.machine, curr.symbols.getSign(dtr));
    if (! up::call_proc(g.machine, g.fill_runtime_features, dtrFSP)) {
        throw std::runtime_error("fill_runtime_features failed");
    }

    lilfes::FSP motherFSP(g.machine);
    lilfes::FSP dcpFSP(g.machine);

    bool ok = g.grammar->idSchemaUnary(g.unary[rule._schema], dtrFSP, motherFSP, dcpFSP)
            && lilfes::builtin::lilfes_call(*g.machine, dcpFSP)
            && g.machine->GetIP() != NULL
            && g.machine->Execute(g.machine->GetIP());

    if (! ok) {
        return;
    }

    up::Fs motherFs;
    Restrictor::getInstance()->restrict(*g.machine, level, motherFSP, motherFs);

    std::vector<unsigned> currSub;
    if (! g.maxSymSet.empty() && ! curr.symbols.known(motherFs)) {
        if (! checkSubsumption(motherFs, rule._mother, prev, currSub)) {
            return; // the new mother symbol was not found in the derivbank
        }
    }

    std::pair<unsigned, bool> motherId = curr.symbols.getId(motherFs);

    curr.rules.push_back(Rule(rule._schema, dtr, motherId.first));

    if (motherId.second) { // new symbol
        if (curr.subsumed.size() <= motherId.first) {
            curr.subsumed.resize(motherId.first + 1);
        }
        curr.subsumed[ motherId.first ].swap(currSub);

        successors[ rule._mother ].push_back(motherId.first);
        if (precursor.size() <= motherId.first) {
            precursor.resize(motherId.first + 1, std::numeric_limits<unsigned>::max());
        }
        precursor[ motherId.first ] = rule._mother;
    }
}

void applyBinary(
    unsigned level,
    const Rule &rule,
    unsigned ldtr,
    unsigned rdtr,
    const SymbolSet &prev,
    SymbolSet &curr,
    std::vector<std::vector<unsigned> > &successors,
    std::vector<unsigned> &precursor
) {
    lilfes::IPTrailStack iptrail(g.machine);

    lilfes::FSP ldtrFSP(g.machine, curr.symbols.getSign(ldtr));
    if (! up::call_proc(g.machine, g.fill_runtime_features, ldtrFSP)) {
        throw std::runtime_error("fill_runtime_features failed");
    }

    lilfes::FSP rdtrFSP(g.machine, curr.symbols.getSign(rdtr));
    if (! up::call_proc(g.machine, g.fill_runtime_features, rdtrFSP)) {
        throw std::runtime_error("fill_runtime_features failed");
    }

    lilfes::FSP motherFSP(g.machine);
    lilfes::FSP dcpFSP(g.machine);

    bool ok = g.grammar->idSchemaBinary(g.binary[rule._schema], ldtrFSP, rdtrFSP, motherFSP, dcpFSP)
            && lilfes::builtin::lilfes_call(*g.machine, dcpFSP)
            && g.machine->GetIP() != NULL
            && g.machine->Execute(g.machine->GetIP());
    if (! ok) {
        return;
    }

    up::Fs motherFs;
    Restrictor::getInstance()->restrict(*g.machine, level, motherFSP, motherFs);

    std::vector<unsigned> currSub;
    if (! g.maxSymSet.empty() && ! curr.symbols.known(motherFs)) {
        if (! checkSubsumption(motherFs, rule._mother, prev, currSub)) {
            return; // the new mother symbol was not found in the derivbank
        }
    }

    std::pair<unsigned, bool> motherId = curr.symbols.getId(motherFs);

    curr.rules.push_back(Rule(rule._schema, ldtr, rdtr, motherId.first));

    if (motherId.second) { // new symbol
        if (curr.subsumed.size() <= motherId.first) {
            curr.subsumed.resize(motherId.first + 1);
        }
        curr.subsumed[motherId.first].swap(currSub);

        successors[ rule._mother ].push_back(motherId.first);
        if (precursor.size() <= motherId.first) {
            precursor.resize(motherId.first + 1, std::numeric_limits<unsigned>::max());
        }
        precursor[ motherId.first ] = rule._mother;
    }
}

////////////////////////////////////////////////////////////////////////////////
// main loop
////////////////////////////////////////////////////////////////////////////////
void compile(
    unsigned level,
    TemplateDB &templates,
    const SymbolSet &prev,
    SymbolSet &curr
) {
    std::vector<std::vector<unsigned> > ruleIndex(prev.symbols.size());
    std::vector<std::vector<unsigned> > successors(prev.symbols.size());
    std::vector<unsigned> precursor;

    // make rule index
    for (unsigned i = 0; i < prev.rules.size(); ++i) {
        const Rule &r = prev.rules[i];
        ruleIndex[ r._left ].push_back(i);
        if (! r.isUnary() && r._left != r._right) {
            ruleIndex[ r._right ].push_back(i);
        }
    }

    // stage 0: restriction of lexical templates
    curr.tmplSymbols.clear();
    curr.tmplSymbols.resize(prev.tmplSymbols.size(), std::numeric_limits<unsigned>::max());
    unsigned tmplIx = 0;
    for (TemplateDB::iterator it = templates.begin(); it != templates.end(); ++it, ++tmplIx) {
        if (prev.tmplSymbols[ tmplIx ] == std::numeric_limits<unsigned>::max()) {
            continue; // precursor is not in the derivbank
        }

        up::Fs tmpl;
        Restrictor::getInstance()->restrict(*g.machine, level, it->second, tmpl);

        std::vector<unsigned> currSub;
        if (! g.maxSymSet.empty() && ! curr.symbols.known(tmpl)) {
            if (! checkSubsumption(tmpl, prev.tmplSymbols[tmplIx], prev, currSub)) {
                continue; // the new symbol was not found in the derivbank
            }
        }

        std::pair<unsigned, bool> id = curr.symbols.getId(tmpl);
        curr.tmplSymbols[ tmplIx ] = id.first;

        if (id.second) { // new symbol
            if (curr.subsumed.size() <= id.first) {
                curr.subsumed.resize(id.first + 1);
            }
            curr.subsumed[id.first].swap(currSub);

            successors[ prev.tmplSymbols[ tmplIx ] ].push_back(id.first);
            if (precursor.size() <= id.first) {
                precursor.resize(id.first + 1, std::numeric_limits<unsigned>::max());
            }
            precursor[ id.first ] = prev.tmplSymbols[ tmplIx ];
        }
    }

    *lilfes::error_stream << getTime() << ": restriction of templates: done" << std::endl;
    *lilfes::error_stream << "  # of symbols = " << curr.symbols.size() << std::endl;

    // stage 1: iteration until reaching fix-point
    unsigned symbol = 0;
    while (symbol < curr.symbols.size()) {
        //lilfes::IPTrailStack iptrail(&mach);

        //lilfes::FSP symbolFSP(mach, currSymbols.getSign(symbol));
        //if (! up::call_proc(&mach, fill_runtime_features, symbolFSP)) {
            //throw std::runtime_error("fill_runtime_features failed");
        //}

        unsigned pre = precursor[symbol];
        const std::vector<unsigned> &rules = ruleIndex[pre];
        for (std::vector<unsigned>::const_iterator r = rules.begin(); r != rules.end(); ++r) {
            const Rule &rule = prev.rules[*r];
            if (rule.isUnary()) {
                applyUnary(level, rule, symbol, prev, curr, successors, precursor);
            }
            else {
                if (rule._left == pre) { // symbol <right> -> <mother>
                    for (unsigned j = 0; j < successors[ rule._right ].size(); ++j) {
                        if (symbol < successors[ rule._right ][j]) {
                            break;
                        }
                        applyBinary(level, rule, symbol, successors[ rule._right ][j], prev, curr, successors, precursor);
                    }
                }
                if (rule._right == pre) { 
                    for (unsigned j = 0; j < successors[ rule._left ].size(); ++j) {
                        if (symbol <= successors[ rule._left ][j]) { // <left> = <right> = symbol --> processed in (rule._left == pre) case
                            break;
                        }
                        applyBinary(level, rule, successors[ rule._left ][j], symbol, prev, curr, successors, precursor);
                    }
                }
            }
        }
        ++symbol;

        if (symbol % 100 == 0) {
            *lilfes::error_stream << getTime() << ": " << symbol << " symbols processed, "
                                  << "# of symbols = " << curr.symbols.size() << ", "
                                  << "# of rules = " << curr.rules.size() << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// wrapper of lilfes predicates
////////////////////////////////////////////////////////////////////////////////
unsigned getMaxLevel(void)
{
    lilfes::FSP maxLevelFSP(g.machine);
    if (! up::call_proc(g.machine, g.max_level, maxLevelFSP)) {
        throw std::runtime_error("max_level/1 failed");
    }
    if (! maxLevelFSP.IsInteger()) {
        throw std::runtime_error("max_level/1 returned non-integer");
    }
    if (maxLevelFSP.ReadInteger() < 0) {
        throw std::runtime_error("max_level/1 returned negative-integer");
    }
    return maxLevelFSP.ReadInteger();
}

void getSchemaList(void)
{
    lilfes::FSP unaryListFSP(g.machine);
    lilfes::FSP binaryListFSP(g.machine);
    if (! up::call_proc(g.machine, g.schema_list, unaryListFSP, binaryListFSP)) {
        throw std::runtime_error("schema_list/2 failed");
    }

    if (! lilfes::list_to_vector<lilfes::FSP>(*g.machine, unaryListFSP, g.unary)) {
        std::runtime_error("schema_list/2 returned non-list");
    }

    if (! lilfes::list_to_vector<lilfes::FSP>(*g.machine, binaryListFSP, g.binary)) {
        std::runtime_error("schema_list/2 returned non-list");
    }
}

std::string sign2string(const up::Fs &sign)
{
    lilfes::IPTrailStack iptrail(g.machine);

    lilfes::FSP signFSP(g.machine, sign);
    lilfes::FSP stringFSP(g.machine);
    if (! up::call_proc(g.machine, g.sign_to_string, signFSP, stringFSP)) {
        throw std::runtime_error("sign_to_string/2 failed");
    }

    if (! stringFSP.IsString()) {
        throw std::runtime_error("sign_to_string/2 returned non-string in the arg");
    }

    return stringFSP.ReadString();
}

bool isRoot(const up::Fs &sign)
{
    lilfes::IPTrailStack iptrail(g.machine);

    lilfes::FSP signFSP(g.machine, sign);
    return g.grammar->rootSign(signFSP);
}

bool derivToParseTree(lilfes::FSP deriv, mayz::ParseTree tree)
{
    lilfes::FSP treeFSP = tree;
    return up::call_proc(g.machine, g.derivation_to_parse_tree, deriv, treeFSP);
}

////////////////////////////////////////////////////////////////////////////////
// printer
////////////////////////////////////////////////////////////////////////////////
void printRules(
    TemplateDB &templates,
    const SymbolSet &ss,
    std::ostream &out
) {
    // make symbol names
    std::vector<std::string> symbolNames(ss.symbols.size());
    for (unsigned i = 0; i < ss.symbols.size(); ++i) {
        std::ostringstream oss;
        oss << sign2string(ss.symbols.getSign(i)) << "_" << i;
        symbolNames[i] = oss.str();
    }

    // schema names
    std::vector<std::string> unarySchemaNames(g.unary.size());
    std::vector<std::string> binarySchemaNames(g.binary.size());
    for (unsigned i = 0; i < g.unary.size(); ++i) {
        unarySchemaNames[i] = g.unary[i].GetType()->GetSimpleName();
    }
    for (unsigned i = 0; i < g.binary.size(); ++i) {
        binarySchemaNames[i] = g.binary[i].GetType()->GetSimpleName();
    }


    // terminal: lexical template -> CFG symbol
    unsigned tmplIx = 0;
    for (TemplateDB::iterator it = templates.begin(); it != templates.end(); ++it, ++tmplIx) {
        if (ss.tmplSymbols[ tmplIx ] != std::numeric_limits<unsigned>::max()) {
            out << "t " << symbolNames[ ss.tmplSymbols[ tmplIx ] ] << " -> " << it->first << std::endl;
        }
        else { // (ss.tmplSymbols[ tmplIx ] == std::numeric_limits<unsigned>::max()) => not covered in the derivbank
            out << "t " << "DEAD" << " -> " << it->first << std::endl;
        }
    }

    // unary & binary rules
    for (std::vector<Rule>::const_iterator r = ss.rules.begin(); r != ss.rules.end(); ++r) {
        if (r->isUnary()) {
            out << "u " << symbolNames[ r->_mother ] << " -> "
                << symbolNames[ r->_left ] << " "
                << unarySchemaNames[ r->_schema ] << std::endl;
        }
        else { // binary
            out << "b " << symbolNames[ r->_mother ] << " -> "
                << symbolNames[ r->_left ] << " "
                << symbolNames[ r->_right ] << " "
                << binarySchemaNames[ r->_schema ] << std::endl;
        }
    }

    // start symbol
    for (unsigned i = 0; i < ss.symbols.size(); ++i) {
        if (isRoot(ss.symbols.getSign(i))) {
            out << "s " << symbolNames[i] << std::endl;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Load signs from a derivank file
////////////////////////////////////////////////////////////////////////////////
void storeSign(unsigned maxLevel, mayz::ParseTree tree, SignSet &sset)
{
    lilfes::FSP symbol(g.machine);
    Restrictor::getInstance()->restrict(*g.machine, maxLevel, tree.edgeSign(), symbol);

    up::Fs symbolFs;
    symbol.Serialize(symbolFs);
    sset.insert(symbolFs);

    if (tree.isBinary()) {
        storeSign(maxLevel, tree.LTree(), sset);
        storeSign(maxLevel, tree.RTree(), sset);
    }
    else if (tree.isUnary()) {
        storeSign(maxLevel, tree.UTree(), sset);
    }
}

void loadMaximumSymbolSet(const std::vector<std::string> &derivbankFiles, unsigned level)
{
    *lilfes::error_stream << getTime() << std::endl;
    *lilfes::error_stream << "Loading symbols from derivbank.." << std::endl;

    g.maxSymSet.clear();
	SignSet sset;

	for (unsigned i = 0; i < derivbankFiles.size(); ++i) {

		lilfes::IPTrailStack iptrail(g.machine);

		*lilfes::error_stream << "derivbank " << (i + 1) << ": " << derivbankFiles[i] << std::endl;
		mayz::Derivbank derivbank(g.machine);
		if (! derivbank.load(derivbankFiles[i])) {
			throw std::runtime_error("error in loading derivbank from " + derivbankFiles[i]);
		}

		unsigned n = 0;
		for (mayz::Derivbank::iterator it = derivbank.begin(); it != derivbank.end(); ++it) {
			lilfes::IPTrailStack iptrail2(g.machine);

			mayz::ParseTree tree(*g.machine);
			if (derivToParseTree(it->second, tree)) {
				storeSign(level, tree, sset);
			}

			if (++n % 1000 == 0) {
				*lilfes::error_stream << ".";
				if (n % 10000 == 0) {
					*lilfes::error_stream << n << " sentences" << std::endl;
				}
			}
		}
    	*lilfes::error_stream << " done" << std::endl;
	}

    g.maxSymSet.insert(g.maxSymSet.end(), sset.begin(), sset.end());

    *lilfes::error_stream << getTime() << std::endl;
    *lilfes::error_stream << "number of derivbank symbols = " << g.maxSymSet.size() << std::endl;
}

void loadMaximumSymbolSetFromSymbolDb(const std::vector<std::string> &symbolDbFileNames, unsigned level)
{
    *lilfes::error_stream << getTime() << std::endl;
    *lilfes::error_stream << "Loading symbols from databases .." << std::endl;

    g.maxSymSet.clear();
	SignSet sset;

	for (unsigned i = 0; i < symbolDbFileNames.size(); ++i) {

		lilfes::IPTrailStack iptrail(g.machine);

		*lilfes::error_stream << "symbol database " << (i + 1) << ": " << symbolDbFileNames[i] << std::endl;
        mayz::FSArray<lilfes::FSP, lilfes::FSP> db(g.machine);
		if (! db.load(symbolDbFileNames[i])) {
			throw std::runtime_error("error in loading symbols from " + symbolDbFileNames[i]);
		}

		for (mayz::FSArray<lilfes::FSP, lilfes::FSP>::iterator it = db.begin(); it != db.end(); ++it) {
			lilfes::IPTrailStack iptrail2(g.machine);

            lilfes::FSP symbol(g.machine);
            Restrictor::getInstance()->restrict(*g.machine, level, it->first, symbol);

            up::Fs symbolFs;
            symbol.Serialize(symbolFs);
            sset.insert(symbolFs);
		}
    	*lilfes::error_stream << " done" << std::endl;
	}

    g.maxSymSet.insert(g.maxSymSet.end(), sset.begin(), sset.end());

    *lilfes::error_stream << getTime() << std::endl;
    *lilfes::error_stream << "number of restricted symbols = " << g.maxSymSet.size() << std::endl;
}

#if 0
void loadMaximumSymbolSet(const std::string &derivbankFile, unsigned level)
{
    *lilfes::error_stream << getTime() << std::endl;
    *lilfes::error_stream << "Loading symbols from derivbank.." << std::endl;

    g.maxSymSet.clear();
    SignSet sset;

    lilfes::IPTrailStack iptrail(g.machine);

    *lilfes::error_stream << "derivbank: " << derivbankFile << std::endl;
    mayz::Derivbank derivbank(g.machine);
    if (! derivbank.load(derivbankFile)) {
        throw std::runtime_error("error in loading derivbank from " + derivbankFile);
    }

    unsigned n = 0;
    for (mayz::Derivbank::iterator it = derivbank.begin(); it != derivbank.end(); ++it) {
        lilfes::IPTrailStack iptrail2(g.machine);

        mayz::ParseTree tree(*g.machine);
        if (derivToParseTree(it->second, tree)) {
            storeSign(level, tree, sset);
        }

        if (++n % 1000 == 0) {
            *lilfes::error_stream << ".";
            if (n % 10000 == 0) {
                *lilfes::error_stream << n << " sentences" << std::endl;
            }
        }
    }
    *lilfes::error_stream << " done" << std::endl;

    g.maxSymSet.insert(g.maxSymSet.end(), sset.begin(), sset.end());

    *lilfes::error_stream << getTime() << std::endl;
    *lilfes::error_stream << "number of derivbank symbols = " << g.maxSymSet.size() << std::endl;
}
#endif

void printUsage(void)
{
    *lilfes::error_stream << "Usage: cfg_compile <restriction-def> <template-db> [Options]" << std::endl;
    *lilfes::error_stream << "Options:" << std::endl;
    *lilfes::error_stream << "   -d <derivbank-level> <derivbank1>  .. : filtering by derivbank symbols" << std::endl;
    *lilfes::error_stream << "   -s <symbol-db-level> <symbol-db-1> .. : filtering by symbol database" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
try {
    if (argc != 3 && argc < 6) {
        printUsage();
        std::exit(1);
    }

    std::string lilfesModueName(argv[1]);
    std::string templateFileName(argv[2]);

    unsigned symbolDbLevel = 0;
    std::vector<std::string> symbolDbFileNames;
    unsigned derivbankLevel = 0;
	std::vector<std::string> derivbankFileNames;
    if (argc > 3) {
        if (std::string(argv[3]) == "-d") {
            derivbankLevel = std::atoi(argv[4]);
		    derivbankFileNames.insert(derivbankFileNames.end(), argv + 5, argv + argc);
        }
        else if (std::string(argv[3]) == "-s") {
            symbolDbLevel = std::atoi(argv[4]);
		    symbolDbFileNames.insert(symbolDbFileNames.end(), argv + 5, argv + argc);
        }
        else {
            printUsage();
            std::exit(1);
        }
    }

    *lilfes::error_stream << "cfg_compile (MAYZ " << MAYZ_VERSION << ")" << std::endl;
    *lilfes::error_stream << MAYZ_COPYRIGHT << std::endl;

    // Initialize everything
    lilfes::Initializer::PerformAll();
    g.machine = new lilfes::machine();

    // Register predicates to define restrictions
    Restrictor::init(g.machine);

    // Load restriction definitions
    *lilfes::error_stream << "Loading grammar module \"" << lilfesModueName << "\"... ";
    if ( ! load_module( *g.machine, "grammar module", lilfesModueName ) ) {
        *lilfes::error_stream << "grammar module not found" << std::endl;
        std::exit(1);
    }
    *lilfes::error_stream << "done." << std::endl;

    // Initialize grammar object
    *lilfes::error_stream << "Initializing grammar ...";
    g.grammar = new up::Grammar();
    if (! g.grammar->init(g.machine)) {
        *lilfes::error_stream << "failed." << std::endl;
        std::exit(1);
    }
    *lilfes::error_stream << "done." << std::endl;

    *lilfes::error_stream << "=====================================" << std::endl;
    *lilfes::error_stream << "restrictror definition:" << std::endl;
    Restrictor::getInstance()->dump(*lilfes::error_stream);
    *lilfes::error_stream << "=====================================" << std::endl;

    // Load template DB
    mayz::FSArray<std::string, lilfes::FSP> templates(g.machine);
    if (! templates.load(templateFileName)) {
        *lilfes::error_stream << "failed to load template DB" << std::endl;
        std::exit(1);
    }

    lilfes::module *cfgCompileModule = lilfes::module::SearchModule("mogura-devel:cfg_compile");
    if (! cfgCompileModule) {
        *lilfes::error_stream << "module not found: mogura-devel:cfg_compile" << std::endl;
        std::exit(1);
    }

    bool ok = up::init_proc(cfgCompileModule, g.max_level, "max_level", 1)
           && up::init_proc(cfgCompileModule, g.sign_to_string, "sign_to_string", 2)
           && up::init_proc(cfgCompileModule, g.fill_runtime_features, "fill_runtime_features", 1)
           && up::init_proc(cfgCompileModule, g.schema_list, "schema_list", 2)
           && up::init_proc(cfgCompileModule, g.derivation_to_parse_tree, "derivation_to_parse_tree", 2);
    if (! ok) {
        std::exit(1);
    }

    unsigned maxLevel = getMaxLevel();

    getSchemaList();

    // Load symbol set from derivbank files
    if (! derivbankFileNames.empty()) {
        *lilfes::error_stream << "derivbank symbol restriction level = " << derivbankLevel << std::endl;
        loadMaximumSymbolSet(derivbankFileNames, derivbankLevel);
    }

    // Load symbol set from symbol databases
    if (! symbolDbFileNames.empty()) {
        *lilfes::error_stream << "symbol database restriction level = " << derivbankLevel << std::endl;
        loadMaximumSymbolSetFromSymbolDb(symbolDbFileNames, symbolDbLevel);
    }

    // bottom level
    SymbolSet prev;
    baseLevel(templates, prev);

    for (unsigned level = 0; level <= maxLevel; ++level) {
        *lilfes::error_stream << "============================" << std::endl;
        *lilfes::error_stream << getTime() << ": Level " << level << ": BEGIN" << std::endl;
        *lilfes::error_stream << "============================" << std::endl;

        SymbolSet curr;
        compile(level, templates, prev, curr);

        if (! g.maxSymSet.empty()) {
            for (unsigned i = 0; i < curr.subsumed.size(); ++i) {
                assert(! curr.subsumed[i].empty());
            }
        }

        prev.swap(curr);

        unsigned numDead = 0;
        if (! g.maxSymSet.empty()) {
            for (unsigned i = 0; i < curr.tmplSymbols.size(); ++i) {
                if (curr.tmplSymbols[i] == std::numeric_limits<unsigned>::max()) {
                    ++numDead;
                }
            }
        }

        *lilfes::error_stream << "============================" << std::endl;
        *lilfes::error_stream << getTime() << ": Level " << level << ": END" << std::endl;
        *lilfes::error_stream << " # of symbols = " << prev.symbols.size() << std::endl;
        *lilfes::error_stream << " # of rules = " << prev.rules.size() << std::endl;
        if (! g.maxSymSet.empty()) {
            *lilfes::error_stream << " # of dead templates = " << numDead << std::endl;
        }
        *lilfes::error_stream << "============================" << std::endl;
    }

    // print result
    printRules(templates, prev, std::cout);

    lilfes::Terminator::PerformAll();

    return 0;
}
catch (std::exception &e)
{
    std::cerr << e.what() << std::endl;
    std::exit(1);
}
catch (...)
{
    std::cerr << "unknown exception" << std::endl;
    std::exit(1);
}

