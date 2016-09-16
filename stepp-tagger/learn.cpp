/*
 * $Id: learn.cpp,v 1.6 2011-01-17 12:43:43 matuzaki Exp $
 */

#include <stdio.h>
#include <fstream>
#include <map>
#include <list>
#include <set>
#include <iostream>
#include <cfloat>
#include <sstream>
#include "maxent.h"
#include "crf.h"
#include "common.h"

using namespace std;

extern multimap<string, string> WNdic;

string MODEL_DIR = "."; // the default directory for saving the models

int eftrain(const vector<Sentence> & vs, const string & modeldir, const bool use_l1);
int
crftrain(const CRF_Model::OptimizationMethod method,
	 CRF_Model & c, const vector<Sentence> & vs, double g, const bool use_l1);


void read_UmlsLRAGR(const string & filename)
{
  ifstream ifile(filename.c_str());
  if (!ifile) { cerr << "error: cannot open " << filename << endl; exit(1); }
  cerr << "reading " << filename << endl;

  string line;
  while (getline(ifile, line)) {
    const string::size_type i1 = line.find_first_of('|');
    const string::size_type i2 = line.find_first_of('|', i1+1);
    const string::size_type i3 = line.find_first_of('|', i2+1);
    const string str = line.substr(i1 + 1, i2 - i1 - 1);
    const string pos = line.substr(i2 + 1, i3 - i2 - 1);
    if (str.find_first_of(' ') != string::npos) continue;
    //    cout << str << "#" << pos << endl;
    WNdic.insert(make_pair(str, pos));
  }
  ifile.close();
  //  exit(0);

}

void read_WordNetDict(const string & filename)
{
  ifstream ifile(filename.c_str());

  cerr << "reading " << filename << endl;

  string line;
  while (getline(ifile, line)) {
    if (isspace(line[0])) continue;
    istringstream is(line);
    string str, pos;
    is >> str >> pos;
    WNdic.insert(make_pair(str, pos));
  }
  ifile.close();
}

void read_WordNet(const string & wordnetdir)
{
  read_WordNetDict(wordnetdir + "index.noun");
  read_WordNetDict(wordnetdir + "index.verb");
  read_WordNetDict(wordnetdir + "index.adj");
  read_WordNetDict(wordnetdir + "index.adv");
}


void save_dic(const string & filename)
{
  ofstream ofile(filename.c_str());

  for (multimap<string, string>::const_iterator i = WNdic.begin(); i != WNdic.end(); i++) {
    ofile << i->first << " " << i->second << endl;
  }

  ofile.close();

}

void read_tagged(istream * ifile, vector<Sentence> & vs)
{
  static ParenConverter paren_converter;
 
  string line;
  while (getline(*ifile,line)) {
    istringstream is(line);
    string s;
    Sentence sentence;
    while (is >> s) {
      string::size_type i = s.find_last_of('/');
      string str = s.substr(0, i);
      string pos = s.substr(i+1);
      //                cout << str << " ";

      //      string str0 = str;
      str = paren_converter.Ptb2Pos(str);
      //      if (str != str0) cout << str0 << " " << str << endl;
      //pos = paren_converter.Pos2Ptb(pos);

      //cout << str << "\t" << pos << '\t' << line << endl;
      Token t(str, pos);
      sentence.push_back(t);
    }
    vs.push_back(sentence);
    //    if (vs.size() >= num_sentences) break;
    //            cout << endl;
  }
  // exit(0);
}

void print_help()
{
  cout << "Usage: stepp-learn [OPTION]... [FILE]" << endl;
  cout << "Create probabilistc models for the STePP tagger." << endl;
  cout << "The input must be one-sentence-per-line." << endl;
  cout << "The format for each line is: WORD1/POS1 WORD2/POS2 WORD3/POS3 ..." << endl;
  cout << endl;
  cout << "Mandatory arguments to long options are mandatory for short options too." << endl;
  cout << "  -m, --model=DIR       specify the directory for saving the models" << endl;
  cout << "  -c, --compact         build compact models" << endl;
  cout << "  -f, --fast            build only the model for the fast mode" << endl;
  cout << "  -h, --help            display this help and exit" << endl;
  cout << endl;
  cout << "With no FILE, or when FILE is -, read standard input." << endl;
  cout << endl;
  cout << "Report bugs to <yoshimasa.tsuruoka@manchester.ac.uk>" << endl;
}

int main(int argc, char** argv)
{
  string ifilename;

  bool use_l1_regularization = false;
  bool CRF_ONLY = false;
  CRF_Model::OptimizationMethod opmethod = CRF_Model::BFGS;
  string WORDNET_DIR = "", UMLS_DIC = "";

  double gaussian = 256;
  for (int i = 1; i < argc; i++) {
    string v = argv[i];
    //    if (v == "-i" && i < argc-1) ifilename = argv[i+1];
    //    if (v == "-o" && i < argc-1) ofilename = argv[i+1];
    if (v == "-gp" && i < argc-1) {
      gaussian = atof(argv[i+1]);
      i++;
      continue;
    }
    if ( (v == "-m" || v == "--model") && i < argc-1) {
      MODEL_DIR = argv[i+1];
      i++;
      continue;
    }
    if ( (v == "-wn" || v == "--wordnet") && i < argc-1) {
      WORDNET_DIR = argv[i+1];
      i++;
      continue;
    }
    if ( (v == "--umls") && i < argc-1) {
      UMLS_DIC = argv[i+1];
      i++;
      continue;
    }
    if (v.substr(0, 8) == "--model=") {
      MODEL_DIR = v.substr(8);
      continue;
    }
    if (v == "-c" || v == "--compact") {
      use_l1_regularization = true;
      continue;
    }
    if (v == "-f" || v == "--fast") { 
      CRF_ONLY = true;
      continue; 
    }
    if (v == "-sgd") {
      opmethod = CRF_Model::SGD;
      continue; 
    }
    if (v == "-avp") {
      opmethod = CRF_Model::PERCEPTRON;
      continue; 
    }
    if (v == "-h" || v == "--help") {
      print_help();
      exit(0);
    }
    if (v == "-") {
      ifilename = "";
      continue;
    }
    if (v[0] == '-') {
      cerr << "error: unknown option " << v << endl;
      cerr << "Try `stepp-learn --help' for more information." << endl;
      exit(1);
    }
    ifilename = v;
  }
  //  cerr << ifilename << endl;

  if (MODEL_DIR[MODEL_DIR.size()-1] != '/')  MODEL_DIR += "/";

  const string DICFILE = MODEL_DIR + "dictionary.txt";
  if (WORDNET_DIR != "") {
    if (WORDNET_DIR[WORDNET_DIR.size()-1] != '/') WORDNET_DIR += "/";
    read_WordNet(WORDNET_DIR);
    save_dic(DICFILE);
  }
  if (UMLS_DIC != "") {
    read_UmlsLRAGR(UMLS_DIC);
    save_dic(DICFILE);
  }

  vector<Sentence> trains;

  istream *is(&std::cin);
  ifstream ifile;
  if (ifilename != "") {
    ifile.open(ifilename.c_str());
    if (!ifile) { cerr << "error: cannot open " << ifilename << endl; exit(1); }
    is = &ifile;
  }

  read_tagged(is, trains);

  if (trains.size() == 0) {
    if (ifilename != "") { 
      cerr << "error: cannot read \"" << ifilename << "\"" << endl; exit(1); 
    }
    cerr << "error: no training data." << endl;
    exit(1);
  }
  
  if (!CRF_ONLY) eftrain(trains, MODEL_DIR, use_l1_regularization);

  CRF_Model crfm;
  crftrain(opmethod, crfm, trains, gaussian, use_l1_regularization);
  string mfile = MODEL_DIR + "model.crf";
  //  cerr << "saving the CRF model to " << mfile << "...";
  crfm.save_to_file(mfile, 0.001);
  //  cerr << "done" << endl;

  cerr << endl;
  cerr << "the models have been saved in the directory \"" << MODEL_DIR << "\"." << endl;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2010/04/21 08:56:18  tsuruoka
 * *** empty log message ***
 *
 * Revision 1.4  2008/08/14 23:19:04  tsuruoka
 * add stochastic gradient descent
 *
 * Revision 1.3  2007/09/26 23:03:44  tsuruoka
 * add a function to generate features for chunking
 *
 * Revision 1.2  2007/06/08 08:25:43  tsuruoka
 * read standard input when FILE is -
 *
 * Revision 1.1.1.1  2007/05/15 08:30:35  kyoshida
 * stepp tagger, by Okanohara and Tsuruoka
 *
 */

