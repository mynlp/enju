/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file ForestMaker.h
 *  @version Time-stamp: <2010-06-02 10:39:40 yusuke>
 *  Implementation of forestmaker
 *
 **********************************************************************/

#ifndef MAYZ_FOREST_MAKER_H

#define MAYZ_FOREST_MAKER_H

#include "mconfig.h"
#include <liblilfes/lilfes.h>
#include "ParserCkyFom.h"
#include "Derivation.h"
#include "ParseTree.h"
#include "AmisModel.h"
#include "Supertagger.h"

namespace mayz {

    class ForestMakerException : public std::exception {
    private:
        std::string message;

    public:
        ForestMakerException( const char* m ) : message( m ) {
        }
        ~ForestMakerException() throw () {}
        const char* what() const throw () { return message.c_str(); }
    };

    class ForestMaker : public up::ParserCkyFom {
    public:
        typedef up::ParserCkyFom base_type;
        typedef std::string lex_template_type;
        
    private:
        std::string model_name;
        std::vector< bool > visit_mark;
        std::ostream* output_stream;
        std::ostream* reference_stream;
        double reference_probability;
        int num_templates_threshold;
        // double prob_templates_threshold;
        double fom_templates_threshold;

        lilfes::procedure* derivation_to_lexical_entry_list;
        const lilfes::type* lex_entry_undef;
        const lilfes::type* lex_entry;
        const lilfes::type* lex_word_f;
        const lilfes::type* lex_template_f;
        lilfes::procedure* convert_derivation;
//        lilfes::procedure* fm_correct_lexical_entry;

        lilfes::procedure* extract_terminal_event;
        lilfes::procedure* extract_unary_event;
        lilfes::procedure* extract_binary_event;
        lilfes::procedure* extract_root_event;

        lilfes::procedure* extract_terminal_event_feature_value;
        lilfes::procedure* extract_unary_event_feature_value;
        lilfes::procedure* extract_binary_event_feature_value;
        lilfes::procedure* extract_root_event_feature_value;

      //lilfes::procedure* reference_prob_terminal;
        lilfes::procedure* reference_prob_unary;
        lilfes::procedure* reference_prob_binary;
        lilfes::procedure* reference_prob_root;

        lilfes::procedure* word_to_lookup_keys;

        std::vector<double> derivation_foms;
        up::Supertagger *super;
        
    public:
        virtual bool outputTerminalEvents( lilfes::FSP lexname, lilfes::FSP sign, lilfes::FSP sign_plus, std::ostream& output_file );
        virtual bool outputUnaryEvents( lilfes::FSP schema, lilfes::FSP dtr, lilfes::FSP mother, lilfes::FSP sign_plus, std::ostream& output_file );
        virtual bool outputBinaryEvents( lilfes::FSP schema, lilfes::FSP ldtr, lilfes::FSP rdtr, lilfes::FSP mother, lilfes::FSP sign_plus, std::ostream& output_file );
        virtual bool outputRootEvents( lilfes::FSP sign, std::ostream& output_file );

      //virtual bool terminalReference( lilfes::FSP lexname, lilfes::FSP sign, lilfes::FSP sign_plus, double& ref );
        virtual bool unaryReference( lilfes::FSP schema_name, lilfes::FSP dtr, lilfes::FSP mother, lilfes::FSP sign_plus, double& ref );
        virtual bool binaryReference( lilfes::FSP schema_name, lilfes::FSP left_dtr, lilfes::FSP right_dtr, lilfes::FSP mother, lilfes::FSP sign_plus, double& ref );
        virtual bool rootReference( lilfes::FSP sign, double& ref );

    protected:
        virtual bool outputParseTreeDtrs( ParseTree parse_tree );
        virtual bool outputParseTreeRoot( ParseTree parse_tree );

        virtual bool outputForestConjTerm( up::Edge* edge, up::Link* edge_link, int id );
        virtual bool outputForestConjUnary( up::Edge* edge, up::Link* edge_link, int id );
        virtual bool outputForestConjBinary( up::Edge* edge, up::Link* edge_link, int id );
        virtual bool outputForestDisj( up::Edge* edge );
        virtual bool outputForestRoot();

    public:
        ForestMaker( const std::string& n, int __num_templates_threshold, /* double __prob_templates_threshold, */ double __fom_templates_threshold)
                : model_name( n ),
                  output_stream( NULL ),
                  reference_stream( NULL ),
                  reference_probability( 0.0 ),
                  num_templates_threshold(__num_templates_threshold),
                  // prob_templates_threshold(__prob_templates_threshold),
                  fom_templates_threshold(__fom_templates_threshold)
        {
            // Do full parsing without beam in the chart
            base_type::beam_start = base_type::Beam::noBeam();
            base_type::beam_step  = base_type::Beam::noBeam();
            base_type::beam_end   = base_type::Beam::noBeam();
        }

        /*
        ForestMaker( const std::string& n)
                : model_name( n ),
                  output_stream( NULL ),
                  reference_stream( NULL ),
                  reference_probability( 0.0 ),
                  num_templates_threshold(20),
                  prob_templates_threshold(0.99),
                  fom_templates_threshold(5.5)
        {
            // Do full parsing without beam in the chart
            base_type::beam_start = base_type::Beam::noBeam();
            base_type::beam_step  = base_type::Beam::noBeam();
            base_type::beam_end   = base_type::Beam::noBeam();
        }
        */

        virtual ~ForestMaker() {
        }

        bool init( lilfes::machine* m, up::Grammar *g, up::Supertagger *s );

        unsigned analyzeSentenceLength( const up::WordLattice &word_vec ) {
            unsigned r = 0;
            for (up::WordLattice::const_iterator it = word_vec.begin(); it != word_vec.end(); ++it) {
              r = std::max(r, it->end);
			}
            return r;
        }

		bool checkSentenceLength(unsigned length, unsigned limit_length) {
            if ( limit_length > 0 && length > limit_length) {
                parse_status = up::TOO_LONG;
                return false;
            }
            return true;
		}

        bool checkWordPosition( up::WordLattice& word_vec ) {
			for (unsigned i = 0; i < word_vec.size(); ++i) {
				if (i == word_vec[i].begin && (i+1) == word_vec[i].end) {
					// ok
				}
				else {
					return false;
				}
			}
			return true;
        }
        
        bool makeLeaves( Derivation derivation,
                         //std::vector<word_type>& __word_vec, std::vector<lex_template_type>& __lex_template_vec);
                         up::WordLattice& word_vec, std::vector<lex_template_type>& lex_template_vec);
        virtual bool derivationToParseTree( Derivation derivation, ParseTree parse_tree );
        virtual bool outputParseTree( ParseTree parse_tree, std::ostream& output_file, std::ostream& reference_file );
        virtual bool outputParseTree( ParseTree parse_tree, std::ostream& output_file );
        virtual bool outputForest( std::ostream& output_file, std::ostream& reference_file );
        virtual bool outputForest( std::ostream& output_file );


    protected:
        bool wordToLookupKeys(lilfes::FSP word, std::vector<std::string>& keys);
        void thresholdOutSumLexTemplates(std::vector<LexEntState>& lexents, const std::string &correct_tmpl, unsigned num, double width);
        void thresholdOutLexTemplates(std::vector<LexEntState>& lexents, const std::string &correct_tmpl, unsigned num, double width);
        bool parseLex(const std::vector<lex_template_type>& lex_template_vec);
        bool runSuper(const up::WordLattice& word_vec, const std::vector<lex_template_type>& lex_template_vec);

    public:
        bool fullParse(const up::WordLattice& word_vec, const std::vector<lex_template_type>& lex_template_vec);
    };

}

#endif // MAYZ_FOREST_MAKER_H
