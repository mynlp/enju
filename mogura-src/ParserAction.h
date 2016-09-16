#ifndef ParserAction_h__
#define ParserAction_h__

#include <map>
#include <vector>
#include "Schema.h"
#include "MoguraGrammar.h"
#include "CfgGrammar.h"
#include "ParserState.h"

namespace mogura {

class ParserActionList;

enum ActType {
    ACT_NONE = 0,
    ACT_SHIFT = 1,
    ACT_REDUCE1 = 2,
    ACT_REDUCE2 = 4,
    ACT_OTHER = 8
};

struct ActionCode {
    ActType _type;
    unsigned _rule;

    ActionCode(void) {}
    ActionCode(ActType type, unsigned rule = 0)
        : _type(type)
        , _rule(rule) {}

    bool operator<(const ActionCode &a) const
    {
        return _type < a._type || (_type == a._type && _rule < a._rule);
    }

    bool operator==(const ActionCode &a) const
    {
        return _type == a._type && _rule == a._rule;
    }
};

class ParserAction {
protected:
	ParserAction(void) {}
	friend class ParserActionList;

protected:

	virtual bool applyImpl(Grammar &g, ParserState &s) const = 0;
	
public:
	virtual ~ParserAction(void) {}

	bool apply(Grammar &g, ParserState &s) const
    {
        up::ScopeProf prof("mogura::ParserAction::apply");
		return applyImpl(g, s);
	}

    virtual bool applicable(Grammar &g, const ParserState &s) const = 0;

	virtual std::string asString(void) const = 0;

    virtual ActionCode getCode(void) const = 0;

    /// for debug
	bool applyWithPrint(Grammar &g, ParserState &s, std::ostream &ost) const
    {
		ost << asString();

		bool success = apply(g, s);
		if (success) {
			ost << ": success" << std::endl;
		}
		else {
			ost << ": fail" << std::endl;
		}

		return success;
	}
};

//------------------------------------------------------------------------------

class ParserActionList {
public:
	typedef std::vector<const ParserAction*> ListT;
	typedef std::map<std::string, const ParserAction*> IndexT;

	typedef ListT::const_iterator const_iterator;
	typedef IndexT::const_iterator IndexItr;

private:
	ListT _actions;
	IndexT _index;

public:
    ParserActionList(void) {}

	~ParserActionList(void) { clear(); }

	bool init(Grammar &g, const cfg::Grammar &cfg);

    void clear(void)
	{
		for (const_iterator a = begin(); a != end(); ++a) {
			delete *a;
		}
        _actions.clear();
        _index.clear();
	}

	const ParserAction *find(const std::string &name) const
	{
		IndexItr it = _index.find(name);
		return (it == _index.end()) ? 0 : it->second;
	}

	const_iterator begin(void) const { return _actions.begin(); }
	const_iterator end(void) const { return _actions.end(); }

	unsigned getNumActions(void) const { return _actions.size(); }
};

} // namespace mogura

#endif // ParserAction_h__
