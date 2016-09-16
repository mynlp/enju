#ifndef __SupertaggerExternal_h__
#define __SupertaggerExternal_h__

#include <liblilfes/procstream.h>
#include "LexAnalyzerPipe.h"

namespace up {

    class SupertaggerExternal : public Supertagger {
    public:
        SupertaggerExternal(const std::vector<std::string>& args)
        {
            if (args.empty()) {
                throw std::logic_error("SupertaggerExternal: empty argument");
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
                throw std::runtime_error("cannot initialize supertagger");
            }
        }

        ~SupertaggerExternal()
        {
            pty_stream.close();
        }

        bool analyze(const WordLattice& word_lattice,
                     std::vector<std::vector<LexTemplateFom> >& tmpls) const 
        {
            tmpls.clear();
            tmpls.resize(word_lattice.size());

            // write word_lattice to external command
            for (WordLattice::const_iterator w = word_lattice.begin(); w != word_lattice.end(); ++w) {
                pty_stream << w->begin << '\t'
                           << w->end   << '\t'
                           << w->word_fom;
                for (std::vector<std::string>::const_iterator f = w->word.begin();
                        f != w->word.end(); ++f) {
                    pty_stream << '\t' << *f;
                }
                for (std::vector<std::string>::const_iterator k = w->lookup_keys.begin();
                        k != w->lookup_keys.end(); ++k) {
                    pty_stream << '\t' << *k;
                }
                pty_stream << '\n';
            }
            pty_stream << std::endl; // sentence boundary, and flush

            // read tagging result
            for (unsigned i = 0; i < word_lattice.size(); ++i) {
				std::string line;
#ifdef _MSC_VER
				if (! lilfes::pty_stream::getline(pty_stream, line)) {
#else
                if (! std::getline(pty_stream, line)) {
#endif
                    throw std::runtime_error("SupertaggerExternal: error in external tagger");
				}

                std::istringstream iss(line);
                std::string tmpl;
                double fom;
                while (iss >> tmpl >> fom) {
                    tmpls[i].push_back(LexTemplateFom(tmpl, fom));
                }
            }
            std::string eos;
#ifdef _MSC_VER
			if (! lilfes::pty_stream::getline(pty_stream, eos) || ! eos.empty()) {
#else
            if (! std::getline(pty_stream, eos) || ! eos.empty()) {
#endif
                throw std::runtime_error("SupertaggerExternal: no eos line from external tagger");
            }
//for (unsigned i = 0; i < tmpls.size(); ++i) {
//    for (unsigned j = 0; j < tmpls[i].size(); ++j) {
//        std::cerr << i << ":" << j << "\t" << tmpls[i][j].tmpl << "\t" << tmpls[i][j].fom << std::endl;
//    }
//}

            return true;
        }

    private:
        mutable lilfes::pty_stream pty_stream;
    };
} // namespace up

#endif // __SupertaggerExternal_h__
