#ifndef SymbolTable_h__
#define SymbolTable_h__

#include "hashdef.h"

#include <map>
#include <vector>
#include <stdexcept>
#include <utility>

template<class SymbolT, class StorageT = std::map<SymbolT, int> >
class SymbolTable {
public:
    typedef typename StorageT::const_iterator SymbolItr;
    typedef typename StorageT::const_iterator const_iterator;
public:

    class UnknownSymbolError : public std::runtime_error {
    public:
        UnknownSymbolError(void) : std::runtime_error("Unknown symbol") {}

        template<class DataT>
        UnknownSymbolError(const DataT &d)
            : std::runtime_error("Unknown symbol: " + d) {}
    };

    int getID(const SymbolT &s)
    {
        typename StorageT::iterator i = _m.find(s);
        if (i == _m.end()) {
            int id = _i2s.size();
            _m.insert(std::make_pair(s, id));
            _i2s.push_back(s);
            return id;
        }
        else {
            return i->second;
        }
    }

    int getID(const SymbolT &s) const
    {
        typename StorageT::const_iterator i = _m.find(s);
        if (i == _m.end()) {
            throw UnknownSymbolError(s);
        }
        return i->second;
    }

    int getID(const SymbolT &s, int defaultValue) const
    {
        typename StorageT::const_iterator i = _m.find(s);
        if (i == _m.end()) {
            return defaultValue;
        }
        else {
            return i->second;
        }
    }

    SymbolT getSymbol(int id) const
    {
        if (id >= (int) _i2s.size()) {
            throw UnknownSymbolError();
        }
        return _i2s[id];
    }

    unsigned getNumSymbol(void) const { return _i2s.size(); }

    SymbolItr symbolBegin(void) const { return _m.begin(); }
    SymbolItr symbolEnd(void) const { return _m.end(); }

    const_iterator begin(void) const { return _m.begin(); }
    const_iterator end(void) const { return _m.end(); }

    bool hasID(const SymbolT &s) const
    {
        typename StorageT::const_iterator i = _m.find(s);
        return i != _m.end();
    }

    void swap(SymbolTable<SymbolT> &symtab)
    {
        _m.swap(symtab._m);
        _i2s.swap(symtab._i2s);
    }

    void deleteSymbol(int id)
    {
        typename StorageT::iterator i = _m.find(_i2s[id]);

        if (i != _m.end()) {
            _m.erase(i);
            _i2s[id].clear();
        }
    }

    void setID(const SymbolT &s, int id)
    {
        if ((int) _i2s.size() <= id) {
            _i2s.resize(id + 1);
        }

        if (hasID(s)) {
            throw std::runtime_error("setID: called twice for the same symbol");
        }

        if (hasID(_i2s[id])) {
            throw std::runtime_error("setID: try to use existing symbol id");
        }

        _m.insert(std::make_pair(s, id));
        _i2s[id] = s;
    }

    void clear(void)
    {
        _m.clear();
        _i2s.clear();
    }

protected:
    StorageT _m;
    std::vector<SymbolT> _i2s;
};

namespace SymbolTableUtil {
    typedef _HASHMAP<std::string, int> HashMap;
}

typedef SymbolTable<std::string, SymbolTableUtil::HashMap> HashStringTable;

#endif // SymbolTable_h__
