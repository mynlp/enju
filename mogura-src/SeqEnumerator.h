#ifndef SeqEnumerator_h__
#define SeqEnumerator_h__

#include <string>
#include "Extent.h"
#include "CfgGrammar.h"

//------------------------------------------------------------------------------
namespace mogura {
namespace cfg {
namespace seq_enum {
//------------------------------------------------------------------------------

/// pimpl pattern
class SeqEnumeratorImpl;

class SeqEnumerator {
public:
    struct LeafPtr {
        unsigned _extentIx;
        unsigned _templateIx;

        LeafPtr(void) {}
        LeafPtr(unsigned extentIx, unsigned templateIx)
            : _extentIx(extentIx)
            , _templateIx(templateIx) {}

        bool operator==(const LeafPtr &lp) const
        {
            return _extentIx == lp._extentIx && _templateIx == lp._templateIx;
        }
    };

    typedef std::vector<LeafPtr> TemplateSeq;

public:
	SeqEnumerator(
        const Grammar *grammar,
        const Grammar *restricted, /// when = 0 -> without thomazo's optimization
        double lexThreshold,
        double seqThreshold,
        unsigned maxQueueElem
    );

	~SeqEnumerator(void);

	bool getFirst(const up::LexEntLattice &lattice, TemplateSeq &seq);
	//bool getFirst(const LeafLattice &input, std::vector<unsigned> &seq, double limitScore);

	bool getNext(TemplateSeq &seq);

	bool findBestFailSeq(TemplateSeq &seq);

    void setSeqThreshold(double threshold);
    void setLexThreshold(double threshold);
    void setMaxQueueItem(unsigned max);

private:
	SeqEnumeratorImpl *_impl;
};

//------------------------------------------------------------------------------
} 
typedef seq_enum::SeqEnumerator SeqEnumerator;

} /// namespace cfg
} /// namespace mogura
//------------------------------------------------------------------------------

#endif // SeqEnumerator_h__
