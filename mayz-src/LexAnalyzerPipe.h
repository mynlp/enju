#ifndef __LexAnalyzerPipe_h__
#define __LexAnalyzerPipe_h__

#include "LexAnalyzerBasic.h"

namespace up {

    struct BracketRegion : public Region {};

    struct TokenRegion : public Region {
        std::string word; // we need this since the region [begin, end) includes tags when tagged input ("runs/VBZ")
        std::string tag;
        double prob;
    };

    // interface of POS tagger
    class PosTagger {
    public:
        virtual ~PosTagger() {}
        virtual void analyze(const std::string& sentence,
                             std::vector<TokenRegion>& tokens,
                             std::vector<BracketRegion>& bregions) = 0;
    };

    // interface of morphology analyzer
    class MorphAnalyzer {
    public:
        virtual ~MorphAnalyzer() {}
        virtual void analyze(const std::string& sentence,
                             const std::vector<TokenRegion>& tokens,
                             const std::vector<BracketRegion>& bregions,
                             WordLattice& word_lattice,
                             WordRegions& word_regions,
                             Brackets& brackets) const = 0;
    };

    // interface of supertagger
    class Supertagger {
    public:
        virtual ~Supertagger() {}
        virtual bool analyze(const WordLattice& word_lattice,
                             std::vector<std::vector<LexTemplateFom> >& tmpls) const = 0;
    };

    class LexAnalyzerPipe : public LexAnalyzerBasic {
    public:
        LexAnalyzerPipe(PosTagger* pos_tagger_, MorphAnalyzer* morph_, Supertagger* super_)
            : pos_tagger(pos_tagger_)
            , morph(morph_)
            , super(super_)
            {}

        ~LexAnalyzerPipe()
        {
#ifdef _MSC_VER
            delete pos_tagger;
            delete morph;
            delete super;
#else
            // Work-around for the (infinite?) block at wait() in ~pty_stream
            //delete pos_tagger;
            //delete morph;
            //delete super;
#endif
        }

    protected:
        bool analyzeImpl(const std::string& sentence)
        {
            WordLattice word_lattice;
            {
                ScopeProf prof("word analysis", analyze_word_time);

                try {
                    std::vector<TokenRegion> tokens;
                    std::vector<BracketRegion> bregions;

                    pos_tagger->analyze(sentence, tokens, bregions);
                    morph->analyze(sentence, tokens, bregions, word_lattice, word_regions, brackets);
                }
                catch (const std::runtime_error& e) {
                    std::cerr << "LexAnalyzer: " << e.what() << std::endl;
                    status = WORD_ANALYSIS_ERROR;
                    return false;
                }
            }
            
            std::vector<std::vector<LexTemplateFom> > templates;
            {
                ScopeProf prof("lex analysis", analyze_lexent_time);

                if (! super->analyze(word_lattice, templates)) {
                    status = LEXENT_ANALYSIS_ERROR;
                    return false;
                }
            }

            lexent_lattice.resize(word_lattice.size());
            for (unsigned i = 0; i < lexent_lattice.size(); ++i) {
                lexent_lattice[i].begin = word_lattice[i].begin;
                lexent_lattice[i].end   = word_lattice[i].end;
                lexent_lattice[i].word  = word_lattice[i].word;
                lexent_lattice[i].tmpls.swap(templates[i]);
            }

            return true;
        }

    private:
        PosTagger *pos_tagger;
        MorphAnalyzer *morph;
        Supertagger *super;
    };
} // namespace up

#endif // __LexAnalyzerPipe_h__
