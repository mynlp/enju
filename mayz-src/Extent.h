#ifndef __Extent_h
#define __Extent_h

#include <vector>
#include <string>

// Interface between parser/supertagger/morph-analyzer
namespace up {

    // Text region
    struct Region {
        unsigned begin_offset;
        unsigned end_offset;

        Region() : begin_offset(0), end_offset(0) {}
        Region(unsigned begin_, unsigned end_) : begin_offset(begin_), end_offset(end_) {}
    };

    // lattice edge
    struct Extent {
        unsigned begin; // start node
        unsigned end;   // end node

        Extent() : begin(0), end(0) {}
        Extent(unsigned begin_, unsigned end_) : begin(begin_), end(end_) {}
    };

    struct Bracket : public Extent {
        Bracket() {}
        Bracket(unsigned begin_, unsigned end_) : Extent(begin_, end_) {}
    };

    struct WordExtent : public Extent {
        std::vector<std::string> word; // result of morphological analysis
        std::vector<std::string> lookup_keys; // lexicon look-up keys
        double word_fom; // fom of this word-analysis (segmentation + POS-tagging)
    };

    struct LexTemplateFom {
        std::string tmpl; // lexical template name
        double fom; // fom of this template assignment (including fom of underlying word-analysis)

        LexTemplateFom() {}
        LexTemplateFom(const std::string& tmpl_, double fom_)
            : tmpl(tmpl_)
            , fom(fom_) {}
    };

    struct LexEntExtent : public Extent {
        std::vector<std::string> word;
        std::vector<LexTemplateFom> tmpls;
    };

    typedef std::vector<Region> WordRegions;
    typedef std::vector<Bracket> Brackets;
    typedef std::vector<WordExtent> WordLattice;
    typedef std::vector<LexEntExtent> LexEntLattice;
}

#endif // __Extent_h
