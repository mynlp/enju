/*
 * $Id: eval.cpp,v 1.3 2010-10-01 05:28:45 yusuke Exp $
 */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <list>
#include <set>
#include <iostream>
#include <cfloat>
#include <sstream>
#include "common.h"

using namespace std;

set<string> known_words;

//const bool IGNORE_DISTICTION_BETWEEN_NNP_NN = true;
const bool IGNORE_DISTICTION_BETWEEN_NNP_NN = false;

void evaluate(const vector<Sentence> & vs)
{
  int ncorrect = 0, ntotal = 0;
  int ncorrect_known = 0, ntotal_known = 0;
  int ncorrect_unknown = 0, ntotal_unknown = 0;
  int ncorrect_sen = 0;
  for (vector<Sentence>::const_iterator i = vs.begin(); i != vs.end(); i++) {
    const Sentence & s = *i;
    bool all_correct = true;
    for (unsigned int j = 0; j < s.size(); j++) {
      ntotal++;
      const string pos = s[j].pos;
      const string prd = s[j].prd;
      const string str = s[j].str;
      bool isknown = false;
      if (known_words.find(str) != known_words.end()) isknown = true;
      if (isknown) ntotal_known++;
      else         ntotal_unknown++;

      if (prd == pos) {
	ncorrect++;
	if (isknown) ncorrect_known++;
	else         ncorrect_unknown++;
      } else {
	all_correct = false;
      }
      //      cout << s[j].str << "\t" << s[j].pos << "\t" << s[j].prd << endl;
    }
    if (all_correct) ncorrect_sen++;
  }
  fprintf(stderr, "sentence  %7d / %7d = %f\n", ncorrect_sen, (int)vs.size(), (double)ncorrect_sen / vs.size());
  fprintf(stderr, "token     %7d / %7d = %f\n", ncorrect, ntotal, (double)ncorrect / ntotal);
  if (known_words.size() > 0) {
    fprintf(stderr, "  known   %7d / %7d = %f\n", ncorrect_known, ntotal_known, (double)ncorrect_known / ntotal_known);
    fprintf(stderr, "  unknown %7d / %7d = %f\n", ncorrect_unknown, ntotal_unknown, (double)ncorrect_unknown / ntotal_unknown);
  }
}

void read_pos(const string & filename, vector<Sentence> & vs)
{
  static ParenConverter paren_converter;
 
  ifstream ifile(filename.c_str());

  string line;
  int n = 0;
  cerr << "reading " << filename;
  while (getline(ifile,line)) {
    istringstream is(line);
    string s;
    Sentence sentence;
    while (is >> s) {
      string::size_type i = s.find_last_of('/');
      string str = s.substr(0, i);
      string pos = s.substr(i+1);

      if (IGNORE_DISTICTION_BETWEEN_NNP_NN) {
	if (pos == "NNP") pos = "NN";
	if (pos == "NNPS") pos = "NNS";
      }
      //      string str0 = str;
      str = paren_converter.Ptb2Pos(str);
      //      if (str != str0) cout << str0 << " " << str << endl;
      //      pos = paren_converter.Pos2Ptb(pos);

      //      cout << str << "\t" << pos << endl;
      Token t(str, pos);
      sentence.push_back(t);
    }
    vs.push_back(sentence);
    //if (vs.size() >= num_sentences) break;
    if (n++ % 10000 == 0) cerr << ".";
  }
  cerr << endl;

  ifile.close();
}


int main(int argc, char** argv)
{
  if (argc != 3 && argc != 4) {
    cerr << "Usage: stepp-eval GOLD_STANDARD TAGGED [TRAIN]" << endl;
    exit(1);
  }
  const string gsfile = argv[1];
  const string tgfile = argv[2];
  if (argc == 4) {
    const string trfile = argv[3];
    vector<Sentence> tr;
    read_pos(trfile, tr);
    for (vector<Sentence>::const_iterator i = tr.begin(); i != tr.end(); i++) {
      for (unsigned int j = 0; j < i->size(); j++) {
	known_words.insert((*i)[j].str);
      }
    }
  }

  vector<Sentence> gs, tg;
  read_pos(gsfile, gs);
  read_pos(tgfile, tg);

  for (unsigned int i = 0; i < gs.size(); i++) {
    Sentence & s0 = gs[i];
    const Sentence & s1 = tg[i];
    if (s0.size() != s1.size()) { cerr << "error: sentences do not match at line " << i+1 << endl; exit(1); }
    for (unsigned int j = 0; j < s0.size(); j++) {
      s0[j].prd = s1[j].pos;
    }
  }

  evaluate(gs);

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

