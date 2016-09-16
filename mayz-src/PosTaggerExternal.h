#ifndef __PosTaggerExternal_h__
#define __PosTaggerExternal_h__

#include <cmath>
#include <liblilfes/procstream.h>
#include "LexAnalyzerPipe.h"

namespace up {

    class PosTaggerExternal : public PosTagger {
    public:
        PosTaggerExternal(const std::vector<std::string>& args)
        {
            if (args.empty()) {
                throw std::logic_error("PosTaggerExternal: empty argument");
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
                throw std::runtime_error("cannot initialize POS tagger");
            }
        }

        ~PosTaggerExternal()
        {
            pty_stream.close();
        }

        void analyze(const std::string& sentence,
                     std::vector<TokenRegion>& tokens,
                     std::vector<BracketRegion>& bregions)
        {
            tokens.clear();
            bregions.clear();

            std::string normalized;
            normalizeInput(sentence, normalized);

            pty_stream << normalized << std::endl;

            std::string line;
            std::istringstream lineIss;
            std::istringstream beginIss;
#ifdef _MSC_VER
			while (lilfes::pty_stream::getline(pty_stream, line)) {
#else
            while (std::getline(pty_stream, line)) {
#endif
                if (line.empty()) { // EOS
                    return;
                }

                lineIss.clear();
                lineIss.str(line);

                std::string bstr;
                if (! (lineIss >> bstr)) {
                    throw std::runtime_error("PosTaggerExternal: input format error: " + line);
                }

                if (bstr == "B") {
                    unsigned bpos;
                    unsigned epos;
                    if (! (lineIss >> bpos >> epos)) {
                        throw std::runtime_error("PosTaggerExternal: input format error: " + line);
                    }
                    if (epos <= bpos) {
                        throw std::runtime_error("PosTaggerExternal: input format error: bad bracket: " + line);
                    }

                    BracketRegion br;
                    br.begin_offset = bpos;
                    br.end_offset = epos;
                    bregions.push_back(br);
                }
                else {
                    unsigned b;
                    beginIss.clear();
                    beginIss.str(bstr);
                    if (! (beginIss >> b)) {
                        throw std::runtime_error("PosTaggerExternal: input format error: " + line);
                    }

                    unsigned e;
                    if (! (lineIss >> e)) {
                        throw std::runtime_error("PosTaggerExternal: input format error: " + line);
                    }

                    TokenRegion token;
                    token.begin_offset = b;
                    token.end_offset   = e;
                    token.word = sentence.substr(b, e - b);
                    while (lineIss >> token.tag >> token.prob) {
                        token.prob = std::log(token.prob);
                        tokens.push_back(token);
                    }
                }
            }
        }

    private:
        void normalizeInput(const std::string& s, std::string& n) const
        {
            n = s;
            for (std::string::iterator ch = n.begin(); ch != n.end(); ++ch) {
#ifdef _MSC_VER
              if (::isspace(*ch)) {
#else
              if (std::isspace(*ch)) {
#endif
                *ch = ' ';
                }
            }
        }

    private:
        lilfes::pty_stream pty_stream;
    };

} // namespace up

#endif // __PosTaggerExternal_h__
