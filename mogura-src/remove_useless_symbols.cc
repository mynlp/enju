#include <sstream>
#include <iterator>
#include <cassert>

#include "CfgGrammar.h"
#include "ProgressBar.h"
#include "mayzutil.h"

using namespace mogura;
using namespace cfg;

void makeTrans(
	const Grammar &g, 
	std::vector<std::vector<unsigned> > &trans
) {
	trans.clear();
	trans.resize(g._nonterm.getNumSymbol());

	std::vector<std::set<unsigned> > tmp(trans.size());

#if 0
	for (Grammar::BinaryTable::const_iterator i = g._binary.begin(); i != g._binary.end(); ++i) {
		for (std::vector<MotRule>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			tmp[j->_mot].insert(i->first.first);
			tmp[j->_mot].insert(i->first.second);
		}
	}
#endif
	for (Grammar::BinaryTable::RecordItr i = g._binary.begin(); i != g._binary.end(); ++i) {
        tmp[i->_mot].insert(i->_left);
        tmp[i->_mot].insert(i->_right);
	}

	for (Grammar::UnaryTable::const_iterator i = g._unary.begin(); i != g._unary.end(); ++i) {
		for (std::vector<MotRule>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			tmp[j->_mot].insert(i->first);
		}
	}

	for (unsigned i = 0; i < tmp.size(); ++i) {
		trans[i].insert(trans[i].end(), tmp[i].begin(), tmp[i].end());
	}
}

void propagate(
	const std::vector<std::vector<unsigned> > &trans,
	const std::set<unsigned> &seed,
	std::set<unsigned> &reached
) {
	reached = seed;
	std::set<unsigned> wait(seed);

	ProgressBar pb(1000, 10000, std::cerr);

	while (! wait.empty()) {

		unsigned n = *wait.begin();
		wait.erase(wait.begin());

		for (std::vector<unsigned>::const_iterator i = trans[n].begin(); i != trans[n].end(); ++i) {
			if (reached.find(*i) == reached.end()) {
				wait.insert(*i);
				reached.insert(*i);
			}
		}

		++pb;
	}

	pb.done();
}

void filter(
	std::istream &ist,
	const std::set<std::string> &nt,
	std::ostream &ost
) {
	std::string line;
	while (std::getline(ist, line)) {

		std::istringstream iss(line);
		std::vector<std::string> f;
		std::copy(
			std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			std::back_inserter(f));

		bool ok = false;
		if (f[0] == "t") {
			assert(f.size() == 4);

			ok = (nt.find(f[1]) != nt.end());
			// ok = true;

			if (! ok) {
				std::cerr << "unreachable lex item: " << f[3] << std::endl;
			}
		}
		else if (f[0] == "b") {
			assert(f.size() == 6);

			ok = (nt.find(f[1]) != nt.end() && nt.find(f[3]) != nt.end()
				&& nt.find(f[4]) != nt.end());
		}
		else if (f[0] == "u") {
			assert(f.size() == 5);
			ok = (nt.find(f[1]) != nt.end() && nt.find(f[3]) != nt.end());
		}
		else if (f[0] == "s") {
			assert(f.size() == 2);
			ok = true;
		}
		else {
			assert(false);
		}

		if (ok) {
			ost << line << std::endl;
		}
	}
}
	
int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <grammar-file>" << std::endl;
		exit(1);
	}

    std::istream *ifs1 = up::openInputFile(argv[1]);
	Grammar g;

	std::cerr << "Reading grammar .. ";
	g.loadRawFile(*ifs1);
	std::cerr << "done" << std::endl;

    delete ifs1;

	std::vector<std::vector<unsigned> > trans;
	std::cerr << "Collecting transitions .. ";
	makeTrans(g, trans);
	std::cerr << "done" << std::endl;

    std::cerr << "Calcurating closure of start symbols .. " << std::endl;
	std::set<unsigned> seed(g._root);
	std::set<unsigned> reached;
	propagate(trans, seed, reached);
    std::cerr << "done" << std::endl;

	std::cerr << "number of reachable symbols = " << reached.size() << std::endl;

	std::set<std::string> nt;
	for (std::set<unsigned>::const_iterator it = reached.begin(); it != reached.end(); ++it) {
		nt.insert(g._nonterm.getSymbol(*it));
	}

    std::cerr << "Filtering original grammar .. ";
	std::istream *ifs2 = up::openInputFile(argv[1]);
	filter(*ifs2, nt, std::cout);
    std::cerr << "done" << std::endl;

	return 0;
}
