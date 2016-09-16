#ifndef __LexAnalyzerBasic_h__
#define __LexAnalyzerBasic_h__

#include "LexAnalyzer.h"
#include "mayzutil.h"

namespace up {

    class LexAnalyzerBasic : public LexAnalyzer {
    protected:
        // data structures for each sentence
        LexEntLattice lexent_lattice;
        WordRegions word_regions;
        Brackets brackets;
        ParseStatus status;

        // profile
        unsigned analyze_word_time;
        unsigned analyze_lexent_time;
        unsigned total_time;

    public:
        LexAnalyzerBasic()
            : status(NOT_PARSED_YET)
            , analyze_word_time(0)
            , analyze_lexent_time(0)
            , total_time(0)
        {}

        ~LexAnalyzerBasic() {}

        //-------------------------------------------
        // main functions
        //-------------------------------------------
        bool analyze(const std::string& sentence)
        {
            ScopeProf prof("analyze lex total", total_time);

            status = NOT_PARSED_YET;
            lexent_lattice.clear();
            word_regions.clear();
            brackets.clear();
            analyze_word_time = 0;
            analyze_lexent_time = 0;

            if (! analyzeImpl(sentence)) return false;

            status = SUCCESS;
            return true;
        }

        //-------------------------------------------
        // access to the analysis results
        //-------------------------------------------
        void getLexEntLattice(LexEntLattice& lexent_lattice_) const
        {
            lexent_lattice_ = lexent_lattice;
        }

        void getWordRegions(WordRegions& word_regions_) const
        {
            word_regions_ = word_regions;
        }

        void getBrackets(Brackets& brackets_) const
        {
            brackets_ = brackets;
        }

        ParseStatus getParseStatus() const { return status; }

        //-------------------------------------------
        // profile
        //-------------------------------------------
        unsigned getAnalyzeWordTime()   const { return analyze_word_time; }
        unsigned getAnalyzeLexEntTime() const { return analyze_lexent_time; }
        unsigned getTotalTime()         const { return total_time; }

    protected:
        virtual bool analyzeImpl(const std::string& sentence) = 0;
    };

} // namespace up

#endif // __LexAnalyzerBasic_h__
