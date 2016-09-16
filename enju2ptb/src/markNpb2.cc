#include "PennTree.h"

struct NodeType {
	bool _isNp;
	bool _isPos; /// whether this node directly dominates a POS (possesive)

	NodeType(bool isNp, bool isPos) : _isNp(isNp), _isPos(isPos) {}
};

NodeType markBaseNp(PennTree *t)
{
	assert(t->getString()[0] != '*');

	if (t->isPreTerminal()) {
		return NodeType(false, false);
	}

	bool hasNpDtr = false;
	bool hasPosDtr = false;

	for (PennTree::iterator d = t->dtrBegin(); d != t->dtrEnd(); ++d) {
		
		NodeType type = markBaseNp(*d);

		if (type._isNp && ! type._isPos) {
			hasNpDtr = true;
		}

		if ((*d)->isPreTerminal() && (*d)->getString() == "POS") {
			hasPosDtr = true;
		}
	}

	bool isNp = t->getString() == "NP";
	if (isNp && ! hasNpDtr) {
		t->setString("NPB");
	}

	return NodeType(isNp, hasPosDtr);
}

int main(void)
{
	PennTree t;
	while (std::cin >> t) {

		(void) markBaseNp(&t);

		std::cout << t << std::endl;
	}

	return 0;
}
