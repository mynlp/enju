/*
 * $Id: eftag.cpp,v 1.3 2011-05-02 10:53:53 matuzaki Exp $
 */

#include "common.h"
#include "hashdef.h"
#include "maxent.h"

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <set>

using namespace std;

extern multimap<string, string> WNdic;

const int BEAM_NUM = 100;
const double BEAM_WINDOW = 0.01;

const int M0_REDUCTION_FACTOR = 1;

const bool OUTPUT_MULTIPLE_CANDIDATES = true;
//const bool OUTPUT_MULTIPLE_CANDIDATES = false;

// training
const int CUTOFF = 1;
//const double WIDTHFACTOR = 0.7;
const double WIDTHFACTOR = 1.0;
//const double GAUSSIAN_PRIOR = 650;
//const double GAUSSIAN_PRIOR = 300;

//extern string normalize(const string & s);
//void tokenize(const string & s1, list<string> & lt);
string base_form(const string & s, const string & pos);

extern int push_stop_watch();

static string
normalize(const string & s)
{
  string tmp(s);
  for (size_t i = 0; i < tmp.size(); i++) {
    tmp[i] = tolower(tmp[i]);
    if (isdigit(tmp[i])) tmp[i] = '#';
  }
  //if (tmp[tmp.size()-1] == 's') tmp = tmp.substr(0, tmp.size()-1);
  return tmp;
}


static ME_Sample
mesample(const vector<Token> &vt, int i,
         const string & pos_left2, const string & pos_left1, 
         const string & pos_right1, const string & pos_right2)
{
  ME_Sample sample;

  string str = vt[i].str;
  //  string str = normalize(vt[i].str);

  sample.label = vt[i].pos;

  sample.add_feature("W0_" + str);

  sample.add_feature("NW0_" + normalize(str));

  string prestr = "BOS";
  if (i > 0) prestr = vt[i-1].str;
  string prestr2 = "BOS";
  if (i > 1) prestr2 = vt[i-2].str;
  string poststr = "EOS";
  if (i < (int)vt.size()-1) poststr = vt[i+1].str;
  string poststr2 = "EOS";
  if (i < (int)vt.size()-2) poststr2 = vt[i+2].str;

  sample.add_feature("W-1_" + prestr);
  sample.add_feature("W+1_" + poststr);

  //  sample.add_feature("W-2_" + prestr);
  //  sample.add_feature("W+2_" + poststr);

  sample.add_feature("W-10_" + prestr + "_" + str);
  sample.add_feature("W0+1_" + str  + "_" + poststr);

  sample.add_feature("W-1+1_" + prestr  + "_" + poststr);
  //  sample.add_feature("W-2-1_" + prestr2  + "_" + prestr);
  //  sample.add_feature("W+1+2_" + poststr  + "_" + poststr2);

  // train = 10000 no effect
  //  if (i > 0 && prestr.size() >= 3)                
  //    sample.add_feature("W-1S_" + prestr.substr(prestr.size()-3));
  //  if (i < (int)vt.size()-1 && poststr.size() >= 3) 
  //    sample.add_feature("W+1S_" + poststr.substr(poststr.size()-3));

  // sentence type
  //  sample.add_feature("ST_" + vt[vt.size()-1].str);

  for (unsigned int j = 1; j <= 10; j++) {
    char buf[1000];
    //    if (str.size() > j+1) {
    if (str.size() >= j) {
      sprintf(buf, "SUF%d_%s", j, str.substr(str.size() - j).c_str());
      sample.add_feature(buf);
    }
    //    if (str.size() > j+1) {
    if (str.size() >= j) {
      sprintf(buf, "PRE%d_%s", j, str.substr(0, j).c_str());
      sample.add_feature(buf);
    }
  }
  
  // L
  if (pos_left1 != "") {
    sample.add_feature("P-1_" + pos_left1);
    sample.add_feature("P-1W0_"  + pos_left1 + "_" + str);
  }
  // L2
  if (pos_left2 != "") {
    sample.add_feature("P-2_" + pos_left2);
  }
  // R
  if (pos_right1 != "") {
    sample.add_feature("P+1_" + pos_right1);
    sample.add_feature("P+1W0_"  + pos_right1 + "_" + str);
  }
  // R2
  if (pos_right2 != "") {
    sample.add_feature("P+2_" + pos_right2);
  }
  // LR
  if (pos_left1 != "" && pos_right1 != "") {
    sample.add_feature("P-1+1_" + pos_left1 + "_" + pos_right1);
    sample.add_feature("P-1W0P+1_"  + pos_left1 + "_" + str + "_" + pos_right1);
  }
  // LL
  if (pos_left1 != "" && pos_left2 != "") {
    sample.add_feature("P-2-1_" + pos_left2 + "_" + pos_left1);
    //    sample.add_feature("P-1W0_"  + pos_left + "_" + str);
  }
  // RR
  if (pos_right1 != "" && pos_right2 != "") {
    sample.add_feature("P+1+2_" + pos_right1 + "_" + pos_right2);
    //    sample.add_feature("P-1W0_"  + pos_left + "_" + str);
  }
  // LLR
  if (pos_left1 != "" && pos_left2 != "" && pos_right1 != "") {
    sample.add_feature("P-2-1+1_" + pos_left2 + "_" + pos_left1 + "_" + pos_right1);
    //    sample.add_feature("P-1W0_"  + pos_left + "_" + str);
  }
  // LRR
  if (pos_left1 != "" && pos_right1 != "" && pos_right2 != "") {
    sample.add_feature("P-1+1+2_" + pos_left1 + "_" + pos_right1 + "_" + pos_right2);
    //    sample.add_feature("P-1W0_"  + pos_left + "_" + str);
  }
  // LLRR
  if (pos_left2 != "" && pos_left1 != "" && pos_right1 != "" && pos_right2 != "") {
    sample.add_feature("P-2-1+1+2_" + pos_left2 + "_" + pos_left1 + "_" + pos_right1 + "_" + pos_right2);
    //    sample.add_feature("P-1W0_"  + pos_left + "_" + str);
  }

  /*
  string wordshape;
  for (int j = 0; j < str.size(); j++) {
    char c = str[j];
    if (islower(c))      wordshape += "a";
    else if (isupper(c)) wordshape += "A";
    else if (isdigit(c)) wordshape += "#";
    //    else if (c == '-')   wordshape += "-";
    else wordshape += c;
  }
  sample.add_feature("WS_" + wordshape);
  */
  //  sample.add_feature(make_pair("LEN", str.size()));
  

  for (unsigned int j = 0; j < str.size(); j++) {
    if (isdigit(str[j])) {
      sample.add_feature("CTN_NUM");
      break;
    }
  }
  for (unsigned int j = 0; j < str.size(); j++) {
    if (isupper(str[j])) {
      sample.add_feature("CTN_UPP");
      break;
    }
  }
  for (unsigned int j = 0; j < str.size(); j++) {
    if (str[j] == '-') {
      sample.add_feature("CTN_HPN");
      break;
    }
  }
  bool allupper = true;
  for (unsigned int j = 0; j < str.size(); j++) {
    if (!isupper(str[j])) {
      allupper = false;
      break;
    }
  }
  if (allupper) sample.add_feature("ALL_UPP");

  if (WNdic.size() > 0) {
    const string n = normalize(str);
#ifndef _MSC_VER
    for (map<string, string>::const_iterator i = WNdic.lower_bound(n); i != WNdic.upper_bound(n); i++) {
#else
    for(multimap<string,string>::const_iterator i = WNdic.lower_bound(n) ; i != WNdic.upper_bound(n); i++) {
#endif
      sample.add_feature("WN_" + i->second);
    }
  }

  /*
  if (isupper(str[0])) {
    bool company = false;
    for (int j = i + 1; j <= i + 3; j++) {
      if (j >= vt.size()) break;
      if (vt[j].str == "Co." || vt[j].str == "Inc." || vt[j].str == "Corp.") {
	company = true; break;
      }
    }
    if (company) sample.add_feature("CRUDE_COMPANY_NAME");
  }
  */

  //  bool alllower = true;
  //  for (int j = 0; j < str.size(); j++) {
  //    if (!islower(str[j])) {
  //      alllower = false;
  //      break;
  //    }
  //  }
  //  if (alllower) sample.add_feature("ALL_LOW");


  //  for (int j = 0; j < vt.size(); j++)
  //    cout << vt[j].str << " ";
  //  cout << endl;
  //  cout << i << endl;
  /*
  cout << sample.label << "\t";
  for (vector<string>::const_iterator j = sample.features.begin(); j != sample.features.end(); j++) {
      cout << *j << " ";
  }
  cout << endl;
  */
  
  return sample;
}


int
eftrain(const vector<Sentence> & vs, const string & model_dir, const bool use_l1)
{
  const int num_heldout = 0;

  int num_tokens = 0;
  for (vector<Sentence>::const_iterator i = vs.begin(); i != vs.end(); i++) {
    num_tokens += i->size();
  }

  //  const double gaussian = 650.0;
  //  const double gaussian = 650.0 / sqrt(912344) * sqrt(num_tokens);
  //const double gaussian = 300;
  //    const double gaussian = 1200;
  //    const double gaussian = 400;
  const double gaussian = 650.0 / sqrt(sqrt(912344.0)) * sqrt(sqrt((double)num_tokens));
  //  const double gaussian = 500;
  //const double gaussian = 650.0 / sqrt(sqrt(sqrt(912344))) * sqrt(sqrt(sqrt(num_tokens)));

  ME_Model m;
  {
    ME_Model m0;
    for (vector<Sentence>::const_iterator i = vs.begin(); i != vs.end(); i++) {
      if (rand() % M0_REDUCTION_FACTOR != 0) continue;
      const Sentence & s = *i;
      for (unsigned int j = 0; j < s.size(); j++) {
	ME_Sample mes = mesample(s, j, "", "", "", "");
	m0.add_training_sample(mes);
      }
    }    
    m0.set_heldout(num_heldout, 0);
    //  m0.train(CUTOFF, 0, WIDTHFACTOR);

    if (use_l1) m0.train(CUTOFF, 0, 1.0);
    else        m0.train(CUTOFF, gaussian);

    //  m0.save_to_file("model.bidir.0");
    //  m0.save_to_file(model_dir + "model.ef.0");
    if (!m0.save_to_file(model_dir + "model.ef.0", 0.001)) {
      cerr << endl;
      cerr << "error: failed to save the model." << endl;
      exit(1);
    }

    int num_sentences = 0;
    cerr << "extracting features...";
    for (vector<Sentence>::const_iterator i = vs.begin(); i != vs.end(); i++, num_sentences++) {
      const Sentence & s = *i;

      // determine the structure
      const int n = s.size();
      vector<double> vp(n);

      for (int j = 0; j < n; j++) {
	ME_Sample mes = mesample(s, j, "", "", "", "");
	vector<double> membp = m0.classify(mes);
	double maxp = membp[m0.get_class_id(mes.label)];
	vp[j] = maxp;
	//      cout << s[j].str << "/" << vp[j] << " ";
      }
      //    cout << endl;

      //    for (int j = 0; j < n; j++) vp[j] = 1.0 / (j+1); // left-to-right

      for (int j = 0; j < n; j++) {
	string pos_left1 = "BOS", pos_left2 = "BOS2";
	if (j >= 1) {
	  if (vp[j-1] >= vp[j]) pos_left1 = s[j-1].pos;
	  else                  pos_left1 = "";
	}
	if (j >= 2) {
	  if (vp[j-2] >= vp[j]) pos_left2 = s[j-2].pos;
	  else                  pos_left2 = "";
	}
	string pos_right1 = "EOS", pos_right2 = "EOS2";
	if (j <= n - 2) {
	  if (vp[j+1] > vp[j]) pos_right1 = s[j+1].pos;
	  else                 pos_right1 = "";
	}
	if (j <= n - 3) {
	  if (vp[j+2] > vp[j]) pos_right2 = s[j+2].pos;
	  else                 pos_right2 = "";
	}
	ME_Sample mes = mesample(s, j, pos_left2, pos_left1, pos_right1, pos_right2);
	m.add_training_sample(mes);
      }
      if (num_sentences % 1000 == 0) cerr << num_sentences << "...";
    }
    cerr << "done" << endl;
  }

  m.set_heldout(num_heldout,0);
  //  m.train(CUTOFF, 0, WIDTHFACTOR);

  if (use_l1) m.train(CUTOFF, 0, 1.0);
  else        m.train(CUTOFF, gaussian);

  //  m.save_to_file(model_dir + "model.ef.1");
  m.save_to_file(model_dir + "model.ef.1", 0.001);

  return 0;
}


struct Hypothesis
{
  vector<Token> vt;
  vector<double> vent;
  vector<int> order;
  vector< vector<pair<string, double> > > vvp;
  double prob;
  bool operator<(const Hypothesis & h) const {
    return prob < h.prob;
  }
  Hypothesis(const vector<Token> & vt_,
	     const vector<ME_Model> & vme)
  {
    prob = 1.0;
    vt = vt_;
    const unsigned int n = vt.size();
    vent.resize(n);
    vvp.resize(n);
    order.resize(n);
    for (size_t i = 0; i < n; i++) {
      vt[i].prd = "";
      ME_Sample mes = mesample(vt, i, "", "", "", "");
      vector<double> vp = vme[0].classify(mes);
      vent[i] = -vp[vme[0].get_class_id(mes.label)];

      //      vent[i] = i;
    }
  }
  void Print()
  {
    for (size_t k = 0; k < vt.size(); k++) {
      cout << vt[k].str << "/";
      if (vt[k].prd == "") cout << "?";
      else cout << vt[k].prd;
      cout << " ";
    }
    cout << endl;
  }
};

struct hashfun_str
{
  size_t operator()(const std::string& s) const {
    assert(sizeof(int) == 4 && sizeof(char) == 1);
    const int* p = reinterpret_cast<const int*>(s.c_str());
    size_t v = 0;
    int n = s.size() / 4;
    for (int i = 0; i < n; i++, p++) {
      //      v ^= *p;
      v ^= *p << (4 * (i % 2)); // note) 0 <= char < 128
    }
    int m = s.size() % 4;
    for (int i = 0; i < m; i++) {
      v ^= s[4 * n + i] << (i * 8);
    }
    return v;
  }
};



void generate_hypotheses(const int order, const Hypothesis & h,
			 const vector<ME_Model> & vme,
			 list<Hypothesis> & vh)
{
  const int n = h.vt.size();
  int pred_position = -1;
  double min_ent = 999999;
  string pred = "";
  int nremain = 0;
  for (int j = 0; j < n; j++) {
    if (h.vt[j].prd != "") continue;
    nremain++;
    double ent = h.vent[j];
    if (ent < min_ent) {
      min_ent = ent;
      pred_position = j;
    }
  }
  assert(pred_position >= 0 && pred_position < n);

  string pos_left1 = "BOS", pos_left2 = "BOS2";
  if (pred_position >= 1) {
    pos_left1 = h.vt[pred_position-1].prd; // maybe a bug??
    //    pos_left1 = h.vt[pred_position-1].pos;
  }
  if (pred_position >= 2) {
    pos_left2 = h.vt[pred_position-2].prd;
    //    pos_left2 = h.vt[pred_position-2].pos;
  }
  string pos_right1 = "EOS", pos_right2 = "EOS2";
  if (pred_position <= n - 2) {
    pos_right1 = h.vt[pred_position+1].prd;
    //pos_right1 = h.vt[pred_position+1].pos;
  }
  if (pred_position <= n - 3) {
    pos_right2 = h.vt[pred_position+2].prd;
    //    pos_right2 = h.vt[pred_position+2].pos;
  }
  vector<double> membp;
  ME_Sample mes = mesample(h.vt, pred_position, pos_left2, pos_left1, pos_right1, pos_right2);
  const ME_Model * mp = &(vme[1]);
  //  if (pos_left2 == "" && pos_left1 == "" && pos_right1 == "" && pos_right2 == "") mp = &(vme[0]);
  membp = mp->classify(mes);
  assert(mes.label != "");
  vector<pair<string, double> > vp;
  double maxp = membp[mp->get_class_id(mes.label)];
  for (int i = 0; i < mp->num_classes(); i++) {
    double p = membp[i];
    if (p > maxp * BEAM_WINDOW)
      vp.push_back(pair<string, double>(mp->get_class_label(i), p));
  }
  
  for (vector<pair<string, double> >::const_iterator k = vp.begin(); k != vp.end(); k++) {
    Hypothesis newh = h;
    
    newh.vt[pred_position].prd = k->first;
    newh.order[pred_position] = order + 1;
    newh.prob = h.prob * k->second;
  
    vh.push_back(newh);
  }
}


void
ef_decode_beam(vector<Token> & vt, const vector<ME_Model> & vme,
	       vector< map<string, double> > & tagp)
{
  const unsigned int n = vt.size();
  if (n == 0) return;

  list<Hypothesis> vh;
  Hypothesis h(vt, vme);
  vh.push_back(h);
  
  for (size_t i = 0; i < n; i++) {
    list<Hypothesis> newvh;
    for (list<Hypothesis>::const_iterator j = vh.begin(); j != vh.end(); j++) {
      if (j->prob == 0) cerr << "warning: hypothesis with zero probability" << endl;
      generate_hypotheses(i, *j, vme, newvh);
    }
    newvh.sort();
    list<Hypothesis>::const_iterator j = newvh.end(); j--;
    double max_prob = j->prob;
    while ((int)newvh.size() > BEAM_NUM || newvh.begin()->prob < max_prob * BEAM_WINDOW) {
    //    while (newvh.size() > BEAM_NUM) {
      newvh.pop_front();
    }
    vh = newvh;
  }

  if (OUTPUT_MULTIPLE_CANDIDATES) {

    for (size_t k = 0; k < n; k++) {
      map<string, double> candidates;
      
      list<Hypothesis>::const_iterator j = vh.end(); j--;
      //double max_prob = j->prob;
      list<Hypothesis>::const_iterator i = vh.end();
      double sum = 0;
      while (1) {
        i--;
	//if (i->prob < max_prob * 5.0 * BEAM_WINDOW) break;
        string prd = i->vt[k].prd;
        map<string, double>::iterator j = candidates.find(prd);
        if (j != candidates.end()) {
          j->second = j->second + i->prob;
        } else {
          candidates[prd] = i->prob;
        }
        sum += i->prob;
        if (i == vh.begin()) break;
      }
      
      map<string, double> tp;
      double maxp = -1;
      string maxtag;
      for (map<string, double>::const_iterator i1 = candidates.begin(); i1 != candidates.end(); i1++) {
	double p = i1->second / sum;
	//	double p = i1->second;
	tp[i1->first] = p;
	if (p > maxp) { maxp = p; maxtag = i1->first; }
      }
      tagp.push_back(tp);
      vt[k].prd = maxtag;
    }
    
    return;
  }
  
  h = vh.back();
  for (size_t k = 0; k < n; k++) {
    //    cout << h.vt[k].str << "/" << h.vt[k].prd << "/" << h.order[k] << " ";
    vt[k].prd = h.vt[k].prd;
  }
  //  cout << endl;


}



/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2010/04/21 08:56:18  tsuruoka
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2007/05/15 08:30:35  kyoshida
 * stepp tagger, by Okanohara and Tsuruoka
 *
 */

