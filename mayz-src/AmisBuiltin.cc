#include <memory>
#include <liblilfes/lconfig.h>
#include <liblilfes/builtin.h>
#include <liblilfes/bfstream.h>
#include <liblilfes/gfstream.h>
#include <liblilfes/utility.h>

#include "AmisModel.h"

namespace mayz {
  AmisHandler amis_handler;
}

// To link built-in predicates
namespace lilfes {
    bool amis_encode_feature(machine&, FSP, FSP, FSP);

    void *AMIS_BUILTIN_PRED_SYMBOLS[] = {
        (void*)(amis_encode_feature),
        NULL
    };
}

using namespace std;
using namespace mayz;

//////////////////////////////////////////////////////////////////////

// built-in predicates

namespace lilfes {
    bool amis_encode_feature( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of amis_encode_feature must be a string" );
            return false;
        }
        vector< string > features;
        if ( ! list_to_vector< string >( m, arg2, features ) ) {
            RUNERR( "arg2 of amis_encode_feature must be a list of strings" );
            return false;
        }
        string name;
        AmisModel::encode( arg1.ReadString(), features, name );
        return arg3.Unify( FSP( m, name.c_str() ) );
    }

    bool amis_decode_feature( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of amis_decode_feature must be a string" );
            return false;
        }
        string category;
        vector< string > features;
        AmisModel::decode( arg1.ReadString(), category, features );
        return arg2.Unify( FSP( m, category.c_str() ) )
            && vector_to_list( m, features, arg3 );
    }

    //////////////////////////////////////////////////////////////////////

    bool new_amis_model( machine&, FSP arg1 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of new_amis_model must be a string" );
            return false;
        }
        return amis_handler.newAmisModel( arg1.ReadString() ) != NULL;
    }

    bool delete_amis_model( machine&, FSP arg1 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of delete_amis_model must be a string" );
            return false;
        }
        amis_handler.deleteAmisModel( arg1.ReadString() );
        return true;
    }

    bool add_feature_mask( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of add_feature_mask must be a string" );
            return false;
        }
        if ( ! arg2.IsString() ) {
            RUNERR( "arg2 of add_feature_mask must be a string" );
        }
        std::vector< int > mask;
        if ( ! list_to_vector< int >( m, arg3, mask ) ) {
            RUNERR( "arg3 must be a list of integers" );
            return false;
        }
        AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
        if ( amis_model == NULL ) {
            RUNERR( "amis model not found: " << arg1.ReadString() );
            return false;
        }
        return amis_model->addFeatureMask( arg2.ReadString(), mask );
    }

    bool import_amis_model( machine&, FSP arg1, FSP arg2 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of import_amis_model must be string" );
            return false;
        }
        if ( ! arg2.IsString() ) {
            RUNERR( "arg2 of import_amis_model must be a file name" );
            return false;
        }
        AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
        if ( amis_model == NULL ) {
            RUNERR( "amis model not found: " << arg1.ReadString() );
            return false;
        }
        std::string filename = arg2.ReadString();
        auto_ptr< istream > model_file( NULL );
        if ( filename.substr( filename.size() - 4, std::string::npos ) == ".bz2" ) {
            model_file.reset( new ibfstream( filename.c_str() ) );
        } else if ( filename.substr( filename.size() - 3, std::string::npos ) == ".gz" ) {
            model_file.reset( new igfstream( filename.c_str() ) );
        } else {
            model_file.reset( new ifstream( filename.c_str() ) );
        }
        if ( ! *model_file ) {
            RUNERR( "cannot open a model file: " << filename );
            return false;
        }
        return amis_model->importModel( *model_file );
    }

    bool import_feature_masks( machine& mach, FSP arg1 ) {
        static procedure* feature_mask_proc = prepare_proc( module::UserModule(), "feature_mask", 3 );
        if ( ! feature_mask_proc ) {
            RUNERR( "feature_mask/3 is not defined" );
            return false;
        }
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of import_feature_masks must be a model name" );
            return false;
        }
        AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
        if ( amis_model == NULL ) {
            RUNERR ( "amis model not found: " << arg1.ReadString() );
            return false;
        }
        vector< FSP > args( 3 );
        args[ 0 ] = arg1;
        args[ 1 ] = FSP( mach );  // category
        args[ 2 ] = FSP( mach );  // mask
        vector< bool > flags( 3, true );
        flags[ 0 ] = false;
        list< vector< FSP > > results;
        findall_proc( mach, feature_mask_proc, args, flags, results );
        for ( list< vector< FSP > >::const_iterator result_it = results.begin();
                result_it != results.end();
                ++result_it ) {
            if ( ! (*result_it)[ 0 ].IsString() ) {
                RUNERR( "arg2 of feature_mask/3 must be a string" );
                return false;
            }
            std::vector< int > mask;
            if ( ! list_to_vector< int >( mach, (*result_it)[ 1 ], mask ) ) {
                RUNERR( "arg3 of feature_mask/3 must be a list of integers" );
                return false;
            }
            if ( ! amis_model->addFeatureMask( (*result_it)[ 0 ].ReadString(), mask ) ) {
                return false;
            }
        }
        return true;
    }

    bool init_amis_model( machine& mach, FSP arg1, FSP arg2 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of init_amis_model/2 must be a string" );
            return false;
        }
        if ( ! arg2.IsString() ) {
            RUNERR( "arg2 of init_amis_model/2 must be a string" );
        }
        return new_amis_model( mach, arg1 )
            && import_amis_model( mach, arg1, arg2 )
            && import_feature_masks( mach, arg1 );
    }

    // bool amis_feature_list( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
    //     if ( ! arg1.IsString() ) {
    //         RUNERR( "arg1 of amis_feture_list/3 must be a string (model name)" );
    //         return false;
    //     }
    //     if ( ! arg2.IsString() ) {
    //         RUNERR( "arg2 of amis_feature_list/3 must be a string (event)" );
    //         return false;
    //     }
    //     AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
    //     string category;
    //     vector< AmisFeature > features;
    //     if ( ! amis_model->extractFeatures( arg2.ReadString(), features, category ) ) {
    //         return false;
    //     }
    //     vector< string > ret;
    //     for ( vector< AmisFeature >::const_iterator it = features.begin();
    //             it != features.end();
    //             ++it ) {
    //         ret.push_back( amis_model->featureString( category, *it ) );
    //     }
    //     return c_to_lilfes< vector< string > >().convert( m, ret, arg3 );
    // }

    bool amis_feature_list_4( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of amis_feature_list/4 must be a string (model name)" );
        }
        if ( ! arg2.IsString() ) {
            RUNERR( "arg2 of amis_feature_list/4 must be a string (category)" );
        }
        vector< string > event;
        if ( ! list_to_vector< string >( m, arg3, event ) ) {
            RUNERR( "arg3 of amis_feature_list/4 must be a list of strings (event)" );
        }
        AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
        if ( amis_model == NULL ) return false;
        vector< AmisFeature > features;
        if ( ! amis_model->extractFeatures( arg2.ReadString(), event, features ) ) {
            return false;
        }
        vector< string > ret;
        for ( vector< AmisFeature >::const_iterator it = features.begin();
                it != features.end();
                ++it ) {
            ret.push_back( amis_model->featureString( arg2.ReadString(), *it ) );
        }
        return c_to_lilfes< vector< string > >().convert( m, ret, arg4 );
    }

    // bool amis_feature_weight( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
    //     if ( ! arg1.IsString() ) {
    //         RUNERR( "arg1 of amis_feature_weight must be string (model name)" );
    //         return false;
    //     }
    //     if ( ! arg2.IsString() ) {
    //         RUNERR( "arg2 of amis_feature_weight must be string (feature)" );
    //         return false;
    //     }
    //     AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
    //     if ( amis_model == NULL ) return false;
    //     string feature = arg2.ReadString();
    //     double weight = 0.0;
    //     if ( ! amis_model->featureWeight( feature, weight ) ) return false;
    //     return arg3.Unify( FSP( m, (mfloat) weight ) );
    // }

    // bool amis_event_weight( machine& m, FSP arg1, FSP arg2, FSP arg3 ) {
    //     if ( ! arg1.IsString() ) {
    //         RUNERR( "arg1 of amis_event_weight/3 must be string" );
    //         return false;
    //     }
    //     if ( ! arg2.IsString() ) {
    //         RUNERR( "arg2 of amis_event_weight/3 must be a list of strings" );
    //         return false;
    //     }
    //     AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
    //     if ( amis_model == NULL ) return false;
    //     double weight = amis_model->eventWeight( arg2.ReadString() );
    //     return arg3.Unify( FSP( m, (mfloat) weight ) );
    // }

    bool amis_event_weight_4( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of amis_event_weight/4 must be string" );
            return false;
        }
        if ( ! arg2.IsString() ) {
            RUNERR( "arg2 of amis_event_weight/4 must be string" );
            return false;
        }
        vector< string > event;
        if ( ! list_to_vector< string >( m, arg3, event ) ) {
            RUNERR( "arg3 of amis_event_weight/4 must be list of strings" );
            return false;
        }
        AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
        if ( amis_model == NULL ) return false;
        double weight = amis_model->eventWeight( arg2.ReadString(), event );
        return arg4.Unify( FSP( m, (mfloat) weight ) );
    }

    bool amis_log_probability( machine& m, FSP arg1, FSP arg2, FSP arg3, FSP arg4 ) {
        if ( ! arg1.IsString() ) {
            RUNERR( "arg1 of amis_log_probability must be a string" );
            return false;
        }
        if ( ! arg2.IsString() ) {
            RUNERR( "arg2 of amis_log_probability must be a string" );
            return false;
        }
        vector< vector< string > > event_list;
        if ( ! lilfes_to_c< vector< vector< string > > >().convert( m, arg3, event_list ) ) {
            RUNERR( "arg3 of amis_log_probability must be a list of lists of strings" );
            return false;
        }
        AmisModel* amis_model = amis_handler.getAmisModel( arg1.ReadString() );
        if ( amis_model == NULL ) return false;
        vector< float > weight_list;
        double sum = 0;
        vector< vector< string > >::const_iterator event = event_list.begin();
        for ( ; event != event_list.end(); ++event ) {
            double weight = amis_model->eventWeight( arg2.ReadString(), *event );
            weight_list.push_back( weight );
            sum += exp( weight );
        }
        sum = log( sum );
        for ( vector< float >::iterator weight = weight_list.begin();
                weight != weight_list.end();
                ++weight ) {
            (*weight) -= sum;
        }
        return vector_to_list( m, weight_list, arg4 );
    }

    LILFES_BUILTIN_PRED_3( amis_encode_feature, amis_encode_feature );
    LILFES_BUILTIN_PRED_3( amis_decode_feature, amis_decode_feature );
    LILFES_BUILTIN_PRED_1( new_amis_model, new_amis_model );
    LILFES_BUILTIN_PRED_1( delete_amis_model, delete_amis_model );
    LILFES_BUILTIN_PRED_3( add_feature_mask, add_feature_mask );
    LILFES_BUILTIN_PRED_2( import_amis_model, import_amis_model );
    LILFES_BUILTIN_PRED_1( import_feature_masks, import_feature_masks );
    LILFES_BUILTIN_PRED_2( init_amis_model, init_amis_model );
  //LILFES_BUILTIN_PRED_3( amis_feature_list, amis_feature_list );
  //LILFES_BUILTIN_PRED_OVERLOAD_4( amis_feature_list_4, amis_feature_list_4, amis_feature_list );
    LILFES_BUILTIN_PRED_4( amis_feature_list_4, amis_feature_list );
  //LILFES_BUILTIN_PRED_3( amis_feature_weight, amis_feature_weight );
  //LILFES_BUILTIN_PRED_3( amis_event_weight, amis_event_weight );
  //LILFES_BUILTIN_PRED_OVERLOAD_4( amis_event_weight_4, amis_event_weight_4, amis_event_weight );
    LILFES_BUILTIN_PRED_4( amis_event_weight_4, amis_event_weight );
    LILFES_BUILTIN_PRED_4( amis_log_probability, amis_log_probability );
}
