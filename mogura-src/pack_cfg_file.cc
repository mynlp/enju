#include <iostream>
#include "CfgGrammar.h"

int main(void)
{
    mogura::cfg::Grammar g;

    std::cerr << "Reading grammar .. ";
    g.loadRawFile(std::cin);
    std::cerr << "done" << std::endl;

    g.write(std::cout);

    return 0;
}

#if 0 /// OLD VERSION
//#include "HuffmanCode.h"

class FileFormatError : public std::runtime_error {
public:
    FileFormatError(const std::string &line) : std::runtime_error(line) {}
};

typedef std::map<std::string, unsigned> IdTable;

inline
unsigned getId(const std::string &name, IdTable &table)
{
    return table.insert(std::make_pair(name, table.size())).first->second;
}

struct LexRule {
    unsigned _mot;
    std::string _dtr;

    LexRule(void) {}

    LexRule(const std::string &line, IdTable &symbol)
    {
        std::istringstream iss(line);
        char mark;
        std::string mot;
        std::string arrow;
        if (! (iss >> mark >> mot>> arrow >> _dtr)) {
            throw FileFormatError(line);
        }

        _mot = getId(mot, symbol);
    }
};

struct UnaryRule {
    unsigned _mot;
    unsigned _dtr;
    unsigned _schema;

    UnaryRule(void) {}
    UnaryRule(const std::string &line, IdTable &symbol)
    {
        std::istringstream iss(line);
        char mark;
        std::string mot;
        std::string arrow;
        std::string dtr;
        std::string schema;
        if (! (iss >> mark >> mot>> arrow >> dtr >> schema)) {
            throw FileFormatError(line);
        }

        _mot = getId(mot, symbol);
        _dtr = getId(dtr, symbol);
        _schema = getId(schema, symbol);
    }
};

struct BinaryRule {
    unsigned _mot;
    unsigned _left;
    unsigned _right;
    unsigned _schema;

    BinaryRule(void) {}
    BinaryRule(const std::string &line, IdTable &symbol)
    {
        std::istringstream iss(line);
        char mark;
        std::string mot;
        std::string arrow;
        std::string left;
        std::string right;
        std::string schema;
        if (! (iss >> mark >> mot >> arrow >> left >> right >> schema)) {
            throw FileFormatError(line);
        }

        _mot = getId(mot, symbol);
        _left = getId(left, symbol);
        _right = getId(right, symbol);
        _schema = getId(schema, symbol);
    }
};

unsigned readRootSymbol(const std::string &line, IdTable &symbol)
{
    std::istringstream iss(line);
    char mark;
    std::string root;
    if (! (iss >> mark >> root)) {
        throw FileFormatError(line);
    }

    return getId(root, symbol);
}

inline
void count(std::vector<unsigned> &symbolCnt, unsigned symbol)
{
    if (symbolCnt.size() <= symbol) {
        symbolCnt.resize(symbol + 1, 0);
    }
    ++symbolCnt[symbol];
}

void readFile(
    std::istream &ist,
    std::vector<LexRule> &lex,
    std::vector<UnaryRule> &unary,
    std::vector<BinaryRule> &binary,
    std::vector<unsigned> &root,
    IdTable &symbol,
    std::vector<unsigned> &symbolCnt
) {
    unsigned numLine = 0;

    lex.clear();
    unary.clear();
    binary.clear();
    symbolCnt.clear();

    std::string line;
    while (std::getline(ist, line)) {
        
        if (line.empty() || line[0] == '#') {
            continue;
        }

        switch (line[0]) {
            case 't':
                lex.push_back(LexRule(line, symbol));
                break;
            case 'u':
                unary.push_back(UnaryRule(line, symbol));
                count(symbolCnt, unary.back()._mot);
                count(symbolCnt, unary.back()._dtr);
                count(symbolCnt, unary.back()._schema);
                break;
            case 'b':
                binary.push_back(BinaryRule(line, symbol));
                count(symbolCnt, binary.back()._mot);
                count(symbolCnt, binary.back()._left);
                count(symbolCnt, binary.back()._right);
                count(symbolCnt, binary.back()._schema);
                break;
            case 's':
                root.push_back(readRootSymbol(line, symbol));
                count(symbolCnt, root.back());
                break;
            default:
                std::cerr << "File format error: " << line << std::endl;
                exit(1);
        }

        if (++numLine % 100000 == 0) {
            if (numLine % 1000000 == 0) {
                std::cerr << ' ' << numLine << " lines" << std::endl;
            }
            else {
                std::cerr << '.';
            }
        }
    }
}

void writeSymbol(const IdTable &table, std::ostream &ost)
{
    std::cerr << "Writing symbols ..";

    std::vector<std::string> inv(table.size());
    for (IdTable::const_iterator it = table.begin(); it != table.end(); ++it) {
        assert(it->second < inv.size());
        inv[it->second] = it->first;
    }
    std::copy(inv.begin(), inv.end(), std::ostream_iterator<std::string>(ost, "\n"));

    std::cerr << "done" << std::endl;
}

void writeLexRule(const std::vector<LexRule> &lex, std::ostream &ost)
{
    std::cerr << "Writing lex rules ..";

    for (std::vector<LexRule>::const_iterator it = lex.begin(); it != lex.end(); ++it) {
        ost << it->_mot << '\t' << it->_dtr << std::endl;
    }

    std::cerr << "done" << std::endl;
}

void writeUnaryRule(
    const std::vector<UnaryRule> &unary, 
    const std::vector<std::vector<bool> > &symCode,
    OutputBitStream &obs
) {
    std::cerr << "Writing unary rules ..";

    for (std::vector<UnaryRule>::const_iterator it = unary.begin(); it != unary.end(); ++it) {
        assert(it->_mot <= symCode.size());
        assert(it->_dtr <= symCode.size());
        assert(it->_schema <= symCode.size());

        obs.put(symCode[it->_mot]);
        obs.put(symCode[it->_dtr]);
        obs.put(symCode[it->_schema]);
    }

    std::cerr << "done" << std::endl;
}

void writeBinaryRule(
    const std::vector<BinaryRule> &binary, 
    const std::vector<std::vector<bool> > &symCode,
    OutputBitStream &obs
) {
    std::cerr << "Writing binary rules .." << std::endl;
    int numRule = 0;

    for (std::vector<BinaryRule>::const_iterator it = binary.begin(); it != binary.end(); ++it) {
        assert(it->_mot <= symCode.size());
        assert(it->_left <= symCode.size());
        assert(it->_right <= symCode.size());
        assert(it->_schema <= symCode.size());

        obs.put(symCode[it->_mot]);
        obs.put(symCode[it->_left]);
        obs.put(symCode[it->_right]);
        obs.put(symCode[it->_schema]);

        if (++numRule % 100000 == 0) {
            if (numRule % 1000000 == 0) {
                std::cerr << ' ' << numRule << " rules" << std::endl;
            }
            else {
                std::cerr << '.';
            }
        }
    }

    std::cerr << "done" << std::endl;
}

void writeRootSymbol(
    const std::vector<unsigned> &root, 
    const std::vector<std::vector<bool> > &symCode,
    OutputBitStream &obs
) {
    for (std::vector<unsigned>::const_iterator it = root.begin(); it != root.end(); ++it) {
        assert(*it <= symCode.size());
        obs.put(symCode[*it]);
    }
}

void printCodeTable(
    const IdTable &table,
    const std::vector<std::vector<bool> > &code
) {
    std::vector<std::string> inv(table.size());
    for (IdTable::const_iterator it = table.begin(); it != table.end(); ++it) {
        assert(it->second < inv.size());
        inv[it->second] = it->first;
    }

    for (unsigned i = 0; i < inv.size(); ++i) {
        std::cerr << inv[i] << '\t';
        for (unsigned j = 0; j < code[i].size(); ++j) {
            std::cerr << (code[i][j] ? 1 : 0);
        }
        std::cerr << std::endl;
    }
}

/// layout
/// [0] NUM-SYM NUM-LEX NUM-UNARY NUM-BINARY NUM-ROOT
/// [1] (Symbol '\n')*
/// [2] (Lex-rule '\n')*
/// [3] (Huffman-tree)
/// [4] (unary)*   ... [mot][dtr][schema]
/// [5] (binary)*  ... [mot][left][right][schema]
/// [6] (root)*    ... [sym]

int main(void)
{
    std::vector<LexRule> lex;
    std::vector<UnaryRule> unary;
    std::vector<BinaryRule> binary;
    std::vector<unsigned> root;
    IdTable symbol;
    std::vector<unsigned> symbolCnt;

    std::cerr << "Reading file" << std::endl;
    readFile(std::cin, lex, unary, binary, root, symbol, symbolCnt);
    std::cerr << "done" << std::endl;

    std::cerr << "Making code tree ..";
    HuffmanTree tree(symbolCnt);
    std::cerr << "done" << std::endl;

    std::cerr << "Making encode table ..";
    std::vector<std::vector<bool> > symCode;
    tree.getEncodeTable(symCode);
    std::cerr << "done" << std::endl;

    // printCodeTable(symbol, symCode);

    HuffmanTree::Decoder *decoder = tree.makeDecoder();

    std::cout << symbol.size() << ' '
              << lex.size() << ' '
              << unary.size() << ' '
              << binary.size() << ' '
              << root.size() << std::endl;

    std::cerr << "#symbol = " << symbol.size() << std::endl
              << "#lex    = " << lex.size() << std::endl
              << "#unary  = " << unary.size() << std::endl
              << "#binary = " << binary.size() << std::endl
              << "#root   = " << root.size() << std::endl;

    writeSymbol(symbol, std::cout);
    writeLexRule(lex, std::cout);
    decoder->write(std::cout);
    
    OutputBitStream obs(std::cout);
    writeUnaryRule(unary, symCode, obs);
    writeBinaryRule(binary, symCode, obs);
    writeRootSymbol(root, symCode, obs);

    return 0;
}
#endif
