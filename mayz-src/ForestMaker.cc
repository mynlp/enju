/**********************************************************************
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 *  @file ForestMaker.cc
 *  @version Time-stamp: <2010-06-02 10:40:13 yusuke>
 *  Implementation of forestmaker
 *
 **********************************************************************/

#include "AmisModel.h"
#include "ForestMaker.h"
#include "Initializer.h"

namespace mayz {


    using namespace std;
    using namespace lilfes;
    using namespace up;

    extern AmisHandler amis_handler;
    
    bool ForestMaker::init( machine* m, up::Grammar *g, up::Supertagger *super_ ) {
        if (! base_type::init(m, g)) return false;

        super = super_;

        module* forestmake_module = load_module( *m, "forestmake module", "mayz/forestmake" );
        if ( ! forestmake_module ) {
            RUNERR( "module nod found: mayz/forestmake" );
            return false;
        }
        if ( ! Initializer::initialize( m ) ) {
            RUNERR( "initialization of forest_maker failed" );
            return false;
        }
        convert_derivation = prepare_proc( module::UserModule(), "convert_derivation", 2 );
        if ( ! convert_derivation ) {
            RUNERR( "predicate not defined: convert_derivation/2 (maybe inconsistency of forestmaker version" );
            return false;
        }
        derivation_to_lexical_entry_list = prepare_proc( lilfes::module::UserModule(), "derivation_to_lexical_entry_list", 2);
        if( ! derivation_to_lexical_entry_list ) {
            std::cerr << "error: predicate not defined: derivation_to_lexical_entry_list/2 (maybe inconsistency of unimaker version" << std::endl;
            return false;
        }
        lex_entry_undef = lilfes::module::UserModule()->Search("lex_entry_undef");
        if(! lex_entry_undef ) {
            std::cerr << "error: type lex_entry_undef is not defined" << std::endl;
            return false;
        }
        lex_entry = lilfes::module::UserModule()->Search("lex_entry");
        if(! lex_entry ) {
            std::cerr << "error: type lex_entry is not defined" << std::endl;
            return false;
        }
        lex_word_f = lilfes::module::UserModule()->Search("LEX_WORD\\");
        if(! lex_word_f ) {
            std::cerr << "error: feature LEX_WORD is not defined" << std::endl;
            return false;
        }
        lex_template_f = lilfes::module::UserModule()->Search("LEX_TEMPLATE\\");
        if(! lex_template_f ) {
            std::cerr << "error: feature LEX_TEMPLATE is not defined" << std::endl;
            return false;
        }
/*        fm_correct_lexical_entry = prepare_proc( module::UserModule(), "fm_correct_lexical_entry", 4);
        if( ! fm_correct_lexical_entry ) {
            RUNERR( "predicate not defined: fm_correct_lexical_entry/4 (maybe inconsistency of forestmaker version" );
            return false;
            }*/
        extract_terminal_event = prepare_proc( module::UserModule(), "extract_terminal_event", 6 );
        extract_unary_event = prepare_proc( module::UserModule(), "extract_unary_event", 7 );
        extract_binary_event = prepare_proc( module::UserModule(), "extract_binary_event", 8 );
        extract_root_event = prepare_proc( module::UserModule(), "extract_root_event", 4 );

        extract_terminal_event_feature_value = prepare_proc( module::UserModule(), "extract_terminal_event_feature_value", 7 );
        extract_unary_event_feature_value = prepare_proc( module::UserModule(), "extract_unary_event_feature_value", 8 );
        extract_binary_event_feature_value = prepare_proc( module::UserModule(), "extract_binary_event_feature_value", 9 );
        extract_root_event_feature_value = prepare_proc( module::UserModule(), "extract_root_event_feature_value", 5 );

        //reference_prob_terminal = prepare_proc( module::UserModule(), "reference_prob_terminal", 5 );
        reference_prob_unary = prepare_proc( module::UserModule(), "reference_prob_unary", 6 );
        reference_prob_binary = prepare_proc( module::UserModule(), "reference_prob_binary", 7 );
        reference_prob_root = prepare_proc( module::UserModule(), "reference_prob_root", 3 );
        
        word_to_lookup_keys = prepare_proc( module::UserModule(), "word_to_lookup_keys", 2 );
        
        return true;
    }

        //////////////////////////////////////////////////////////////////////

    bool ForestMaker::wordToLookupKeys(lilfes::FSP word, std::vector<std::string>& keys) {
        std::vector< lilfes::FSP > args( 2 );
        args[ 0 ] = word;
        args[ 1 ] = lilfes::FSP( mach ); // lexicon lookup key list
        if (! lilfes::call_proc( *mach, word_to_lookup_keys, args ) ) return false;

        std::vector<lilfes::FSP> keyFSPs;
        if (! lilfes::list_to_vector( *mach, args[1], keyFSPs ) ) {
            std::cerr << "warning: word_to_lookup_keys returned wrong type in the 2nd argument" << std::endl;
            return false;
        }

        if (keyFSPs.empty()) {
            std::cerr << "warning: word_to_lookup_keys returned an empty list in the 2nd argument" << std::endl;
            return false;
        }

        keys.resize(keyFSPs.size());
        for (unsigned i = 0; i < keyFSPs.size(); ++i) {
            if (! keyFSPs[i].IsString()) {
                std::cerr << "warning: word_to_lookup_keys returned non-string list in the 2nd argument" << std::endl;
                return false;
            }
            keys[i] = keyFSPs[i].ReadString();
        }

        return true;
    }

    bool ForestMaker::makeLeaves( Derivation derivation,
                                  WordLattice& word_vec, std::vector<lex_template_type>& lex_template_vec ) {
        std::vector< lilfes::FSP > args( 2 );
        args[ 0 ] = derivation;
        args[ 1 ] = lilfes::FSP( mach ); // lexical entry list
        if(! lilfes::call_proc( *mach, derivation_to_lexical_entry_list, args ) ) return false;

        unsigned position = 0;

        while( args[1].GetType()->IsSubType(lilfes::cons) ) {
            lilfes::FSP lexent = args[1].Follow(lilfes::hd);
            if( lexent.GetType()->IsSubType(lex_entry_undef) ) {
                std::cerr << "warning: fail to derivation_to_lexical_entry_list/2" << std::endl;
                return false;
            } else if (lexent.GetType()->IsSubType(lex_entry) ) {
                lilfes::FSP wordFSP = lexent.Follow(lex_word_f);
                lilfes::FSP tmplFSP = lexent.Follow(lex_template_f);

                up::WordExtent word;
                grammar->makeVecWord(wordFSP, word.word);
                wordToLookupKeys(wordFSP, word.lookup_keys);
                word.word_fom = 0;
                word.begin = position++;
                word.end = word.begin + 1;

                word_vec.push_back(word);

                if (! tmplFSP.IsString()) {
                    std::cerr << "warning: lex_entry\\LEX_TEMPLATE\\ must be a string" << std::endl;
                    return false;
                }
                lex_template_vec.push_back(tmplFSP.ReadString());
            } else {
                std::cerr << "error: inconsistent lexical entry type (neither lex_entry nor lex_entry_undef) is extracted from derivation" << std::endl; return false;
            }
            args[1] = args[1].Follow(lilfes::tl);
        }
        return true;
    }
    bool ForestMaker::derivationToParseTree( Derivation derivation, ParseTree parse_tree ) {
        vector< FSP > args( 2 );
        args[ 0 ] = derivation;
        args[ 1 ] = parse_tree;
        core_p cutp = mach->GetCutPoint();
        if ( ! call_proc( *mach, convert_derivation, args ) ) {
            return false;
        }
        mach->DoCut( cutp );
        return true;
    }

        //////////////////////////////////////////////////////////////////////

    bool ForestMaker::outputTerminalEvents( FSP lexname, FSP sign, FSP sign_plus, ostream& output_file ) {
        if ( extract_terminal_event ) {
            vector< FSP > args( 6 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = lexname;
            args[ 3 ] = sign;
            args[ 4 ] = sign_plus;
            args[ 5 ] = FSP( mach );    // event
            vector< bool > flags( 6, false );
            flags[ 1 ] = true;
            flags[ 5 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_terminal_event, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_terminal_event/6 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "6th argument of extract_terminal_event/6 must be list of strings" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, event_name );
                output_file << ' ' << event_name;
            }
        }
        if ( extract_terminal_event_feature_value ) {
            vector< FSP > args( 7 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = lexname;
            args[ 3 ] = sign;
            args[ 4 ] = sign_plus;
            args[ 5 ] = FSP( mach );    // event
            args[ 6 ] = FSP( mach );    // feature value
            vector< bool > flags( 7, false );
            flags[ 1 ] = true;
            flags[ 5 ] = true;
            flags[ 6 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_terminal_event_feature_value, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                float feature_value;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_terminal_event_feature_value/7 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "6th argument of extract_terminal_event_feature_value/7 must be list of strings" );
                    return false;
                }
                if ( ! lilfes_to_c< float >().convert( *mach, (*it)[ 2 ], feature_value ) ) {
                    RUNWARN( "7th argument of extract_terminal_event_feature_value/7 must be float" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, feature_value, event_name );
                output_file << ' ' << event_name;
            }
        }
        return true;
    }

    bool ForestMaker::outputUnaryEvents( FSP schema, FSP dtr, FSP mother, FSP sign_plus, ostream& output_file ) {
        if ( extract_unary_event ) {
            vector< FSP > args( 7 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = schema;
            args[ 3 ] = dtr;
            args[ 4 ] = mother;
            args[ 5 ] = sign_plus;
            args[ 6 ] = FSP( mach );    // ME event
            vector< bool > flags( 7, false );
            flags[ 1 ] = true;
            flags[ 6 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_unary_event, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_unary_event/7 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "7th argument of extract_unary_event/7 must be list of strings" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, event_name );
                output_file << ' ' << event_name;
            }
        }
        if ( extract_unary_event_feature_value ) {
            vector< FSP > args( 8 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = schema;
            args[ 3 ] = dtr;
            args[ 4 ] = mother;
            args[ 5 ] = sign_plus;
            args[ 6 ] = FSP( mach );    // ME event
            args[ 7 ] = FSP( mach );    // feature value
            vector< bool > flags( 8, false );
            flags[ 1 ] = true;
            flags[ 6 ] = true;
            flags[ 7 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_unary_event_feature_value, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                float feature_value;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_unary_event_feature_value/8 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "7th argument of extract_unary_event_feature_value/8 must be list of strings" );
                    return false;
                }
                if ( ! lilfes_to_c< float >().convert( *mach, (*it)[ 2 ], feature_value ) ) {
                    RUNWARN( "8th argument of extract_unary_event_feature_value/8 must be float" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, feature_value, event_name );
                output_file << ' ' << event_name;
            }
        }
        return true;
    }

    bool ForestMaker::outputBinaryEvents( FSP schema, FSP ldtr, FSP rdtr, FSP mother, FSP sign_plus, ostream& output_file ) {
        if ( extract_binary_event ) {
            vector< FSP > args( 8 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = schema;
            args[ 3 ] = ldtr;
            args[ 4 ] = rdtr;
            args[ 5 ] = mother;
            args[ 6 ] = sign_plus;
            args[ 7 ] = FSP( mach );    // ME event
            vector< bool > flags( 8, false );
            flags[ 1 ] = true;
            flags[ 7 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_binary_event, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_binary_event/8 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "8th argument of extract_binary_event/8 must be list of strings" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, event_name );
                output_file << ' ' << event_name;
            }
        }
        if ( extract_binary_event_feature_value ) {
            vector< FSP > args( 9 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = schema;
            args[ 3 ] = ldtr;
            args[ 4 ] = rdtr;
            args[ 5 ] = mother;
            args[ 6 ] = sign_plus;
            args[ 7 ] = FSP( mach );    // ME event
            args[ 8 ] = FSP( mach );    // feature value
            vector< bool > flags( 9, false );
            flags[ 1 ] = true;
            flags[ 7 ] = true;
            flags[ 8 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_binary_event_feature_value, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                float feature_value;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_binary_event_feature_value/9 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "8th argument of extract_binary_event_feature_value/9 must be list of strings" );
                    return false;
                }
                if ( ! lilfes_to_c< float >().convert( *mach, (*it)[ 2 ], feature_value ) ) {
                    RUNWARN( "9th argument of extract_binary_event_feature_value/9 must be list of float" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, feature_value, event_name );
                output_file << ' ' << event_name;
            }
        }
        return true;
    }

    bool ForestMaker::outputRootEvents( FSP sign, ostream& output_file ) {
        if ( extract_root_event ) {
            vector< FSP > args( 4 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = sign;
            args[ 3 ] = FSP( mach );    // event
            vector< bool > flags( 4, false );
            flags[ 1 ] = true;
            flags[ 3 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_root_event, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_root_event/4 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "4th argument of extract_root_event/4 must be list of strings" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, event_name );
                output_file << ' ' << event_name;
            }
        }
        if ( extract_root_event_feature_value ) {
            vector< FSP > args( 5 );
            args[ 0 ] = FSP( mach, model_name.c_str() );
            args[ 1 ] = FSP( mach );    // category
            args[ 2 ] = sign;
            args[ 3 ] = FSP( mach );    // event
            args[ 4 ] = FSP( mach );    // feature value
            vector< bool > flags( 5, false );
            flags[ 1 ] = true;
            flags[ 3 ] = true;
            flags[ 4 ] = true;
            list< vector< FSP > > results;
            findall_proc( *mach, extract_root_event_feature_value, args, flags, results );
            for ( list< vector< FSP > >::const_iterator it = results.begin();
                  it != results.end();
                  ++it ) {
                string category;
                vector< string > event;
                float feature_value;
                if ( ! lilfes_to_c< string >().convert( *mach, (*it)[ 0 ], category ) ) {
                    RUNWARN( "2nd argument of extract_root_event_feature_value/5 must be string" );
                    return false;
                }
                if ( ! lilfes_to_c< vector< string > >().convert( *mach, (*it)[ 1 ], event ) ) {
                    RUNWARN( "4th argument of extract_root_event_feature_value/5 must be list of strings" );
                    return false;
                }
                if ( ! lilfes_to_c< float >().convert( *mach, (*it)[ 2 ], feature_value ) ) {
                    RUNWARN( "5th argument of extract_root_event_feature_value/5 must be list of strings" );
                    return false;
                }
                string event_name;
                AmisModel::encode( category, event, feature_value, event_name );
                output_file << ' ' << event_name;
            }
        }
        return true;
    }

        //////////////////////////////////////////////////////////////////////

//     bool ForestMaker::terminalReference( FSP lexname, FSP sign, FSP sign_plus, double& prob ) {
        
//         if ( ! reference_prob_terminal ) {
//             RUNWARN( "predicate not defined: 'reference_prob_terminal/5'" );
//             return false;
//         }
//         vector< FSP > args( 5 );
//         args[ 0 ] = FSP( mach, model_name.c_str() );
//         args[ 1 ] = lexname;
//         args[ 2 ] = sign;
//         args[ 3 ] = sign_plus;
//         args[ 4 ] = FSP( mach );  // reference probability
//         core_p cutp = mach->GetCutPoint();
//         if ( ! call_proc( *mach, reference_prob_terminal, args ) ) {
//             RUNWARN( "execution of reference_prob_terminal/5 failed" );
//             return false;
//         }
//         mach->DoCut( cutp );
//         if ( ! lilfes_to_c< double >().convert( *mach, args[ 4 ], prob ) ) {
//             RUNWARN( "5th argument of reference_prob_terminal/5 must be float" );
//             return false;
//         }
//         return true;
//     }
    bool ForestMaker::unaryReference( FSP schema_name, FSP dtr, FSP mother, FSP sign_plus, double& prob ) {
        if ( ! reference_prob_unary ) {
            RUNWARN( "predicate not defined: 'reference_prob_unary/6'" );
            return false;
        }
        vector< FSP > args( 6 );
        args[ 0 ] = FSP( mach, model_name.c_str() );
        args[ 1 ] = schema_name;
        args[ 2 ] = dtr;
        args[ 3 ] = mother;
        args[ 4 ] = sign_plus;
        args[ 5 ] = FSP( mach );  // reference probability
        core_p cutp = mach->GetCutPoint();
        if ( ! call_proc( *mach, reference_prob_unary, args ) ) {
            RUNWARN( "execution of reference_prob_unary/6 failed" );
            return false;
        }
        mach->DoCut( cutp );
        if ( ! lilfes_to_c< double >().convert( *mach, args[ 5 ], prob ) ) {
            RUNWARN( "6th argument of reference_prob_unary/6 must be float" );
            return false;
        }
        return true;
    }
    bool ForestMaker::binaryReference( FSP schema_name, FSP left_dtr, FSP right_dtr, FSP mother, FSP sign_plus, double& prob ) {
        if ( ! reference_prob_binary ) {
            RUNWARN( "predicate not defined: 'reference_prob_binary/7'" );
            return false;
        }
        vector< FSP > args( 7 );
        args[ 0 ] = FSP( mach, model_name.c_str() );
        args[ 1 ] = schema_name;
        args[ 2 ] = left_dtr;
        args[ 3 ] = right_dtr;
        args[ 4 ] = mother;
        args[ 5 ] = sign_plus;
        args[ 6 ] = FSP( mach );  // reference probability
        core_p cutp = mach->GetCutPoint();
        if ( ! call_proc( *mach, reference_prob_binary, args ) ) {
            RUNWARN( "execution of reference_prob_binary/7 failed" );
            return false;
        }
        mach->DoCut( cutp );
        if ( ! lilfes_to_c< double >().convert( *mach, args[ 6 ], prob ) ) {
            RUNWARN( "7th argument of reference_prob_binary/7 must be float" );
            return false;
        }
        return true;
    }
    bool ForestMaker::rootReference( FSP sign, double& prob ) {
        if ( ! reference_prob_root ) {
            RUNWARN( "predicate not defined: 'reference_prob_root/3'" );
            return false;
        }
        vector< FSP > args( 3 );
        args[ 0 ] = FSP( mach, model_name.c_str() );
        args[ 1 ] = sign;
        args[ 2 ] = FSP( mach );  // reference probability
        core_p cutp = mach->GetCutPoint();
        if ( ! call_proc( *mach, reference_prob_root, args ) ) {
            RUNWARN( "execution of reference_prob_root/3 failed" );
            return false;
        }
        mach->DoCut( cutp );
        if ( ! lilfes_to_c< double >().convert( *mach, args[ 2 ], prob ) ) {
            RUNWARN( "4th argument of reference_prob_root/3 must be float" );
            return false;
        }
        return true;
    }

        //////////////////////////////////////////////////////////////////////

    bool ForestMaker::outputForestConjTerm( Edge* edge, Link* edge_link, int id ) {
        IPTrailStack iptrail( mach );
        FSP lexname( mach, edge_link->getLexName() );
        FSP sign( mach, edge->getSign() );
        FSP sign_plus( mach, edge_link->getSignPlus() );
        *output_stream << " ( term" << edge->getSerialNo() << '_' << id;
        if ( ! outputTerminalEvents( lexname, sign, sign_plus, *output_stream ) ) {
            return false;
        }
        *output_stream << " )";
        if ( reference_stream ) {
//              double prob = 0.0;
//              if ( ! terminalReference( lexname, sign, sign_plus, prob ) ) {
//                  return false;
//              }
            double prob = exp(edge_link->getFom());
            *reference_stream << "term" << edge->getSerialNo() << '_' << id << '\t' << prob << '\n';
        }
        return true;
    }

    bool ForestMaker::outputForestConjUnary( Edge* edge, Link* edge_link, int id ) {
        IPTrailStack iptrail( mach );
        *output_stream << " ( u" << edge->getSerialNo() << '_' << id;
        FSP schema_name( mach, edge_link->getAppliedSchema() );
        FSP dtr( mach, base_type::edge_vec[edge_link->getUDtr()].getSign() );
        FSP mother( mach, edge->getSign() );
        FSP sign_plus( mach, edge_link->getSignPlus() );
        if ( ! outputUnaryEvents( schema_name, dtr, mother, sign_plus, *output_stream ) ) {
            return false;
        }
        if ( reference_stream && reference_prob_unary ) {
            double prob = 0.0;
            if ( ! unaryReference( schema_name, dtr, mother, sign_plus, prob ) ) {
                return false;
            }
            *reference_stream << 'u' << edge->getSerialNo() << '_' << id << '\t' << prob << '\n';
        }
        if ( ! outputForestDisj( &(base_type::edge_vec[edge_link->getUDtr()]) ) ) {
            return false;
        }
        *output_stream << " )";
        return true;
    }

    bool ForestMaker::outputForestConjBinary( Edge* edge, Link* edge_link, int id ) {
        IPTrailStack iptrail( mach );
        *output_stream << " ( b" << edge->getSerialNo() << '_' << id;
        FSP schema_name( mach, edge_link->getAppliedSchema() );
        FSP left_dtr( mach, base_type::edge_vec[edge_link->getLDtr()].getSign() );
        FSP right_dtr( mach, base_type::edge_vec[edge_link->getRDtr()].getSign() );
        FSP mother( mach, edge->getSign() );
        FSP sign_plus( mach, edge_link->getSignPlus() );
        if ( ! outputBinaryEvents( schema_name, left_dtr, right_dtr, mother, sign_plus, *output_stream ) ) {
            return false;
        }
        if ( reference_stream && reference_prob_binary ) {
            double prob = 0.0;
            if ( ! binaryReference( schema_name, left_dtr, right_dtr, mother, sign_plus, prob ) ) {
                return false;
            }
            *reference_stream << 'b' << edge->getSerialNo() << '_' << id << '\t' << prob << '\n';
        }
        if ( ( ! outputForestDisj( &(base_type::edge_vec[edge_link->getLDtr()]) ) )
             || ( ! outputForestDisj( &(base_type::edge_vec[edge_link->getRDtr()]) ) ) ) {
            return false;
        }
        *output_stream << " )";
        return true;
    }

    bool ForestMaker::outputForestDisj( Edge* edge ) {
        if ( visit_mark[ edge->getSerialNo() ] ) {
                // already visited
            *output_stream << " $node" << edge->getSerialNo();
            return true;
        } else {
                // not visited yet
            visit_mark[ edge->getSerialNo() ] = true;
            vector<lserial>& edge_links = edge->getLinks();
            *output_stream << " { node" << edge->getSerialNo();
            for ( vector<lserial>::const_iterator it = edge_links.begin();
                  it != edge_links.end();
                  ++it ) {
                int id = it - edge_links.begin();
                if ( base_type::link_vec[*it].isBinary() ) {
                    if ( ! outputForestConjBinary( edge, &(base_type::link_vec[*it]), id ) ) {
                        return false;
                    }
                } else if ( base_type::link_vec[*it].isUnary() ) {
                    if ( ! outputForestConjUnary( edge, &(base_type::link_vec[*it]), id ) ) {
                        return false;
                    }
                } else if ( base_type::link_vec[*it].isTerminal() ) {
                        // terminal
                    if ( ! outputForestConjTerm( edge, &(base_type::link_vec[*it]), id ) ) {
                        return false;
                    }
                } else {
                        // ??
                    cerr << "Unknown edge type" << endl; ASSERT( 0 );
                }
            }
            *output_stream << " }";
            return true;
        }
    }

    bool ForestMaker::outputForestRoot() {
        visit_mark.clear();
        visit_mark.resize( getEdgeNumber() );
        std::vector<eserial> ee;
        getEdges(0, getSentenceLength(), ee);
        std::vector<eserial>::iterator it = ee.begin(), last = ee.end();
        IPTrailStack iptrail( mach );
        *output_stream << "{ _";
        for ( ; it != last; ++it ) {
            FSP root = FSP(mach, base_type::edge_vec[*it].getSign());
            if ( grammar->rootSign( root ) ) {
                *output_stream << " ( root" << base_type::edge_vec[*it].getSerialNo();
                if ( ! outputRootEvents( root, *output_stream ) ) {
                    return false;
                }
                if ( reference_stream && reference_prob_root ) {
                    double prob = 0.0;
                    if ( ! rootReference( root, prob ) ) {
                        return false;
                    }
                    *reference_stream << "root" << base_type::edge_vec[*it].getSerialNo() << '\t' << prob << '\n';
                }
                if ( ! outputForestDisj( &(base_type::edge_vec[*it]) ) ) {
                    return false;
                }
                *output_stream << " )";
            }
        }
        *output_stream << " }\n";
        return true;
    }

        //////////////////////////////////////////////////////////////////////

    bool ForestMaker::outputParseTreeDtrs( ParseTree parse_tree ) {
        if ( parse_tree.isTerminal() ) {
                // terminal
            FSP lexname = parse_tree.lexName();
            FSP sign = parse_tree.edgeSign();
            FSP sign_plus = parse_tree.signPlus();
            if ( ! outputTerminalEvents( lexname, sign, sign_plus, *output_stream ) ) return false;
            if ( reference_stream ) {
//                double prob = 0.0;
//                 if ( ! terminalReference( lexname, sign, sign_plus, prob ) ) {
//                     return false;
//                 }
                //const lilfes::type * t = lilfes::module::UserModule()->Search("lex_entry");
                //if (!t) {std::cerr << "error: lex_entry is not defined" << std::endl; return false;}
                //if (! lexname.GetType()->IsSubType(t) ) { std::cerr << "error: lexname is not a subtype of lex_entry" << std::endl; return false; }
                //word_type wd(lexname.Follow(lilfes::module::UserModule()->Search("LEX_WORD\\")));
                //double prob = exp(derivation_foms[wd.getPosition()]);
                FSP wordIdFSP = parse_tree.wordId();
                if (! wordIdFSP.IsInteger() || wordIdFSP.ReadInteger() < 0 || wordIdFSP.ReadInteger() >= (int) derivation_foms.size()) {
                    std::logic_error("word_id is not properly set in a leaf link");
                }
                double prob = exp( derivation_foms[ wordIdFSP.ReadInteger() ] );
                reference_probability *= prob;
            }
            return true;
        } else if ( parse_tree.isUnary() ) {
                // unary tree
            FSP mother = parse_tree.edgeSign();
            FSP sign_plus = parse_tree.signPlus();
            FSP schema_name = parse_tree.appliedSchema();
            ParseTree udtr_tree = parse_tree.UTree();
            FSP udtr_sign = udtr_tree.edgeSign();
            if ( ! outputUnaryEvents( schema_name, udtr_sign, mother, sign_plus, *output_stream ) ) return false;
            if ( reference_stream && reference_prob_unary ) {
                double prob = 0.0;
                if ( ! unaryReference( schema_name, udtr_sign, mother, sign_plus, prob ) ) {
                    return false;
                }
                reference_probability *= prob;
            }
            return outputParseTreeDtrs( udtr_tree );
        } else {
                // binary tree
            FSP mother = parse_tree.edgeSign();
            FSP sign_plus = parse_tree.signPlus();
            FSP schema_name = parse_tree.appliedSchema();
            ParseTree ldtr_tree = parse_tree.LTree();
            ParseTree rdtr_tree = parse_tree.RTree();
            FSP ldtr_sign = ldtr_tree.edgeSign();
            FSP rdtr_sign = rdtr_tree.edgeSign();
            if ( ! outputBinaryEvents( schema_name, ldtr_sign, rdtr_sign, mother, sign_plus, *output_stream ) ) return false;
            if ( reference_stream && reference_prob_binary ) {
                double prob = 0.0;
                if ( ! binaryReference( schema_name, ldtr_sign, rdtr_sign, mother, sign_plus, prob ) ) {
                    return false;
                }
                reference_probability *= prob;
            }
            return outputParseTreeDtrs( ldtr_tree ) && outputParseTreeDtrs( rdtr_tree );
        }
    }

    bool ForestMaker::outputParseTreeRoot( ParseTree parse_tree ) {
        reference_probability = 1.0;
        if ( reference_stream && reference_prob_root ) {
            double prob = 0.0;
            if ( ! rootReference( parse_tree.edgeSign(), prob ) ) {
                return false;
            }
            reference_probability *= prob;
        }
        *output_stream << "1\t";
        if ( ! outputRootEvents( parse_tree.edgeSign(), *output_stream ) ) return false;
        if ( ! outputParseTreeDtrs( parse_tree ) ) return false;
        *output_stream << '\n';
        if ( reference_stream ) {
            *reference_stream << reference_probability << '\n';
        }
        return true;
    }

    bool ForestMaker::outputParseTree( ParseTree parse_tree, ostream& output_file, ostream& reference_file ) {
        output_stream = &output_file;
        reference_stream = &reference_file;
        return outputParseTreeRoot( parse_tree );
    }
    bool ForestMaker::outputParseTree( ParseTree parse_tree, ostream& output_file ) {
        output_stream = &output_file;
        reference_stream = NULL;
        return outputParseTreeRoot( parse_tree );
    }

    bool ForestMaker::outputForest( ostream& output_file, ostream& reference_file ) {
        output_stream = &output_file;
        reference_stream = &reference_file;
        return outputForestRoot();
    }
    bool ForestMaker::outputForest( ostream& output_file ) {
        output_stream = &output_file;
        reference_stream = NULL;
        return outputForestRoot();
    }
/////////////////

    std::string getTemplateName(lilfes::FSP lexent)
    {
        static const lilfes::feature *LEX_TEMPLATE = lilfes::module::UserModule()->SearchFeature("LEX_TEMPLATE\\");
        if (LEX_TEMPLATE == 0) {
            throw std::runtime_error("cannot find LEX_TEMPLATE feature");
        }

        lilfes::FSP tmplName = lexent.Follow(LEX_TEMPLATE);
        if (! tmplName.IsValid() || ! tmplName.IsString()) {
            throw std::runtime_error("non-string value in LEX_TEMPLATE fature");
        }

        return tmplName.ReadString();
    }

    void ForestMaker::thresholdOutSumLexTemplates(std::vector<LexEntState> &lexents, const std::string &correct_tmpl, unsigned num, double width)
    {
        if (lexents.size() == 0) return;

        for (std::vector<LexEntState>::iterator it = lexents.begin(); it != lexents.end(); ++it) {
            // correct template should be always alive
            it->state = (getTemplateName(it->lexent) == correct_tmpl) ? 0 : 1;
        }

        unsigned num_alive = 1; // 1 for correct_tmpl
        double width_sum = 0.0;
        for (std::vector<LexEntState>::iterator it = lexents.begin(); it != lexents.end(); ++it) {
            if ( num > 0 && num_alive >= num ) break;
            if ( width > 0.0 && width_sum >= width ) break;
            it->state = 0;
            ++num_alive;
            width_sum += std::exp(it->fom);
        }
    }

    void ForestMaker::thresholdOutLexTemplates(std::vector<LexEntState> &lexents, const std::string &correct_tmpl,  unsigned num, double width)
    {
        if (lexents.size() == 0) return;

        for (std::vector<LexEntState>::iterator it = lexents.begin(); it != lexents.end(); ++it) {
            // correct template should be always alive
            it->state = (getTemplateName(it->lexent) == correct_tmpl) ? 0 : 1;
        }

        unsigned num_alive = 1; // 1 for correct_tmpl
        double bestfom = lexents.front().fom;
        double fom_thresh = bestfom - width;
        for (std::vector<LexEntState>::iterator it = lexents.begin(); it != lexents.end(); ++it) {
            if ( num > 0 && num_alive >= num ) break;
            if ( width > 0.0 && it->fom < fom_thresh ) break;
            it->state = 0;
            ++num_alive;
        }
    }
   
    struct SameLexTemplate {
        SameLexTemplate(const std::string& t) : _t(t) {}

        bool operator()(const LexTemplateFom &t) const { return t.tmpl == _t; }
    private:
        std::string _t;
    };

    bool ForestMaker::runSuper(const WordLattice& word_vec,
                                const std::vector<lex_template_type>& lex_template_vec) {
        P2("ForestMaker::runSuper");

        // run the supertagger
        std::vector<std::vector<LexTemplateFom> > tmplss;
        if (! super->analyze(word_vec, tmplss)) {
            std::cerr << "Superatagger failed" << std::endl;
            return false;
        }

        // add the correct lex-template if it's missing in the superatagger's output
        // save the correct template's score
        derivation_foms.resize(word_vec.size());
        for (unsigned i = 0; i < word_vec.size(); ++i) {
            SameLexTemplate correct_tmpl(lex_template_vec[i]);
            std::vector<LexTemplateFom>::const_iterator it = std::find_if(tmplss[i].begin(), tmplss[i].end(), correct_tmpl);
            if (it == tmplss[i].end()) {
                // fom = 0 for out-of-lexicon word-tmpl assoc, following the old implementation.
                // Is it OK? (2009-12-02 matuzaki)
                tmplss[i].push_back(LexTemplateFom(lex_template_vec[i], 0));
                derivation_foms[i] = 0;
            }
            else {
                derivation_foms[i] = it->fom;
            }
        }

        // make lexent lattice
        lexent_lattice.resize(word_vec.size());
        for (unsigned i = 0; i < word_vec.size(); ++i) {
            lexent_lattice[i].begin = word_vec[i].begin;
            lexent_lattice[i].end   = word_vec[i].end;
            lexent_lattice[i].word  = word_vec[i].word;
            lexent_lattice[i].tmpls.swap(tmplss[i]);
        }

        brackets.clear();
        sentence_length = ParserBasic::analyzeSentenceLength(lexent_lattice, brackets);

        // initialize word_chart
        word_chart.clear();
        word_chart.resize(sentence_length, std::vector<std::vector<unsigned> >(sentence_length + 1));
        for (unsigned word_id = 0; word_id < lexent_lattice.size(); ++word_id) {
            const LexEntExtent &lex = lexent_lattice[ word_id ];
            word_chart[ lex.begin ][ lex.end ].push_back(word_id);
        }

		return true;
    }
        
    bool ForestMaker::parseLex(const std::vector<std::string> &lex_template_vec)
    {
        P2("ForestMaker::parseLex");
        lilfes::IPTrailStack iptrail1(mach);

        static std::vector<eserial> queue;
        static std::vector<Fs*> signs;
        static std::vector<lserial> links;

        // fill leaf cells of the chart
        for (unsigned i = 0; i < lex_template_vec.size(); ++i) {

            unsigned begin = i;
            unsigned end = i + 1;
            std::vector<LexEntState>& cell = lexent_chart[begin][end];

            // thresholdOutLexSumTemplates(cell, lex_template_vec[i], num_templates_threshold, fom_templates_threshold);
            thresholdOutLexTemplates(cell, lex_template_vec[i], num_templates_threshold, fom_templates_threshold);
            queue.clear();
            signs.clear();
            links.clear();

            for (std::vector<LexEntState>::iterator it = cell.begin(); it != cell.end(); ++it) {
                if (it->state == 1) continue;
                grammar->lookupLexicalEntry(it->word_id, it->lexent, signs, links, link_vec, it->fom);
            }

            if (! storeEdge(begin, end, signs, links, queue) ) {
                return false;
            }

            if (grammar->isUnary()) {
                parseUnary(begin, end, queue);
            }
        }
        
        if (derivation_foms.size() != sentence_length) { // ?? never happens
            std::cerr << "error: inconsistent size of derivation_foms" << std::endl;
            return false;
        }

        return true;
    }

    bool ForestMaker::fullParse(const WordLattice& word_vec,
                                const std::vector<lex_template_type>& lex_template_vec)
    {
        if (! runSuper(word_vec, lex_template_vec) ) return false;
        initState();
        if (! parseLex(lex_template_vec) ) return false;
        //if (! parsePhrase(beam_start.edge_num, beam_start.edge_width, beam_start.edge_width) ) return false;
        if (! parsePhrase(0, 0, 0) ) return false;
        parse_status = SUCCESS;
        return true;
    }

} /// namespace mayz

