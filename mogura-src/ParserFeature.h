#ifndef ParserFeature_h__
#define ParserFeature_h__

#include <stdexcept>
#include <liblilfes/utility.h>
#include "ParserState.h"

namespace mogura {

typedef std::vector<std::string> FeatureVec;

class StackElemBasicFeature {
public:
	StackElemBasicFeature(unsigned elemIx, unsigned posIndex, unsigned inputWordIndex)
		: _elemIx(elemIx)
        , _posIndex(posIndex)
        , _inputWordIndex(inputWordIndex)
	{
		assert(elemIx < 10);

		char ix = '0' + _elemIx;
		std::string s("S");

        _labelFeature = s + ix + ".nt=";
		_posFeature   = s + ix + ".h.p=";
		_wordFeature  = s + ix + ".h.w=";
		_tmplFeature  = s + ix + ".h.t=";
        _leftBoundPosFeature  = s + ix + ".lbp=";
        _rightBoundPosFeature = s + ix + ".rbp=";
		_noneFeature  = s + ix + "=none";
	}

	void extract(const ParserState &state, FeatureVec &fv) const
	{
		const ParseTree *n = state._stack.at(_elemIx);

		if (n == 0) {
			fv.push_back(_noneFeature);
		}
		else {
			fv.push_back(_labelFeature + n->_signLabel);

            const LexEnt *head = n->_lexHead;
			//fv.push_back(_posFeature  + head->_word.getPos());
			//fv.push_back(_wordFeature + head->_word.getInput());
			//fv.push_back(_tmplFeature + head->_lexTemplateName);
            //fv.push_back(_leftBoundPosFeature  + n->_leftBound->_word.getPos());
            //fv.push_back(_rightBoundPosFeature + n->_rightBound->_word.getPos());
			fv.push_back(_posFeature  + head->_word[ _posIndex ]);
			fv.push_back(_wordFeature + head->_word[ _inputWordIndex ]);
			fv.push_back(_tmplFeature + head->_lexTemplate);
            fv.push_back(_leftBoundPosFeature  + n->_leftBound->_word[ _posIndex ]);
            fv.push_back(_rightBoundPosFeature + n->_rightBound->_word[ _posIndex ]);
		}
	}

private:
	unsigned _elemIx;
    std::string _labelFeature;
	std::string _posFeature;
	std::string _wordFeature;
	std::string _tmplFeature;
	std::string _noneFeature;

    std::string _leftBoundPosFeature;
    std::string _rightBoundPosFeature;

    unsigned _posIndex;
    unsigned _inputWordIndex;
};

class MostRecentDependentFeature {
public:
	MostRecentDependentFeature(
		unsigned elemIx,
        ParseTree::Direction dir,
        unsigned posIndex,
        unsigned inputWordIndex
	)
		: _elemIx(elemIx)
		, _dir(dir)
        , _posIndex(posIndex)
        , _inputWordIndex(inputWordIndex)
	{
		assert(elemIx < 10);

		char ixChar = '0' + _elemIx;
		char dchar = (_dir  == ParseTree::LEFT) ? 'l' : 'r';
		std::string s(std::string("S") + ixChar + ".mrd" + dchar);

		_posFeature  = s + ".p=";
		_wordFeature = s + ".w=";
		_tmplFeature = s + ".t=";
		_noneFeature = s + "=none";
	}

	void extract(const ParserState &state, FeatureVec &fv) const
	{
		const ParseTree *n = state._stack.at(_elemIx);
        if (n == 0) {
			fv.push_back(_noneFeature);
            return;
        }

        const ParseTree *d = n->getRecentDep(_dir);
		if (d == 0) {
			fv.push_back(_noneFeature);
            return;
		}

        const LexEnt *head = d->_lexHead;
        //fv.push_back(_posFeature  + head->_word.getPos());
        //fv.push_back(_wordFeature + head->_word.getInput());
        //fv.push_back(_tmplFeature + head->_lexTemplateName);
        fv.push_back(_posFeature  + head->_word[ _posIndex ]);
        fv.push_back(_wordFeature + head->_word[ _inputWordIndex ]);
        fv.push_back(_tmplFeature + head->_lexTemplate);
	}
private:
	unsigned _elemIx;
	ParseTree::Direction _dir;
	std::string _posFeature;
	std::string _wordFeature;
	std::string _tmplFeature;
	std::string _noneFeature;

    unsigned _posIndex;
    unsigned _inputWordIndex;
};

class LookaheadBasicFeature {
public:
	LookaheadBasicFeature(unsigned elemIx, unsigned posIndex, unsigned inputWordIndex)
		: _elemIx(elemIx)
        , _posIndex(posIndex)
        , _inputWordIndex(inputWordIndex)
	{
		assert(elemIx < 10);

		char ixChar = '0' + _elemIx;
		std::string w(std::string("W") + ixChar);

        _labelFeature = w + ".nt=";
		_posFeature   = w + ".h.p=";
		_wordFeature  = w + ".h.w=";
		_tmplFeature  = w + ".h.t=";
		_noneFeature  = w + "=none";
	}
		
	void extract(const ParserState &state, FeatureVec &fv) const
	{
		const ParseTree *n = state._words.at(_elemIx);

		if (n == 0) {
			fv.push_back(_noneFeature);
		}
		else {
            fv.push_back(_labelFeature + n->_signLabel);

            //const LexInfo *head = n->_lexHead;
            //fv.push_back(_posFeature  + head->_word.getPos());
            //fv.push_back(_wordFeature + head->_word.getInput());
            //fv.push_back(_tmplFeature + head->_lexTemplateName);
            const LexEnt *head = n->_lexHead;
            fv.push_back(_posFeature  + head->_word[ _posIndex ]);
            fv.push_back(_wordFeature + head->_word[ _inputWordIndex ]);
            fv.push_back(_tmplFeature + head->_lexTemplate);
		}
	}
private:
	unsigned _elemIx;
	std::string _labelFeature;
	std::string _posFeature;
	std::string _wordFeature;
	std::string _tmplFeature;
	std::string _noneFeature;

    unsigned _posIndex;
    unsigned _inputWordIndex;
};

class BasicFeature {
public:
	BasicFeature(unsigned posIndex, unsigned inputWordIndex)
		: _stack0(0, posIndex, inputWordIndex)
		, _stack1(1, posIndex, inputWordIndex)
		, _stack2(2, posIndex, inputWordIndex)
		, _stack3(3, posIndex, inputWordIndex)
		, _lookahead0(0, posIndex, inputWordIndex)
		, _lookahead1(1, posIndex, inputWordIndex)
		, _lookahead2(2, posIndex, inputWordIndex)
		, _lookahead3(3, posIndex, inputWordIndex)
		, _mrd0L(0, ParseTree::LEFT,  posIndex, inputWordIndex)
		, _mrd0R(0, ParseTree::RIGHT, posIndex, inputWordIndex)
		, _mrd1L(1, ParseTree::LEFT,  posIndex, inputWordIndex)
		, _mrd1R(1, ParseTree::RIGHT, posIndex, inputWordIndex)
	{}

	void extract(const ParserState &state, FeatureVec &fv) const
	{
		_stack0.extract(state, fv);
		_stack1.extract(state, fv);
		_stack2.extract(state, fv);
		_stack3.extract(state, fv);

		_lookahead0.extract(state, fv);
		_lookahead1.extract(state, fv);
		_lookahead2.extract(state, fv);
		_lookahead3.extract(state, fv);

		const ParseTree *s0 = state._stack.at(0);
		if (s0 != 0) {
			if (s0->_leftDtr != 0) {
				fv.push_back("S0.l.nt=" + s0->_leftDtr->_signLabel);
			}

			if (s0->_rightDtr != 0) {
				fv.push_back("S0.r.nt=" + s0->_rightDtr->_signLabel);
			}
		}

		const ParseTree *s1 = state._stack.at(1);
		if (s1 != 0) {
			if (s1->_leftDtr != 0) {
				fv.push_back("S1.l.nt=" + s1->_leftDtr->_signLabel);
			}

			if (s1->_rightDtr != 0) {
				fv.push_back("S1.r.nt=" + s1->_rightDtr->_signLabel);
			}
		}

		if (s0 != 0 && s1 != 0) {
            // int dist = s0->_lexHead->_word.getPosition() - s1->_lexHead->_word.getPosition();
            int dist = s0->_lexHead->_begin - s1->_lexHead->_begin;
			char tmp[128];
			std::sprintf(tmp, "%d", dist);
			fv.push_back(std::string("dist=") + tmp);
		}

		_mrd0L.extract(state, fv);
		_mrd0R.extract(state, fv);
		_mrd1L.extract(state, fv);
		_mrd1R.extract(state, fv);
	}
private:
	StackElemBasicFeature _stack0;
	StackElemBasicFeature _stack1;
	StackElemBasicFeature _stack2;
	StackElemBasicFeature _stack3;

	LookaheadBasicFeature _lookahead0;
	LookaheadBasicFeature _lookahead1;
	LookaheadBasicFeature _lookahead2;
	LookaheadBasicFeature _lookahead3;

	MostRecentDependentFeature _mrd0L;
	MostRecentDependentFeature _mrd0R;
	MostRecentDependentFeature _mrd1L;
	MostRecentDependentFeature _mrd1R;
};


class EnjuFeature {
public:
	EnjuFeature(Grammar &g)
        : _grammar(g)
    {}

	void extract(const ParserState &state, FeatureVec &fv) const
	{
        lilfes::IPTrailStack iptrail(_grammar.getMachine());

		const ParseTree *left  = state._stack.at(1);
		const ParseTree *right = state._stack.at(0);

		if (right == 0) {
			return;
		}

        lilfes::machine *m = _grammar.getMachine();

        lilfes::FSP lSign = (left == 0) ? lilfes::FSP(m) : left->_sign;
        lilfes::FSP rSign = right->_sign;
        lilfes::FSP fvFsp(m);

        std::vector<std::string> fs;

        if (_grammar.getEnjuFeature(lSign, rSign, fvFsp) && lilfes::list_to_vector(*m, fvFsp, fs)) {
            fv.insert(fv.end(), fs.begin(), fs.end());
        }
        else {
            throw std::logic_error("bad definition of get_enju_feature/3");
        }
	}
private:
    Grammar &_grammar;
};

class ValenceFeature {
public:
	ValenceFeature(Grammar &g)
        : _grammar(g)
    {}

	void extract(const ParserState &state, FeatureVec &fv) const
	{
		extractFromElem("S0.", state._stack.at(0), fv);
		extractFromElem("S1.", state._stack.at(1), fv);
		extractFromElem("W0.", state._words.at(0), fv);
		extractFromElem("W1.", state._words.at(1), fv);
	}

private:
	void extractFromElem(
		const std::string &prefix,
		const ParseTree *n,
		FeatureVec &fv
	) const {

		if (n == 0) {
			return;
		}

        lilfes::IPTrailStack iptrail(_grammar.getMachine());

        lilfes::machine *m = _grammar.getMachine();

        std::vector<std::string> fs;
		lilfes::FSP fvFsp(m);
        if (_grammar.getValFeature(n->_sign, fvFsp) && lilfes::list_to_vector(*m, fvFsp, fs)) {
            for (std::vector<std::string>::const_iterator f = fs.begin(); f != fs.end(); ++f) {
                if (*f != "none") {
                    fv.push_back(prefix + *f);
                }
            }
        }
        else {
            throw std::logic_error("bad definition of get_val_feature/2");
        }
	}

private:
    Grammar &_grammar;
};

class FeatureExtractor {
public:
	FeatureExtractor(Grammar &g)
        : _basic(g.getPosIndex(), g.getInputWordIndex())
        , _enju(g)
        , _val(g)
    {}

	void extract(const ParserState &state, FeatureVec &fv) const
	{
        up::ScopeProf prof("mogura::FeatureExtractor::extract");

        _basic.extract(state, fv);
        _enju.extract(state, fv);
        _val.extract(state, fv);
	}

private:
    BasicFeature _basic;
    EnjuFeature _enju;
    ValenceFeature _val;
};

} // namespace mogura {

#endif // ParserFeature_h__
