#ifndef __MorphAnalyzerExternal_h__
#define __MorphAnalyzerExternal_h__

#include <iterator>
#include <set>
#include <sstream>
#include <liblilfes/procstream.h>
#include "LexAnalyzerPipe.h"


namespace up {

    class MorphAnalyzerExternal : public MorphAnalyzer {
    public:
        MorphAnalyzerExternal(const std::vector<std::string>& args)
        {
            if (args.empty()) {
                throw std::logic_error("MorphAnalyzerExternal: empty argument");
            }

            std::vector<std::vector<char> > strs(args.size());
            std::vector<char*> ptrs(args.size());
            for (unsigned i = 0; i < args.size(); ++i) {
                strs[i].insert(strs[i].begin(), args[i].begin(), args[i].end());
                strs[i].push_back(0);
                ptrs[i] = &(strs[i][0]);
            }
            ptrs.push_back(0);

            pty_stream.open(ptrs[0], ptrs.size() - 1, &ptrs[0]);

            if (! pty_stream) {
                throw std::runtime_error("cannot initialize morph-analyzer");
            }
        }

        ~MorphAnalyzerExternal()
        {
            pty_stream.close();
        }

        void analyze(const std::string& sentence,
                     const std::vector<TokenRegion>& tokens,
                     const std::vector<BracketRegion>& bregions,
                     WordLattice& word_lattice,
                     WordRegions& word_regions,
                     Brackets& brackets) const
        {
            word_lattice.clear();
            word_regions.clear();
            brackets.clear();

            // Write to external morph analyzer
            for (std::vector<TokenRegion>::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
                pty_stream << it->begin_offset << '\t'
                           << it->end_offset << '\t'
                           << it->word << '\t'
                           << it->tag << '\t'
                           << it->prob << std::endl;
            }
            pty_stream << std::endl << std::flush;

            // Read from external morph analyzer
            bool terminated = false;
            std::string offset_line;
#ifdef _MSC_VER
			while (lilfes::pty_stream::getline(pty_stream, offset_line)) {
#else
            while (std::getline(pty_stream, offset_line)) {
#endif
                if (offset_line.empty()) {
                    terminated = true;
                    break;
                }

                std::string word_features;
                std::string lookup_keys;
#ifdef _MSC_VER
				if (! (lilfes::pty_stream::getline(pty_stream, word_features))) {
#else
                if (! (std::getline(pty_stream, word_features))) {
#endif
                    std::runtime_error("morph-analyzer output format error: no word-feature line");
                }
#ifdef _MSC_VER
				if (! (lilfes::pty_stream::getline(pty_stream, lookup_keys))) {
#else
                if (! (std::getline(pty_stream, lookup_keys))) {
#endif
                    std::runtime_error("morph-analyzer output format error: no lookup-key line");
                }

                WordExtent w;
                Region r;
                std::istringstream iss_offset_line(offset_line);
                if (! (iss_offset_line >> r.begin_offset >> r.end_offset >> w.word_fom)) {
                    std::runtime_error("morph-analyzer output format error: " + offset_line);
                }

                std::istringstream iss_word_features(word_features);
                std::copy(std::istream_iterator<std::string>(iss_word_features),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(w.word));

                std::istringstream iss_lookup_keys(lookup_keys);
                std::copy(std::istream_iterator<std::string>(iss_lookup_keys),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(w.lookup_keys));

                if (w.lookup_keys.empty()) {
                    std::runtime_error("morph-analyzer output format error: no lookup keys" + offset_line);
                }

                word_lattice.push_back(w);
                word_regions.push_back(r);
            }

            if (! terminated) {
                throw std::runtime_error("morph-analyzer output error: no empty line after the last token");
            }

            //--------------------------------------------------
            // offset -> lattice node-ID mapping
            //--------------------------------------------------
            std::set<unsigned> bs; // begin offsets
            for (WordRegions::const_iterator r = word_regions.begin(); r != word_regions.end(); ++r) {
                bs.insert(r->begin_offset);
            }

            // nearest begin position of each character position
            std::vector<unsigned> next_begin(sentence.size() + 1);
            std::set<unsigned, std::less<unsigned> >::const_iterator b = bs.begin();
            for (unsigned i = 0; i < next_begin.size(); ++i) {
                while (b != bs.end() && *b < i) {
                    ++b;
                }
                next_begin[i] = (b != bs.end()) ? *b : sentence.size();
            }

            // begin position -> node-ID mapping
            std::vector<unsigned> node_id_of(sentence.size() + 1);
            unsigned next_id = 0;
            for (std::set<unsigned>::const_iterator b = bs.begin(); b != bs.end(); ++b) {
                node_id_of[*b] = next_id++;
            }
            node_id_of.back() = next_id; // 'EOS' node

            for (unsigned i = 0; i < word_lattice.size(); ++i) {
                word_lattice[i].begin = node_id_of[ word_regions[i].begin_offset ];
                word_lattice[i].end   = node_id_of[ next_begin[ word_regions[i].end_offset ] ];
            }

            for (std::vector<BracketRegion>::const_iterator it = bregions.begin(); it != bregions.end(); ++it) {
                unsigned begin = node_id_of[ next_begin[ it->begin_offset ] ];
                unsigned end   = node_id_of[ next_begin[ it->end_offset ] ];
                brackets.push_back(Bracket(begin, end));
            }
        }

    private:
        mutable lilfes::pty_stream pty_stream;
    };

} // namespace up

#endif // __MorphAnalyzerExternal_h__
