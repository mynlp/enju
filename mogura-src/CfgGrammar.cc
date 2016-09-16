#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include "CfgGrammar.h"
#include "diffEncode.h"

//------------------------------------------------------------------------------
namespace mogura {
namespace cfg {
//------------------------------------------------------------------------------

class FileFormatError : public std::runtime_error {
public:
    FileFormatError(const std::string &msg) : std::runtime_error(msg) {}
};

#if 0 /// OLD BINARY FORMAT VERSION
//------------------------------------------------------------------------------
// Header of binary format file
//------------------------------------------------------------------------------

struct Header {
    int _numSymbol;
    int _numLex;
    int _numUnary;
    int _numBinary;
    int _numRoot;
};

void printHeaderInfo(const Header &h, std::ostream &ost)
{
    ost << "# lex rules    = " << h._numLex << std::endl;
    ost << "# binary rules = " << h._numBinary << std::endl;
    ost << "# unary rules  = " << h._numUnary << std::endl;
    ost << "# root symbols = " << h._numRoot << std::endl;
}

//------------------------------------------------------------------------------
// translate IDs with cache
//------------------------------------------------------------------------------
unsigned getSymbol(
    unsigned id,
    const std::vector<std::string> &symbols,
    std::vector<int> &cache,
    SymbolTable<std::string> &symtab
) {
    if (id >= cache.size()) {
        throw FileFormatError("too large ID value");
    }

    if (cache[id] < 0) {
        cache[id] = symtab.getID(symbols[id]);
    }

    return cache[id];
}

//------------------------------------------------------------------------------
// reader functions for the parts of the binary file
//------------------------------------------------------------------------------
void readHeader(
    std::istream &ist,
    Header &header
) {
    std::string line;
    if (! std::getline(ist, line)) {
        throw FileFormatError("no header");
    }

    std::istringstream iss(line);
    if (! (iss >> header._numSymbol
               >> header._numLex
               >> header._numUnary
               >> header._numBinary
               >> header._numRoot)) {

        throw FileFormatError("header format error");
    }
}

void readSymbol(std::istream &ist, std::vector<std::string> &ss)
{
    for (std::vector<std::string>::iterator s = ss.begin(); s != ss.end(); ++s) {
        if (! std::getline(ist, *s)) {
            throw FileFormatError("too few symbols");
        }
    }
}

void readLexRule(
    std::istream &ist,
    const std::vector<std::string> &symbols,
    std::vector<int> &nonterm,
    unsigned numLex,
    Grammar &g
) {
    for (unsigned i = 0; i < numLex; ++i) {
        std::string line;
        if (! std::getline(ist, line)) {
            throw FileFormatError("too few lex rules");
        }

        std::istringstream iss(line);
        unsigned mot;
        std::string dtr;
        if (! (iss >> mot >> dtr)) {
            throw FileFormatError("lex rule format error");
        }

        unsigned m = getSymbol(mot, symbols, nonterm, g._nonterm);
        unsigned d = g._lexent.getID(dtr);

        g._lexToNonterm[d] = m;
    }
}

void readUnary(
    InputBitStream &ibs,
    const HuffmanTree::Decoder &decoder,
    const std::vector<std::string> &symbols,
    std::vector<int> &nonterm,
    std::vector<int> &rule,
    unsigned numRule,
    Grammar &g
) {
    for (unsigned i = 0; i < numRule; ++i) {
        unsigned m = getSymbol(decoder.decode(ibs), symbols, nonterm, g._nonterm);
        unsigned d = getSymbol(decoder.decode(ibs), symbols, nonterm, g._nonterm);
        unsigned r = getSymbol(decoder.decode(ibs), symbols, rule, g._rule);

        g._unary[d].push_back(MotRule(m, r));
    }
}

void readBinary(
    InputBitStream &ibs,
    const HuffmanTree::Decoder &decoder,
    const std::vector<std::string> &symbols,
    std::vector<int> &nonterm,
    std::vector<int> &rule,
    unsigned numRule,
    Grammar &g
) {
    for (unsigned i = 0; i < numRule; ++i) {
        unsigned m = getSymbol(decoder.decode(ibs), symbols, nonterm, g._nonterm);
        unsigned d1 = getSymbol(decoder.decode(ibs), symbols, nonterm, g._nonterm);
        unsigned d2 = getSymbol(decoder.decode(ibs), symbols, nonterm, g._nonterm);
        unsigned r = getSymbol(decoder.decode(ibs), symbols, rule, g._rule);

        g._binary[std::make_pair(d1, d2)].push_back(MotRule(m, r));
    }
}

void readRoot(
    InputBitStream &ibs,
    const HuffmanTree::Decoder &decoder,
    const std::vector<std::string> &symbols,
    std::vector<int> &nonterm,
    unsigned numRoot,
    Grammar &g
) {
    for (unsigned i = 0; i < numRoot; ++i) {
        unsigned root = getSymbol(decoder.decode(ibs), symbols, nonterm, g._nonterm);
        g._root.insert(root);
    }
}

void Grammar::load(std::istream &ist)
try {
    clear();

    //time_t bTime = std::time(0);
    //std::cerr << "Loading CFG grammar .. " << std::endl;

    Header header;
    readHeader(ist, header);
    // printHeaderInfo(header, std::cerr);

    /// file-inside ID -> string table
    std::vector<std::string> symbols(header._numSymbol);

    /// file-inside ID -> grammar internal ID translation table (cache)
    std::vector<int> nonterm(header._numSymbol, -1);
    std::vector<int> rule(header._numSymbol, -1);

    readSymbol(ist, symbols);
    readLexRule(ist, symbols, nonterm, header._numLex, *this);
    HuffmanTree::Decoder decoder(ist);

    InputBitStream ibs(ist);
    readUnary(ibs, decoder, symbols, nonterm, rule, header._numUnary, *this);
    readBinary(ibs, decoder, symbols, nonterm, rule, header._numBinary, *this);
    readRoot(ibs, decoder, symbols, nonterm, header._numRoot, *this);

    //time_t eTime = std::time(0);
    //std::cerr << "(" << (eTime - bTime) << " sec)" << std::endl;
}
catch (std::runtime_error &e)
{
    throw std::runtime_error(std::string("cannot load CFG grammar: ") + e.what());
}
#endif /// OLD BINARY FORMAT VERSION

//------------------------------------------------------------------------------
// New binary format reader/writer
//------------------------------------------------------------------------------

void Grammar::load(std::istream &ist)
try {
    loadDiffEncodedCfgFile(ist, *this);
}
catch (std::runtime_error &e)
{
    throw std::runtime_error(std::string("cannot load CFG grammar: ") + e.what());
}

void Grammar::write(std::ostream &ost)
{
    writeDiffEncodedCfgFile(*this, ost);
}

//------------------------------------------------------------------------------
// Raw file loader
//------------------------------------------------------------------------------
void Grammar::loadRawFile(std::istream &ist)
try {
    clear();

	std::string line;

	while (std::getline(ist, line)) {

		std::istringstream iss(line);
		char type;

		if (! (iss >> type)) {
			throw FileFormatError("no type field: " + line);
		}

		switch (type) {
			case 'b': {
				std::string mot;
				std::string arrow;
				std::string left;
				std::string right;
				std::string rule;

				if (! (iss >> mot >> arrow >> left >> right >> rule)) {
					throw std::runtime_error("binary rule format error: " + line);
				}

				unsigned motId = _nonterm.getID(mot);
				unsigned leftId = _nonterm.getID(left);
				unsigned rightId = _nonterm.getID(right);
				unsigned ruleId = _rule.getID(rule);

				// BinaryTable::key_type key(leftId, rightId);

				// _binary[key].push_back(MotRule(motId, ruleId));
                _binary.put(leftId, rightId, motId, ruleId);

				break;
			}
				
			case 'u': {
				std::string mot;
				std::string arrow;
				std::string dtr;
				std::string rule;

				if (! (iss >> mot >> arrow >> dtr >> rule)) {
					throw std::runtime_error("unary rule format error: " + line);
				}

				unsigned motId = _nonterm.getID(mot);
				unsigned dtrId = _nonterm.getID(dtr);
				unsigned ruleId = _rule.getID(rule);

				_unary[dtrId].push_back(MotRule(motId, ruleId));

				break;
			}

			case 't': {

				std::string mot;
				std::string arrow;
				std::string dtr;

				if (! (iss >> mot >> arrow >> dtr)) {
					throw std::runtime_error("lex rule format error: " + line);
				}

				unsigned lexId = _lexent.getID(dtr);
				unsigned motId = _nonterm.getID(mot);

				_lexToNonterm[lexId] = motId;

				break;
			}

			case 's': {
				
				std::string sym;
				if (! (iss >> sym)) {
					throw std::runtime_error("start symbol format error: " + line);
				}

				_root.insert(_nonterm.getID(sym));

				break;
			}

			default:
				throw std::runtime_error("unknown type: " + line);
		}
	}

    _binary.sort();
}
catch (std::runtime_error &e)
{
    throw std::runtime_error(std::string("cannot load CFG grammar: ") + e.what());
}

void BinaryRuleTable::writeRawFile(
    const SymbolTable<std::string> &nonterm,
    const SymbolTable<std::string> &rule,
    std::ostream &ost
) const {
    for (RecordItr it = _recs.begin(); it != _recs.end(); ++it) {
        ost << 'b' << ' '
            << nonterm.getSymbol(it->_mot)
            << " -> "
            << nonterm.getSymbol(it->_left)
            << ' '
            << nonterm.getSymbol(it->_right)
            << ' '
            << rule.getSymbol(it->_rule)
            << std::endl;
    }
}

void Grammar::writeRawFile(std::ostream &ost)
{
    for (LexTable::const_iterator it = _lexToNonterm.begin(); it != _lexToNonterm.end(); ++it) {
        ost << 't' << ' '
            << _nonterm.getSymbol(it->second)
            << " -> "
            << _lexent.getSymbol(it->first)
            << std::endl;
    }

#if 0
    for (BinaryTable::const_iterator it = _binary.begin(); it != _binary.end(); ++it) {
        const ParentSet &ps = it->second;
        for (ParentSet::const_iterator p = ps.begin(); p != ps.end(); ++p) {
            ost << 'b' << ' '
                << _nonterm.getSymbol(p->_mot)
                << " -> "
                << _nonterm.getSymbol(it->first.first)
                << ' '
                << _nonterm.getSymbol(it->first.second)
                << ' '
                << _rule.getSymbol(p->_rule)
                << std::endl;
        }
    }
#endif
    _binary.writeRawFile(_nonterm, _rule, ost);

    for (UnaryTable::const_iterator it = _unary.begin(); it != _unary.end(); ++it) {
        const ParentSet &ps = it->second;
        for (ParentSet::const_iterator p = ps.begin(); p != ps.end(); ++p) {
            ost << 'u' << ' '
                << _nonterm.getSymbol(p->_mot)
                << " -> "
                << _nonterm.getSymbol(it->first)
                << ' '
                << _rule.getSymbol(p->_rule)
                << std::endl;
        }
    }

    for (std::set<unsigned>::const_iterator it = _root.begin(); it != _root.end(); ++it) {
        ost << 's' << ' ' << _nonterm.getSymbol(*it) << std::endl;
    }
}

void Grammar::changeRuleId(const SymbolTable<std::string> &rule)
{
    // make translation table: transTable[oldID] = newID
    std::vector<unsigned> transTable(_rule.getNumSymbol(), (unsigned) -1);
    for (SymbolTable<std::string>::const_iterator it = _rule.begin(); it != _rule.end(); ++it) {
        if (! rule.hasID(it->first)) {
            throw std::runtime_error("CfgGrammar::resetRuleId: rule set is not covered: " + it->first);
        }
        transTable[it->second] = rule.getID(it->first);
    }

    // binary
    _binary.changeRuleId(transTable);

    // unary
    for (UnaryTable::iterator it = _unary.begin(); it != _unary.end(); ++it) {
        for (ParentSet::iterator p = it->second.begin(); p != it->second.end(); ++p) {
            p->_rule = transTable[p->_rule];
        }
    }

    // rule name table
    _rule = rule;
}

//------------------------------------------------------------------------------
} /// namespace cfg
} /// namespace mogura
//------------------------------------------------------------------------------
