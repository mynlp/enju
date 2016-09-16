#ifndef ChunkFinder_h__
#define ChunkFinder_h__

#include <stdexcept>
#include <vector>
#include <iostream>
#include <limits>

#ifdef _MSC_VER
#undef max
#endif

namespace mogura {

class NonConnectedChart : public std::runtime_error {
public:
    NonConnectedChart(void) : std::runtime_error("parse error (\"non-connected chart\")") {}
};

template<class CellT, class EdgeSetT>
struct ChunkFinderBase {
public:

	struct Chunk {
		unsigned _start;
		unsigned _end;
		const EdgeSetT *_es;
		unsigned _forwardChunkNum;
		double _forwardScore;

		Chunk(void) : _es(0) {}

		void print(std::ostream &ost) const
		{
			ost << "start=" << _start
			    << ", end=" << _end
				<< ", eset=" << _es
				<< ", num=" << _forwardChunkNum
				<< ", score=" << _forwardScore
				<< std::endl;
		}
	};

public:

	virtual ~ChunkFinderBase(void) {}

	virtual const EdgeSetT *findBestEdgeSet(const CellT &c) const = 0;

	virtual double getEdgeSetScore(const EdgeSetT *e) const = 0;

	void initLattice(const std::vector<std::vector<CellT> > &chart, std::vector<Chunk> &lattice)
    {
		unsigned len = chart.size();

		lattice.clear();
		lattice.resize(len + 1);

		lattice[0]._forwardChunkNum = 0;
		lattice[0]._forwardScore = 0; /// not required?

		for (unsigned end = 1; end <= len; ++end) {
			
            Chunk &chunk = lattice[end];
            chunk._end = end;
            chunk._forwardChunkNum = std::numeric_limits<unsigned>::max();

			for (unsigned start = 0; start < end; ++start) {
				
				const CellT &c = chart[start][end];
				if (c.empty()) {
					continue;
				}

				unsigned numChunk = lattice[start]._forwardChunkNum + 1;

				if (numChunk > chunk._forwardChunkNum) {
                    continue;
                }
                else {
					const EdgeSetT *es = findBestEdgeSet(c);
					double score = lattice[start]._forwardScore + getEdgeSetScore(es);

					if (numChunk < chunk._forwardChunkNum || score > lattice[end]._forwardScore) {
						chunk._start = start;
						chunk._es = es;
						chunk._forwardScore = score;
						chunk._forwardChunkNum = numChunk;
					}
				}
			}

            if (chunk._es == 0) {
                throw NonConnectedChart();
            }
		}
	}

    void findBestChunkSeq(const std::vector<std::vector<CellT> > &chart, std::vector<Chunk> &chunkSeq)
    {
        std::vector<Chunk> lattice;
        initLattice(chart, lattice);

        chunkSeq.resize(lattice.back()._forwardChunkNum);

        int ix = chunkSeq.size() - 1;
	    for (int end = chart.size(); end > 0; end = lattice[end]._start, --ix) {
            chunkSeq[ix] = lattice[end];
        }
    }

	void dumpLattice(const std::vector<Chunk> &lattice, std::ostream &ost)
	{
		for (unsigned i = 0; i < lattice.size(); ++i) {
			ost << "ch[" << i << "]: ";
			lattice[i].print(ost);
		}
	}
};

} // namespace mogura

#endif // ChunkFinder_h__
