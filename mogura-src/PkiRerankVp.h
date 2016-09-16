#include <vector>
#include <limits>
#include "ProgressBar.h"
#include "SymbolTable.h"

//------------------------------------------------------------------------------
namespace mogura {
//------------------------------------------------------------------------------
    
namespace PkiRvpUtil {
    template<unsigned n>
    inline double pow(double v) { assert(false); return 0; }

    template<> inline double pow<1>(double v) { return v; }
    template<> inline double pow<2>(double v) { return v * v; }
    template<> inline double pow<3>(double v) { return v * v * v; }
    template<> inline double pow<4>(double v) { return v * v * v * v; }
    template<> inline double pow<5>(double v) { return v * v * v * v * v; }
};

class PkiRerankingVotedPerceptron {
private:
    struct ErrorHist {
        int _exampleIx;
        int _negSampleIx;
        int _t;

        ErrorHist(void) {}
        ErrorHist(int eIx, int nIx, int t)
            : _exampleIx(eIx)
            , _negSampleIx(nIx)
            , _t(t) {}

        bool operator<(const ErrorHist &h) const
        {
            if (_exampleIx != h._exampleIx) {
                return _exampleIx < h._exampleIx;
            }

            if (_negSampleIx != h._negSampleIx) {
                return _negSampleIx < h._negSampleIx;
            }

            return _t < h._t;
        }
    };

    struct Alpha {
        int _exampleIx;
        int _candidateIx;
        int _weight;

        Alpha(void) {}
        Alpha(int eIx, int cIx, int w)
            : _exampleIx(eIx)
            , _candidateIx(cIx)
            , _weight(w) {}
    };

 public:
    template<class LabelRepT, class FeatureRepT>
    struct ExampleType {
        /// _labels[0]           : correct label
        /// _labels[ix] (ix > 0) : wrong labels
        std::vector<LabelRepT> _labels;
        std::vector<FeatureRepT> _features;

        void swap(ExampleType &e)
        {
            _labels.swap(e._labels);
            _features.swap(e._features);
        }
    };
 private:

    typedef ExampleType<unsigned, unsigned> IntExample;

    typedef std::vector<std::vector<unsigned> > PkiTable;
    typedef std::vector<std::vector<int> > CoeffTable;

public:
    typedef ExampleType<std::string, std::string> Example;

    struct SupportVector {
        double _weight;
        std::string _label;
        std::vector<std::string> _features;
    };

public:
    PkiRerankingVotedPerceptron(unsigned nLoop)
        : _nLoop(nLoop)
    {}

    template<unsigned PolynomialOrder, class ExampleItrT>
    void train(
        ExampleItrT first,
        ExampleItrT last,
        std::ostream &log
    ) {
        _labels.clear();
        _features.clear();
        loadExamples(first, last);

        /// From std::rundom_shuffle
        for (unsigned i = 0; i < _examples.size(); ++i) {
            _examples[i].swap(_examples[std::rand() % (i + 1)]);
        }

        std::vector<ErrorHist> h;
        trainImpl<PolynomialOrder>(h, log);

        calcAverage(h);
    }

    void getModel(std::vector<SupportVector> &svs)
    {
        svs.clear();
        svs.resize(_alpha.size());

        for (unsigned i = 0; i < _alpha.size(); ++i) {
            const Alpha &a = _alpha[i];
            SupportVector &sv = svs[i];

            const IntExample &e = _examples[a._exampleIx];

            sv._weight = a._weight;
            sv._label = _labels.getSymbol(e._labels[a._candidateIx]);

            for (std::vector<unsigned>::const_iterator f = e._features.begin(); f != e._features.end(); ++f) {
                sv._features.push_back(_features.getSymbol(*f));
            }
        }
    }

    void write(std::ostream &ost) const
    {
        for (std::vector<Alpha>::const_iterator a = _alpha.begin(); a != _alpha.end(); ++a) {

            const IntExample &e = _examples[a->_exampleIx];
            ost << a->_weight << ' ' << _labels.getSymbol(e._labels[a->_candidateIx]);

            for (std::vector<unsigned>::const_iterator f = e._features.begin(); f != e._features.end(); ++f) {
                ost << ' ' << _features.getSymbol(*f);
            }

            ost << std::endl;
        }
    }

private:
    template<class ExampleItrT>
    void loadExamples(
        ExampleItrT first,
        ExampleItrT last
    ) {
        _examples.clear();

        for (ExampleItrT it = first; it != last; ++it) {

            _examples.push_back(IntExample());
            IntExample &e = _examples.back();

            translate(_labels, it->_labels, e._labels);
            translate(_features, it->_features, e._features);
        }
    }

    void translate(
        SymbolTable<std::string> &dict,
        const std::vector<std::string> &ss,
        std::vector<unsigned> &ns
    ) {
        ns.resize(ss.size());
        for (unsigned i = 0; i < ss.size(); ++i) {
            ns[i] = dict.getID(ss[i]);
        }
    }

    template<unsigned PolynomialOrder>
    void trainImpl(std::vector<ErrorHist> &h, std::ostream &log)
    {
        h.clear();

        unsigned t = 1;

        std::vector<unsigned> supportExamples;
        CoeffTable a(_examples.size());
        PkiTable table(_features.getNumSymbol());

        for (unsigned i = 0; i < _nLoop; ++i) {

            ProgressBar pb(_examples.size() / 40, _examples.size(), log);
            unsigned nWrong = 0;

            for (unsigned j = 0; j < _examples.size(); ++j, ++t, ++pb) {

                unsigned ix = eval<PolynomialOrder>(supportExamples, a, table, _examples[j]);

                if (ix != 0) {

                    h.push_back(ErrorHist(j, ix, t));

                    if (a[j].empty()) { /// First wrong answer on this example
                        a[j].resize(_examples[j]._labels.size(), 0);
                        addFv(table, j, _examples[j]._features);
                        supportExamples.push_back(j);
                    }

                    ++a[j][0];
                    --a[j][ix];

                    ++nWrong;
                }
            }

            double errRate = (double) nWrong / _examples.size();
            log << "[ err = " << errRate << " (" << nWrong << "/" << _examples.size() << ") ]" << std::endl;
        }
    }

    void addFv(
        PkiTable &table,
        unsigned exampleIx,
        const std::vector<unsigned> &fv
    ) const {
        for (std::vector<unsigned>::const_iterator f = fv.begin(); f != fv.end(); ++f) {
            table[*f].push_back(exampleIx);
        }
    }

    template<unsigned PolynomialOrder>
    unsigned eval(
        const std::vector<unsigned> &supportExamples,
        const CoeffTable &a,
        const PkiTable &table,
        const IntExample &example
    ) const {

        std::vector<int> esum(_examples.size(), 0);

        const std::vector<unsigned> &fv = example._features;
        for (std::vector<unsigned>::const_iterator f = fv.begin(); f != fv.end(); ++f) {

            const std::vector<unsigned> &es = table[*f];
            for (std::vector<unsigned>::const_iterator e = es.begin(); e != es.end(); ++e) {
                ++esum[*e];
            }
        }

        std::vector<double> score(_labels.getNumSymbol(), 0);
        for (std::vector<unsigned>::const_iterator e = supportExamples.begin(); e != supportExamples.end(); ++e) {

            double kernelValue = PkiRvpUtil::pow<PolynomialOrder>(esum[*e] + 1);
            for (unsigned j = 0; j < a[*e].size(); ++j) {
                score[_examples[*e]._labels[j]] += a[*e][j] * kernelValue;
            }
        }

        double maxScore = score[example._labels[0]];
        unsigned maxIx = 0;
        for (unsigned i = 0; i < example._labels.size(); ++i) {
            double s = score[example._labels[i]];
            if (s >= maxScore) {
                maxIx = i;
                maxScore = s;
            }
        }

        return maxIx;
    }

    void calcAverage(std::vector<ErrorHist> &h)
    {
        #if 0 /// 'Riemann integral' :-)
        std::sort(h.begin(), h.end());
        h.push_back(ErrorHist(-1, -1, -1)); /// sentinel

        /// a[i][j] = -(sum_{t} a_ij,t)
        /// a_ij,t : coefficient of (example[i]._labels[j]; example[i]._features) at time t
        // std::vector<std::vector<int> > a(_examples.size());
        std::map<int, std::map<int, int> > a;

        int lastT = _nLoop * _examples.size();
        ErrorHist curr(-1, -1, 0);
        int currAlpha = 0;

        for (std::vector<ErrorHist>::const_iterator next = h.begin(); next != h.end(); ++next) {

            if (next->_exampleIx != curr._exampleIx || next->_negSampleIx != curr._negSampleIx) {
                /// update for new a_ij
                if (curr._exampleIx != -1) {
                    a[curr._exampleIx][curr._negSampleIx] += currAlpha * (lastT - curr._t);
                }

                currAlpha = 1;
            }
            else if (next->_exampleIx != -1) {
                /// second, third, ... update for the same a_ij
                a[curr->_exampleIx][curr->_negSampleIx] += currAlpha * (next->_t - curr._t);
                currAlpha += 1;
            }

            curr = *next;
        }
        #endif

        /// 'Lebesgue integral' :-)
        int lastT = _nLoop * _examples.size();
        std::map<int, std::map<int, int> > a;
        for (std::vector<ErrorHist>::const_iterator it = h.begin(); it != h.end(); ++it) {
            assert(lastT >= it->_t);
            a[it->_exampleIx][it->_negSampleIx] += (lastT + 1 - it->_t);
        }

        _alpha.clear();
        for (std::map<int, std::map<int, int> >::const_iterator it1 = a.begin(); it1 != a.end(); ++it1) {

            int i = it1->first;
            int negSum = 0;

            for (std::map<int, int>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {

                int j = it2->first;
                int val = it2->second;
                if (val != 0) {
                    _alpha.push_back(Alpha(i, j, -val));
                    negSum += val;
                }
            }

            if (negSum != 0) {
                _alpha.push_back(Alpha(i, 0, negSum));
            }
        }
    }

private:
    unsigned _nLoop;

    std::vector<IntExample> _examples;
    SymbolTable<std::string> _features;
    SymbolTable<std::string> _labels;

    std::vector<Alpha> _alpha;
};

//------------------------------------------------------------------------------
} /// namespace mogura {
//------------------------------------------------------------------------------
