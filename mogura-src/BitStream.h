#ifndef BitStream_h__
#define BitStream_h__

#include <iostream>
#include <vector>
#include <stdexcept>

namespace mogura {

class InputBitStream {
public:
    InputBitStream(std::istream &ist)
        : _ist(ist)
        , _ix(8)
        , _byte(0)
        , _eof(false)
    {}

    InputBitStream &get(unsigned int &n)
    {
        if (_ix == 8) {
            if (! _ist.get(_byte)) {
                _eof = true;
                return *this;
            }
            _ix = 0;
        }

        n = 1 & (_byte >> _ix++);
        return *this;
    }

    InputBitStream &get(std::vector<bool> &bits)
    {
        unsigned int bit;
        for (unsigned i = 0; i < bits.size(); ++i) {
            if (! get(bit)) {
                return *this;
            }
            bits[i] = (bit == 1);
        }
        return *this;
    }

    operator bool() const { return ! _eof; }

private:
    std::istream &_ist;
    unsigned _ix;
    char _byte;
    bool _eof;
};

inline
InputBitStream &operator>>(InputBitStream &ibs, unsigned int &bit) { return ibs.get(bit); }

inline
InputBitStream &operator>>(InputBitStream &ibs, std::vector<bool> &bits) { return ibs.get(bits); }

class OutputBitStream {
public:
    OutputBitStream(std::ostream &ost)
        : _ix(0)
        , _byte(0)
        , _ost(ost)
    {}

    ~OutputBitStream(void) { flush(); }

    void put(unsigned int bit)
    {
        _byte |= ((bit & 1) << _ix);
        if (++_ix == 8) {
            flush();
        }
    }

    void put(const std::vector<bool> &bits)
    {
        for (std::vector<bool>::const_iterator it = bits.begin(); it != bits.end(); ++it) {
            put((*it) ? 1 : 0);
        }
    }

    void flush(void)
    {
        if (_ix > 0) {
            _ost.put(_byte);
        }
        _ix = 0;
        _byte = 0;
    }

private:
    unsigned _ix;
    char _byte;
    std::ostream &_ost;
};

inline
OutputBitStream &operator<<(OutputBitStream &obs, unsigned int bit)
{
    obs.put(bit);
    return obs;
}

} /// namespace mogura {

#endif // BitStream_h__
