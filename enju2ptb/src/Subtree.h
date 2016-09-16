#ifndef Subtree_h__
#define Subtree_h__

#include <list>
#include <iostream>
#include <stdexcept>

struct SubtreeNode {
public:
	typedef std::list<SubtreeNode> DtrListType;
	typedef DtrListType::const_iterator const_iterator;
	typedef DtrListType::iterator iterator;
public:
	std::string _label;
	std::list<SubtreeNode> _dtrs;

public:
	bool isLeaf(void) const { return _dtrs.empty(); }

	void swap(SubtreeNode &n)
	{
		_label.swap(n._label);
		_dtrs.swap(n._dtrs);
	}

	/// Caution: yield symbols are *appended* to ys
	void getYield(std::vector<std::string> &ys) const
	{
		if (isLeaf()) {
			ys.push_back(_label);
		}
		else {
			for (const_iterator it = _dtrs.begin(); it != _dtrs.end(); ++it) {
				it->getYield(ys);
			}
		}
	}
};

inline bool operator<(const SubtreeNode &n1, const SubtreeNode &n2)
{
	if (n1._label < n2._label) {
		return true;
	}
	else if (n1._label > n2._label) {
		return false;
	}
	else {
		return n1._dtrs < n2._dtrs;
	}
}

inline
std::ostream &operator<<(std::ostream &ost, const SubtreeNode &n)
{
	ost << "( " << n._label;
	for (SubtreeNode::const_iterator d = n._dtrs.begin();
			d != n._dtrs.end(); ++d) {
		ost << ' ' << *d;
	}
	ost << " )";

	return ost;
}

inline
bool read(std::istream &ist, SubtreeNode &n)
{
	char ch;
	if (! (ist >> ch)) {
		return false;
	}

	if (ch != '(') {
		ist.putback(ch);
		return false;
	}

	n._dtrs.clear();
	if (! (ist >> n._label)) {
		throw std::runtime_error("SubtreeNode format error: a label expected");
	}

	SubtreeNode d;
	while (read(ist, d)) {
		n._dtrs.push_back(d);
	}

	if (! (ist >> ch) || ch != ')') {
		throw std::runtime_error(
			"SubtreeNode format error: ')' expected but got "
			+ std::string(1, ch));
	}

	return true;
}

#endif // Subtree_h__
