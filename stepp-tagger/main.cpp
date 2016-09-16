/*
 * $Id: main.cpp,v 1.19 2011-05-06 13:01:06 matuzaki Exp $
 */

#include <stdio.h>
#include <fstream>
#include <map>
#include <list>
#include <set>
#include <iomanip>
#include <iostream>
#include <cfloat>
#include <sstream>
#include "maxent.h"
#include "crf.h"
#include "common.h"
#ifndef _MSC_VER
#include <sys/time.h>
#endif
using namespace std;

bool PERFORM_TOKENIZATION = false;
bool OUTPUT_TAG_PROBS = false;
bool CRF_ONLY = false;
bool STANDOFF = false;
bool UIMA     = false;
bool ENJU     = false;
int  NBEST = 0;
const bool CONVERT_SQUARE_BRACKETS = true;

extern multimap<string, string> WNdic;

string MODEL_DIR = "."; // the default directory for saving the models

const double PROB_OUTPUT_THRESHOLD = 0.001; // suppress output of tags with a very low probability

void
tokenize(const string & s, vector<Token> & vt, const bool use_upenn_tokenizer);

void
ef_decode_beam(vector<Token> & vt, const vector<ME_Model> & vme,
	       vector< map<string, double> > & tagp);
void
crf_decode_forward_backward(Sentence & s, CRF_Model & m, vector< map<string, double> > & tagp);
void
crf_decode_nbest(Sentence & s, CRF_Model & m, 
		 vector<pair<double, vector<string> > > & nbest_seqs, int n);

void read_WordNet(const string & wordnetdir);


ParenConverter paren_converter;

/*
int push_stop_watch()
{
  static struct timeval start_time, end_time;
  static bool start = true;
  if (start) {
    gettimeofday(&start_time, NULL);
    start = false;
    return 0;
  }

  gettimeofday(&end_time, NULL);
  int elapse_msec = (end_time.tv_sec - start_time.tv_sec) * 1000 +
    (int)((end_time.tv_usec - start_time.tv_usec) * 0.001);
  cerr << elapse_msec << " msec" << endl;
  start = true;
  return elapse_msec;
}
*/

void load_dic(const string & filename)
{
  ifstream ifile(filename.c_str());

  if (!ifile) return;
  if (!ENJU) {
    cerr << "reading " << filename << endl;
  }

  WNdic.clear();
  string line;
  while (getline(ifile, line)) {
    if (line.size() < 3) continue;
    istringstream is(line);
    string str, pos;
    is >> str >> pos;
    WNdic.insert(make_pair(str, pos));
  }

  ifile.close();

}


void print_help()
{
  cout << "STePP Tagger, a simple trainable part-of-speech tagger." << endl;
  cout << "Usage: stepp [OPTION]... [FILE]" << endl;
  cout << "Annotate each word in FILE with a part-of-speech tag." << endl;
  cout << "By default, the tagger assumes that FILE contains one sentence per line" << endl;
  cout << "and the words are tokenized with white spaces." << endl;
  //  cout << "Try -t and -s options if you want the tagger to process raw text." << endl;
  cout << "Try -t option if you want to process untokenized sentences." << endl;
  cout << endl;
  cout << "Mandatory arguments to long options are mandatory for short options too." << endl;
  cout << "  -m, --model=DIR        specify the directory containing the models" << endl;
  cout << "  -f, --fast             enable the fast tagging mode" << endl;
  cout << "  -t, --tokenize         perform tokenization" << endl;
  //  cout << "  -s, --split-sentence   perform sentence-splitting" << endl;
  cout << "  -p, --probability      output tag probabilities" << endl;
  cout << "  -n, --nbest=N          output N-best tagging sequences" << endl;
  cout << "  -s, --standoff         output in stand-off format" << endl;
  cout << "  -u, --uima             output in UIMA format" << endl;
  cout << "  -e, --enju             output in Enju format" << endl;
  cout << "  -h, --help             display this help and exit" << endl;
  cout << endl;
  cout << "With no FILE, or when FILE is -, read standard input." << endl;
  cout << endl;
  cout << "Report bugs to <yoshimasa.tsuruoka@manchester.ac.uk>" << endl;
  exit(0);
}

void print_version()
{
  cout << "STePP tagger 0.5" << endl;
  exit(0);
}

struct TagProb
{
  string tag;
  double prob;
  TagProb(const string & t_, const double & p_) : tag(t_), prob(p_) {}
  bool operator<(const TagProb & x) const { return prob > x.prob; }
};

int main(int argc, char** argv)
{
  string WORDNET_DIR = "";
  
  string ifilename;

  for (int i = 1; i < argc; i++) {
    string v = argv[i];
    if ( (v == "-m" || v == "--model") && i < argc-1) {
      MODEL_DIR = argv[i+1];
      i++;
      continue;
    }
    //    if ( (v == "-wn" || v == "--wordnet") && i < argc-1) {
    //      WORDNET_DIR = argv[i+1];
    //      i++;
    //      continue;
    //    }
    if (v.substr(0, 8) == "--model=") {
      MODEL_DIR = v.substr(8);
      continue;
    }
    if (v == "-t" || v == "--tokenize") { 
      PERFORM_TOKENIZATION = true; 
      continue; 
    }
    if (v == "-s" || v == "--standoff") { 
      STANDOFF = true;
      continue; 
    }
    if (v == "-u" || v == "--uima") {
      UIMA = true;
      continue;
    }
    if (v == "-e" || v == "--enju") {
      ENJU = true;
      continue;
    }
    if (v == "-p" || v == "--probability") { 
      OUTPUT_TAG_PROBS = true;
      continue; 
    }
    if (v == "-f" || v == "--fast") { 
      CRF_ONLY = true;
      continue; 
    }
    if ( (v == "-n" || v == "--nbest") && i < argc-1) {
      NBEST = atoi(argv[i+1]);
      i++;
      continue;
    }
    if (v.substr(0, 8) == "--nbest=") {
      NBEST = atoi(v.substr(8).c_str());
      continue;
    }
    if (v == "-") {
      ifilename = "";
      continue;
    }
    if (v == "-h" || v == "--help")  print_help();
    if (v == "--version")            print_version();

    if (v[0] == '-') {
      cerr << "error: unknown option " << v << endl;
      cerr << "Try `stepp --help' for more information." << endl;
      exit(1);
    }
    ifilename = v;
  }

  if (NBEST) {
    if (STANDOFF) {
      cerr << "error: n-best output is currently not supported in stand-off format." << endl;
      exit(1);
    }
    if (!CRF_ONLY) { 
      cerr << "error: n-best output is currently supported only in the fast mode. Please use -f option, too." << endl; exit(1); 
    }
  }

  istream *is(&std::cin);
  ifstream ifile;
  if (ifilename != "") {
    ifile.open(ifilename.c_str());
    if (!ifile) { cerr << "error: cannot open " << ifilename << endl; exit(1); }
    is = &ifile;
  }

  if (MODEL_DIR[MODEL_DIR.size()-1] != '/')  MODEL_DIR += "/";

  //  if (WORDNET_DIR != "") {
  //    if (WORDNET_DIR[WORDNET_DIR.size()-1] != '/') WORDNET_DIR += "/";
  //    read_WordNet(WORDNET_DIR);
  //  }
  load_dic(MODEL_DIR + "dictionary.txt");

  vector<ME_Model> vme(2);
  CRF_Model crfm;

  if (!ENJU) {
    cerr << "loading the models from the directory \"" << MODEL_DIR << "\" ...";
  }
  if (!crfm.load_from_file(MODEL_DIR + "model.crf", ENJU ? false : true)) exit(1);
  if (!CRF_ONLY) {
    if (!vme[0].load_from_file(MODEL_DIR + "model.ef.0")) exit(1);
    if (!vme[1].load_from_file(MODEL_DIR + "model.ef.1")) exit(1);
  }
  if (!ENJU) {
    cerr << "done" << endl;
  }

  //  crfm.save_to_file("test");

  //  push_stop_watch();

  string line;
  int nlines = 0;
  while (getline(*is, line)) {
    nlines++;
    vector<Token> vt;
    tokenize(line, vt, PERFORM_TOKENIZATION);

    if (vt.size() > 990) {
      cerr << "warning: the sentence is too long. it has been truncated." << endl;
      while (vt.size() > 990) vt.pop_back();
    }

    // convert parantheses
    vector<string> org_strs;
    for (vector<Token>::iterator i = vt.begin(); i != vt.end(); i++) {
      org_strs.push_back(i->str);
      i->str = paren_converter.Ptb2Pos(i->str);
      if (CONVERT_SQUARE_BRACKETS) {
	if (i->str == "[") i->str = "{";
	if (i->str == "]") i->str = "}";
      }
      i->prd = "?";
    }

    if (STANDOFF) cout << line << endl;
    if (vt.size() == 0) { cout << endl; continue; }

    if (NBEST) {
      vector<pair<double, vector<string> > > nbest_seqs;
      crf_decode_nbest(vt, crfm, nbest_seqs, NBEST);
      for (vector<pair<double, vector<string> > >::const_iterator i = nbest_seqs.begin(); i != nbest_seqs.end(); i++) {
	cout << i->first << "\t";
	int k = 0;
	for (vector<string>::const_iterator j = i->second.begin(); j != i->second.end(); j++, k++) {
	  const string s = org_strs[k];
	  cout << s << "/" << *j;
	  if (k != org_strs.size() - 1) cout << " ";
	}
	cout << endl;
      }
      cout << endl;
      crfm.incr_line_counter();
      continue;
    }

    // tag the words
    vector< map<string, double> > tagp0, tagp1;
    crf_decode_forward_backward(vt, crfm, tagp0);
    if (!CRF_ONLY) {
      ef_decode_beam(vt, vme, tagp1);
    } else {
      for (vector<Token>::const_iterator i = vt.begin(); i != vt.end(); i++) {
	map<string, double> dummy;
	tagp1.push_back(dummy);
      }
    }

    // merge the outputs (simple interpolation of probabilities)
    vector< map<string, double> > tagp; // merged
    //if(tagp0.size() > 0) {
    for (size_t i = 0; i < vt.size(); i++) {
      const map<string, double> & crf = tagp0[i];
      const map<string, double> & ef  = tagp1[i];
      map<string, double> m, m2; // merged

      double sum = 0;
      for (map<string, double>::const_iterator j = crf.begin(); j != crf.end(); j++) {
	//	cout << j->first << ":" << j->second << " ";
	m.insert(pair<string, double>(j->first, j->second));
	sum += j->second;
      }
      //      cout << endl;

      for (map<string, double>::const_iterator j = ef.begin(); j != ef.end(); j++) {
	//	cout << j->first << ":" << j->second << " ";
	sum += j->second;
	if (m.find(j->first) == m.end()) {
	  m.insert(pair<string, double>(j->first, j->second));
	} else {
	  m[j->first] += j->second;
	}
      }
      //      cout << endl;

      const double th = PROB_OUTPUT_THRESHOLD * sum;
      for (map<string, double>::iterator j = m.begin(); j != m.end(); j++) {
	if (j->second >= th) m2.insert(*j);
      }
      double maxp = -1;
      string maxtag;
      for (map<string, double>::iterator j = m2.begin(); j != m2.end(); j++) {
	const double p = j->second;
	if (p > maxp) { maxp = p; maxtag = j->first; }
	//	cout << j->first << ":" << j->second << " ";
      }
      //      cout << endl;
      tagp.push_back(m2);
      vt[i].prd = maxtag;
    }
    //}
    // print the resutls
    for (size_t i = 0; i < vt.size(); i++) {
      const string s = org_strs[i];
      const string p = vt[i].prd;
      if (STANDOFF || OUTPUT_TAG_PROBS || UIMA || ENJU) {
	if (STANDOFF || UIMA || ENJU) {
	  cout << vt[i].begin << "\t" << vt[i].end;
	  if (!UIMA && !ENJU){
	    cout << "\t";
	  }
	}
	if (!UIMA && !ENJU){
	  cout << s;
	}
	if (OUTPUT_TAG_PROBS) {
	  vector<TagProb> tp;
	  double sum = 0;
	  for (map<string, double>::iterator j = tagp[i].begin(); j != tagp[i].end(); j++) {
	    tp.push_back(TagProb(j->first, j->second));
	    sum += j->second;
	  }
	  stable_sort(tp.begin(), tp.end());
	  for (vector<TagProb>::iterator j = tp.begin(); j != tp.end(); j++) {
	    const double p = j->prob / sum; // normalize
	    if (p == 1) cout << resetiosflags(ios::fixed);
	    else        cout << setiosflags(ios::fixed) << setprecision(3);
	    cout << "\t" << j->tag << "\t" << p;
	  }
	} else {
	  cout << "\t" + p;
          if (ENJU) {
            cout << "\t1";
          }
	}
	if (UIMA){
	  cout << "\t0";
	}
	cout << endl;
      } else {
	if (i == 0) cout << s + "/" + p;
	else        cout << " " + s + "/" + p;
      }
    }
    cout << endl;
    crfm.incr_line_counter();
  }

  //  int msec = push_stop_watch();
  //  cerr << "tagging time = " << msec << " msec" << endl;
  //  cerr << 1000.0 * nlines / msec << " lines / sec" << endl;

}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.18  2011/05/02 10:53:53  matuzaki
 *
 * windows
 *
 * Revision 1.17  2011/01/07 05:48:54  tsuruoka
 * add CONVERT_SQUARE_BRACKETS
 *
 * Revision 1.16  2010/05/15 07:19:47  yusuke
 *
 * suppress messages for enju output
 *
 * Revision 1.15  2010/04/21 08:56:18  tsuruoka
 * *** empty log message ***
 *
 * Revision 1.14  2009/12/03 23:35:47  matuzaki
 *
 * "-e" option for enju's input
 *
 * Revision 1.13  2008/04/12 17:55:50  tsuruoka
 * crf bugfix
 *
 * Revision 1.12  2007/08/07 06:44:29  hillbig
 * change overflow process
 *
 * Revision 1.11  2007/06/19 09:17:22  tsuruoka
 * check input length
 *
 * Revision 1.10  2007/06/19 09:00:37  tsuruoka
 * truncate a sentence if it's too long
 *
 * Revision 1.9  2007/06/12 07:06:08  hillbig
 * add --uint option
 *
 * Revision 1.8  2007/06/08 08:38:37  tsuruoka
 * add version
 *
 * Revision 1.7  2007/06/08 08:25:43  tsuruoka
 * read standard input when FILE is -
 *
 * Revision 1.6  2007/06/07 20:24:26  tsuruoka
 * PROB_OUTPUT_THRESHOLD: 0.01 -> 0.001
 *
 * Revision 1.5  2007/06/07 20:12:49  tsuruoka
 * add --nbest option
 *
 * Revision 1.4  2007/06/07 14:27:18  tsuruoka
 * add nbest output for CRF
 *
 * Revision 1.3  2007/05/15 10:26:47  tsuruoka
 * output input sentences in stand-off mode
 *
 * Revision 1.2  2007/05/15 09:57:16  tsuruoka
 * change help message
 *
 * Revision 1.1.1.1  2007/05/15 08:30:35  kyoshida
 * stepp tagger, by Okanohara and Tsuruoka
 *
 */

