/// 2007.07.30
/// Modification of PCFG/raisecomma.C

#include <list>
#include <algorithm>
#include <vector>
#include <functional>

#include "PennTree.h"

struct Global {
	bool _raiseAllPunct;
} global;

static const char *punctCh[] = { ".", ",", ":", "``", "\'\'" };
static const unsigned NUM_PUNCT = 5;

static const std::vector<std::string> punct(punctCh, punctCh + NUM_PUNCT);

inline
bool isComma(const std::string &pos)
{
	if (global._raiseAllPunct) {
		return std::find(punct.begin(), punct.end(), pos) != punct.end();
	}
	else {
		//return pos == "." || pos == "," || pos == ":";
		return pos == "," || pos == ":";
	}
}

inline
bool isCommaPreTerm(const PennTree *n)
{
	return n->isPreTerminal() && isComma((*n->begin())->getString());
}

class PredBase {
public:
	virtual ~PredBase(void) {}
	virtual bool operator()(const std::string &s) const = 0;

	bool operator()(const PennTree *n) const
	{
		return n->isPreTerminal() && operator()(n->getString());
	}
};

class IsPeriod : public PredBase {
public:
	using PredBase::operator();

	bool operator()(const std::string &s) const { return s == "."; }
};

class IsPunct : public PredBase {
public:
	using PredBase::operator();

	bool operator()(const std::string &s) const
	{
		return std::find(punct.begin(), punct.end(), s) != punct.end();
	}
};

class IsCommaOld : public PredBase {
public:
	using PredBase::operator();

	bool operator()(const std::string &s) const
	{
		return s == "," || s == ":";
	}
};

class IsComma : public PredBase {
public:
	using PredBase::operator();

	bool operator()(const std::string &s) const
	{
		return s == ",";
	}
};

class IsQuote : public PredBase {
public:
	using PredBase::operator();

	bool operator()(const std::string &s) const
	{
		return s == "\'\'" || s == "``";
	}
};

template<class Pred1, class Pred2>
class Or : public PredBase {
public:
	using PredBase::operator();

	bool operator()(const std::string &s) const
	{
		return Pred1()(s) || Pred2()(s);
	}
};


template<class Pred>
PennTree *raise(
	const PennTree *t,
	std::list<PennTree*> &leftMost,
	std::list<PennTree*> &rightMost,
	const Pred &pred
) {
	if (t->isPreTerminal()) {
		leftMost.clear();
		rightMost.clear();
		return t->clone();
	}
	else {
		std::list<PennTree*> dtrs;
		std::list<PennTree*> leftPunct;
		std::list<PennTree*> rightPunct;

		for (PennTree::const_iterator d = t->dtrBegin();
				d != t->dtrEnd(); ++d) {

			PennTree *dtree = raise(*d, leftPunct, rightPunct, pred);
			dtrs.insert(dtrs.end(), leftPunct.begin(), leftPunct.end());

			if (dtree) {
				dtrs.push_back(dtree);
			}

			dtrs.insert(dtrs.end(), rightPunct.begin(), rightPunct.end());
		}

		leftMost.clear();
		rightMost.clear();

		std::list<PennTree*>::iterator it0 = dtrs.begin();
		for ( ; it0 != dtrs.end() && pred(*it0); ++it0) {
			leftMost.push_back(*it0);
		}

		if (it0 == dtrs.end()) { /// all the dtrs are commas
			return 0;
		}

		std::list<PennTree*>::iterator it1 = --(dtrs.end());
		for ( ; it1 != it0 && pred(*it1); --it1) {
			rightMost.push_front(*it1);
		}

		++it1;

		PennTree *res = new PennTree(t->getString());
		for_each(it0, it1, std::bind1st(std::mem_fun(&PennTree::addDtr), res));
		
		return res;
	}
}

int main(int argc, char **argv)
{
	bool raiseAllPunct = false;
	bool raiseQuote = false;

	if (argc == 2) {
		if (argv[1] == std::string("-a")) {
			raiseAllPunct = true;
		}
		else if (argv[1] == std::string("-q")) {
			raiseQuote = true;
		}
		else {
			std::cerr << "Unknown option: " << argv[1] << std::endl;
			exit(1);
		}
	}
	else if (argc > 1) {
		std::cerr << "Usage: " << argv[0] << " [-q | -a]" << std::endl;
		exit(1);
	}

	PennTree t;
	std::list<PennTree*> leftMost;
	std::list<PennTree*> rightMost;

	while (std::cin >> t) {

		if (t.isPreTerminal()) {
			std::cout << t << std::endl;
		}
		else {

			PennTree *newt = 0;
			if (raiseAllPunct) {
				newt = raise(&t, leftMost, rightMost, IsPunct());
			}
			else if (raiseQuote) {
				// newt = raise(&t, leftMost, rightMost, IsCommaOld());
				newt = raise(&t, leftMost, rightMost, Or<IsComma, IsQuote>());
			}
			else {
				// newt = raise(&t, leftMost, rightMost, IsCommaOld());
				newt = raise(&t, leftMost, rightMost, IsComma());
			}

			PennTree *root = new PennTree(t.getString());

			std::for_each(leftMost.begin(), leftMost.end(),
					std::bind1st(std::mem_fun(&PennTree::addDtr), root));

			if (newt != 0) {
				root->raiseDtr(root->end(), *newt);
			}

			std::for_each(rightMost.begin(), rightMost.end(),
					std::bind1st(std::mem_fun(&PennTree::addDtr), root));

			std::cout << *root << std::endl;

			delete newt;
		}
	}

	return 0;
}

