/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file LexiconTable.h
 *  @version Time-stamp: <2009-12-23 10:03:51 yusuke>
 *  Interface for the lexicon in text file format
 *
 **********************************************************************/

#ifndef __LexiconTable_h__
#define __LexiconTable_h__

#include <stdexcept>
#include <string>
#include <vector>
#include <iterator>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace up {

    class LexiconTable {
    public:
        LexiconTable() {}
        LexiconTable(std::istream &ist)
        {
            load(ist);
        }

        bool load(const std::string &fname)
        {
            std::ifstream in(fname.c_str());
            if (! in) {
                return false;
            }
            try {
                load(in);
            }
            catch (std::runtime_error &e) {
                return false;
            }
            return true;
        }

        void load(std::istream &ist)
        {
            _lexicon.clear();

            std::string line;
            while (std::getline(ist, line)) {
                std::istringstream iss(line);

                std::string key;
                if (! (iss >> key)) {
                    throw std::runtime_error("LexiconTable: lexicon table file format error");
                }

                std::vector<std::string> tmpls;
                std::copy(std::istream_iterator<std::string>(iss),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(tmpls));

                if (tmpls.empty()) {
                    throw std::runtime_error("LexiconTable: Empty line in the lexicon");
                }

                /// We don't care about the uniqueness of the key in the lexicon
                std::vector<std::string> &lexiconItem = _lexicon[key];
                lexiconItem.insert(lexiconItem.end(), tmpls.begin(), tmpls.end());
            }
        }

        bool save(const std::string &fname)
        {
            std::ofstream out(fname.c_str());
            if (! out) {
                return false;
            }
            save(out);
            return true;
        }

        void save(std::ostream &ost)
        {
            for (LexiconMap::const_iterator it = _lexicon.begin(); it != _lexicon.end(); ++it) {
                ost << it->first;
                const std::vector<std::string> &tmpls = it->second;
                for (std::vector<std::string>::const_iterator tmpl = tmpls.begin(); tmpl != tmpls.end(); ++tmpl) {
                    ost << ' ' << *tmpl;
                }
                ost << std::endl;
            }
        }

        void add(const std::string &key, const std::string &temp)
        {
            std::vector<std::string> &lex_list = _lexicon[key];
            if (std::find(lex_list.begin(), lex_list.end(), temp) == lex_list.end()) {
                lex_list.push_back(temp);
            }
        }

        void insert(const std::string &key, const std::vector<std::string> &temp_list)
        {
            _lexicon[key] = temp_list;
        }

        bool erase(const std::string &key)
        {
            LexiconMap::iterator it = _lexicon.find(key);
            if (it != _lexicon.end()) {
                _lexicon.erase(it);
            }
            else {
                return false;
            }
            return true;
        }

        bool lookup(const std::string &key, std::vector<std::string> &temp_list)
        {
            temp_list.clear();
            LexiconMap::const_iterator it = _lexicon.find(key);
            if (it != _lexicon.end()) {
                temp_list = it->second;
                return true;
            }
            else {
                return false;
            }
        }

        bool lookup(
            const std::vector<std::string> &keys,
            std::vector<std::string> &templates
        ) const {
            for (std::vector<std::string>::const_iterator key = keys.begin();
                    key != keys.end(); ++key) {
                LexiconMap::const_iterator it = _lexicon.find(*key);
                if (it != _lexicon.end()) {
                    templates = it->second;
                    return true;
                }
            }
            return false;
        }

    private:
        typedef std::map<std::string, std::vector<std::string> > LexiconMap;
        LexiconMap _lexicon;

    public:
        typedef LexiconMap::iterator iterator;
        iterator begin() { return _lexicon.begin(); }
        iterator end() { return _lexicon.end(); }
    };
}

#endif // __LexiconTable_h__
