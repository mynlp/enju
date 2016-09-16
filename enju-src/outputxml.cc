/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file outputxml.cc
 *  @version Time-stamp: <2008-07-10 16:07:43 yusuke>
 *  For XML-style output
 *
 **********************************************************************/

#ifdef _MSC_VER
#include "config_mayz_win.h"
#else
#include "config_mayz.h"
#endif
#include <liblilfes/lilfes.h>
#include <liblilfes/builtin.h>
#include <liblilfes/stream.h>
#include <liblilfes/utility.h>
#include <sstream>

using namespace lilfes;
using namespace std;

//////////////////////////////////////////////////////////////////////

static const char* OUTPUTXML_MODULE_NAME = "enju/xmltypes";

static const module* outputxml_module = NULL;
static const type* t_xml_word_or_phrase = NULL;
static const type* t_xml_word = NULL;
static const type* t_xml_phrase = NULL;
static const type* t_xml_sentence = NULL;
static const type* t_xml_rel = NULL;
static const feature* f_xml_node_id = NULL;
static const feature* f_xml_cat = NULL;
static const feature* f_xml_surface = NULL;
static const feature* f_xml_pos = NULL;
static const feature* f_xml_base = NULL;
static const feature* f_xml_lexent = NULL;
static const feature* f_xml_pred = NULL;
static const feature* f_xml_rel_list = NULL;
static const feature* f_xml_rel_label = NULL;
static const feature* f_xml_rel_value = NULL;
static const feature* f_xml_xcat = NULL;
static const feature* f_xml_head_id = NULL;
static const feature* f_xml_sem_head_id = NULL;
static const feature* f_xml_schema = NULL;
static const feature* f_xml_dtrs = NULL;
static const feature* f_xml_sentence_id = NULL;
static const feature* f_xml_parse_status = NULL;
static const feature* f_xml_fom = NULL;
static const feature* f_xml_parse_tree = NULL;

static bool initialize_types( machine& mach ) {
  outputxml_module = load_module( mach, "outputxml module", OUTPUTXML_MODULE_NAME );
  if ( ! outputxml_module ) return false;
  t_xml_word_or_phrase = module::UserModule()->Search( "xml_word_or_phrase" );
  t_xml_word = module::UserModule()->Search( "xml_word" );
  t_xml_phrase = module::UserModule()->Search( "xml_phrase" );
  t_xml_sentence = module::UserModule()->Search( "xml_sentence" );
  t_xml_rel = module::UserModule()->Search( "xml_rel" );
  f_xml_node_id = module::UserModule()->SearchFeature( "XML_NODE_ID\\" );
  f_xml_cat = module::UserModule()->SearchFeature( "XML_CAT\\" );
  f_xml_surface = module::UserModule()->SearchFeature( "XML_SURFACE\\" );
  f_xml_pos = module::UserModule()->SearchFeature( "XML_POS\\" );
  f_xml_base = module::UserModule()->SearchFeature( "XML_BASE\\" );
  f_xml_lexent = module::UserModule()->SearchFeature( "XML_LEXENTRY\\" );
  f_xml_pred = module::UserModule()->SearchFeature( "XML_PRED\\" );
  f_xml_rel_list = module::UserModule()->SearchFeature( "XML_REL_LIST\\" );
  f_xml_rel_label = module::UserModule()->SearchFeature( "XML_REL_LABEL\\" );
  f_xml_rel_value = module::UserModule()->SearchFeature( "XML_REL_VALUE\\" );
  f_xml_xcat = module::UserModule()->SearchFeature( "XML_XCAT\\" );
  f_xml_head_id = module::UserModule()->SearchFeature( "XML_HEAD_ID\\" );
  f_xml_sem_head_id = module::UserModule()->SearchFeature( "XML_SEM_HEAD_ID\\" );
  f_xml_schema = module::UserModule()->SearchFeature( "XML_SCHEMA\\" );
  f_xml_dtrs = module::UserModule()->SearchFeature( "XML_DTRS\\" );
  f_xml_sentence_id = module::UserModule()->SearchFeature( "XML_SENTENCE_ID\\" );
  f_xml_parse_status = module::UserModule()->SearchFeature( "XML_PARSE_STATUS\\" );
  f_xml_fom = module::UserModule()->SearchFeature( "XML_FOM\\" );
  f_xml_parse_tree = module::UserModule()->SearchFeature( "XML_PARSE_TREE\\" );
  return t_xml_word_or_phrase
    && t_xml_word
    && t_xml_phrase
    && t_xml_sentence
    && t_xml_rel
    && f_xml_node_id
    && f_xml_cat
    && f_xml_surface
    && f_xml_pos
    && f_xml_base
    && f_xml_lexent
    && f_xml_pred
    && f_xml_rel_list
    && f_xml_rel_label
    && f_xml_rel_value
    && f_xml_xcat
    && f_xml_head_id
    && f_xml_sem_head_id
    && f_xml_schema
    && f_xml_dtrs
    && f_xml_sentence_id
    && f_xml_parse_status
    && f_xml_fom
    && f_xml_parse_tree;
}

//////////////////////////////////////////////////////////////////////

struct xml_tag {
public:
  typedef pair< string::size_type, string::size_type > position;
  string tag;
  vector< pair< string, string > > attrs;
  position pos;

  xml_tag( const string& t, position p ) : tag( t ), pos( p ) {}
  xml_tag( const string& t ) : tag( t ), pos( 0, 0 ) {}
};

class output_xml_error : public exception {
private:
  const char* msg;
public:
  output_xml_error( const char* m ) : msg( m ) {}
  ~output_xml_error() throw() {}
  const char* what() const throw() { return msg; }
};

// static string inttostr( int i ) {
//   ostringstream s;
//   s << i;
//   return s.str();
// }
//static string inttoidstr( const string& prefix, int i ) {
//  ostringstream s;
//  s << prefix << i;
//  return s.str();
//}
static void valueappend( string& attr, const string& value ) {
  if ( attr.empty()) {
    attr += value;
  }
  else {
    attr += (" " + value);
  }
}

//////////////////////////////////////////////////////////////////////

static string encode_xml_string( const string& str ) {
  string out;
  string::size_type spos = 0;
  string::size_type epos = 0;
  while ( ( epos = str.find_first_of( "&<>\"\'", spos ) ) != string::npos ) {
    out += str.substr( spos, epos - spos );
    switch ( str[ epos ] ) {
    case '&':
      out += "&amp;";
      break;
    case '<':
      out += "&lt;";
      break;
    case '>':
      out += "&gt;";
      break;
    case '"':
      out += "&quot;";
      break;
    case '\'':
      out += "&apos;";
      break;
    default:
      ;
    }
    spos = epos + 1;
  }
  out += str.substr( spos );
  return out;
}

//////////////////////////////////////////////////////////////////////

static xml_tag::position make_tag_stack( machine& mach, FSP xml, const string& text, xml_tag::position position, vector< xml_tag >& tag_stack ) {
  const type* t = xml.GetType();
//   if ( t == t_xml_sentence ) {
//     // top-level tag
//     int sentence_id = 0;
//     if ( ! lilfes_to_c< int >().convert( mach, xml.Follow( f_xml_sentence_id ), sentence_id ) ) {
//       throw output_xml_error( "output_xml: XML_SENTENCE_ID is not filled" );
//     }
//     xml_tag sentence( "sentence", position );
//     sentence.attrs.push_back( make_pair( "id", inttoidstr( "s", sentence_id ) ) );
//     tag_stack.push_back( sentence );
//     make_tag_stack( mach, xml.Follow( f_xml_sentence ), text, position, tag_stack );
//     return position;
//   }
  if ( ! t->IsSubType( t_xml_word_or_phrase ) ) {
    throw output_xml_error( "output_xml: input must be xml_word_or_phrase" );
  }
  string node_id; //= 0;
  if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_node_id ), node_id ) ) {
    throw output_xml_error( "output_xml: XML_NODE_ID is not filled" );
  }
  if ( t == t_xml_word ) {
    // terminal
    string word;
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_surface ), word ) ) {
      throw output_xml_error( "output_xml: XML_SURFACE is not filled" );
    }
    string::size_type spos = text.find( word, position.first );
    string::size_type len = word.size();
    if ( word == "``" || word == "''" ) {
      // double quotation
      // (needs special handling because of the tokenizer modifying an input text)
      string::size_type dpos = text.find( "\"", position.first );
      if ( spos == string::npos ||
           ( dpos != string::npos && dpos < spos ) ) {
        spos = dpos;
        len = 1;
      }
    }
    if ( spos == string::npos ) {
      throw output_xml_error( "output_xml: input text and xml tree are inconsistent" );
    }
    if ( spos > position.second ) {
      throw output_xml_error( "output_xml: input text and xml tree are inconsistent" );
    }
    string cat;
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_cat ), cat ) ) {
      throw output_xml_error( "output_xml: XML_CAT is not filled" );
    }
    string pos;
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_pos ), pos ) ) {
      throw output_xml_error( "output_xml: XML_POS is not filled" );
    }
    string base;
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_base ), base ) ) {
      throw output_xml_error( "output_xml: XML_BASE is not filled" );
    }
    string lexent;
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_lexent ), lexent ) ) {
      throw output_xml_error( "output_xml: XML_LEXENTRY is not filled" );
    }
    string pred;
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_pred ), pred ) ) {
      throw output_xml_error( "output_xml: XML_PRED is not filled" );
    }
    vector< FSP > rels;
    if ( ! lilfes_to_c< vector< FSP > >().convert( mach, xml.Follow( f_xml_rel_list ), rels ) ) {
      throw output_xml_error( "output_xml: XML_REL_LIST is not filled" );
    }
    xml_tag word_tag( "tok", make_pair( spos, spos + len ) );
    word_tag.attrs.push_back( make_pair( "id", node_id ) );
    word_tag.attrs.push_back( make_pair( "cat", cat ) );
    word_tag.attrs.push_back( make_pair( "pos", pos ) );
    word_tag.attrs.push_back( make_pair( "base", base ) );
    word_tag.attrs.push_back( make_pair( "lexentry", lexent ) );
    word_tag.attrs.push_back( make_pair( "pred", pred ) );
    for ( vector< FSP >::iterator it = rels.begin();
          it != rels.end();
          ++it ) {
      if ( it->GetType() != t_xml_rel ) {
        throw output_xml_error( "output_xml: XML_REL_LIST must be a list of 'xml_rel'" );
      }
      string rel_label;
      if ( ! lilfes_to_c< string >().convert( mach, it->Follow( f_xml_rel_label ), rel_label ) ) {
        throw output_xml_error( "output_xml: XML_REL_LABEL is not filled" );
      }
      string rel_value;
      if ( ! lilfes_to_c< string >().convert( mach, it->Follow( f_xml_rel_value ), rel_value ) ) {
        throw output_xml_error( "output_xml: XML_REL_VALUE is not filled" );
      }
      word_tag.attrs.push_back( make_pair( rel_label, rel_value ) );
    }
    tag_stack.push_back( word_tag );
    return word_tag.pos;
  }
  // nonterminal
  string cat;
  if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_cat ), cat ) ) {
    throw output_xml_error( "output_xml: XML_CAT is not filled" );
  }
  vector< string > xcat_list;
  if ( ! lilfes_to_c< vector< string > >().convert( mach, xml.Follow( f_xml_xcat ), xcat_list ) ) {
      throw output_xml_error( "output_xml: XML_XCAT is not filled" );
  }
  string head_id;
  if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_head_id ), head_id ) ) {
    throw output_xml_error( "output_xml: XML_HEAD_ID is not filled" );
  }
  string sem_head_id;
  if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_sem_head_id ), sem_head_id ) ) {
    throw output_xml_error( "output_xml: XML_SEM_HEAD_ID is not filled" );
  }
  string schema_name;
  if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_schema ), schema_name ) ) {
    throw output_xml_error( "output_xml: XML_SCHEMA is not filled" );
  }
  xml_tag phrase_tag( "cons" );
  phrase_tag.attrs.push_back( make_pair( "id", node_id ) );
  phrase_tag.attrs.push_back( make_pair( "cat", cat ) );
  string xcat;
  for ( vector< string >::const_iterator it = xcat_list.begin();
        it != xcat_list.end();
        ++it ) {
    valueappend( xcat, *it );
  }
  phrase_tag.attrs.push_back( make_pair( "xcat", xcat ) );
  phrase_tag.attrs.push_back( make_pair( "head", head_id ) );
  phrase_tag.attrs.push_back( make_pair( "sem_head", sem_head_id ) );
  if (! schema_name.empty()) {
    phrase_tag.attrs.push_back( make_pair( "schema", schema_name ) );
  }
  size_t phrase_tag_index = tag_stack.size();
  tag_stack.push_back( phrase_tag );
  FSP dtrs = xml.Follow( f_xml_dtrs );
  xml_tag::position new_pos;
  xml_tag::position pos = position;
  bool is_left = true;
  while ( dtrs.GetType() == cons ) {
    FSP dtr = dtrs.Follow( hd );
    xml_tag::position pos1 = make_tag_stack( mach, dtr, text, pos, tag_stack );
    if ( is_left ) { new_pos.first = pos1.first; is_left = false; }
    pos.first = pos1.second;
    dtrs = dtrs.Follow( tl );
  }
  new_pos.second = pos.first;
  tag_stack[ phrase_tag_index ].pos = new_pos;
  return new_pos;
}

void make_tag_stack_top( machine& mach, FSP xml, const string& text, vector< xml_tag >& tag_stack ) {
  static bool initialized = initialize_types( mach );
  xml_tag::position position = std::make_pair( 0, text.size() );
  string sentence_id;
  string parse_status;
  try {
    if ( ! initialized ) {
      throw output_xml_error( "output_xml: loading lilfes module failed" );
    }
    const type* t = xml.GetType();
    if ( t != t_xml_sentence ) {
      throw output_xml_error( "output_xml: input must be xml_sentence" );
    }
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_sentence_id ), sentence_id ) ) {
      throw output_xml_error( "output_xml: XML_SENTENCE_ID is not filled" );
    }
    if ( ! lilfes_to_c< string >().convert( mach, xml.Follow( f_xml_parse_status ), parse_status ) ) {
      throw output_xml_error( "output_xml: XML_PARSE_STATUS is not filled" );
    }
    xml_tag sentence_tag( "sentence", position );
    sentence_tag.attrs.push_back( make_pair( "id", sentence_id ) );
    sentence_tag.attrs.push_back( make_pair( "parse_status", parse_status ) );
    double fom = 0.0;
    if ( lilfes_to_c< double >().convert( mach, xml.Follow( f_xml_fom ), fom ) ) {
      ostringstream fom_str;
      fom_str << fom;
      sentence_tag.attrs.push_back( make_pair( "fom", fom_str.str() ) );
    }
    tag_stack.push_back( sentence_tag );
    FSP parse_tree = xml.Follow( f_xml_parse_tree );
    if ( parse_tree.GetType()->IsSubType( t_xml_word_or_phrase ) ) {
      make_tag_stack( mach, parse_tree, text, position, tag_stack );
    } else if ( parse_tree.GetType() == cons ) {
      // partial trees
      FSP list = parse_tree;
      xml_tag::position pos = position;
      while ( list.GetType() == cons ) {
        FSP tree = list.Follow( hd );
        xml_tag::position pos1 = make_tag_stack( mach, tree, text, pos, tag_stack );
        pos.first = pos1.second;
        list = list.Follow( tl );
      }
    }
  } catch ( output_xml_error& e ) {
    tag_stack.clear();
    //cerr << e.what() << endl;
    xml_tag error_tag( "sentence", position );
    error_tag.attrs.push_back( make_pair( "id", sentence_id ) );
    error_tag.attrs.push_back( make_pair( "parse_status", "XML encoding error" ) );
    tag_stack.push_back( error_tag );
  }
}

//////////////////////////////////////////////////////////////////////

static void output_tag_stack_so(int offset, const vector< xml_tag >& tag_stack, ostream& ostr) {
  for ( vector< xml_tag >::const_iterator it = tag_stack.begin();
        it != tag_stack.end();
        ++it ) {
    ostr << offset + it->pos.first << '\t' << offset + it->pos.second << '\t' << it->tag;
    for ( vector< pair< string, string > >::const_iterator attr_it = it->attrs.begin();
          attr_it != it->attrs.end();
          ++attr_it ) {
      ostr << ' ' << attr_it->first << "=\"" << encode_xml_string( attr_it->second ) << '\"';
    }
    ostr << '\n';
  }
  ostr << '\n';
}

static void output_tag_stack_xml(const string& text, const vector< xml_tag >& tag_stack, ostream& ostr) {
  string::size_type spos = 0;
  vector< vector< xml_tag >::const_iterator > end_stack;
  for ( vector< xml_tag >::const_iterator it = tag_stack.begin();
        it != tag_stack.end();
        ++it ) {
    while ( ! end_stack.empty() && it->pos.first >= end_stack.back()->pos.second ) {
      ostr << encode_xml_string( text.substr( spos, end_stack.back()->pos.second - spos ) );
      spos = end_stack.back()->pos.second;
      ostr << "</" << end_stack.back()->tag << '>';
      end_stack.pop_back();
    }
    ostr << encode_xml_string( text.substr( spos, it->pos.first - spos ) );
    spos = it->pos.first;
    ostr << '<' << it->tag;
    for ( vector< pair< string, string > >::const_iterator attr_it = it->attrs.begin();
          attr_it != it->attrs.end();
          ++attr_it ) {
      ostr << ' ' << attr_it->first << "=\"" << encode_xml_string( attr_it->second ) << '\"';
    }
    ostr << '>';
    end_stack.push_back( it );
  }
  while ( ! end_stack.empty() ) {
    ostr << encode_xml_string( text.substr( spos, end_stack.back()->pos.second - spos ) );
    spos = end_stack.back()->pos.second;
    ostr << "</" << end_stack.back()->tag << '>';
    end_stack.pop_back();
  }
  ostr << encode_xml_string( text.substr( spos ) ) << '\n';
}

//////////////////////////////////////////////////////////////////////

static bool output_so_parse_tree(machine& mach, FSP ofs, FSP text, FSP xml, FSP outstr) {
  if ( ! ofs.IsInteger() ) {
    RUNERR( "arg1 of output_phrase_structure_so/4 must be an integer" );
    return false;
  }
  int offset = ofs.ReadInteger();
  if ( ! text.IsString() ) {
    RUNERR( "arg1 of output_phrase_structure_so/4 must be a string" );
    return false;
  }
  std::string text_str( text.ReadString() );

  lilfes_stream *s = lilfes_stream::GetStreamFSP( outstr );
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 3rd argument of 'output_phrase_structure_so/4'" );
    return false;
  }
  #ifdef IS_GXX
  std::ostream* ostr = dynamic_cast< std::ostream* >( s->GetCStream() );
  #else
  std::ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'output_phrase_structure_so/4'" );
    return false;
  }

  if ( ! (*ostr) ) return false;

  vector< xml_tag > tag_stack;
  make_tag_stack_top( mach, xml, text_str, tag_stack );
  output_tag_stack_so( offset, tag_stack, *ostr );

  if ( ! (*ostr) ) return false;

  return true;
}

static bool output_xml_parse_tree(machine& mach, FSP text, FSP xml, FSP outstr) {
  if( ! text.IsString() ) {
    RUNERR( "arg1 of output_phrase_structure_xml/3 must be a string" );
    return false;
  }
  std::string text_str( text.ReadString() );

  lilfes_stream *s = lilfes_stream::GetStreamFSP(outstr);
  if ( s == NULL ) {
    RUNERR( "An invalid handle in the 3rd argument of 'output_phrase_structure_xml/3'" );
    return false;
  }
  #ifdef IS_GXX
  std::ostream* ostr = dynamic_cast< std::ostream* >( s->GetCStream() );
  #else
  std::ostream* ostr = s->GetOStream();
  #endif
  if ( ostr == NULL ) {
    RUNERR( "Stream is not an output stream in 'output_phrase_structure_xml/3'" );
    return false;
  }

  if ( ! (*ostr) ) return false;

  vector< xml_tag > tag_stack;
  make_tag_stack_top( mach, xml, text_str, tag_stack );
  output_tag_stack_xml( text_str, tag_stack, *ostr );

  if ( ! (*ostr) ) return false;

  return true;
}

LILFES_BUILTIN_PRED_4(output_so_parse_tree, output_so_parse_tree);
LILFES_BUILTIN_PRED_3(output_xml_parse_tree, output_xml_parse_tree);

