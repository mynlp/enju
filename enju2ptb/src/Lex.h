#ifndef __Lex_h__
#define __Lex_h__

#include <iostream>
#include <string>
#include <cctype>

using std::istream;
using std::string;

class Lex {
public:
	enum TokenType { OPEN_T, WORD_T, CLOSE_T, EOF_T };

public:
	Lex(istream &ist)
	: _lookahead(false), _nextType(EOF_T), _next(), _ist(ist) {;}

	TokenType lookahead(void)
	{
		if (_lookahead) {
			return _nextType;
		}
		else {
			get();
			_lookahead = true;
			return _nextType;
		}
	}

	void getToken(string &word)
	{
		word = _next;
		_lookahead = false;
	}

	void advance(void)
	{
		_lookahead = false;
	}

	~Lex(void)
	{
		//std::cout << "Lex deleted" << std::endl;
	}
private:
	void get(void)
	{
		char ch;
		while (_ist.get(ch) && isspace(ch))
			;

		if (! _ist) {
			_nextType = EOF_T;
			return;
		}

		switch (ch) {
		case '(':
			_nextType = OPEN_T;
			return;
		case ')':
			_nextType = CLOSE_T;
			return;
		default:
			_nextType = WORD_T;
			_next = string(1, ch);
			while (_ist.get(ch)) {
				if (ch == '(' || ch == ')') {
					_ist.unget();
					return;
				}
				else if (isspace(ch)) {
					return;
				}
				else {
					_next += ch;
				}
			}
			return;
		}
	}
		
private:
	bool _lookahead;
	TokenType _nextType;
	string _next;
	istream &_ist;
};

#endif // __Lex_h__
