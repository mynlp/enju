#ifndef __utility_h
#define __utility_h

#include "lconfig.h"
#include "builtin.h"
#include <cassert>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace lilfes {

//////////////////////////////////////////////////////////////////////
////
////  calling predicates from a C program
////
//////////////////////////////////////////////////////////////////////

procedure* prepare_proc(const type* type);
procedure* prepare_proc(const type* type, size_t nargs);
procedure* prepare_proc(const module* mod, const std::string& name, size_t nargs);
bool call_proc(machine& mach, procedure* proc, std::vector<FSP>& args);
void findall_proc(machine& mach, procedure* proc, const std::vector<FSP> & args, const std::vector<bool> & output_flags, std::list<std::vector<FSP> > & results);

//////////////////////////////////////////////////////////////////////
////
////  Loading LiLFeS program
////
//////////////////////////////////////////////////////////////////////

module* load_module( machine& mach, const std::string& title, const std::string& module_name, module* parent_module = module::UserModule() );

int pushfile(machine *m, const std::string &fname, int pi, const std::string &prefix = "");

//////////////////////////////////////////////////////////////////////
////
////  conversion between list and vector
////
//////////////////////////////////////////////////////////////////////

template < class T >
class c_to_lilfes {
public:
  bool convert( machine&, T c, FSP fsp ) {
    return fsp.Unify( c );
  }
};

template <>
class c_to_lilfes< int > {
public:
  bool convert( machine&, int c, FSP fsp ) {
    return fsp.Unify( static_cast<mint>( c ) );
  }
};

template <>
class c_to_lilfes< float > {
public:
  bool convert( machine&, float c, FSP fsp ) {
    return fsp.Unify( static_cast<mfloat>( c ) );
  }
};

template <>
class c_to_lilfes< double > {
public:
  bool convert( machine&, double c, FSP fsp ) {
    return fsp.Unify( static_cast<mfloat>( c ) );
  }
};

template <>
class c_to_lilfes< std::vector< cell > > {
public:
  bool convert( machine& mach, const std::vector< cell >& vec, FSP fsp ) {
    FSP f( mach, vec );
    return fsp.Unify( f );
  }
};

template < class T >
class c_to_lilfes< std::vector< T > > {
public:
  bool convert( machine& mach, const std::vector< T >& vec, FSP fsp ) {
    for ( typename std::vector< T >::const_iterator it = vec.begin();
          it != vec.end();
          ++it ) {
      FSP v( mach );
      if ( fsp.Coerce( cons )
           && c_to_lilfes< T >().convert( mach, *it, v )
           && fsp.Follow( hd ).Unify( v ) ) {
        fsp = fsp.Follow( tl );
        continue;
      } else {
        return false;
      }
    }
    return fsp.Coerce( nil );
  }
};

template < class T1, class T2 >
class c_to_lilfes< std::pair< T1, T2 > > {
public:
  bool convert( machine& mach, const std::pair< T1, T2 >& pr, FSP fsp ) {
    FSP p1( mach );
    FSP p2( mach );
    return fsp.Coerce( t_comma )
      && c_to_lilfes< T1 >().convert( mach, pr.first, p1 )
      && fsp.Follow( f_arg[ 1 ] ).Unify( p1 )
      && c_to_lilfes< T2 >().convert( mach, pr.second, p2 )
      && fsp.Follow( f_arg[ 2 ] ).Unify( p2 );
  }
};

template < class T >
bool vector_to_list( machine& mach,
                     const std::vector< T >& vec,
                     FSP fsp ) {
  return c_to_lilfes< std::vector< T > >().convert( mach, vec, fsp );
}

// template < class T >
// bool vector_to_list( machine& mach,
//                      const std::vector< T >& vec,
//                      FSP fsp ) {
//   FSP list( mach );
//   FSP root( list );
//   for ( typename std::vector< T >::const_iterator it = vec.begin();
// 	it != vec.end();
// 	++it ) {
//     list.Coerce( cons );
//     list.Follow( hd ).Unify( *it );
//     list = list.Follow( tl );
//   }
//   list.Coerce( nil );
//   return fsp.Unify( root );
// }

//////////////////////////////////////////////////////////////////////

template < class T >
class lilfes_to_c {
public:
  bool convert( machine&, FSP fsp, T& c );
};

template <>
inline bool lilfes_to_c< FSP >::convert( machine&, FSP arg1, FSP& arg2 ) {
  arg2 = arg1;
  return true;
}

template <>
inline bool lilfes_to_c< std::string >::convert( machine&, FSP fsp, std::string& s ) {
  if ( ! fsp.IsString() ) return false;
  s = fsp.ReadString();
  return true;
}

template <>
inline bool lilfes_to_c< const char* >::convert( machine&, FSP fsp, const char*& s ) {
  if ( ! fsp.IsString() ) return false;
  s = fsp.ReadString();
  return true;
}

template <>
inline bool lilfes_to_c< int >::convert( machine&, FSP fsp, int& i ) {
  if ( ! fsp.IsInteger() ) return false;
  i = fsp.ReadInteger();
  return true;
}

template <>
inline bool lilfes_to_c< float >::convert( machine&, FSP fsp, float& f ) {
  mfloat x = 0.0;
  if ( fsp.IsInteger() ) {
    x = fsp.ReadInteger();
    f = static_cast< float >( x );
    return true;
  }
  if ( fsp.IsFloat() ) {
    x = fsp.ReadFloat();
    f = static_cast< float >( x );
    return true;
  }
  return false;
}

template <>
inline bool lilfes_to_c< double >::convert( machine&, FSP fsp, double& f ) {
  mfloat x = 0.0;
  if ( fsp.IsInteger() ) {
    x = fsp.ReadInteger();
    f = static_cast< double >( x );
    return true;
  }
  if ( fsp.IsFloat() ) {
    x = fsp.ReadFloat();
    f = static_cast< double >( x );
    return true;
  }
  return false;
}

template < class T >
class lilfes_to_c< std::vector< T > > {
public:
  bool convert( machine& mach, FSP fsp, std::vector< T >& vec ) {
    if ( ! fsp.GetType()->IsSubType( lilfes::t_list ) ) return false;
    while ( fsp.GetType() == cons ) {
      FSP head = fsp.Follow( hd );
      vec.push_back( T() );
      if ( ! lilfes_to_c< T >().convert( mach, head, vec.back() ) ) return false;
      fsp = fsp.Follow( tl );
    }
    return true;
  }
};

template <>
inline bool lilfes_to_c< std::vector< cell > >::convert( machine&, FSP fsp, std::vector< cell >& f ) {
  fsp.Serialize( f );
  return true;
}

// template <>
// bool lilfes_to_c< FSP >::convert( machine&, FSP, FSP& );

// template <>
// bool lilfes_to_c< std::string >::convert( machine&, FSP, std::string& );

// template <>
// bool lilfes_to_c< const char* >::convert( machine&, FSP, const char*& );

// template <>
// bool lilfes_to_c< int >::convert( machine&, FSP, int& );

// template <>
// bool lilfes_to_c< float >::convert( machine&, FSP, float& );

// template <>
// bool lilfes_to_c< std::vector< cell > >::convert( machine&, FSP, std::vector< cell >& );


template < class T1, class T2 >
class lilfes_to_c< std::pair< T1, T2 > > {
public:
  bool convert( machine& mach, FSP fsp, std::pair< T1, T2 >& pr ) {
    if ( ! fsp.GetType()->IsSubType( lilfes::t_comma ) ) return false;
    return lilfes_to_c< T1 >().convert( mach, fsp.Follow( f_arg[ 1 ] ), pr.first )
      && lilfes_to_c< T2 >().convert( mach, fsp.Follow( f_arg[ 2 ] ), pr.second );
  }
};

template < class T >
bool list_to_vector( machine& mach,
                     FSP fsp,
                     std::vector< T >& vec ) {
  return lilfes_to_c< std::vector< T > >().convert( mach, fsp, vec );
}

// template < class T >
// bool list_to_vector( machine& mach,
//                      FSP fsp,
//                      std::vector< T >& vec );

// template <>
// bool list_to_vector< FSP >( machine&,
//                             FSP fsp,
//                             std::vector< FSP >& vec );

// template <>
// bool list_to_vector< std::string >( machine&,
//                                     FSP fsp,
//                                     std::vector< std::string >& vec );

// template <>
// bool list_to_vector< int >( machine&,
//                             FSP fsp,
//                             std::vector< int >& vec );

// template <>
// bool list_to_vector< float >( machine&,
//                               FSP fsp,
//                               std::vector< float >& vec );

//////////////////////////////////////////////////////////////////////
////
////  push/pop IP and TrailPoint
////
//////////////////////////////////////////////////////////////////////

  class IPTrailStack {
  private:
    machine* mach;
    code* IP;
    core_p trailp;
  public:
    IPTrailStack( machine* m ) : mach( m ), IP( m->GetIP() ), trailp( m->SetTrailPoint( NULL ) ) {}
    ~IPTrailStack() {
      mach->TrailBack( trailp );
      mach->SetIP( IP );
    }
  };

} // namespace lilfes

#endif // __utility_h
