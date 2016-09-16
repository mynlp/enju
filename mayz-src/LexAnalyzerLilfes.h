#ifndef __LexAnalyzerLilfes_h__
#define __LexAnalyzerLilfes_h__

#include "LexAnalyzerBasic.h"
#include "DataConv.h"

namespace up {

    class LexAnalyzerLilfes : public LexAnalyzerBasic {
    public:
        LexAnalyzerLilfes(lilfes::machine* mach_, Grammar *grammar_)
            : mach(mach_)
            , grammar(grammar_)
            {}

    protected:
        bool analyzeImpl(const std::string& sentence)
        {
            lilfes::IPTrailStack iptrail(mach);
            ScopeProf prof("word analysis", analyze_word_time);

            lilfes::FSP sntFsp(mach, sentence.c_str());
            lilfes::FSP latticeFsp(mach);
            if (! grammar->sentence2Lattice(sntFsp, latticeFsp)) {
                status = WORD_ANALYSIS_ERROR;
                return false;
            }

            if (! conv::convLexEntLattice(mach, grammar, latticeFsp, lexent_lattice, brackets)) {
                status = WORD_ANALYSIS_ERROR;
                return false;
            }

            // NOTE: don't use word region from this implementation
            word_regions.clear();

            return true;
        }

    private:

        unsigned getLastNode(const LexEntLattice& lattice) const
        {
            unsigned last_node = 0;
            for (LexEntLattice::const_iterator it = lattice.begin(); it != lattice.end(); ++it) {
                last_node = std::max(last_node, it->end);
            }
            return last_node;
        }

    private:
        lilfes::machine* mach;
        Grammar* grammar;
    };

} // namespace up

#endif // __LexAnalyzerLilfes_h__
