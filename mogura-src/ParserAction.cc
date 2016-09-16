#include <stdexcept>
#include "ParserAction.h"

namespace mogura {

////////////////////////////////////////////////////////////////////////////////
/// Action classes
////////////////////////////////////////////////////////////////////////////////
class Shift : public ParserAction {
protected:
	Shift(void) {}
	friend class ParserActionList;
public:

	bool applyImpl(Grammar &, ParserState &s) const
    {
        return s.shift();
	}

    bool applicable(Grammar &, const ParserState &s) const
    {
        return ! s._words.empty();
    }

	std::string asString(void) const { return "shift"; }

    ActionCode getCode(void) const { return ActionCode(ACT_SHIFT); }
};

class ReduceBase : public ParserAction {
public:
    std::string asString(void) const { return _schema->getName(); }
    ActionCode getCode(void) const { return _code; }

protected:
	ReduceBase(const Schema *schema, ActionCode code) : _schema(schema), _code(code) {}
	friend class ParserActionList;

protected:
    const Schema *_schema;
    ActionCode _code;
};

class ReduceUnary : public ReduceBase {
protected:
	ReduceUnary(const Schema *schema, ActionCode code)
        : ReduceBase(schema, code)
	{
        assert(schema->isUnary());
    }

	friend class ParserActionList;
public:

	bool applyImpl(Grammar &g, ParserState &s) const
    {
		return s.reduceUnary(g, _schema);
	}

    bool applicable(Grammar &g, const ParserState &s) const
    {
        if (s._stack.empty()) {
            return false;
        }

        lilfes::IPTrailStack iptrail(g.getMachine());
        lilfes::FSP mother(g.getMachine());
        return g.applyIdSchemaUnary(_schema->getLilfesType(), s._stack.at(0)->_sign, mother);
    }
};

class ReduceBinary : public ReduceBase {
protected:
	ReduceBinary(const Schema *schema, ActionCode code)
        : ReduceBase(schema, code)
	{
        assert(schema->isBinary());
    }

	friend class ParserActionList;
public:

	bool applyImpl(Grammar &g, ParserState &s) const
    {
		return s.reduceBinary(g, _schema);
	}

    bool applicable(Grammar &g, const ParserState &s) const
    {
        if (s._stack.size() < 2) {
            return false;
        }

        lilfes::IPTrailStack iptrail(g.getMachine());
        lilfes::FSP mother(g.getMachine());
        return g.applyIdSchemaBinary(_schema->getLilfesType(), s._stack.at(1)->_sign, s._stack.at(0)->_sign, mother);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Action list initialization
////////////////////////////////////////////////////////////////////////////////
void makeVector(
	lilfes::machine *m,
	lilfes::FSP fsp,
	Schema::Type type, 
	std::vector<const Schema*> &v
) {
	std::vector<lilfes::FSP> fspv;
	lilfes::list_to_vector(*m, fsp, fspv);

	v.clear();
	for (std::vector<lilfes::FSP>::const_iterator it = fspv.begin(); it != fspv.end(); ++it) {
		v.push_back(new Schema(type, it->GetType()));
	}
}

typedef std::vector<const Schema*> SchemaList;

bool getSchemaList(
    lilfes::machine *m,
    Grammar &g,
    SchemaList &leftHead,
    SchemaList &rightHead,
    SchemaList &unary
) {
    leftHead.clear();
    rightHead.clear();
    unary.clear();

	lilfes::FSP leftFsp(m);
	lilfes::FSP rightFsp(m);
	lilfes::FSP unaryFsp(m);

    if (! g.getSchemaList(leftFsp, rightFsp, unaryFsp)) {
        return false;
    }

	makeVector(m, leftFsp, Schema::LEFT_HEAD, leftHead);
	makeVector(m, rightFsp, Schema::RIGHT_HEAD, rightHead);
	makeVector(m, unaryFsp, Schema::UNARY, unary);

    return true;
}

void add(
    const ParserAction *a,
    ParserActionList::ListT &list,
    ParserActionList::IndexT &index
) {
    list.push_back(a);

    if (index.find(a->asString()) != index.end()) {
        throw std::runtime_error("parser action \"" + a->asString() + "\"is registered twice");
    }

    index[a->asString()] = a;
}

bool ParserActionList::init(Grammar &g, const cfg::Grammar &cfg)
{
    add(new Shift(), _actions, _index);

    SchemaList leftHead;
    SchemaList rightHead;
    SchemaList unary;
    if (! getSchemaList(g.getMachine(), g, leftHead, rightHead, unary)) {
        return false;
    }

    for (SchemaList::const_iterator it = leftHead.begin(); it != leftHead.end(); ++it) {
        ActionCode code(ACT_REDUCE2, cfg._rule.getID((*it)->getName()));
        add(new ReduceBinary(*it, code), _actions, _index);
    }

    for (SchemaList::const_iterator it = rightHead.begin(); it != rightHead.end(); ++it) {
        ActionCode code(ACT_REDUCE2, cfg._rule.getID((*it)->getName()));
        add(new ReduceBinary(*it, code), _actions, _index);
    }

    for (SchemaList::const_iterator it = unary.begin(); it != unary.end(); ++it) {
        ActionCode code(ACT_REDUCE1, cfg._rule.getID((*it)->getName()));
        add(new ReduceUnary(*it, code), _actions, _index);
    }

    return true;
}

} // namespace mogura
