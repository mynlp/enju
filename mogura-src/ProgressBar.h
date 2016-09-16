#ifndef ProgressBar_h__
#define ProgressBar_h__

#include <iostream>
#include <string>

namespace mogura {

class ProgressBar {
public:
	// t1 -> 1 dot
	// t2 -> new line + print count
	ProgressBar(int t1, int t2, std::ostream &ost, const std::string &unit)
		: _t1(t1)
		, _t2(t2)
		, _cnt1(0)
		, _cnt2(0)
		, _cnt3(0)
		, _ost(&ost)
		, _unit(unit)
	{}

	ProgressBar(int t1, int t2, std::ostream &ost)
		: _t1(t1)
		, _t2(t2)
		, _cnt1(0)
		, _cnt2(0)
		, _cnt3(0)
		, _ost(&ost)
	{}

	ProgressBar(void)
		: _t1(1)
		, _t2(10)
		, _cnt1(0)
		, _cnt2(0)
		, _cnt3(0)
		, _ost(&std::cerr)
	{}

	void advance()
	{
		_cnt1++;
		if (_cnt1 == _t1) {
			_cnt1 = 0;
			_cnt2 += _t1;
			(*_ost) << '.' << std::flush;
			if (_cnt2 >= _t2) {
				_cnt2 = 0;
				_cnt3 += _t2;
				(*_ost) << _cnt3 << std::endl;
			}
		}
	}

	void operator++() {
		advance();
	}

	void operator++(int) {
		advance();
	}

	void done()
	{
		(*_ost) << "done (" << (_cnt3 + _cnt2 + _cnt1);
		if (_unit != "") {
			(*_ost) << ' ' << _unit;
		}
		(*_ost) << ")" << std::endl;
	}

private:
	int _t1;
	int _t2;
	int _cnt1;
	int _cnt2;
	int _cnt3;
	std::ostream *_ost;
	std::string _unit;
};

} // namespace mogura

#endif // ProgressBar_h__
