#ifndef MultVpModel_h__
#define MultVpModel_h__

#include <iterator>
#include <memory>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>
#include <cmath>
#include <ctime>

#include "mayzutil.h" /// for profiling

#include "SymbolTable.h"
#include "PkiRerankVp.h"

//------------------------------------------------------------------------------
namespace mogura {
//------------------------------------------------------------------------------

typedef HashStringTable FeatureDict;

struct Dict {
    SymbolTable<std::string> _labels;
    FeatureDict _features;
};

class Linear {
public:
    enum { Degree = 1 };
    double poly(double x) const { return x; }
};

class Poly2nd {
public:
    enum { Degree = 2 };
    double poly(double x) const { return x * x; }
};

class Poly3rd {
public:
    enum { Degree = 3 };
    double poly(double x) const { return x * x * x; }
};

/// Pki version
class VpTrainer {
private:
    typedef PkiRerankingVotedPerceptron Vp;
    typedef Vp::Example ExampleType;

public:
    void addTrainingSample(
        const std::string &key,
        const std::vector<std::string> &labels,
        const std::vector<std::string> &features
    ) {
        unsigned keyId = _keys.getID(key);

        if (_samples.size() <= keyId) {
            _samples.resize(keyId + 1);
        }

        _samples[keyId].push_back(ExampleType());
        ExampleType &e = _samples[keyId].back();
        e._labels = labels;
        e._features = features;
    }

    template<class KernelT>
    void train(int nLoop, std::ostream &out, std::ostream &log)
    {
        time_t totalBegin = std::time(0);

        /// header
        out << KernelT::Degree << std::endl;

        for (unsigned i = 0; i < _keys.getNumSymbol(); ++i) {
            
            Vp vp(nLoop);

            log << "BEGIN: " << _keys.getSymbol(i) << std::endl;
            time_t timeBegin = std::time(0);

            vp.train<KernelT::Degree>(_samples[i].begin(), _samples[i].end(), log);

            time_t timeEnd = std::time(0);
            log << "END: " << _keys.getSymbol(i) << '(' << (timeEnd - timeBegin) << " sec)" << std::endl;

            std::vector<Vp::SupportVector> svs;
            vp.getModel(svs);
            writeModel(out, _keys.getSymbol(i), svs);
        }

        time_t totalEnd = std::time(0);
        log << "Total " << (totalEnd - totalBegin) << " sec" << std::endl;
    }

private:
    void writeModel(std::ostream &out, const std::string &key, const std::vector<Vp::SupportVector> &svs)
    {
        for (std::vector<Vp::SupportVector>::const_iterator sv = svs.begin(); sv != svs.end(); ++sv) {
            out << key << ' ' << sv->_weight << ' ' << sv->_label;
            for (std::vector<std::string>::const_iterator f = sv->_features.begin(); f != sv->_features.end(); ++f) {
                out << ' ' << *f;
            }
            out << std::endl;
        }
    }

private:
    SymbolTable<std::string> _keys;
    std::vector<std::list<ExampleType> > _samples;
};

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////

struct Sv {
    int _label;
    double _weight;
    std::vector<int> _fv;
};

/// for debug
inline
void printFv(const std::string &pos, const std::vector<std::string> &fv)
{
    static int n = 0;
    std::cerr << "(" << n++ << ") " << pos << " ";
    std::copy(fv.begin(), fv.end(), std::ostream_iterator<std::string>(std::cerr, " "));
    std::cerr << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// Base class of element models
////////////////////////////////////////////////////////////////////////////////
class VpModelBase {
public:
    virtual ~VpModelBase(void) {}

    int classify(
        const std::vector<bool> &candidates,
        std::vector<unsigned> &fv /// non-const for in-place sort
    ) const {

        std::vector<double> scores(candidates.size(), 0);
        getScore(fv, scores);

        return findMaxLabel(candidates, scores);
    }

    /// score must have sufficient size and be initialized (to zeros).
    virtual void getScore(
        std::vector<unsigned> &fv,
        std::vector<double> &score /// non-const for in-place sort
    ) const = 0;

    virtual void addSv(const Sv &sv) = 0;
    virtual void endSv(void) = 0;

private:
    int findMaxLabel(
        const std::vector<bool> &candidates,
        const std::vector<double> &scores
    ) const {

        double maxScore = 0;
        int maxLabel = -1;

        for (unsigned i = 0; i < candidates.size(); ++i) {

            if (! candidates[i]) {
                continue;
            }

            if (maxLabel == -1 || scores[i] > maxScore) {
                maxLabel = i;
                maxScore = scores[i];
            }
        }

        return maxLabel;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Specialized for linear kernel
////////////////////////////////////////////////////////////////////////////////
class LinearVpModel : public VpModelBase {
private:

    struct LabelScore {
        LabelScore(unsigned label, double score)
            : _label(label)
            , _score(score)
        {}

        LabelScore(void) {}

        unsigned _label;
        double _score;
    };

public:

    ~LinearVpModel(void) {}

    void addSv(const Sv &sv)
    {
        _svs.push_back(sv);
    }

    void endSv(void)
    {
        if (_svs.empty()) {
            return;
        }

        std::vector<std::map<unsigned, double> > sum;
        std::map<unsigned, double> bias;

        for (std::vector<Sv>::const_iterator it = _svs.begin(); it != _svs.end(); ++it) {
            for (std::vector<int>::const_iterator f = it->_fv.begin(); f != it->_fv.end(); ++f) {

                if (sum.size() <= (unsigned) *f) {
                    sum.resize(*f + 1);
                }

                sum[*f][it->_label] += it->_weight;
            }

            bias[it->_label] += it->_weight;
        }

        _score.clear();
        _score.resize(sum.size());

        for (unsigned f = 0; f < sum.size(); ++f) {
            for (std::map<unsigned, double>::const_iterator it = sum[f].begin(); it != sum[f].end(); ++it) {
                _score[f].push_back(LabelScore(it->first, it->second));
            }
        }

        _bias.clear();
        for (std::map<unsigned, double>::const_iterator it = bias.begin(); it != bias.end(); ++it) {
            _bias.push_back(LabelScore(it->first, it->second));
        }

        _svs.clear();
    }

    /// score must have sufficient size and be initialized.
    void getScore(
        std::vector<unsigned> &fv,
        std::vector<double> &score
    ) const {

        for (std::vector<unsigned>::const_iterator f = fv.begin(); f != fv.end(); ++f) {

            const std::vector<LabelScore> &s = _score[*f];
            for (std::vector<LabelScore>::const_iterator it = s.begin(); it != s.end(); ++it) {
                score[it->_label] += it->_score;
            }
        }

        for (std::vector<LabelScore>::const_iterator b = _bias.begin(); b != _bias.end(); ++b) {
            score[b->_label] += b->_score;
        }
    }

private:
    std::vector<Sv> _svs;
    std::vector<std::vector<LabelScore> > _score;
    std::vector<LabelScore> _bias;
};

////////////////////////////////////////////////////////////////////////////////
/// PKI version
////////////////////////////////////////////////////////////////////////////////
template<class KernelT>
class PkiVpModel : public VpModelBase {
public:

    void addSv(const Sv &sv)
    {
        _svs.push_back(sv);
    }

    void endSv(void)
    {
        if (_svs.empty()) {
            return;
        }

        unsigned tableSize = 0;
        std::map<int, std::vector<int> > m;
        for (unsigned i = 0; i < _svs.size(); ++i) {
            const std::vector<int> &fv = _svs[i]._fv;
            tableSize += fv.size();
            for (std::vector<int>::const_iterator f = fv.begin(); f != fv.end(); ++f) {
                m[*f].push_back(i);
            }
        }

        tableSize += m.size();

        _psum.clear();
        _psum.resize(tableSize);

        _index.clear();
        _index.resize(m.size(), 0);

        std::vector<int>::iterator psumItr = _psum.begin();

        for (std::map<int, std::vector<int> >::const_iterator it = m.begin(); it != m.end(); ++it) {

            _index[it->first] = &(*psumItr);
            std::copy(it->second.begin(), it->second.end(), psumItr);

            psumItr += it->second.size();
            *psumItr = -1; /// end marker

            ++psumItr;
        }
    }

    /// score must have sufficient size and be initialized.
    void getScore(std::vector<unsigned> &fv, std::vector<double> &scores) const
    {
        std::vector<unsigned> sum(_svs.size(), 0);

        for (std::vector<unsigned>::const_iterator f = fv.begin(); f != fv.end(); ++f) {
            for (int *p = _index[*f]; *p != -1; ++p) {
                ++sum[*p];
            }
        }

        for (unsigned i = 0; i < sum.size(); ++i) {
            scores[_svs[i]._label] += _svs[i]._weight * _kernel.poly(sum[i] + 1);
        }
    }

private:
    std::vector<Sv> _svs;
    KernelT _kernel;

    std::vector<int> _psum;
    std::vector<int*> _index;
};


////////////////////////////////////////////////////////////////////////////////
/// Aggregation
////////////////////////////////////////////////////////////////////////////////
class MultVpModel {
private:
    typedef VpModelBase ElemModelT;

    struct ModelData {
        FeatureDict *_features;
        ElemModelT *_model;

        ModelData(void) : _features(0), _model(0) {}
        ModelData(FeatureDict *features, ElemModelT *model)
            : _features(features)
            , _model(model)
        {}

        void clear(void)
        {
            delete _features;
            delete _model;
        }
    };

    typedef std::map<std::string, ModelData> ModelSet;

public:

    MultVpModel(void) {}

    ~MultVpModel(void)
    {
        for (ModelSet::iterator i = _models.begin(); i != _models.end(); ++i) {
            i->second.clear();
        }
    }

    VpModelBase *makeModel(unsigned order)
    {
        switch (order) {
            case 1: return new LinearVpModel();
            case 2: return new PkiVpModel<Poly2nd>();
            case 3: return new PkiVpModel<Poly3rd>();
            default: assert(false); return 0;
        }
    }

    void load(std::istream &ist)
    {
        std::string header;
        if (! std::getline(ist, header)) {
            throw std::runtime_error("vp model file format error: no header");
        }

        unsigned order = 0;
        std::istringstream issHeader(header);
        if (! (issHeader >> order)) {
            throw std::runtime_error("vp model file header format error");
        }

        if (order < 1 || 3 < order) {
            std::ostringstream msg;
            msg << "vp model: polynomial order " << order << " is not supported";
            throw std::runtime_error(msg.str());
        }

        std::string line;
        while (std::getline(ist, line)) {
            std::istringstream iss(line);
            std::string key;
            if (! (iss >> key)) {
                throw std::runtime_error("vp model file format error: empty line");
            }

            ModelData &m = _models[key];
            if (m._model == 0) {
                assert(m._features == 0);
                m._model = makeModel(order);
                m._features = new FeatureDict();
            }

            Sv sv;
            if (! readSv(iss, *m._features, sv)) {
                throw std::runtime_error("vp model file format error\n invalid line: " + line);
            }

            m._model->addSv(sv);
        }

        for (std::map<std::string, ModelData>::iterator it = _models.begin(); it != _models.end(); ++it) {
            it->second._model->endSv();
        }
    }

    unsigned getNumClasses(void) const { return _labels.getNumSymbol(); }

    std::string getClassLabel(unsigned ix) const
    {
        return _labels.getSymbol(ix);
    }

    int getClassId(const std::string &name) const
    {
        return _labels.getID(name, -1);
    }

    int classify(
        const std::string &key,
        const std::vector<std::string> &fv,
        const std::vector<bool> &candidates
    ) const {
        up::ScopeProf p("mogura::MultVpModel::classify");

        ModelSet::const_iterator m = _models.find(key);
        if (m == _models.end()) {
            return -1;
        }

        const FeatureDict &features = *(m->second._features);
        const ElemModelT &model = *(m->second._model);

        std::vector<unsigned> ifv;
        translateFv(fv, features, ifv);

        return model.classify(candidates, ifv);
    }

    void getScore(
        const std::string &key,
        const std::vector<std::string> &fv,
        std::vector<double> &scores
    ) const {
        up::ScopeProf p("mogura::MultVpModel::getScore");

        ModelSet::const_iterator m = _models.find(key);
        if (m == _models.end()) {
            scores.clear();
            scores.resize(getNumClasses(), -std::numeric_limits<double>::infinity());
            return;
        }

        const FeatureDict &features = *(m->second._features);
        const ElemModelT &model = *(m->second._model);

        std::vector<unsigned> ifv;
        translateFv(fv, features, ifv);

        scores.clear();
        scores.resize(getNumClasses(), 0);

        model.getScore(ifv, scores);

        /// elem model might have resized the score vector
        scores.resize(getNumClasses(), 0);
    }

private:

    bool readSv(
        std::istream &ist,
        FeatureDict &features,
        Sv &sv
    ) {
        double weight;
        std::string label;

        if (! (ist >> weight >> label)) {
            return false;
        }

        sv._weight = weight;
        sv._label = _labels.getID(label);

        sv._fv.clear();

        std::string f;
        while (ist >> f) {
            sv._fv.push_back(features.getID(f));
        }

        return true;
    }

    void translateFv(
        const std::vector<std::string> &fv,
        const FeatureDict &dict,
        std::vector<unsigned> &ifv
    ) const {
        up::ScopeProf p("mogura::MultVpModel::translateFv");

        for (std::vector<std::string>::const_iterator f = fv.begin(); f != fv.end(); ++f) {
            int ix = dict.getID(*f, -1);
            if (ix >= 0) {
                ifv.push_back(ix);
            }
        }
    }

private:
    SymbolTable<std::string> _labels;
    ModelSet _models;
};

//------------------------------------------------------------------------------
} /// namespace mogura
//------------------------------------------------------------------------------

#endif // MultVpModel_h__
