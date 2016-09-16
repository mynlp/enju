#ifndef Schema_h__
#define Schema_h__

#include <liblilfes/structur.h>

namespace mogura {

class Schema {
public:

    enum Type { LEFT_HEAD = 0, RIGHT_HEAD = 1, UNARY = 2 };

	Schema(Type t, const lilfes::type *lilfesType)
		: _type(t)
		, _lilfesType(lilfesType)
	{}

	Type getType(void) const { return _type; }
	const lilfes::type *getLilfesType(void) const { return _lilfesType; }

	bool isUnary(void) const { return _type == UNARY; }
	bool isBinary(void) const { return _type != UNARY; }

	std::string getName(void) const { return _lilfesType->GetSimpleName(); }

private:
	Type _type;
	const lilfes::type *_lilfesType;
};

} // namespace mogura

#endif // Schema_h__
