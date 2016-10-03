/*
 * $Id: ldbm-cell.h,v 1.5 2011-05-02 08:48:58 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#ifndef __ldbm_cell_h
#define __ldbm_cell_h

#include "../pstl/pstl_pheap.h"
#include "../pstl/pstl_vector.h"
#include "../pstl/pstl_hash_map.h"

#include <iostream>
#include <ostream>

namespace ldbm {

        /////// mint   is int32 (32bit) on 32bit arch, or int64  (64bit) on 64bit arch
        /////// mfloat is float (32bit) on 32bit arch, or double (64bit) on 64bit arch
    
    enum tag_t { 
        T_PTR  =0,/// ポインタ
        T_PTR1 =1,
        T_PTR2 =2,
        T_PTR3 =3,
        T_STR  =4,/// structure
        T_FSTR =5,/// freeze structure
        T_VAR  =6,/// 変数
        T_FVAR =7,/// freeze 変数
        T_INT  =8,/// 整数
        T_FLO  =9,/// 実数
        T_CHR =10,/// 文字
        T_STG =11,/// 文字列
        T_NFT =12,/// 素性数
        T_FTR =13,/// 素性
        T_SYS =14 /// システム用データ
    };
    
    template <class _Bit> class cell {};

    template <> class cell<pstl::b32> {
    public:
            // typedef
        typedef pstl::b32 bit_type;
	typedef bit_type::size_type size_type;
        typedef bit_type::size_type word_type;
        typedef bit_type::size_type tserial;
        typedef bit_type::size_type fserial;
        typedef bit_type::size_type core_p;
        typedef bit_type::mint mint;
        typedef bit_type::mfloat mfloat;
        
            // class data
        static const int charlen = 3;
        static const int tag_width = 4;
        static const int tag_shift = 28;
        static const int char_shift = 24;
        static const word_type ptr_mask = 0xc0000000UL;
        static const word_type tag_mask = 0xf0000000UL;
        static const word_type ntag_mask = 0x0fffffffUL;
        static const word_type invalid_word = (((word_type) 1) << (tag_shift+2)) -1;
        static const core_p invalid_core_p = (core_p) invalid_word;

            // data
        word_type word;

        cell() {}
        cell(const cell& c) : word(c.word) {}
        cell(word_type w) : word(w) {}
        ~cell() {}
    };

    template <> class cell<pstl::b64> {
    public:
            // typedef
        typedef pstl::b64 bit_type;
	typedef bit_type::size_type size_type;
        typedef bit_type::size_type word_type;
        typedef bit_type::size_type tserial;
        typedef bit_type::size_type fserial;
        typedef bit_type::size_type core_p;
        typedef bit_type::mint mint;
        typedef bit_type::mfloat mfloat;
        
            // class data
        static const int charlen = 7;
        static const int tag_width = 4;
        static const int tag_shift = 60;
        static const int char_shift = 56;
        static const word_type ptr_mask = 0xc000000000000000ULL;
        static const word_type tag_mask = 0xf000000000000000ULL;
        static const word_type ntag_mask = 0x0fffffffffffffffULL;
        static const word_type invalid_word = (((word_type) 1) << (tag_shift+2)) -1;
        static const core_p invalid_core_p = (core_p) invalid_word;

            // data
        word_type word;

        cell() {}
        cell(const cell& c) : word(c.word) {}
        cell(word_type w) : word(w) {}
        ~cell() {}
    };

  template <class _Bit1, class _Bit2>
  bool operator==(const cell<_Bit1>& x, const cell<_Bit2>& y) {
    return (x.word == y.word);
  }
  template <class _Bit1, class _Bit2>
  bool operator!=(const cell<_Bit1>& x, const cell<_Bit2>& y) {
    return (x.word != y.word);
  }
    
    template <class _Bit>
    cell<_Bit> PTR2c(typename cell<_Bit>::core_p p) {
      return cell<_Bit>((typename cell<_Bit>::word_type) p);
    }
    template <class _Bit>
    cell<_Bit> STR2c(typename cell<_Bit>::tserial ts) {
        return cell<_Bit>(((typename cell<_Bit>::word_type) ts) |
                          (((typename cell<_Bit>::word_type) T_STR) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> VAR2c(typename cell<_Bit>::tserial ts) {
        return cell<_Bit>(((typename cell<_Bit>::word_type) ts) |
                          (((typename cell<_Bit>::word_type) T_VAR) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> INT2c(typename cell<_Bit>::mint x) {
        return cell<_Bit>((((typename cell<_Bit>::word_type) x) & cell<_Bit>::ntag_mask) |
                          (((typename cell<_Bit>::word_type) T_INT) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> FLO2c(typename cell<_Bit>::mfloat x) {
      return cell<_Bit>((( static_cast<typename cell<_Bit>::word_type>(float2int(x)) +
                             ((typename cell<_Bit>::word_type) 1 << cell<_Bit>::tag_width - 1)) >>
                           cell<_Bit>::tag_width) |
                          (((typename cell<_Bit>::word_type) T_FLO) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> CHR2c(typename cell<_Bit>::word_type x) {
        return cell<_Bit>(x & cell<_Bit>::ntag_mask | (((typename cell<_Bit>::word_type) T_CHR) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> STG2c(typename cell<_Bit>::size_type x) {
        return cell<_Bit>(((typename cell<_Bit>::word_type) x) |
                          (((typename cell<_Bit>::word_type) T_STG) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> NFT2c(typename cell<_Bit>::size_type x) {
        return cell<_Bit>(((typename cell<_Bit>::word_type) x) |
                          (((typename cell<_Bit>::word_type) T_NFT) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> FTR2c(typename cell<_Bit>::fserial fs) {
        return cell<_Bit>(((typename cell<_Bit>::word_type) fs) |
                          (((typename cell<_Bit>::word_type) T_FTR) << cell<_Bit>::tag_shift));
    }
    template <class _Bit>
    cell<_Bit> SYS2c(int x) {
        return cell<_Bit>(((typename cell<_Bit>::word_type) x) |
                          (((typename cell<_Bit>::word_type) T_SYS) << cell<_Bit>::tag_shift));
    }

    
    template <class _Bit>
    typename cell<_Bit>::core_p c2PTR(cell<_Bit> c) {
        return (typename cell<_Bit>::core_p) c.word;
    }
    template <class _Bit>
    typename cell<_Bit>::tserial c2VAR(cell<_Bit> c) {
        return (typename cell<_Bit>::tserial)((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    typename cell<_Bit>::tserial c2STR(cell<_Bit> c) {
        return (typename cell<_Bit>::tserial)((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    typename cell<_Bit>::mint c2INT(cell<_Bit> c) {
        return (typename cell<_Bit>::mint) ((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    typename cell<_Bit>::mfloat c2FLO(cell<_Bit> c) {
        c.word = c.word << cell<_Bit>::tag_width;
        return static_cast<typename cell<_Bit>::mfloat>(int2float(c.word));
    }
    template <class _Bit>
    typename cell<_Bit>::word_type c2CHR(cell<_Bit> c) {
        return (c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width;
    }
    template <class _Bit>
    char c2CHR0(cell<_Bit> c) {
        return (char)(c.word >> cell<_Bit>::char_shift);
    }
    template <class _Bit>
    char c2CHR1(cell<_Bit> c) {
        return (char)((c.word << 8) >> cell<_Bit>::char_shift);
    }
    template <class _Bit>
    typename _Bit::size_type c2STG(cell<_Bit> c) {
        return (typename _Bit::size_type)((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    typename _Bit::size_type c2NFT(cell<_Bit> c) {
        return (typename _Bit::size_type)((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    typename cell<_Bit>::fserial c2FTR(cell<_Bit> c) {
        return (typename cell<_Bit>::fserial)((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    int c2SYS(cell<_Bit> c) {
        return (int)((c.word << cell<_Bit>::tag_width) >> cell<_Bit>::tag_width);
    }
    template <class _Bit>
    bool IsPTR(cell<_Bit> c) {
        return (c.word & cell<_Bit>::ptr_mask) == ((typename cell<_Bit>::word_type)T_PTR << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsSTR(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_STR << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsVAR(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_VAR << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsINT(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_INT << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsFLO(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_FLO << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsCHR(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_CHR << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsSTG(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_STG << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsNFT(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_NFT << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsFTR(cell<_Bit> c) {
        return (c.word & cell<_Bit>::tag_mask) == ((typename cell<_Bit>::word_type)T_FTR << cell<_Bit>::tag_shift);
    }
    template <class _Bit>
    bool IsInvalid(cell<_Bit> c) {
        return (c.word == cell<_Bit>::invalid_word);
    }
//        static dcore_p mint2dcore_p(mint a0, mint a1, mint) { return (((dcore_p) a0) << dtag_width) | ((dcore_p) a1); };
//        static triple<mint, mint, mint> dcore_p2mint(dcore_p dcp) { return triple<mint, mint, mint>(((mint) (dcp >> dtag_width)), ((mint) ((dcp << dtag_shift) >> dtag_shift)), 0); };

    template <class _Bit> tag_t Tag(cell<_Bit> c) { return (tag_t)(c.word >> cell<_Bit>::tag_shift); }

    template <class _Bit> cell<_Bit> cell2cell(lilfes::cell c);
    template <class _Bit> lilfes::cell cell2cell(cell<_Bit> c);
#if SIZEOF_INTP == 4
        /// 32bit <=> 32bit
    template <> cell<pstl::b32> cell2cell<pstl::b32>(lilfes::cell c) {
      return cell<pstl::b32>((cell<pstl::b32>::word_type) lilfes::cell2int(c));
    }
    template <> lilfes::cell cell2cell<pstl::b32>(cell<pstl::b32> c) { return (lilfes::cell) c.word; }
        /// 64bit DB <=> 32bit LiL
    template <> cell<pstl::b64> cell2cell<pstl::b64>(lilfes::cell c) {
        switch(lilfes::Tag(c)) {
            case lilfes::T_PTR: case lilfes::T_PTR1: case lilfes::T_PTR2: case lilfes::T_PTR3:
	      return PTR2c<pstl::b64>((cell<pstl::b64>::core_p) lilfes::c2PTR(c));
            case lilfes::T_VAR:
	      return VAR2c<pstl::b64>((cell<pstl::b64>::tserial) lilfes::c2VARS(c));
            case lilfes::T_STR:
	      return STR2c<pstl::b64>((cell<pstl::b64>::tserial) lilfes::c2STRS(c));
            case lilfes::T_INT:
	      return INT2c<pstl::b64>((cell<pstl::b64>::mint) lilfes::c2INT(c));
            case lilfes::T_FLO:
	      return FLO2c<pstl::b64>((cell<pstl::b64>::mfloat) lilfes::c2FLO(c));
            case lilfes::T_CHR:
                std::cerr << "cell2cell for T_CHR is not supported" << std::endl; exit(0);
            case lilfes::T_STG:
                std::cerr << "cell2cell for T_STG is not supported" << std::endl; exit(0);
            default:
                std::cerr << "unknown tag" << std::endl; exit(0);
        }
    }
    template <> lilfes::cell cell2cell<pstl::b64>(cell<pstl::b64> c) {
        switch(Tag<pstl::b64>(c)) {
            case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3:
                return lilfes::PTR2c((lilfes::core_p) c2PTR<pstl::b64>(c));
            case T_VAR:
                return lilfes::VAR2c((lilfes::tserial) c2VAR<pstl::b64>(c));
            case T_STR:
                return lilfes::STR2c((lilfes::tserial) c2STR<pstl::b64>(c));
            case T_INT:
                return lilfes::INT2c((lilfes::mint) c2INT<pstl::b64>(c));
            case T_FLO:
                return lilfes::FLO2c((lilfes::mfloat) c2FLO<pstl::b64>(c));
            case T_CHR:
                std::cerr << "cell2cell for T_STG is not supported" << std::endl; exit(0);
            case T_STG:
                std::cerr << "cell2cell for T_STG is not supported" << std::endl; exit(0);
            default:
                std::cerr << "unknown tag" << std::endl; exit(0);
        }
    }
#elif SIZEOF_INTP == 8
        /// 64bit <=> 64bit
    template <> cell<pstl::b64> cell2cell<pstl::b64>(lilfes::cell c) { return cell<pstl::b64>((cell<pstl::b64>::word_type) c); }
    template <> lilfes::cell cell2cell<pstl::b64>(cell<pstl::b64> c) { return (lilfes::cell) c.word; }
        /// 32bit DB <=> 64bit LiL
    template <> cell<pstl::b32> cell2cell<pstl::b32>(lilfes::cell c) {
        switch(lilfes::Tag(c)) {
            case lilfes::T_PTR: case lilfes::T_PTR1: case lilfes::T_PTR2: case lilfes::T_PTR3:
	      return PTR2c<pstl::b32>((cell<pstl::b32>::core_p) lilfes::c2PTR(c));
            case lilfes::T_VAR:
	      return VAR2c<pstl::b32>((cell<pstl::b32>::tserial) lilfes::c2VARS(c));
            case lilfes::T_STR:
	      return STR2c<pstl::b32>((cell<pstl::b32>::tserial) lilfes::c2STRS(c));
            case lilfes::T_INT:
	      return INT2c<pstl::b32>((cell<pstl::b32>::mint) lilfes::c2INT(c));
            case lilfes::T_FLO:
	      return FLO2c<pstl::b32>((cell<pstl::b32>::mfloat) lilfes::c2FLO(c));
            case lilfes::T_CHR:
                std::cerr << "cell2cell for T_CHR is not supported" << std::endl; exit(0);
            case lilfes::T_STG:
                std::cerr << "cell2cell for T_STG is not supported" << std::endl; exit(0);
            default:
                std::cerr << "unknown tag" << std::endl; exit(0);
        }
    }
    template <> lilfes::cell cell2cell<pstl::b32>(cell<pstl::b32> c) {
        switch(Tag<pstl::b32>(c)) {
            case T_PTR: case T_PTR1: case T_PTR2: case T_PTR3:
                return lilfes::PTR2c((lilfes::core_p) c2PTR<pstl::b32>(c));
            case T_VAR:
                return lilfes::VAR2c((lilfes::tserial) c2VAR<pstl::b32>(c));
            case T_STR:
                return lilfes::STR2c((lilfes::tserial) c2STR<pstl::b32>(c));
            case T_INT:
                return lilfes::INT2c((lilfes::mint) c2INT<pstl::b32>(c));
            case T_FLO:
                return lilfes::FLO2c((lilfes::mfloat) c2FLO<pstl::b32>(c));
            case T_CHR:
                std::cerr << "cell2cell for T_STG is not supported" << std::endl; exit(0);
            case T_STG:
                std::cerr << "cell2cell for T_STG is not supported" << std::endl; exit(0);
            default:
                std::cerr << "unknown tag" << std::endl; exit(0);
        }
    }
#endif
}

#endif // __ldbm_cell_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/05/21 11:31:47  yusuke
 *
 * fix type-punning bug
 *
 * Revision 1.3  2005/05/23 01:18:55  ninomi
 * push_backまで実装。バグがでている状態。
 *
 * Revision 1.2  2005/05/13 10:12:07  ninomi
 * データベース作成中。
 *
 * Revision 1.1  2005/04/18 05:40:59  ninomi
 * coreからcellの部分を分離しました。dbを作りはじめました。
 *
 */
