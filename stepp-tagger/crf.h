/*
 * $Id: crf.h,v 1.18 2011-05-02 10:53:53 matuzaki Exp $
 */

#ifndef __CRF_H_
#define __CRF_H_

#include "hashdef.h"
#include "strdic.h"

#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <cassert>

// use this option and change MAX_LABEL_TYPES below accordingly, if you want to use more than 255 labels
//#define USE_MANY_LABELS

//
// data format for each sample for training/testing
//
struct CRF_State
{
public:
  CRF_State() : label("") {};
  CRF_State(const std::string & l) : label(l) {};
  void set_label(const std::string & l) { label = l; }

  // to add a binary feature
  void add_feature(const std::string & f) {
    if (f.find_first_of('\t') != std::string::npos) { std::cerr << "error: illegal characters in a feature string" << std::endl; exit(1); }
    features.push_back(f);   
  }

public:
  std::string label;
  std::vector<std::string> features;
};


struct CRF_Sequence
{
public:
  void add_state(const CRF_State & s) {
    vs.push_back(s);
  }
public:
  std::vector<CRF_State> vs;
};

class CRF_Model
{
 public:
  
  enum OptimizationMethod { BFGS, PERCEPTRON, SGD };

  void add_training_sample(const CRF_Sequence & s);
  int train(OptimizationMethod method, const int cutoff = 0, const double sigma = 0, const double widthfactor = 0);
  //  std::vector<double> classify(CRF_State & s) const;
  void decode_forward_backward(CRF_Sequence & s, std::vector<std::map<std::string, double> > & tagp);
  void decode_viterbi(CRF_Sequence & s);
  void decode_nbest(CRF_Sequence & s0, std::vector<std::pair<double, std::vector<std::string> > > & nbest, const int num, const double min_prob);
  bool load_from_file(const std::string & filename, bool verbose = true);
  bool save_to_file(const std::string & filename, const double t = 0) const;
  int num_classes() const { return _num_classes; }
  std::string get_class_label(int i) const { return _label_bag.Str(i); }
  int get_class_id(const std::string & s) const { return _label_bag.Id(s); }
  void get_features(std::list< std::pair< std::pair<std::string, std::string>, double> > & fl);
  void set_heldout(const int h, const int n = 0) { _nheldout = h; _early_stopping_n = n; };
  //  bool load_from_array(const CRF_Model_Data data[]);

  enum { MAX_LABEL_TYPES = 256 };
  //  const static int MAX_LABEL_TYPES = 1000;
  enum { MAX_LEN = 1000 };

  CRF_Model() {
    _nheldout = 0;
    _early_stopping_n = 0;
    _line_counter = 0;

    p_edge_feature_id  = (int*)malloc(sizeof(int) * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
    p_state_weight     = (double*)malloc(sizeof(double) * MAX_LEN * MAX_LABEL_TYPES);
    p_edge_weight      = (double*)malloc(sizeof(double) * MAX_LABEL_TYPES * MAX_LABEL_TYPES);
    p_forward_cache    = (double*)malloc(sizeof(double) * MAX_LEN * MAX_LABEL_TYPES);
    p_backward_cache   = (double*)malloc(sizeof(double) * MAX_LEN * MAX_LABEL_TYPES);
    p_backward_pointer = (int*)malloc(sizeof(int) * MAX_LEN * MAX_LABEL_TYPES);
  }
  ~CRF_Model() {
    free(p_edge_feature_id);
    free(p_state_weight);
    free(p_edge_weight);
    free(p_forward_cache);
    free(p_backward_cache);
    free(p_backward_pointer);
  }

  void incr_line_counter() { _line_counter++; }
      
private:  
  
  struct Sample {
    int label;
    std::vector<int> positive_features;
  };
  struct Sequence {
    std::vector<Sample> vs;
  };

#ifdef USE_MANY_LABELS
  typedef unsigned long mefeature_type;

  struct ME_Feature
  {
    ME_Feature(const int l, const int f) : _label(l), _feature(f) {};
    int label()   const { return _label; }
    int feature() const { return _feature; }
    //    mefeature_type body() const { return (((long)_feature) << 32) + _label; }
    mefeature_type body() const { 
      unsigned long a = _feature;
      return (a << 32) + _label;
    }
  private:
    int _label;
    int _feature;
  };
#else
  typedef unsigned int mefeature_type;

  struct ME_Feature
  {
    ME_Feature(const int l, const int f) : _body((f << 8) + l) {
      assert(l >= 0 && l <= MAX_LABEL_TYPES);
      assert(f >= 0 && f <= 0xffffff);
    };
    int label()   const { return _body & 0xff; }
    int feature() const { return _body >> 8; }
    mefeature_type body() const { return _body; }
  private:
    mefeature_type _body;
  };
#endif

  struct ME_FeatureBag
  {
    typedef _HASHMAP<mefeature_type, int> map_type;
    map_type mef2id;
    std::vector<ME_Feature> id2mef;
    int Put(const ME_Feature & i) {
      map_type::const_iterator j = mef2id.find(i.body());
      if (j == mef2id.end()) {
        const int id = id2mef.size();
        id2mef.push_back(i);
        mef2id[i.body()] = id;
        return id;
      }
      return j->second;
    }
    int Id(const ME_Feature & i) const {
      map_type::const_iterator j = mef2id.find(i.body());
      if (j == mef2id.end()) {
        return -1;
      }
      return j->second;
    }
    ME_Feature Feature(int id) const {
      assert(id >= 0 && id < (int)id2mef.size());
      return id2mef[id];
    }
    int Size() const {
      return id2mef.size();
    }
    void Clear() {
      mef2id.clear();
      id2mef.clear();
    }
  };

  struct MiniStringBag
  {
    typedef _HASHMAP<std::string, int> map_type;
    int _size;
    map_type str2id;
    MiniStringBag() : _size(0) {}
    int Put(const std::string & i) {
      map_type::const_iterator j = str2id.find(i);
      if (j == str2id.end()) {
        int id = _size;
        _size++;
        str2id[i] = id;
        return id;
      }
      return j->second;
    }
    int Id(const std::string & i) const {
      map_type::const_iterator j = str2id.find(i);
      if (j == str2id.end())  return -1;
      return j->second;
    }
    int Size() const { return _size; }
    void Clear() { str2id.clear(); _size = 0; }
    map_type::const_iterator begin() const { return str2id.begin(); }
    map_type::const_iterator end()   const { return str2id.end(); }
  };

  struct StringBag : public MiniStringBag
  {
    std::vector<std::string> id2str;
    int Put(const std::string & i) {
      map_type::const_iterator j = str2id.find(i);
      if (j == str2id.end()) {
        int id = id2str.size();
        id2str.push_back(i);
        str2id[i] = id;
        return id;
      }
      return j->second;
    }
    std::string Str(const int id) const {
      assert(id >= 0 && id < (int)id2str.size());
      return id2str[id];
    }
    int Size() const { return id2str.size(); }
    void Clear() {
      str2id.clear();
      id2str.clear();
    }
  };

  struct Path
  {
    double score;
    double new_score;
    std::vector<int> vs;
    Path(const double s, const std::vector<int> & v) : score(s), vs(v) {};
    bool operator<(const Path & p) const {
      return score > p.score;
    }
    std::string str() const {
      char buf[100];
      std::sprintf(buf, "%f\t", score);
      std::string s(buf);
      for (std::vector<int>::const_iterator i = vs.begin(); i != vs.end(); i++) {
        char buf[100];
        std::sprintf(buf, "%d ", *i);
        s += std::string(buf);
      }
      return s;
    }
  };

  std::vector<Sequence> _vs; // vector of training_samples
  StringBag _label_bag;
  //  MiniStringBag _featurename_bag;
  StrDic _featurename_bag;
  double _sigma; // Gaussian prior
  double _inequality_width;
  std::vector<double> _vl;  // vector of lambda
  ME_FeatureBag _fb;
  int _num_classes;
  std::vector<double> _vee;  // empirical expectation
  std::vector<double> _vme;  // model expectation
  std::vector< std::vector< int > > _feature2mef;
  std::vector< Sequence > _heldout;
  double _train_error;   // current error rate on the training data
  double _heldout_error; // current error rate on the heldout data
  int _nheldout;
  int _early_stopping_n;
  std::vector<double> _vhlogl;

  double heldout_likelihood();
  double forward_backward(const Sequence & s);
  double viterbi(const Sequence & seq, std::vector<int> & best_seq);
  void initialize_edge_weights();
  void initialize_state_weights(const Sequence & seq);
  int make_feature_bag(const int cutoff);
  //  int classify(const Sample & nbs, std::vector<double> & membp) const;
  double update_model_expectation();
  double add_sample_model_expectation(const Sequence & seq, std::vector<double>& vme, int & ncorrect);
  void add_sample_empirical_expectation(const Sequence & seq, std::vector<double>& vee);
  int perform_BFGS();
  int perform_AveragedPerceptron();
  int perform_StochasticGradientDescent();
  void init_feature2mef();
  double calc_loglikelihood(const Sequence & seq);
  //  std::vector<double> calc_state_weight(const Sequence & seq, const int i) const;
  std::vector<double> calc_state_weight(const int i) const;
  void nbest_search(const double lb, const int len, const int x, const int y, const double rhs_score, std::vector<Path> & vp);
  double nbest(const Sequence & seq, std::vector<Path> & sequences, const int max_num, const double min_prob);

  double FunctionGradient(const std::vector<double> & x, std::vector<double> & grad);
  static double FunctionGradientWrapper(const std::vector<double> & x, std::vector<double> & grad);

  int _line_counter; // for error message. Incremented at forward_backward

  int nbest_search_path[CRF_Model::MAX_LEN];
  /*
  static int edge_feature_id[CRF_Model::MAX_LABEL_TYPES][CRF_Model::MAX_LABEL_TYPES];
  static double state_weight[MAX_LEN][CRF_Model::MAX_LABEL_TYPES];
  static double edge_weight[CRF_Model::MAX_LABEL_TYPES][CRF_Model::MAX_LABEL_TYPES];
  static double forward_cache[MAX_LEN][CRF_Model::MAX_LABEL_TYPES];
  static double backward_cache[MAX_LEN][CRF_Model::MAX_LABEL_TYPES];
  static int backward_pointer[MAX_LEN][CRF_Model::MAX_LABEL_TYPES];
  */
  int *p_edge_feature_id;
  double *p_state_weight;
  double *p_edge_weight;
  double *p_forward_cache;
  double *p_backward_cache;
  int *p_backward_pointer;

  int & edge_feature_id(const int l, const int r) const
    { assert(l >= 0 && l < MAX_LABEL_TYPES);
      assert(r >= 0 && r < MAX_LABEL_TYPES);
      return p_edge_feature_id[l * MAX_LABEL_TYPES + r]; } 
  double & state_weight(const int x, const int l) const
    { return p_state_weight[x * MAX_LABEL_TYPES + l]; }
  double & edge_weight(const int l, const int r) const
    { return p_edge_weight[l * MAX_LABEL_TYPES + r]; }
  double & forward_cache(const int x, const int l) const
    { return p_forward_cache[x * MAX_LABEL_TYPES + l]; }
  double & backward_cache(const int x, const int l) const
    { return p_backward_cache[x * MAX_LABEL_TYPES + l]; }
  int & backward_pointer(const int x, const int l) const
    { return p_backward_pointer[x * MAX_LABEL_TYPES + l]; }


  double forward_prob(const int len);
  double backward_prob(const int len);

};


#endif


/*
 * $Log: not supported by cvs2svn $
 * Revision 1.16  2010/10/01 05:28:45  yusuke
 *
 * support for gcc 4.4
 *
 * Revision 1.15  2009/12/03 23:35:47  matuzaki
 *
 * "-e" option for enju's input
 *
 * Revision 1.14  2008/12/05 16:11:31  tsuruoka
 * move temporary arrays to heap
 *
 * Revision 1.13  2008/12/04 17:45:00  tsuruoka
 * add USE_MANY_LABELS
 *
 * Revision 1.12  2008/10/19 22:50:45  tsuruoka
 * use subset empricial expectation
 * add pseudogradient, l1-ball projection
 *
 * Revision 1.11  2008/08/14 23:19:04  tsuruoka
 * add stochastic gradient descent
 *
 * Revision 1.10  2008/04/12 17:55:50  tsuruoka
 * crf bugfix
 *
 * Revision 1.9  2007/08/13 17:30:43  tsuruoka
 * add averaged_perceptron and single best MIRA
 *
 * Revision 1.8  2007/08/07 06:44:29  hillbig
 * change overflow process
 *
 * Revision 1.7  2007/07/11 19:20:31  tsuruoka
 * replace calc_likelihood() with calc_loglikelihood()
 *
 * Revision 1.6  2007/07/05 13:21:04  tsuruoka
 * add StrDic
 *
 * Revision 1.5  2007/06/18 13:04:34  tsuruoka
 * remove blmvm.h
 *
 * Revision 1.4  2007/06/18 09:09:57  tsuruoka
 * replace BLMVM with LBFGS and OWLQN
 *
 * Revision 1.3  2007/06/07 14:40:34  tsuruoka
 * add add_state()
 *
 * Revision 1.2  2007/06/07 14:27:18  tsuruoka
 * add nbest output for CRF
 *
 * Revision 1.1.1.1  2007/05/15 08:30:35  kyoshida
 * stepp tagger, by Okanohara and Tsuruoka
 *
 */
