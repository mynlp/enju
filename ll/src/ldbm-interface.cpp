/*
 * $Id: ldbm-interface.cpp,v 1.7 2011-05-02 10:38:23 matuzaki Exp $
 *
 *    Copyright (c) 2005, Takashi Ninomiya
 *
 *    You may distribute this file under the terms of the Artistic License.
 *
 */

#include "builtin.h"
#include "ldbm-db.h"

#include <iostream>
#include <ostream>

namespace lilfes {

using std::cerr;
using std::cout;
using std::endl;
    
    static void initialize_ldbm(void) {
#ifdef DEBUG
        cout << "Initialize " << __FILE__ << endl;
#endif
    }
    static void terminate_ldbm(void) {
    }

    static InitFunction INIT_ldbm(initialize_ldbm, 103);
    static TermFunction TERM_ldbm(terminate_ldbm);

    bool push_back(machine&, FSP f) {
#if SIZEOF_INTP == 4
        typedef pstl::b32 mem_bit_type;
#elif SIZEOF_INTP == 8
        typedef pstl::b64 mem_bit_type;
#endif
        typedef pstl::pmem mem_dev_type;
        typedef pstl::vmem file_dev_type;
        typedef pstl::b64 file_bit_type;
            /// pheap
        pstl::pheap<mem_dev_type, mem_bit_type> phmem;
        pstl::pheap<file_dev_type, file_bit_type> phfile;
        if(! phfile.open("test.ldb")) {cerr << "fail to open" << endl; exit(0); }

            /// allocator
        pstl::ptr<pstl::allocator<mem_dev_type, mem_bit_type>, mem_bit_type> __allocmem =
            pstl::allocator<mem_dev_type, mem_bit_type>::pnew(&phmem);
        pstl::ptr<pstl::allocator<file_dev_type, file_bit_type>, file_bit_type> __allocfile =
            pstl::allocator<file_dev_type, file_bit_type>::pnew(&phfile);
        pstl::ref<pstl::allocator<mem_dev_type, mem_bit_type>, mem_dev_type, mem_bit_type> allocmem = __allocmem(&phmem);
        pstl::ref<pstl::allocator<file_dev_type, file_bit_type>, file_dev_type, file_bit_type> allocfile = __allocfile(&phfile);

            /// type manager
        pstl::ptr<ldbm::TypeManager<mem_dev_type, mem_bit_type>, mem_bit_type> __tpmmem =
            ldbm::TypeManager<mem_dev_type, mem_bit_type>::pnew(&phmem, __allocmem);
        pstl::ptr<ldbm::TypeManager<file_dev_type, file_bit_type>, file_bit_type> __tpmfile =
            ldbm::TypeManager<file_dev_type, file_bit_type>::pnew(&phfile, __allocfile);
        pstl::ref<ldbm::TypeManager<mem_dev_type, mem_bit_type>, mem_dev_type, mem_bit_type> tpmmem = __tpmmem(&phmem);
        pstl::ref<ldbm::TypeManager<file_dev_type, file_bit_type>, file_dev_type, file_bit_type> tpmfile = __tpmfile(&phfile);
    
            /// type mapper
        ldbm::TypeMapper<ldbm::TypeManager<file_dev_type, file_bit_type>, ldbm::TypeManager<mem_dev_type, mem_bit_type> > typemap(&phfile, tpmfile, allocfile, &phmem, tpmmem, allocmem);

            /*
              for(int i = 0 ; i < lilfes::type::GetSerialCount(); i++) {
              tpmfile->add(&phfile, __allocfile, &phmem, tpmmem->getPropername(&phmem, i));
              }

              typemap.sync();
              typemap.print(&phfile, &phmem);
            */

            /// feat mapper
        ldbm::FeatureMapper<ldbm::TypeManager<file_dev_type, file_bit_type>, ldbm::TypeManager<mem_dev_type, mem_bit_type> > featmap(tpmfile, tpmmem, &typemap);

            /// DB
        ldbm::DbListFsp<pstl::allocator<pstl::vmem, pstl::b64> > fodb(&phfile,__allocfile);

        fodb.push_back(&phfile, &typemap, &featmap, f);
        return true;
    }

    bool ldbm_dbopen(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_dbclose(machine&, FSP arg1) {
    }
    bool ldbm_declare_variable(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_declare_variable(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_declare_list(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_declare_list(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_declare_vector(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_declare_vector(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_declare_hash_map(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_declare_hash_map(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_push_front(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_push_back(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_pop_front(machine&, FSP arg1) {
    }
    bool ldbm_pop_back(machine&, FSP arg1) {
    }
    bool ldbm_write(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_read(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_find(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_insert(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_insert(machine&, FSP arg1, FSP arg2, FSP arg3) {
    }
    bool ldbm_erase(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_begin(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_end(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_next(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_prev(machine&, FSP arg1, FSP arg2) {
    }
    bool ldbm_clear(machine&, FSP arg1) {
    }
    bool ldbm_size(machine&, FSP arg1, FSP arg2) {
    }
    
    
    LILFES_BUILTIN_PRED_3(ldbm_dbopen, dbopen);
    LILFES_BUILTIN_PRED_1(ldbm_dbclose, dbclose);
    
    LILFES_BUILTIN_PRED_2(ldbm_declare_variable, declare_variable);
    LILFES_BUILTIN_PRED_OVERLOAD_3(ldbm_declare_variable, declare_variable_2, declare_variable);
    LILFES_BUILTIN_PRED_2(ldbm_declare_list, declare_list);
    LILFES_BUILTIN_PRED_OVERLOAD_3(ldbm_declare_list, declare_list_2, declare_list);
    LILFES_BUILTIN_PRED_2(ldbm_declare_vector, declare_vector);
    LILFES_BUILTIN_PRED_OVERLOAD_3(ldbm_declare_vector, declare_vector_3, declare_vector);
    LILFES_BUILTIN_PRED_2(ldbm_declare_hash, declare_hash);
    LILFES_BUILTIN_PRED_OVERLOAD_3(ldbm_declare_hash, declare_hash_3, declare_hash);

    LILFES_BUILTIN_PRED_2(ldbm_push_front, push_front);
    LILFES_BUILTIN_PRED_2(ldbm_push_back, push_back);
    LILFES_BUILTIN_PRED_1(ldbm_pop_front, pop_front);
    LILFES_BUILTIN_PRED_1(ldbm_pop_back, pop_back);
    
    LILFES_BUILTIN_PRED_3(ldbm_read, read);
    LILFES_BUILTIN_PRED_3(ldbm_write, write);
    
    LILFES_BUILTIN_PRED_3(ldbm_find, find);
    LILFES_BUILTIN_PRED_2(ldbm_insert, insert);
    LILFES_BUILTIN_PRED_OVERLOAD_3(ldbm_insert, insert_3, insert);
    LILFES_BUILTIN_PRED_2(ldbm_erase, erase);
    
    LILFES_BUILTIN_PRED_2(ldbm_begin, begin);
    LILFES_BUILTIN_PRED_2(ldbm_end, end);
    LILFES_BUILTIN_PRED_2(ldbm_next, next);
    LILFES_BUILTIN_PRED_2(ldbm_prev, prev);
    
    LILFES_BUILTIN_PRED_1(ldbm_clear, clear);
    
    LILFES_BUILTIN_PRED_2(ldbm_size, size);
    
} // end of namespace lilfes

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/06/10 10:27:44  ninomi
 * no comment
 *
 * Revision 1.5  2005/06/09 06:58:18  ninomi
 * DbListとDbHashができました。
 *
 * Revision 1.4  2005/05/24 01:31:33  ninomi
 * ldbm-interface.cppをamd64で動くようにしました。
 *
 * Revision 1.3  2005/05/23 08:58:33  ninomi
 * push_backを小さなデータで成功。push_backだけlilfes本体とmerge
 *
 * Revision 1.2  2005/05/23 01:19:43  ninomi
 * push_backまで実装。まだバグつき
 *
 * Revision 1.1  2005/05/13 10:13:19  ninomi
 * ldbm用のinterfaceファイルをとりあえず作りました。
 *
 */
