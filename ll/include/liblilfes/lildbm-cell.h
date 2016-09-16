/*
 * $Id: lildbm-cell.h,v 1.10 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2002-2004, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

//////////////////////////////////////////////////////////////////////
//
//  lildbm-cell.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __lildbm_cell_h
#define __lildbm_cell_h

#include "lildbm-basic.h"

#include <cstddef>
#include <vector>

namespace lilfes {

    namespace lildbm {

        template <class _Key> struct hash { };

        template <class _Tp>
        struct equal_to
        {
            bool operator()(const _Tp& __x, const _Tp& __y) const { return __x == __y; }
        };
        
        inline std::size_t __stl_hash_string(const char* __s)
        {
            unsigned long __h = 0; 
            for ( ; *__s; ++__s)
                __h = 5*__h + *__s;
  
            return std::size_t(__h);
        }

        template<> struct hash<char*>
        {
            std::size_t operator()(const char* __s) const { return __stl_hash_string(__s); }
        };

        template<> struct hash<const char*>
        {
            std::size_t operator()(const char* __s) const { return __stl_hash_string(__s); }
        };

        template<> struct hash<char> {
            std::size_t operator()(char __x) const { return __x; }
        };
        template<> struct hash<unsigned char> {
            std::size_t operator()(unsigned char __x) const { return __x; }
        };
        template<> struct hash<signed char> {
            std::size_t operator()(unsigned char __x) const { return __x; }
        };
        template<> struct hash<short> {
            std::size_t operator()(short __x) const { return __x; }
        };
        template<> struct hash<unsigned short> {
            std::size_t operator()(unsigned short __x) const { return __x; }
        };
        template<> struct hash<int> {
            std::size_t operator()(int __x) const { return __x; }
        };
        template<> struct hash<unsigned int> {
            std::size_t operator()(unsigned int __x) const { return __x; }
        };
        template<> struct hash<long> {
            std::size_t operator()(long __x) const { return __x; }
        };
        template<> struct hash<unsigned long> {
            std::size_t operator()(unsigned long __x) const { return __x; }
        };


/////////////////////
//// hash for long long int
        template <> struct hash<long long int> {
            std::size_t operator() (long long int __x) const { return __x;}
        };
        template <> struct hash<long long unsigned int> {
            std::size_t operator() (long long unsigned int __x) const { return __x;}
        };
/////////////////////
//// hash for float

        template <> struct hash<float> {
          std::size_t operator() (float __x) const { return static_cast<std::size_t>( float2int( __x ) ); }
        };
        template <> struct hash<double> {
          std::size_t operator() (double __x) const { return static_cast<std::size_t>( float2int( __x ) ); }
        };
    }

/// on file   page size = Y byte (= 16384 byte = 2^14 byte)
///                                                              DTAG - PAGE - OFFSET      PAGEMASK
/// 32bit memory X/4 cell (=1024 cell), log(X/4) bit (= 10 bit)    4     0      28  (bit)
/// 32bit file   Y/8 cell (=2048 cell), log(X/8) bit (= 11 bit)    4     49     11  (bit) 0ffffffffffff800
/// 64bit memory X/8 cell (= 512 cell), log(X/8) bit (=  9 bit)    4     0      60  (bit)
/// 64bit file   Y/8 cell (=2048 cell), log(X/8) bit (= 11 bit)    4     49     11  (bit) 0ffffffffffff800
///

////
//// ldbmmem
////

    namespace lildbm {

        typedef _HASHMAP<core_p, int, hash<core_p>, equal_to<core_p> > core_p_mark;
            // storage type
        class mem {
#ifdef CELL_64BIT
        public:
            typedef uint64 serial_type;
        public:
            typedef uint64 dcell;
            typedef uint64 dcore_p;
            typedef uint64 page_p;
            typedef uint64 pcore_p;

        public:
            static const int charlen = 7;
            static const int dcharlen = 7;
            static const int dtag_width = 4;
            static const int dtag_shift = 60;
            static const int char_shift = 56;
            static const uint64 dptr_mask = 0xc000000000000000ULL;
            static const uint64 dtag_mask = 0xf000000000000000ULL;
            static const uint64 ntag_mask = 0x0fffffffffffffffULL;
        
            static dcell cell2dcell(cell c) { return (dcell) c; };
            static cell dcell2cell(dcell c) { return (cell) c; };
#else // CELL_64BIT
                //for 32 bit architecture
        public:
            typedef uint32 serial_type;
        public:
            typedef uint32 dcell;
            typedef uint32 dcore_p;
            typedef uint32 page_p;
            typedef uint32 pcore_p;

        public:
            static const int charlen = 3;
            static const int dcharlen = 3;
            static const int dtag_width = 4;
            static const int dtag_shift = 28;
            static const int char_shift = 24;
            static const uint32 dptr_mask = 0xc0000000UL;
            static const uint32 dtag_mask = 0xf0000000UL;
            static const uint32 ntag_mask = 0x0fffffffUL;
    
            static dcell cell2dcell(cell c) { return(dcell) c; };
            static cell dcell2cell(dcell c) { return (cell) c; };
#endif // CELL_64BIT

            static dcell PTR2dc(dcore_p p)     { return ( (dcell) p); };
            static dcell STR2dc(serial<dtype<mem> > ds) { return (((dcell) ds.v) | (((dcell) DT_STR) << dtag_shift)); };
            static dcell VAR2dc(serial<dtype<mem> > ds) { return (((dcell) ds.v) | (((dcell) DT_VAR) << dtag_shift)); };
            static dcell INT2dc(mint x)        { return (((dcell) x & ntag_mask)  | (((dcell) DT_INT) << dtag_shift)); };
            static dcell FLO2dc(mfloat x)      { return (((static_cast<dcell>(float2int(x)) + ((dcell) 1 << (dtag_width-1))) >> dtag_width) | (((dcell) DT_FLO) << dtag_shift)); };
            static dcell CHR2dc(dcell x)       { return ((x & ntag_mask) | (((dcell) DT_CHR) << dtag_shift)); };
            static dcell STG2dc(int x)         { return (((dcell) x)  | (((dcell) DT_STG) << dtag_shift)); };
            static dcell NFT2dc(int x)         { return (((dcell) x)  | (((dcell) DT_NFT) << dtag_shift)); };
            static dcell FTR2dc(serial<dtype<mem> > ds) { return (((dcell) ds.v) | (((dcell) DT_FTR) << dtag_shift)); };
            static dcell SYS2dc(int x)         { return (((dcell) x)  | (((dcell) DT_SYS) << dtag_shift)); };

            static dcore_p dc2PTR(dcell c)    { return (dcore_p)     c;                               };
            static serial<dtype<mem> > dc2VAR(dcell c) { return serial<dtype<mem> >((c << dtag_width) >> dtag_width); };
            static serial<dtype<mem> > dc2STR(dcell c) { return serial<dtype<mem> >((c << dtag_width) >> dtag_width); };
            static mint dc2INT(dcell c)       { return (mint)      ((c << dtag_width) >> dtag_width); };
            static mfloat dc2FLO(dcell c)     { c = c << dtag_width; return static_cast<mfloat>(int2float(c)); };
            static dcell dc2CHR(dcell c)      { return             ((c << dtag_width) >> dtag_width); };
            static char dc2CHR0(dcell c)      { return             ((char) (c >> char_shift)); };
            static char dc2CHR1(dcell c)      { return             ((char) ((c << 8) >> char_shift)); };
            static int dc2STG(dcell c)        { return (int)       ((c << dtag_width) >> dtag_width); };
            static int dc2NFT(dcell c)        { return (int)       ((c << dtag_width) >> dtag_width); };
            static serial<dtype<mem> > dc2FTR(dcell c) { return serial<dtype<mem> >((c << dtag_width) >> dtag_width); };
            static int dc2SYS(dcell c)        { return (int)       ((c << dtag_width) >> dtag_width); };

            static bool IsDPTR(dcell c) { return (c & dptr_mask) == ((dcell)DT_PTR << dtag_shift); };
            static bool IsDSTR(dcell c) { return (c & dtag_mask) == ((dcell)DT_STR << dtag_shift); };
            static bool IsDVAR(dcell c) { return (c & dtag_mask) == ((dcell)DT_VAR << dtag_shift); };
            static bool IsDINT(dcell c) { return (c & dtag_mask) == ((dcell)DT_INT << dtag_shift); };
            static bool IsDFLO(dcell c) { return (c & dtag_mask) == ((dcell)DT_FLO << dtag_shift); };
            static bool IsDCHR(dcell c) { return (c & dtag_mask) == ((dcell)DT_CHR << dtag_shift); };
            static bool IsDSTG(dcell c) { return (c & dtag_mask) == ((dcell)DT_STG << dtag_shift); };
            static bool IsDNFT(dcell c) { return (c & dtag_mask) == ((dcell)DT_NFT << dtag_shift); };
            static bool IsDFTR(dcell c) { return (c & dtag_mask) == ((dcell)DT_FTR << dtag_shift); };

            static const dcell invalid_dcell = (((dcell) 1) << (dtag_shift+2)) -1;
            static const dcore_p invalid_dcore_p = (dcore_p) invalid_dcell;
            static bool IsInvalid(dcell c) { return (c == invalid_dcell); };

            static dcore_p mint2dcore_p(mint a0, mint a1, mint) { return (((dcore_p) a0) << dtag_width) | ((dcore_p) a1); };
            static triple<mint, mint, mint> dcore_p2mint(dcore_p dcp) { return triple<mint, mint, mint>(((mint) (dcp >> dtag_width)), ((mint) ((dcp << dtag_shift) >> dtag_shift)), 0); };

            static dtag_t DTag(dcell c) { return (dtag_t)(c >> dtag_shift); };

            typedef _HASHMAP<core_p, dcore_p, hash<core_p>, equal_to<core_p> > lcp2dcp_map;
            typedef _HASHMAP<dcore_p, core_p, hash<dcore_p>, equal_to<dcore_p> > dcp2lcp_map;
            typedef _HASHMAP<dcore_p, int, hash<dcore_p>, equal_to<dcore_p> > dcore_p_mark;


            struct equal_vcp {
                bool operator() (const std::vector<dcell> *v1, const std::vector<dcell> *v2) const {
                    return *(v1) == *(v2);
                }
            };
            struct hash_vcp {
                std::size_t operator()(const std::vector<dcell>* v) const {
                    dcell r = 0;
                    std::vector<dcell>::const_iterator it = v->begin(), last = v->end();
                    for ( ; it != last; it++)
                        r = (r << 1) ^ *it;
                    return std::size_t(r);
                }
            };
            struct equal_vc {
                bool operator() (const std::vector<dcell>& v1, const std::vector<dcell>& v2) const {
                    return v1 == v2;
                }
            };
            struct hash_vc {
                std::size_t operator()(const std::vector<dcell>& v) const {
                    dcell r = 0;
                    std::vector<dcell>::const_iterator it = v.begin(), last = v.end();
                    for ( ; it != last; it++)
                        r = (r << 1) ^ *it;
                    return std::size_t(r);
                }
            };
	
                /*        static dcore_p DDeref(dcore_p addr, DCore<mem>& core);
                              // IHeap2Core is used for store the data to the storage
                              static void LCore2DCore(core_p, DCore<mem>&, lcp2dcp_map&, DbTypeMapper*, DbFeatureMapper*);
                              static void DCore2LCore(dcore_p, DCore<mem>&, dcp2lcp_map&, DbTypeMapper*, DbFeatureMapper*);
                                  // IHeap2DHeap is used for hashing FS key
                                  static void LCore2DVec(core_p, std::vector<dcell>&, DbTypeMapper*, DbFeatureMapper*);*/
                /*	static void __Serialize(machine* mach, core_p addr, std::vector<dcell>& vec, lcp2dcp_map& mapping);
                        static void Serialize(machine* mach, core_p addr, std::vector<dcell>& vec);*/

        };
    }

////
//// ldbmbdb
////

    namespace lildbm {
        class bdb {
#ifdef CELL_64BIT
        public:
            typedef uint64 serial_type;
        public:
            typedef uint64 dcell;
            typedef uint64 dcore_p;
            typedef uint64 page_p;
            typedef uint64 pcore_p;

        public:
            static const int charlen = 7;
            static const int dcharlen = 7;
            static const int dtag_width = 4;
            static const int dtag_shift = 60;
            static const int page_size = 2048;
            static const int page_width = 49;
            static const int offset_width = 11;
            static const int char_shift = 56;
            static const uint64 dtag_mask = 0xf000000000000000ULL;
            static const uint64 ntag_mask = 0x0fffffffffffffffULL;
            static const uint64 dptr_mask = 0xc000000000000000ULL;
            static const uint64 page_mask = 0x0ffffffffffff800ULL;
            static const uint64 offset_mask = 0x00000000000007ffULL;
    
            static dcell cell2dcell(cell c) { return (dcell) c; };
            static cell dcell2cell(dcell c) { return (cell) c; };
            static dcore_p mint2dcore_p(mint a0, mint a1, mint) { return (((dcore_p) a0) << dtag_width) | ((dcore_p) a1); };
            static triple<mint, mint, mint> dcore_p2mint(dcore_p dcp) { return triple<mint, mint, mint>(((mint) (dcp >> dtag_width)), ((mint) ((dcp << dtag_shift) >> dtag_shift)), 0); };

#else // CELL_64BIT
                // for 32bit architecture
        public:
            typedef uint64 serial_type;
        public:
            typedef uint64 dcell;
            typedef uint64 dcore_p;
            typedef uint64 page_p;
            typedef uint32 pcore_p;

        public:
            static const int charlen = 3;
            static const int dcharlen = 7;
            static const int dtag_width = 4;
            static const int dtag_shift = 60;
            static const int page_size = 2048;
            static const int page_width = 49;
            static const int offset_width = 11;
            static const int char_shift = 56;
            static const uint64 dtag_mask = 0xf000000000000000ULL;
            static const uint64 ntag_mask = 0x0fffffffffffffffULL;
            static const uint64 dptr_mask = 0xc000000000000000ULL;
            static const uint64 page_mask = 0x0ffffffffffff800ULL;
            static const uint64 offset_mask = 0x00000000000007ffULL;

            static dcell cell2dcell(cell c) {
                if(IsFLO(c)) {
                    cell_int tmp = cell2int(c) << TAG_WIDTH;
                    double g = static_cast<double>(int2float(tmp));
                    return (((static_cast<dcell>(float2int(g)) + ((dcell) 1 << (dtag_width-1))) >> dtag_width) | ((dcell)DT_FLO << dtag_shift));
                }
                return ((((dcell) ((TAG_MASK & cell2int(c))>>TAG_SHIFT)) << dtag_shift) | ((dcell) (NTAG_MASK & cell2int(c))));
            };
            static cell dcell2cell(dcell c) {
                if(IsDFLO(c)) {
                    c = c << dtag_width;
                    mfloat f = static_cast<mfloat>(int2float(c));
                    return FLO2c(f);
                }
                return (cell)((((dtag_mask & c) >> dtag_shift) << TAG_SHIFT) | (0x000000000fffffff & c));
            };
            static dcore_p mint2dcore_p(mint a0, mint a1, mint a2) { return (((dcore_p) a0) << 36) | (((dcore_p) a1) << 8) | ((dcore_p) a2); };
            static triple<mint, mint, mint> dcore_p2mint(dcore_p dcp) { return triple<mint, mint, mint>(((mint) (dcp >> 36)), ((mint) ((dcp << 28) >> 36)), ((mint) ((dcp << 56) >> 56))); };
#endif // CELL_64BIT

            static dcell PTR2dc(dcore_p p)     { return ((dcell) p); };
            static dcell STR2dc(serial<dtype<bdb> > ds) { return (((dcell) ds.v) | (((dcell) DT_STR) << dtag_shift)); };
            static dcell VAR2dc(serial<dtype<bdb> > ds) { return (((dcell) ds.v) | (((dcell) DT_VAR) << dtag_shift)); };
            static dcell INT2dc(mint x)        { return (((dcell) x & ntag_mask)  | (((dcell) DT_INT) << dtag_shift)); };
            static dcell FLO2dc(double x)        { return ((( static_cast< dcell >( float2int( static_cast< mfloat >( x ) ) ) + ((dcell) 1 << (dtag_width-1))) >> dtag_width) | ((dcell) DT_FLO << dtag_shift)); };
            static dcell CHR2dc(dcell x)       { return ((x & ntag_mask) | (((dcell) DT_CHR) << dtag_shift)); };
            static dcell STG2dc(int x)         { return (((dcell) x)  | (((dcell) DT_STG) << dtag_shift)); };
            static dcell NFT2dc(int x)         { return (((dcell) x)  | (((dcell) DT_NFT) << dtag_shift)); };
            static dcell FTR2dc(serial<dtype<bdb> > ds) { return (((dcell) ds.v) | (((dcell) DT_FTR) << dtag_shift)); };
            static dcell SYS2dc(int x)         { return (((dcell) x)  | (((dcell) DT_SYS) << dtag_shift)); };


            static dcore_p dc2PTR(dcell c)    { return (dcore_p)     c;                               };
            static serial<dtype<bdb> > dc2VAR(dcell c) { return serial<dtype<bdb> >((c << dtag_width) >> dtag_width); };
            static serial<dtype<bdb> > dc2STR(dcell c) { return serial<dtype<bdb> >((c << dtag_width) >> dtag_width); };
            static mint dc2INT(dcell c)       { return (mint)      ((c << dtag_width) >> dtag_width); };
            static double dc2FLO(dcell c)     { c = c << dtag_width; return static_cast< double >( int2float( static_cast< dcell >( c ) ) ); }
            static dcell dc2CHR(dcell c)      { return             ((c << dtag_width) >> dtag_width); };
            static char dc2CHR0(dcell c)      { return             ((char) (c >> char_shift)); };
            static char dc2CHR1(dcell c)      { return             ((char) ((c << 8) >> char_shift)); };
            static int dc2STG(dcell c)        { return (int)       ((c << dtag_width) >> dtag_width); };
            static int dc2NFT(dcell c)        { return (int)       ((c << dtag_width) >> dtag_width); };
            static serial<dtype<bdb> > dc2FTR(dcell c) { return serial<dtype<bdb> >((c << dtag_width) >> dtag_width); };
            static int dc2SYS(dcell c)        { return (int)       ((c << dtag_width) >> dtag_width); };

            static bool IsDPTR(dcell c) { return (c & dptr_mask) == ((dcell)DT_PTR << dtag_shift); };
            static bool IsDSTR(dcell c) { return (c & dtag_mask) == ((dcell)DT_STR << dtag_shift); };
            static bool IsDVAR(dcell c) { return (c & dtag_mask) == ((dcell)DT_VAR << dtag_shift); };
            static bool IsDINT(dcell c) { return (c & dtag_mask) == ((dcell)DT_INT << dtag_shift); };
            static bool IsDFLO(dcell c) { return (c & dtag_mask) == ((dcell)DT_FLO << dtag_shift); };
            static bool IsDCHR(dcell c) { return (c & dtag_mask) == ((dcell)DT_CHR << dtag_shift); };
            static bool IsDSTG(dcell c) { return (c & dtag_mask) == ((dcell)DT_STG << dtag_shift); };
            static bool IsDNFT(dcell c) { return (c & dtag_mask) == ((dcell)DT_NFT << dtag_shift); };
            static bool IsDFTR(dcell c) { return (c & dtag_mask) == ((dcell)DT_FTR << dtag_shift); };

            static const dcell invalid_dcell = (((dcell) 1) << (dtag_shift+2)) -1;
            static const dcore_p invalid_dcore_p = (dcore_p) invalid_dcell;
            static bool IsInvalid(dcell c) { return (c == invalid_dcell); };

            static page_p getPage(dcore_p p) { return (p & page_mask) >> offset_width; };
            static pcore_p getOffset(dcore_p p) { return (p & offset_mask); };
            static dcore_p DCoreP(page_p p, pcore_p o) { return ((p << offset_width) + o); };

            static dtag_t DTag(dcell c) { return (dtag_t)(c >> dtag_shift); };

            typedef _HASHMAP<core_p, dcore_p, hash<core_p>, equal_to<core_p> > lcp2dcp_map;
            typedef _HASHMAP<dcore_p, core_p, hash<dcore_p>, equal_to<dcore_p> > dcp2lcp_map;
            typedef _HASHMAP<dcore_p, int, hash<dcore_p>, equal_to<dcore_p> > dcore_p_mark;

            struct equal_vcp {
                bool operator() (const std::vector<dcell> *v1, const std::vector<dcell> *v2) const {
                    return *(v1) == *(v2);
                }
            };
            struct hash_vcp {
                std::size_t operator()(const std::vector<dcell>* v) const {
                    dcell r = 0;
                    std::vector<dcell>::const_iterator it = v->begin(), last = v->end();
                    for ( ; it != last; it++)
                        r = (r << 1) ^ *it;
                    return std::size_t(r);
                }
            };
            struct equal_vc {
                bool operator() (const std::vector<dcell>& v1, const std::vector<dcell>& v2) const {
                    return v1 == v2;
                }
            };
            struct hash_vc {
                std::size_t operator()(const std::vector<dcell>& v) const {
                    dcell r = 0;
                    std::vector<dcell>::const_iterator it = v.begin(), last = v.end();
                    for ( ; it != last; it++)
                        r = (r << 1) ^ *it;
                    return std::size_t(r);
                }
            };
                /*
                  static dcore_p DDeref(dcore_p addr, DCore<bdb>& core);

                      // IHeap2Core is used for store the data to the storage
                      static void LCore2DCore(core_p, DCore<bdb>&, lcp2dcp_map&, DbTypeMapper*, DbFeatureMapper*);
                      static void DCore2LCore(dcore_p, DCore<bdb>&, dcp2lcp_map&, DbTypeMapper*, DbFeatureMapper*);
                          // IHeap2DHeap is used for hashing FS key
                          void LCore2DVec(core_p, std::vector<dcell>&, DbTypeMapper*, DbFeatureMapper*);*/
                /*	static void __Serialize(DbTypeMapper<bdb, mem>*, machine*, core_p, std::vector<dcell>&, lcp2dcp_map&);
                        static void Serialize(DbTypeMapper<bdb, mem>*, machine*, core_p, std::vector<dcell>&);*/
        };


    } //end of namespace lildbm
} // end of namespace lilfes
#endif // __lildbm_cell_h
// end of lildbm-cell.h

/*
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2008/12/28 08:51:12  yusuke
 *
 * support for gcc 4.3
 *
 * Revision 1.8  2006/09/29 03:40:25  yusuke
 *
 * merge "cell-64bit" branch
 *
 * Revision 1.7.2.1  2006/09/29 01:57:18  yusuke
 *
 * support for 64bit cell
 *
 * Revision 1.7  2006/05/22 05:45:01  yusuke
 *
 * fix for gcc 4.1
 *
 * Revision 1.6  2006/05/21 11:31:47  yusuke
 *
 * fix type-punning bug
 *
 * Revision 1.5  2006/01/15 10:50:21  ninomi
 * warningを消しましたが、実はbugになっていて、あまり使われたくないfloat
 * のhash をなおしました。
 *
 * Revision 1.4  2004/08/24 10:20:15  ninomi
 * DBにstringとして日本語(charで負の数)をいれられないばぐをとりました。
 *
 * Revision 1.3  2004/08/20 15:24:51  ninomi
 * key_integer, key_inthash, array_value_integerに負の値があるとdb_saveできない
 * ばぐをのぞきました。
 *
 * Revision 1.2  2004/04/14 04:52:50  ninomi
 * lildbmのfirst versionです。
 *
 *
 */
