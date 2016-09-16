#ifndef __PennTree_h__
#define __PennTree_h__

#include <string>
#include <list>
#include <sstream>
#include <cassert>
#include <stdexcept>
#include "Lex.h"

using std::string;
using std::istream;
using std::ostream;
using std::ostringstream;

////////////////////////////////////////////////////////////////////////////////
//
// PennNode class
//
////////////////////////////////////////////////////////////////////////////////
class PennFormatError : public std::runtime_error {
public:
	PennFormatError(const string &msg) : std::runtime_error(msg) {}

	// string msg(void) { return _msg; }
	string msg(void) { return what(); }
// private:
//	string _msg;
};

template<class AnnotationData>
class PennNode {
private:
	typedef std::list<PennNode*> ContainerType;

	//// Inhibit naive copy
	PennNode(const PennNode &n) {}

public:
	typedef typename ContainerType::iterator iterator;
	typedef typename ContainerType::const_iterator const_iterator;

	typedef typename ContainerType::reverse_iterator reverse_iterator;
	typedef typename ContainerType::const_reverse_iterator 
			const_reverse_iterator;

	typedef AnnotationData AnnotationType;

public:
	PennNode() {}
	explicit PennNode(const string &s) : _str(s) {}

	virtual ~PennNode(void)
	{ 
		for (iterator d = dtrBegin(); d != dtrEnd(); d++) {
			delete *d;
		}
	}

	// virtual void readExtraData(Lex &lex) {}
	// virtual void writeExtraData(ostream &ost) const {}

	//--------------------------------------------------------------------------
	// Equality
	//--------------------------------------------------------------------------
	bool operator==(const PennNode<AnnotationData> &n) const
	{
		if (_str != n._str || _annot != n._annot) {
			return false;
		}

		const_iterator dtr1 = dtrBegin();
		const_iterator dtr1End = dtrEnd();

		const_iterator dtr2 = dtrBegin();
		const_iterator dtr2End = dtrEnd();

		for ( ; dtr1 != dtr1End && dtr2 != dtr2End; dtr1++, dtr2++) {
			if (**dtr1 != **dtr2) {
				return false;
			}
		}

		return  (dtr1 == dtr1End && dtr2 == dtr2End);
	}

	bool operator!=(const PennNode<AnnotationData> &n) const
	{
		return ! (*this == n);
	}

	//--------------------------------------------------------------------------
	// Node type test
	//--------------------------------------------------------------------------
	bool isLeaf(void) const { return _dtrs.empty(); }

	bool isUnary(void) const
	{
		return ! _dtrs.empty() && ++begin() == end();
	}

private:
	/// namae ga okasii
	bool isTerminal(void) const
	{
		return (! isLeaf())
			&& (*dtrBegin())->isLeaf()
			&& ++dtrBegin() == dtrEnd();
	}
public:

	bool isPreTerminal(void) const
	{
		return isTerminal();
	}

	//--------------------------------------------------------------------------
	// set/get label
	//--------------------------------------------------------------------------
	string getString(void) const { return _str; }
	void setString(const string &s) { _str = s; }

	//--------------------------------------------------------------------------
	// set/get annotation
	//--------------------------------------------------------------------------
	AnnotationData &getAnnotation(void) { return _annot; }
	const AnnotationData &getAnnotation(void) const { return _annot; }
	void setAnnotation(const AnnotationData &a) { _annot = a; }

	//--------------------------------------------------------------------------
	// Operation on dtrs
	//--------------------------------------------------------------------------
	iterator dtrBegin(void) { return _dtrs.begin(); }
	iterator dtrEnd(void) { return _dtrs.end(); }

	const_iterator dtrBegin(void) const { return _dtrs.begin(); }
	const_iterator dtrEnd(void) const { return _dtrs.end(); }

	reverse_iterator dtrRevBegin(void) { return _dtrs.rbegin(); }
	reverse_iterator dtrRevEnd(void) { return _dtrs.rend(); }

	const_reverse_iterator dtrRevBegin(void) const { return _dtrs.rbegin(); }
	const_reverse_iterator dtrRevEnd(void) const { return _dtrs.rend(); }

	iterator begin(void) { return dtrBegin(); }
	iterator end(void) { return dtrEnd(); }

	const_iterator begin(void) const { return dtrBegin(); }
	const_iterator end(void) const { return dtrEnd(); }

	void raiseDtr(iterator pos, PennNode<AnnotationData> &n)
	{
		_dtrs.insert(pos, n.dtrBegin(), n.dtrEnd());
		n._dtrs.clear();
	}

	void addDtr(PennNode *d) { _dtrs.push_back(d); }

	void pushBackDtr(PennNode *d) { addDtr(d); }
	void pushFrontDtr(PennNode *d) { _dtrs.push_front(d); }

	template<class IteratorT>
	void insertDtr(IteratorT it, PennNode<AnnotationData>* d)
	{
		_dtrs.insert(it, d);
	}

	void eraseDtr(iterator d) 
	{
		delete *d;
		_dtrs.erase(d);
	}

	/// Returned ptr should be deleted properly
	PennNode *removeDtr(iterator d)
	{
		PennNode *dtr = *d;
		_dtrs.erase(d);
		return dtr;
	}

	//// Erase all descendants
	void clear(void)
	{
		for (iterator d = dtrBegin(); d != dtrEnd(); d++) {
			delete *d;
		}

		_dtrs.clear();
		_str.clear();
	}

	//--------------------------------------------------------------------------
	// Clone
	//--------------------------------------------------------------------------
	PennNode *clone(void) const
	{
		PennNode *n = new PennNode;
		n->setString(getString());
		n->_annot = _annot;
		for (const_iterator d = dtrBegin(); d != dtrEnd(); d++) {
			n->addDtr((*d)->clone());
		}

		return n;
	}

private:
	string _str;
	AnnotationData _annot;
	ContainerType _dtrs;
};

////////////////////////////////////////////////////////////////////////////////
//
// Plain tree
//
////////////////////////////////////////////////////////////////////////////////
class NoAnnotation {
public:
	void write(ostream &) const {}
	void read(Lex &) {}

	bool operator==(const NoAnnotation &) const
	{
		return true;
	}

	bool operator!=(const NoAnnotation &) const
	{
		return false;
	}
};

//class PennTree : public PennNode<NoAnnotation> {
//public:
	//PennTree() {}
	//PennTree(const string &s) : PennNode<NoAnnotation>(s) {}
//};
typedef PennNode<NoAnnotation> PennTree;

//// Specialization for plain penn-tree
//istream &operator>>(istream &ist, PennTree &t);
//ostream &operator<<(ostream &ost, const PennTree &t);

////////////////////////////////////////////////////////////////////////////////
//
// Input/Output
//
////////////////////////////////////////////////////////////////////////////////
template<class AnnotationData>
bool readTree(Lex &lex, PennNode<AnnotationData> &t)
{
	typedef PennNode<AnnotationData> TreeType;

	if (lex.lookahead() == Lex::EOF_T) {
		return false;
	}

	//// Check beginning '('
	if (lex.lookahead() != Lex::OPEN_T) {
		throw PennFormatError("Missing beginning \'(\'");
	}
	lex.advance();


	//// Read label
	if (lex.lookahead() != Lex::WORD_T) {
		string type;
		switch (lex.lookahead()) {
		  case Lex::EOF_T:
		  	type = "EOF";
			break;
		  case Lex::OPEN_T:
		  	type = "(";
			break;
		  case Lex::CLOSE_T:
		  	type = ")";
			break;
		  default:
		  	assert(false && "Never reached");
		}

		throw PennFormatError(
			"Format error : expecting a label, but got " + type);
	}
	string label;
	lex.getToken(label);
	lex.advance();
	t.setString(label);

	//// Read annotation data
	AnnotationData a;
	a.read(lex);
	t.setAnnotation(a);

	//lex.advance();

	switch (lex.lookahead()) {
	case Lex::OPEN_T:
		while (lex.lookahead() == Lex::OPEN_T) {
			TreeType *d = new TreeType();
			if (! readTree(lex, *d)) {
				throw PennFormatError("Internal error");
			}
			t.addDtr(d);
		}

		if (lex.lookahead() != Lex::CLOSE_T) {
			throw PennFormatError("Unterminated node");
		}
		lex.advance(); //// Consume ending ')'
		return true;

	case Lex::CLOSE_T:
		throw PennFormatError("Empty node");

	case Lex::WORD_T: //// Terminal
	{
		string word;
		lex.getToken(word);
		lex.advance();
		TreeType *w = new TreeType(word);
		t.addDtr(w);

		if (lex.lookahead() != Lex::CLOSE_T) {
			throw PennFormatError("unterminated node");
		}
		lex.advance(); // Consume ending ')'
		return true;
	}
	default:
		throw PennFormatError("Empty node");
		
	}
}

template<class AnnotationData>
istream &operator>>(istream &ist, PennNode<AnnotationData> &t)
{
	t.clear();
	Lex lex(ist);
	readTree(lex, t);
	return ist;
}

template<class AnnotationData>
ostream &operator<<(ostream &ost, const PennNode<AnnotationData> &t)
{
	typedef PennNode<AnnotationData> TreeType;
	typedef typename TreeType::const_iterator TreeItr;

	ost << '(';
	ost << t.getString();

	//ost << ' ';
	t.getAnnotation().write(ost);

	for (TreeItr d = t.dtrBegin(); d != t.dtrEnd(); d++) {

		if ((*d)->isLeaf()) {
			ost << ' ' << (*d)->getString();
		}
		else {
			ost << ' ' << **d;
		}
	}
	ost << ')';

	return ost;
}

template<class AnnotationData>
void prettyPrint(
	ostream &ost,
	const PennNode<AnnotationData> &t,
	int indent = 0,
	bool newline = false
) {
	typedef PennNode<AnnotationData> TreeType;
	typedef typename TreeType::const_iterator TreeItr;

	if (newline) {
		ost << std::endl;
		for (int i = 0; i < indent; i++) {
			ost << ' ';
		}
	}

	ostringstream oss;
	oss << '(';
	oss << t.getString();
	t.getAnnotation().write(oss);

	string parent = oss.str();
	ost << parent;

	bool first = true;
	for (TreeItr d = t.dtrBegin(); d != t.dtrEnd(); d++) {

		if ((*d)->isLeaf()) {
			ost << ' ' << (*d)->getString();
		}
		else {
			ost << ' ';
			prettyPrint(ost, **d, indent + parent.length() + 1, ! first);
			if (first) {
				first = false;
			}
		}
	}
	ost << ')';
}

template<class AnnotationData>
void prettyPrint2(
	ostream &ost,
	const PennNode<AnnotationData> &t,
	int delta = 4,
	int indent = 0,
	bool line = false
) {
	typedef PennNode<AnnotationData> TreeType;
	typedef typename TreeType::const_iterator TreeItr;

	ost << std::endl;
	if (! line) {
		for (int i = 0; i < indent; i++) {
			ost << ' ';
		}
	}
	else {
		for (int i = 0; i < indent; ++i) {
			if (i % delta == 0) {
				ost << '.';
			}
			else {
				ost << ' ';
			}
		}
	}

	ostringstream oss;
	oss << '(';
	oss << t.getString();
	t.getAnnotation().write(oss);

	string parent = oss.str();
	ost << parent;

	bool first = true;
	for (TreeItr d = t.dtrBegin(); d != t.dtrEnd(); d++) {

		if ((*d)->isLeaf()) {
			ost << ' ' << (*d)->getString();
		}
		else {
			prettyPrint2(ost, **d, delta, indent + delta, line);
		}
	}
	ost << ')';
}

#endif //  __PennTree_h__
