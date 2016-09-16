/*
 * $Id: crfpos.cpp,v 1.9 2011-05-02 10:53:53 matuzaki Exp $
 */

#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <stdio.h>
#include <fstream>
#include <map>
#include <list>
#include <iostream>
#include <sstream>
#include <cmath>
#include <set>
#include "crf.h"
#include "common.h"

using namespace std;

multimap<string, string> WNdic;

//extern string normalize(const string & s);
void tokenize(const string & s1, list<string> & lt);
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

//--------------------------------------------------------------------
// If you want to use stepp as a chunker, use this function instead
// of the original crfstate(). 
// Also, make sure that you use -f option both in training and testing
//--------------------------------------------------------------------
/*
static CRF_State
crfstate(const vector<Token> &vt, int i)
{
  CRF_State sample(vt[i].pos);

  string posm1 = "!BOS!", strm1 = "!BOS!"; // -1: previous position
  string pos0,  str0;                      //  0: current position
  string posp1 = "!EOS!", strp1 = "!EOS!"; // +1: next position

  string::size_type p = vt[i].str.find_last_of('/');
  str0 = vt[i].str.substr(0, p);
  pos0 = vt[i].str.substr(p+1);

  if (i >= 1) {
    string::size_type p = vt[i-1].str.find_last_of('/');
    strm1 = vt[i-1].str.substr(0, p);
    posm1 = vt[i-1].str.substr(p+1);
  }

  if (i < (int)vt.size() - 1) {
    string::size_type p = vt[i+1].str.find_last_of('/');
    strp1 = vt[i+1].str.substr(0, p);
    posp1 = vt[i+1].str.substr(p+1);
  }

  sample.add_feature("W0_" + str0);
  sample.add_feature("P0_" + pos0);

  sample.add_feature("W-1_" + strm1);
  sample.add_feature("P-1_" + posm1);

  sample.add_feature("W+1_" + strp1);
  sample.add_feature("P+1_" + posp1);

  //  cout << str0 << pos0 << endl;
  //  exit(0);

  return sample;
}
*/


static CRF_State
crfstate(const vector<Token> &vt, int i)
{
  CRF_State sample;

  string str = vt[i].str;
  //  string str = normalize(vt[i].str);

  sample.label = vt[i].pos;

  sample.add_feature("W0_" + vt[i].str);

  sample.add_feature("NW0_" + normalize(str));

  string prestr = "BOS";
  if (i > 0) prestr = vt[i-1].str;
  //  if (i > 0) prestr = normalize(vt[i-1].str);

  string prestr2 = "BOS";
  if (i > 1) prestr2 = vt[i-2].str;
  //  if (i > 1) prestr2 = normalize(vt[i-2].str);

  string poststr = "EOS";
  if (i < (int)vt.size()-1) poststr = vt[i+1].str;
  //  if (i < (int)vt.size()-1) poststr = normalize(vt[i+1].str);

  string poststr2 = "EOS";
  if (i < (int)vt.size()-2) poststr2 = vt[i+2].str;
  //  if (i < (int)vt.size()-2) poststr2 = normalize(vt[i+2].str);


  sample.add_feature("W-1_" + prestr);
  sample.add_feature("W+1_" + poststr);

  sample.add_feature("W-2_" + prestr2);
  sample.add_feature("W+2_" + poststr2);

  sample.add_feature("W-10_" + prestr + "_" + str);
  sample.add_feature("W0+1_" + str  + "_" + poststr);
  sample.add_feature("W-1+1_" + prestr  + "_" + poststr);

  //sample.add_feature("W-10+1_" + prestr  + "_" + str + "_" + poststr);

  //  sample.add_feature("W-2-1_" + prestr2  + "_" + prestr);
  //  sample.add_feature("W+1+2_" + poststr  + "_" + poststr2);

  // train = 10000 no effect
  //  if (i > 0 && prestr.size() >= 3)                
  //    sample.add_feature("W-1S_" + prestr.substr(prestr.size()-3));
  //  if (i < (int)vt.size()-1 && poststr.size() >= 3) 
  //    sample.add_feature("W+1S_" + poststr.substr(poststr.size()-3));

  // sentence type
  //  sample.add_feature("ST_" + vt[vt.size()-1].str);

  for (size_t j = 1; j <= 10; j++) {
    char buf[1000];
    //    if (str.size() > j+1) {
    if (str.size() >= j) {
      sprintf(buf, "SUF%d_%s", (int)j, str.substr(str.size() - j).c_str());
      sample.add_feature(buf);
    }
    //    if (str.size() > j+1) {
    if (str.size() >= j) {
      sprintf(buf, "PRE%d_%s", (int)j, str.substr(0, j).c_str());
      sample.add_feature(buf);
    }
  }
  
  for (size_t j = 0; j < str.size(); j++) {
    if (isdigit(str[j])) {
      sample.add_feature("CTN_NUM");
      break;
    }
  }
  for (size_t j = 0; j < str.size(); j++) {
    if (isupper(str[j])) {
      sample.add_feature("CTN_UPP");
      break;
    }
  }
  for (size_t j = 0; j < str.size(); j++) {
    if (str[j] == '-') {
      sample.add_feature("CTN_HPN");
      break;
    }
  }
  bool allupper = true;
  for (size_t j = 0; j < str.size(); j++) {
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
    for (multimap<string, string>::const_iterator i = WNdic.lower_bound(n); i != WNdic.upper_bound(n); i++) {
#endif
      sample.add_feature("WN_" + i->second);
    }
  }
  //  for (int j = 0; j < vt.size(); j++)
  //    cout << vt[j].str << " ";
  //  cout << endl;
  //  cout << i << endl;

  //  cout << sample.label << "\t";
  //  for (vector<string>::const_iterator j = sample.features.begin(); j != sample.features.end(); j++) {
  //      cout << *j << " ";
  //  }
  //  cout << endl;
  
  return sample;
}


int
crftrain(const CRF_Model::OptimizationMethod method,
	 CRF_Model & m, const vector<Sentence> & vs, double gaussian, const bool use_l1)
{
  if (method != CRF_Model::BFGS && use_l1) { cerr << "error: L1 regularization is currently not supported in this mode. Please use other optimziation methods." << endl; exit(1); }

  for (vector<Sentence>::const_iterator i = vs.begin(); i != vs.end(); i++) {
    const Sentence & s = *i;
    CRF_Sequence cs;
    for (size_t j = 0; j < s.size(); j++) cs.add_state(crfstate(s, j));
    m.add_training_sample(cs);
  }    
  //  m.set_heldout(50, 0);

  if (use_l1) m.train(method, 0, 0, 1.0);
  else        m.train(method, 0, gaussian);

  //  m.save_to_file("model.crf");

  return 0;
}

void
crf_decode_forward_backward(Sentence & s, CRF_Model & m, vector< map<string, double> > & tagp)
{
  CRF_Sequence cs;
  for (size_t j = 0; j < s.size(); j++) cs.add_state(crfstate(s, j));

  m.decode_forward_backward(cs, tagp);
  //  m.decode_viterbi(cs);

  for (size_t k = 0; k < s.size(); k++) s[k].prd = cs.vs[k].label;
}

void
crf_decode_nbest(Sentence & s, CRF_Model & m, 
		 vector<pair<double, vector<string> > > & nbest_seqs, int n)
{
  CRF_Sequence cs;
  for (size_t j = 0; j < s.size(); j++) cs.add_state(crfstate(s, j));

  m.decode_nbest(cs, nbest_seqs, n, 0);

  for (size_t k = 0; k < s.size(); k++) s[k].prd = cs.vs[k].label;

}




/*
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2010/04/21 08:56:18  tsuruoka
 * *** empty log message ***
 *
 * Revision 1.7  2008/08/14 23:19:04  tsuruoka
 * add stochastic gradient descent
 *
 * Revision 1.6  2008/04/12 17:55:50  tsuruoka
 * crf bugfix
 *
 * Revision 1.5  2007/09/26 23:03:44  tsuruoka
 * add a function to generate features for chunking
 *
 * Revision 1.4  2007/06/07 20:12:49  tsuruoka
 * add --nbest option
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

