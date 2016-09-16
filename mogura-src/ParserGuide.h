#ifndef ParserGuide_h__
#define ParserGuide_h__

#include <iostream>
#include <vector>
#include <string>

#include "CfgGrammar.h"
#include "ParserAction.h"

namespace mogura {

/// pimpl pattern
struct ParserGuideImpl;

class ParserGuide {
public:
    ParserGuide(const cfg::Grammar *grammar, const cfg::Grammar *restricted);
	~ParserGuide(void);

	bool init(const std::vector<std::string> &lexitems);
	bool accept(ActionCode a) const;
	bool feed(ActionCode a);

	unsigned getNumChunk(void);
	void forceConnect(void);

    unsigned getNumParse(void) const;

	/// for debug
	void dump(void);
	void printExpectedActions(std::ostream &ost) const;

private:
	ParserGuideImpl *_impl;
};

} // namespace mogura

#endif // ParserGuide_h__
