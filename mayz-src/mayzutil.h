/*
 * $Id: mayzutil.h,v 1.3 2011-05-02 08:35:20 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef mayzutil_h__
#define mayzutil_h__

#include <liblilfes/proc.h>
#include <liblilfes/utility.h>
#include <liblilfes/gfstream.h>
#include <liblilfes/bfstream.h>
#include <stdexcept>

// Specialization for avoiding ambiguity
namespace lilfes {
    template<>
    class c_to_lilfes<unsigned int> {
    public:
        bool convert( machine&, unsigned int n, FSP fsp) {
            return fsp.Unify( static_cast<mint>( n ) );
        }
    };
}

namespace up {

    /// Similar to call_proc in liblilfes/utility.h, but returns false when pt == 0
    inline
    bool call_proc(lilfes::machine *mach, lilfes::procedure *proc, lilfes::FSP arg1) {
        lilfes::procthunk *pt = proc->GetThunk();
        if(! pt) return false;
        mach->SetRP(1);
        mach->WriteLocal(0, lilfes::PTR2c(arg1.GetAddress()));
        pt->exec(*mach);
        if( mach->GetIP() == NULL ) return false;
        return mach->Execute(mach->GetIP());
    }

    inline
    bool call_proc(lilfes::machine *mach, lilfes::procedure *proc, lilfes::FSP arg1, lilfes::FSP arg2) {
        lilfes::procthunk *pt = proc->GetThunk();
        if(! pt) return false;
        mach->SetRP(2);
        mach->WriteLocal(0, lilfes::PTR2c(arg1.GetAddress()));
        mach->WriteLocal(1, lilfes::PTR2c(arg2.GetAddress()));
        pt->exec(*mach);
        if( mach->GetIP() == NULL ) return false;
        return mach->Execute(mach->GetIP());
    }

    inline
    bool call_proc(lilfes::machine *mach, lilfes::procedure *proc, lilfes::FSP arg1, lilfes::FSP arg2, lilfes::FSP arg3) {
        lilfes::procthunk *pt = proc->GetThunk();
        if(! pt) return false;
        mach->SetRP(3);
        mach->WriteLocal(0, lilfes::PTR2c(arg1.GetAddress()));
        mach->WriteLocal(1, lilfes::PTR2c(arg2.GetAddress()));
        mach->WriteLocal(2, lilfes::PTR2c(arg3.GetAddress()));
        pt->exec(*mach);
        if( mach->GetIP() == NULL ) return false;
        return mach->Execute(mach->GetIP());
    }

    inline
    bool call_proc(
        lilfes::machine *mach, lilfes::procedure *proc,
        lilfes::FSP arg1, lilfes::FSP arg2, lilfes::FSP arg3, lilfes::FSP arg4
    ) {
        lilfes::procthunk *pt = proc->GetThunk();
        if(! pt) return false;
        mach->SetRP(4);
        mach->WriteLocal(0, lilfes::PTR2c(arg1.GetAddress()));
        mach->WriteLocal(1, lilfes::PTR2c(arg2.GetAddress()));
        mach->WriteLocal(2, lilfes::PTR2c(arg3.GetAddress()));
        mach->WriteLocal(3, lilfes::PTR2c(arg4.GetAddress()));
        pt->exec(*mach);
        if( mach->GetIP() == NULL ) return false;
        return mach->Execute(mach->GetIP());
    }

    inline
    bool call_proc(
        lilfes::machine *mach, lilfes::procedure *proc,
        lilfes::FSP arg1, lilfes::FSP arg2, lilfes::FSP arg3,
        lilfes::FSP arg4, lilfes::FSP arg5
    ) {
        lilfes::procthunk *pt = proc->GetThunk();
        if(! pt) return false;
        mach->SetRP(5);
        mach->WriteLocal(0, lilfes::PTR2c(arg1.GetAddress()));
        mach->WriteLocal(1, lilfes::PTR2c(arg2.GetAddress()));
        mach->WriteLocal(2, lilfes::PTR2c(arg3.GetAddress()));
        mach->WriteLocal(3, lilfes::PTR2c(arg4.GetAddress()));
        mach->WriteLocal(4, lilfes::PTR2c(arg5.GetAddress()));
        pt->exec(*mach);
        if( mach->GetIP() == NULL ) return false;
        return mach->Execute(mach->GetIP());
    }

    inline
    bool call_proc(
        lilfes::machine *mach, lilfes::procedure *proc,
        lilfes::FSP arg1, lilfes::FSP arg2, lilfes::FSP arg3,
        lilfes::FSP arg4, lilfes::FSP arg5, lilfes::FSP arg6
    ) {
        lilfes::procthunk *pt = proc->GetThunk();
        if(! pt) return false;
        mach->SetRP(6);
        mach->WriteLocal(0, lilfes::PTR2c(arg1.GetAddress()));
        mach->WriteLocal(1, lilfes::PTR2c(arg2.GetAddress()));
        mach->WriteLocal(2, lilfes::PTR2c(arg3.GetAddress()));
        mach->WriteLocal(3, lilfes::PTR2c(arg4.GetAddress()));
        mach->WriteLocal(4, lilfes::PTR2c(arg5.GetAddress()));
        mach->WriteLocal(5, lilfes::PTR2c(arg6.GetAddress()));
        pt->exec(*mach);
        if( mach->GetIP() == NULL ) return false;
        return mach->Execute(mach->GetIP());
    }

    inline
    bool init_proc(
        lilfes::module *module,
        lilfes::procedure *&proc,
        const std::string &name,
        int arity
    ) {
        proc = lilfes::prepare_proc(module, name, arity);
        if (! proc) {
            std::cerr << "predicate not defined: \'" << name << "\'/" << arity << std::endl;
            return false;
        }
        else {
            return true;
        }
    }

    // Data conversion between C++ types <--> lilfes TFS
    template<class DataT>
    inline
    bool lilfes2c(lilfes::machine *m, lilfes::FSP fsp, DataT &d) {
        return lilfes::lilfes_to_c<DataT>().convert(*m, fsp, d);
    }

    template<class DataT>
    inline
    bool c2lilfes(lilfes::machine *m, const DataT &d, lilfes::FSP fsp) {
        return lilfes::c_to_lilfes<DataT>().convert(*m, d, fsp);
    }

    // Profiling
    class ScopeProf {
    public:
        ScopeProf(const char *name, unsigned &time)
            : _name(name)
            , _time(&time)
        {
            lilfes::start_prof(_name);
        }

        ScopeProf(const char *name)
            : _name(name)
            , _time(0)
        {
            lilfes::start_prof(_name);
        }

        ~ScopeProf() {
            if (_time) {
                *_time = lilfes::stop_prof(_name);
            }
            else {
                (void) lilfes::stop_prof(_name);
            }
        }
    private:
        const char *_name;
        unsigned *_time;
    };

    // Open a compressed or raw file
    namespace fopen_util {

        inline
        bool hasSuffix(const std::string &full, const std::string &suffix)
        {
            return full.size() >= suffix.size()
                && full.substr(full.size() - suffix.size()) == suffix;
        }

        template<class BaseT, class Bz2T, class GzT, class RawT>
        BaseT *openAndCheck(const std::string &fname)
        {
            BaseT *ptr = hasSuffix(fname, ".bz2") ? static_cast<BaseT*>(new Bz2T(fname.c_str())) :
                         hasSuffix(fname, ".gz")  ? static_cast<BaseT*>(new GzT(fname.c_str())) :
#ifdef _MSC_VER
                                                    static_cast<BaseT*>(new RawT(fname.c_str(), std::ios::binary));
#else
                                                    static_cast<BaseT*>(new RawT(fname.c_str()));
#endif
            if (! (*ptr)) {
                throw std::runtime_error("Cannot open '" + fname + "'");
            }
        
            return ptr;
        }
    }

    inline
    std::ostream *openOutputFile(const std::string &fname)
    {
        return fopen_util::openAndCheck<std::ostream, lilfes::obfstream, lilfes::ogfstream, std::ofstream>(fname);
    }

    inline
    std::istream *openInputFile(const std::string &fname)
    {
        return fopen_util::openAndCheck<std::istream, lilfes::ibfstream, lilfes::igfstream, std::ifstream>(fname);
    }
}

#endif // mayzutil_h__
