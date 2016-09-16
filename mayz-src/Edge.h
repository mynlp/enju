/*
 *
 * $Id: Edge.h,v 1.4 2009-12-03 23:33:28 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __Edge_h
#define __Edge_h

#include <liblilfes/structur.h>
#include <utility>
#include <vector>
#include "Fs.h"

// Parser internal data structure
namespace up {

////////////////////////////////////////////////////
//////
////// Edge
//////

    typedef lilfes::uint32 eserial;       // index in ParserBasic::edge_vec
    typedef lilfes::uint32 lserial;       // index in ParserBasic::link_vec

    const eserial invalid_eserial = static_cast<eserial>(-1);
    const lserial invalid_lserial = static_cast<lserial>(-1);
    
    class Edge {
    private:
        eserial serialno;
        std::pair<int, int> position; // left postion and right position
        Fs sign;
        std::vector<lserial> links;

        double fom;
        unsigned state; // 0 = alive, 1 = dead in ParserCkyFom. the iteration number when the edge is generated
    public:
        Edge(eserial __serial_no, std::pair<int, int> __position, const Fs& __sign, double __fom=0.0, unsigned __state=0)
                : serialno(__serial_no),
                  position(__position),
                  sign(__sign),
                  fom(__fom),
                  state(__state) {}
        ~Edge() {}
    
        Fs& getSign() { return sign; }
        const Fs& getSign() const { return sign; }
        void setSign(const Fs& s) { sign = s; }

        std::pair<int, int> getPosition() const { return position; }
        void setPosition(std::pair<int, int> p) { position = p; }
        
        int getLeftPosition() const { return position.first; }
        void setLeftPosition(int p) { position.first = p; }
        
        int getRightPosition() const { return position.second; }
        void setRightPosition(int p) { position.second = p; }
        
        eserial getSerialNo() const { return serialno; }
        void setSerialNo(eserial sn) { serialno = sn; }
        
        double getFom() const { return fom; }
        void setFom(double f) { fom = f; }
        
        unsigned getState() const { return state; }
        void setState(unsigned s) { state = s; }
        
        std::vector<lserial>& getLinks() { return links; }
        void addLink(lserial l) { links.push_back(l); }

        lilfes::FSP makeFSP(lilfes::machine *mach) const { return lilfes::FSP(mach, sign); }
    };

////////////////////////////////////////////////////
//////
////// Link
//////
    class Link {
    private:
        lserial serialno;
        Fs sign_plus;
        eserial my_edge_serialno;
        eserial l_dtr_serialno;  //  terminal ... null,      unary ... daughter, binary ... left daughter
        eserial r_dtr_serialno;  //  terminal ... word-ID,   unary ... null,     binary ... right daughter
        Fs applied_schema;       //  terminal ... lex_entry, unary/binary ... applied_schema
        
        double fom;
    public:
        // terminal
        Link(lserial __serialno, double __fom = 0.0)
                : serialno(__serialno),
                  l_dtr_serialno(invalid_eserial),
                  r_dtr_serialno(invalid_eserial),
                  fom(__fom) {}
        Link(lserial __serialno, const Fs& __sign_plus, const Fs& __lexent, eserial word_id, double __fom = 0.0)
                : serialno(__serialno),
                  sign_plus(__sign_plus),
                  l_dtr_serialno(invalid_eserial),
                  r_dtr_serialno(word_id),
                  applied_schema(__lexent),
                  fom(__fom) {}

        // unary
        Link(lserial __serialno, eserial udtrsn, double __fom=0.0)
                : serialno(__serialno),
                  l_dtr_serialno(udtrsn),
                  r_dtr_serialno(invalid_eserial),
                  fom(__fom) {}
        Link(lserial __serialno, eserial udtrsn, const Fs& __sign_plus, const Fs& __applied_schema, double __fom=0.0)
                : serialno(__serialno),
                  sign_plus(__sign_plus),
                  l_dtr_serialno(udtrsn),
                  r_dtr_serialno(invalid_eserial),
                  applied_schema(__applied_schema),
                  fom(__fom) {}
        
        // binary
        Link(lserial __serialno, eserial ldtrsn, eserial rdtrsn, double __fom=0.0)
                : serialno(__serialno),
                  l_dtr_serialno(ldtrsn),
                  r_dtr_serialno(rdtrsn),
                  fom(__fom) {}
        Link(lserial __serialno, eserial ldtrsn, eserial rdtrsn, const Fs& __sign_plus, const Fs& __applied_schema, double __fom=0.0)
                : serialno(__serialno),
                  sign_plus(__sign_plus),
                  l_dtr_serialno(ldtrsn),
                  r_dtr_serialno(rdtrsn),
                  applied_schema(__applied_schema),
                  fom(__fom) {}
        ~Link() {}

        lserial getSerialNo() const { return serialno; }
        void setSerialNo(lserial sn) { serialno = sn; }
        
        Fs& getSignPlus() { return sign_plus; }
        const Fs& getSignPlus() const { return sign_plus; }
        void setSignPlus(const Fs& s) { sign_plus = s; }
        
        eserial getMyEdgeSerialNo() const { return my_edge_serialno; }
        void setMyEdgeSerialNo(eserial sn) { my_edge_serialno = sn; }

        eserial getWordId() const { return r_dtr_serialno; }
        void setWordId(eserial word_id) { r_dtr_serialno = word_id; }

        bool isTerminal() const { return (l_dtr_serialno == invalid_eserial && r_dtr_serialno != invalid_eserial); }
        bool isUnary()    const { return (l_dtr_serialno != invalid_eserial && r_dtr_serialno == invalid_eserial); }
        bool isBinary()   const { return (l_dtr_serialno != invalid_eserial && r_dtr_serialno != invalid_eserial); }

        eserial getLDtr() const { return l_dtr_serialno; }
        eserial getUDtr() const { return l_dtr_serialno; }
        eserial getRDtr() const { return r_dtr_serialno; }
        
        Fs& getAppliedSchema() { return applied_schema; }
        const Fs& getAppliedSchema() const { return applied_schema; }

        Fs& getLexName() { return applied_schema; }
        const Fs& getLexName() const { return applied_schema; }

        double getFom() const { return fom; }
        void setFom(double d) { fom = d; }
        
        lilfes::FSP makeFSP(lilfes::machine *mach) const {
            if( isTerminal() ) {
                lilfes::FSP y(mach);
                const lilfes::type *t = lilfes::module::UserModule()->Search("terminal");
                if(!t) { std::cerr << "error: type terminal is not defined" << std::endl; return lilfes::FSP(); }
                y.Coerce(t);
                y.Follow(lilfes::module::UserModule()->Search("SIGN_PLUS\\")).Unify(lilfes::FSP(mach, getSignPlus()));
                y.Follow(lilfes::module::UserModule()->Search("LEX_NAME\\")).Unify(lilfes::FSP(mach, getLexName()));
                y.Follow(lilfes::module::UserModule()->Search("WORD_ID\\")).Unify(lilfes::FSP(mach, static_cast<lilfes::mint>(getWordId())));
                return y;
            }
            if( isUnary() ) {
                lilfes::FSP y(mach);
                const lilfes::type *t = lilfes::module::UserModule()->Search("nonterminal_unary");
                if(!t) { std::cerr << "error: type nonterminal_unary is not defined" << std::endl; return lilfes::FSP(); }
                y.Coerce(t);
                y.Follow(lilfes::module::UserModule()->Search("SIGN_PLUS\\")).Unify(lilfes::FSP(mach, getSignPlus()));
                y.Follow(lilfes::module::UserModule()->Search("APPLIED_SCHEMA\\")).Unify(lilfes::FSP(mach, getAppliedSchema()));
                y.Follow(lilfes::module::UserModule()->Search("U_DTR\\")).Unify(lilfes::FSP(mach, static_cast<lilfes::mint>(getUDtr())));
                return y;
            }
            if( isBinary() ) {
                lilfes::FSP y(mach);
                const lilfes::type *t = lilfes::module::UserModule()->Search("nonterminal_binary");
                if(!t) { std::cerr << "error: type nonterminal_binary is not defined" << std::endl; return lilfes::FSP(); }
                y.Coerce(t);
                y.Follow(lilfes::module::UserModule()->Search("SIGN_PLUS\\")).Unify(lilfes::FSP(mach, getSignPlus()));
                y.Follow(lilfes::module::UserModule()->Search("APPLIED_SCHEMA\\")).Unify(lilfes::FSP(mach, getAppliedSchema()));
                y.Follow(lilfes::module::UserModule()->Search("L_DTR\\")).Unify(lilfes::FSP(mach, static_cast<lilfes::mint>(getLDtr())));
                y.Follow(lilfes::module::UserModule()->Search("R_DTR\\")).Unify(lilfes::FSP(mach, static_cast<lilfes::mint>(getRDtr())));
                return y;
                
            }
            std::cerr << "error: undefined Link" << std::endl;
            return lilfes::FSP();
        }
    };

} // end of namespace up

#endif // __Edge_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
