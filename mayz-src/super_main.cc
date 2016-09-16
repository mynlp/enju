/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file
 *  @version Time-stamp: <2008-07-14 14:52:16 yusuke>
 *
 **********************************************************************/

#include <iostream>
#include <limits>
#include <string>
#include <set>

#undef PROFILE
#include <liblilfes/gfstream.h>
#include <liblilfes/bfstream.h>

#include "Supertagger.h"

//////////////////////////////////////////////////////////////////////
// Commmand line arguments
//////////////////////////////////////////////////////////////////////

struct Setting {
    std::string lexicon_name;
    std::string model_name;
    std::string conf_name;
    unsigned num_word_features;
    bool multi_tagging;
    bool lattice_input;
    int num_max_ambiguity;

    Setting(void)
        : num_word_features(0)
        , multi_tagging(false)
        , lattice_input(false)
	, num_max_ambiguity(5)
    {}
};

void printHelp(std::ostream& ost)
{
    ost << "Usage: super [-L] [-M] -l lexicon_file -m model_file -c conf_file -n number" << std::endl
        << "Options:" << std::endl
        << "  -l lexicon_file  text-format lexicon file" << std::endl
        << "  -m model_file    amis output file" << std::endl
        << "  -c conf_file     feature configuration file" << std::endl
        << "  -n number        number of word-feature fields" << std::endl
        << "  -AM number       maximum number of ambiguous POS tags" << std::endl
        << "  -M               output all possible lexical templates with their probabilities" << std::endl
        << "  -L               use lattice input format" << std::endl
        << "  -h               print this message" << std::endl
        << "  -H               print this message and input format definition" << std::endl
        ;
    return;
}

void printFormatDefinition(std::ostream& ost)
{
    ost << "Simple input format (default):" << std::endl
        << "  <input>        ::= <sentence>*" << std::endl
        << "  <sentence>     ::= <token-line>* <empty-line>" << std::endl
        << "  <token-line>   ::= <word-feature>* <lexicon-key>+ <nl>" << std::endl
        << "  <empty-line>   ::= <nl>" << std::endl
        << "  <word-feature> ::= string" << std::endl
        << "  <lexicon-key>  ::= string" << std::endl
        << "  <nl>           ::= new-line character (\"\\n\")" << std::endl
        << std::endl
        << "Lattice input format (-L option)" << std::endl
        << "  <input>        ::= <sentence>*" << std::endl
        << "  <sentence>     ::= <token-line>* <empty-line>" << std::endl
        << "  <token-line>   ::= <begin-node> <end-node> <node-prob> <word-feature>* <lexicon-key>+ <nl>" << std::endl
        << "  <empty-line>   ::= <nl>" << std::endl
        << "  <begin-node>   ::= integer; ID of begin lattice node" << std::endl
        << "  <end-node>     ::= integer; ID of end lattice node" << std::endl
        << "  <node-prob>    ::= logarithm of the marginal probability of the token" << std::endl
        << "  <word-feature> ::= string" << std::endl
        << "  <lexicon-key>  ::= string" << std::endl
        << "  <nl>           ::= new-line character (\"\\n\")" << std::endl
        << std::endl
        << "Notes on the input format:" << std::endl
        << "  * <word-feature> and <lexicon-key> may not include 'isspace' characters" << std::endl
        << "  * <word-feature> and <lexicon-key> may not be an empty string" << std::endl
        << "  * Number of <word-features> in a <token-line> must be equal to '-n' option's argument." << std::endl
        << "    Any remaining fields in a line are interpreted as <lexicon-key> fields." << std::endl
        ;
    return;
}

bool analyzeArguments(const std::vector<std::string>& args, Setting& setting)
{
    std::vector<std::string>::const_iterator arg = args.begin();
    while (arg != args.end()) {
        if (*arg == "-l") {
            if (++arg == args.end()) return false;
            setting.lexicon_name = *arg;
        }
        else if (*arg == "-m") {
            if (++arg == args.end()) return false;
            setting.model_name = *arg;
        }
        else if (*arg == "-c") {
            if (++arg == args.end()) return false;
            setting.conf_name = *arg;
        }
        else if (*arg == "-n") {
            if (++arg == args.end()) return false;
            setting.num_word_features = std::atoi(arg->c_str());
        }
	else if (*arg == "-AM") {
	  if (++arg == args.end()) return false;
	  setting.num_max_ambiguity = std::atoi(arg->c_str());
	}
        else if (*arg == "-M") {
            setting.multi_tagging = true;
        }
        else if (*arg == "-L") {
            setting.lattice_input = true;
        }
        else {
            std::cerr << "Unknown option: " << *arg << std::endl;
            return false;
        }
        ++arg;
    }

    return ! setting.lexicon_name.empty()
        && ! setting.model_name.empty()
        && ! setting.conf_name.empty();
}

//////////////////////////////////////////////////////////////////////

std::istream* openFile(const std::string& fname)
{
    std::istream *file = 0;
    //if (fname.size() >= 4 && fname.substr(fname.size() - 4) == ".bz2") {
    //    file = lilfes::ibfstream(fname.c_str());
    //}
    //else
    if (fname.size() >= 3 && fname.substr(fname.size() - 3) == ".gz") {
        file = new lilfes::igfstream(fname.c_str());
    }
    else { // uncompressed file
        file = new std::ifstream(fname.c_str());
    }
    if (! *file) {
        throw std::runtime_error("Cannot open input file '" + fname + "'");
    }

    return file;
}

//////////////////////////////////////////////////////////////////////
// Input
//////////////////////////////////////////////////////////////////////

bool readSimpleFormat(std::istream& ist, unsigned num_word_features, up::WordLattice& word_lattice)
{
    word_lattice.clear();

    std::string line;
    while (std::getline(ist, line)) {
        if (line.empty()) {
            return true;
        }

        unsigned begin = word_lattice.size();

        word_lattice.push_back(up::WordExtent());
        up::WordExtent& w = word_lattice.back();

        w.begin = begin;
        w.end = begin + 1;
        w.word_fom = 0;
        w.word.resize(num_word_features);

        std::istringstream iss(line);
        for (unsigned i = 0; i < num_word_features; ++i) {
            if (! (iss >> w.word[i])) {
                throw std::runtime_error("input format error: too few word-features:\n" + line);
            }
        }

        std::string key;
        while (iss >> key) {
            w.lookup_keys.push_back(key);
        }

        if (w.lookup_keys.empty()) {
            throw std::runtime_error("input format error: no lexicon lookup keys:\n" + line);
        }
    }

    if (! word_lattice.empty()) {
        throw std::runtime_error("input format error: no empty line after the last sentence");
    }
    return false;
}

bool readLatticeFormat(std::istream& ist, unsigned num_word_features, up::WordLattice& word_lattice)
{
    word_lattice.clear();

    std::string line;
    while (std::getline(ist, line)) {
        if (line.empty()) {
            return true;
        }

        word_lattice.push_back(up::WordExtent());
        up::WordExtent& w = word_lattice.back();

        std::istringstream iss(line);

        if (! (iss >> w.begin >> w.end)) {
            throw std::runtime_error("input format error: no begin/end fields:\n" + line);
        }
        if (w.begin >= w.end) {
            throw std::runtime_error("invalid begin/end pair:\n" + line);
        }

        if (! (iss >> w.word_fom)) {
            throw std::runtime_error("input format error: no node-probability field:\n" + line);
        }

        w.word.resize(num_word_features);
        for (unsigned i = 0; i < num_word_features; ++i) {
            if (! (iss >> w.word[i])) {
                throw std::runtime_error("input format error: too few word-features:\n" + line);
            }
        }

        std::string key;
        while (iss >> key) {
            w.lookup_keys.push_back(key);
        }

        if (w.lookup_keys.empty()) {
            throw std::runtime_error("input format error: no lexicon lookup keys:\n" + line);
        }
    }

    if (! word_lattice.empty()) {
        throw std::runtime_error("input format error: no empty line after the last sentence");
    }
    return false;
}

bool readSentence(const Setting& setting, std::istream& ist, up::WordLattice& word_lattice)
{
    return (setting.lattice_input) ? readLatticeFormat(ist, setting.num_word_features, word_lattice)
                                   : readSimpleFormat(ist, setting.num_word_features, word_lattice);
}

//////////////////////////////////////////////////////////////////////
// Output
//////////////////////////////////////////////////////////////////////
struct LessBegin {
    LessBegin(const up::WordLattice& lattice_) : lattice(lattice_) {}

    bool operator()(unsigned ix1, unsigned ix2) const
    {
        return lattice[ix1].begin < lattice[ix2].begin;
    }
private:
    const up::WordLattice& lattice;
};

struct LessFom {
    bool operator()(const up::LexTemplateFom& lex1, const up::LexTemplateFom& lex2) const
    {
        return lex1.fom < lex2.fom;
    }
};

typedef std::vector<std::pair<unsigned, unsigned> > ViterbiPath;
void findViterbiPath(
    const up::WordLattice& word_lattice,
    const std::vector<std::vector<up::LexTemplateFom> >& tmplss,
    ViterbiPath& viterbi_path
) {
    viterbi_path.clear();
    if (word_lattice.empty()) {
        return;
    }

    // topological sort of the extents
    std::vector<unsigned> topo_order(word_lattice.size());
    for (unsigned i = 0; i < topo_order.size(); ++i) {
        topo_order[i] = i;
    }
    std::stable_sort(topo_order.begin(), topo_order.end(), LessBegin(word_lattice));

    // find the last lattice node ID
    unsigned last_node = 0;
    for (up::WordLattice::const_iterator it = word_lattice.begin(); it != word_lattice.end(); ++it) {
        last_node = std::max(last_node, it->end);
    }

    // viterbi score upto each lattice node
    std::vector<double> forward(last_node + 1, -std::numeric_limits<double>::infinity());
    forward[0] = 0;

    // back pointer to the best incoming extent for each node
    std::vector<unsigned> back_ptr(last_node + 1, std::numeric_limits<unsigned>::max());

    // best template name for each extent
    std::vector<unsigned> best_tmpl(word_lattice.size(), std::numeric_limits<unsigned>::max());

    // viterbi algorithm
    for (std::vector<unsigned>::const_iterator it = topo_order.begin(); it != topo_order.end(); ++it) {
        const up::WordExtent& ext = word_lattice[ *it ];
        const std::vector<up::LexTemplateFom>& tmpls = tmplss[ *it ];
        if (tmpls.empty()) {
            continue;
        }

        std::vector<up::LexTemplateFom>::const_iterator max_tmpl
            = std::max_element(tmpls.begin(), tmpls.end(), LessFom());

        best_tmpl[ *it ] = std::distance(tmpls.begin(), max_tmpl);
        double score = forward[ ext.begin ] + max_tmpl->fom; // sum of fom upto this extent
        if (score >= forward[ ext.end ]) {
            forward[ ext.end ] = score;
            back_ptr[ ext.end ] = *it;
        }
    }

    // follow the back pointers to get the viterbi path
    std::vector<unsigned> inv_path;
    unsigned extent_ix = back_ptr.back();
    while (extent_ix < word_lattice.size()) {
        inv_path.push_back(extent_ix);
        extent_ix = back_ptr[ word_lattice[ extent_ix ].begin ];
    }
    for (std::vector<unsigned>::reverse_iterator it = inv_path.rbegin(); it != inv_path.rend(); ++it) {
        viterbi_path.push_back(std::make_pair(*it, best_tmpl[ *it ]));
    }
}


void printMultiTaggingResult(
    bool lattice_output,
    const up::WordLattice& word_lattice,
    const std::vector<std::vector<up::LexTemplateFom> >& tmplss,
    std::ostream& ost
) {
    for (unsigned i = 0; i < word_lattice.size(); ++i) {
        const up::WordExtent& e = word_lattice[i];
        if (lattice_output) {
            ost << e.begin << '\t' << e.end << '\t';
        }

        for (std::vector<std::string>::const_iterator it = e.word.begin(); it != e.word.end(); ++it) {
            ost << *it << '\t';
        }

        const std::vector<up::LexTemplateFom>& tmpls = tmplss[i];
        for (std::vector<up::LexTemplateFom>::const_iterator it = tmpls.begin(); it != tmpls.end(); ++it) {
            if (it != tmpls.begin()) {
                ost << '\t';
            }
            ost << it->tmpl << '\t' << it->fom;
        }
        ost << std::endl;
    }
    ost << std::endl;
}

void printViterbiResult(
    bool lattice_output,
    const up::WordLattice& word_lattice,
    const std::vector<std::vector<up::LexTemplateFom> >& tmplss,
    std::ostream& ost
) {
    ViterbiPath viterbi_path;
    findViterbiPath(word_lattice, tmplss, viterbi_path);

    for (ViterbiPath::const_iterator it = viterbi_path.begin(); it != viterbi_path.end(); ++it) {
        const up::WordExtent& e = word_lattice[ it->first ];
        const up::LexTemplateFom& t = tmplss[ it->first ][ it->second ];

        if (lattice_output) {
            ost << e.begin << '\t' << e.end << '\t';
        }

        for (std::vector<std::string>::const_iterator it = e.word.begin(); it != e.word.end(); ++it) {
            ost << *it << '\t';
        }
        
        ost << t.tmpl << std::endl;
    }
    ost << std::endl;
}

void printResult(
    const Setting& setting,
    const up::WordLattice& word_lattice,
    const std::vector<std::vector<up::LexTemplateFom> >& tmplss,
    std::ostream& ost
) {
    assert(word_lattice.size() == tmplss.size());
    if (setting.multi_tagging) {
        printMultiTaggingResult(setting.lattice_input, word_lattice, tmplss, ost);
    }
    else {
        printViterbiResult(setting.lattice_input, word_lattice, tmplss, ost);
    }
}

//////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
try {
    std::vector<std::string> args(argv + 1, argv + argc);

    // show help message
    if (std::find(args.begin(), args.end(), "-H") != args.end()) {
        printHelp(std::cout);
        std::cout << std::endl;
        printFormatDefinition(std::cout);
        return 0;
    }
    if (std::find(args.begin(), args.end(), "-h") != args.end()) {
        printHelp(std::cout);
        return 0;
    }

    // parse command-line arguments
    Setting setting;
    if (! analyzeArguments(args, setting)) {
        printHelp(std::cerr);
        return 1;
    }

    // open model file etc.
    std::istream *lexicon_file = openFile(setting.lexicon_name);
    std::istream *model_file = openFile(setting.model_name);
    std::istream *conf_file = openFile(setting.conf_name);

    // Initialize supertagger
    std::cerr << "Initializing supertagger...\n";
    up::Supertagger *tagger = 0;
    if (setting.lattice_input) {
        tagger = new up::SupertaggerMultiMorph(*lexicon_file, *model_file, *conf_file, setting.num_max_ambiguity);
    }
    else {
        tagger = new up::SupertaggerViterbiMorph(*lexicon_file, *model_file, *conf_file);
    }
    std::cerr << "done." << std::endl;

    // main loop
    up::WordLattice word_lattice;
    while (readSentence(setting, std::cin, word_lattice)) {
        if (word_lattice.empty()) { /// empty sentence
            std::cout << std::endl;
            continue;
        }

        std::vector<std::vector<up::LexTemplateFom> > tmplss;
        if (! tagger->analyze(word_lattice, tmplss)) {
            std::cerr << "WARN: supertagging failed for a sentence" << std::endl;
            std::cout << std::endl;
            continue;
        }

        printResult(setting, word_lattice, tmplss, std::cout);
    }

    return 0;
} 
catch (std::bad_alloc) {
    std::cerr << "Memory allocation failed." << std::endl;
    std::exit(1);
}
catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    std::exit(1);
}
catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    std::exit(1);
}

