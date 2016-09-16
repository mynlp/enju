/*
 * $Id: Grammar.h,v 1.9 2009-12-03 23:33:28 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __Grammar_h
#define __Grammar_h

#include <stdexcept>

#include <liblilfes/structur.h>
#include <liblilfes/proc.h>
#include <liblilfes/machine.h>
#include <liblilfes/errors.h>
#include <liblilfes/utility.h>

//#include "pvector.h"
#include "Qc.h"
#include "mayzutil.h"
#include "ParserBuiltin.h"

namespace up {

    class Grammar {
    public:
        typedef std::vector<Edge> EdgeVec;
        typedef std::vector<Link> LinkVec;
        //typedef pvector<Edge> EdgeVec;
        //typedef pvector<Link> LinkVec;
        
    private:
        bool is_open;

    protected:        
        lilfes::machine* mach;

        // Quick Check
        QcManager qc;
        Fs quick_check_path;
        bool is_quick_check;

        std::string grammar_name;
        std::string grammar_version;

    private:
        // data conversion
        lilfes::procedure *strlist_to_word;
        lilfes::procedure *word_to_strlist;

        //
        lilfes::procedure *sentence_to_lattice;
        lilfes::procedure *lexical_entry_sign;
        lilfes::procedure *reduce_sign;
        lilfes::procedure *root_sign;
        lilfes::procedure *id_schema_unary;
        lilfes::procedure *id_schema_binary;

        // For DCP processing
        const lilfes::type *t_id_schema_unary;
        const lilfes::type *t_id_schema_binary;
        
        // From old 'GrammarFom'
        lilfes::procedure *fom_root;
        lilfes::procedure *fom_binary;
        lilfes::procedure *fom_unary;
        lilfes::procedure *fom_terminal;

        // mayz:lex_entry type
        const lilfes::type *t_lex_entry;
        const lilfes::feature *f_lex_word;
        const lilfes::feature *f_lex_template;

        bool is_unary;

    public:
        std::string getName()    const { return grammar_name; }
        std::string getVersion() const { return grammar_version; }
        void setName(const std::string& n)    { grammar_name = n; }
        void setVersion(const std::string& v) { grammar_version = v; }

        // lilfes predicate call
        bool sentence2Lattice(lilfes::FSP sentence, lilfes::FSP lattice)
        {
            P2("Grammar::sentence2Lattice");
            return call_proc(mach, sentence_to_lattice, sentence, lattice);
        }

        bool lexicalEntrySign(lilfes::FSP& lex, lilfes::FSP sign)
        {
            P2("Grammar::lexicalEntrySign");
            return call_proc(mach, lexical_entry_sign, lex, sign);
        }

        bool idSchemaUnary(lilfes::FSP name, lilfes::FSP dtr, lilfes::FSP mother, lilfes::FSP dcp)
        {
            P2("Grammar::idSchemaUnary");
            if (! id_schema_unary) return false;
            return call_proc(mach, id_schema_unary, name, dtr, mother, dcp);
        }

        bool idSchemaBinary(lilfes::FSP name, lilfes::FSP left, lilfes::FSP right, lilfes::FSP mother, lilfes::FSP dcp)
        {
            P2("Grammar::idSchemaBinary");
            return call_proc(mach, id_schema_binary, name, left, right, mother, dcp);
        }

        bool reduceSign(lilfes::FSP sign, lilfes::FSP reduced_sign, lilfes::FSP sign_plus)
        {
            P2("Grammar::reduceSign");
            return call_proc(mach, reduce_sign, sign, reduced_sign, sign_plus);
        }

        bool rootSign(lilfes::FSP sign)
        {
            P2("Grammar::rootSign");
            return call_proc(mach, root_sign, sign);
        }

        bool fomRoot(lilfes::FSP sign, lilfes::FSP fom)
        {
            P2("Grammar::fomRoot");
            return call_proc(mach, fom_root, sign, fom);
        }

    protected:
        bool fomBinary(lilfes::FSP name,   lilfes::FSP left,      lilfes::FSP right,
                       lilfes::FSP mother, lilfes::FSP sign_plus, lilfes::FSP fom)
        {
            P2("Grammar::fomBinary");
            return call_proc(mach, fom_binary, name, left, right, mother, sign_plus, fom);
        }
    
        bool fomUnary(lilfes::FSP name,   lilfes::FSP dtr,
                      lilfes::FSP mother, lilfes::FSP sign_plus, lilfes::FSP fom)
        {
            P2("Grammar::fomUnary");
            return call_proc(mach, fom_unary, name, dtr, mother, sign_plus, fom);
        }
    
        bool fomTerminal(lilfes::FSP lex, lilfes::FSP sign, lilfes::FSP sign_plus, lilfes::FSP fom)
        {
            P2("Grammar::fomTerminal");
            return call_proc(mach, fom_terminal, lex, sign, sign_plus, fom);
        }
    
    public:
        Grammar()
            : is_open(false)
            , is_quick_check(false)
            , is_unary(false) {}

        virtual ~Grammar() { term(); }
        
        bool isUnary() { return is_unary; }

        bool init(lilfes::machine* m)
        {
            if (is_open) return true;

            mach = m;

            // set modules
            lilfes::module *up_interface = lilfes::module::SearchModule("mayz:parser");
            if (! up_interface) {
                std::cerr << "module not found: mayz:parser" << std::endl;
                return false;
            }

            // set procedures, types, and features
            bool ok = init_proc(up_interface, strlist_to_word, "strlist_to_word", 4)
                   && init_proc(up_interface, word_to_strlist, "word_to_strlist", 2)
                   && init_proc(up_interface, lexical_entry_sign, "lexical_entry_sign", 2)
                   && init_proc(up_interface, sentence_to_lattice, "sentence_to_lattice", 2)
                   && init_proc(up_interface, id_schema_binary, "id_schema_binary", 5)
                   && init_proc(up_interface, reduce_sign, "reduce_sign", 3)
                   && init_proc(up_interface, root_sign, "root_sign", 1)
                   && init_proc(up_interface, fom_root, "fom_root", 2)
                   && init_proc(up_interface, fom_binary, "fom_binary", 6)
                   && init_proc(up_interface, fom_unary, "fom_unary", 5)
                   && init_proc(up_interface, fom_terminal, "fom_terminal", 4)
                   && (t_lex_entry    = up_interface->Search("lex_entry"))
                   && (f_lex_word     = up_interface->SearchFeature("LEX_WORD\\"))
                   && (f_lex_template = up_interface->SearchFeature("LEX_TEMPLATE\\"))
                   && (t_id_schema_binary = up_interface->Search("id_schema_binary"))
                   ;

            // unary schema may not be defined
            init_proc(up_interface, id_schema_unary, "id_schema_unary", 4);
            t_id_schema_unary = up_interface->Search("id_schema_unary");

            if (! ok) return false;

            // unary grammar check
            {
                lilfes::IPTrailStack iptrail(mach);
                is_unary = idSchemaUnary(lilfes::FSP(mach), lilfes::FSP(mach), lilfes::FSP(mach), lilfes::FSP(mach));
            }

            // quick check
            if (! qc.init(m, this)) return false;
 
            is_open = true;

            return true;
        }
        
        bool term()
        {
            if (! is_open) return true;

            is_quick_check = false;
            if (! qc.term()) return false;
            is_open = false;

            return true;
        }

        // data format conversion
        lilfes::FSP makeWordFSP(const std::vector<std::string>& word, unsigned begin, unsigned end)
        {
            lilfes::FSP listFsp(mach);
            if (! c2lilfes(mach, word, listFsp)) {
                throw std::logic_error("Grammar::makeWordFSP: data conversion failed");
            }
            lilfes::FSP beginFsp(mach, static_cast<lilfes::mint>(begin));
            lilfes::FSP endFsp(mach, static_cast<lilfes::mint>(end));

            lilfes::FSP wordFsp(mach);
            //if (! call_proc(mach, strlist_to_word, listFsp, beginFsp, endFsp, wordFsp)) {
            if (! strlist_to_word->Call(*mach, listFsp, beginFsp, endFsp, wordFsp)) {
                throw std::runtime_error("Grammar::makeWordFSP: strlist_to_word/4 failed");
            }
            return wordFsp;
        }

        void makeVecWord(lilfes::FSP wordFsp, std::vector<std::string>& word)
        {
            lilfes::IPTrailStack iptrail(mach);

            word.clear();

            lilfes::FSP listFsp(mach);
            if (! call_proc(mach, word_to_strlist, wordFsp, listFsp)) {
                throw std::runtime_error("Grammar::makeVecWord: word_to_strlist/3 failed");
            }

            if (! lilfes2c(mach, listFsp, word)) {
                throw std::runtime_error("Grammar::makeVecWord: data conversion failed");
            }
        }

        lilfes::FSP makeLexicalEntryFSP(lilfes::FSP wordFsp, const std::string& tmpl)
        {
            lilfes::FSP lexentFsp(mach);

            lexentFsp.Coerce(t_lex_entry);
            lexentFsp.Follow(f_lex_word).Unify(wordFsp);
            lexentFsp.Follow(f_lex_template).Unify(lilfes::FSP(mach, tmpl.c_str()));

            return lexentFsp;
        }

        // grammar rule applications
        bool lookupLexicalEntry(unsigned word_id, lilfes::FSP lexentFsp,
                                std::vector<Fs*>& signs, std::vector<lserial>& links,
                                LinkVec& link_vec, double lexent_fom) {
            P2("Grammar::lookupLexicalEntry");
            lilfes::IPTrailStack iptrail(mach);

            lilfes::FSP sign(mach);
            lilfes::FSP reduced_sign(mach);
            lilfes::FSP sign_plus(mach);
            lilfes::core_p cut_point = mach->GetCutPoint();
            if ( ! lexicalEntrySign(lexentFsp, sign) ) return false;
            mach->DoCut( cut_point );
            if ( ! reduceSign(sign, reduced_sign, sign_plus) ) return false;

            lilfes::FSP fom(mach);
#if 0 // backtracking removed (2011/09/27 yusuke)
            bool loop;
            {
                P2("GrammarFom::lookupLexicalEntry--fomTerminal");
                loop = fomTerminal(lexentFsp, reduced_sign, sign_plus, fom);
            }
            {
                P2("GrammarFom::lookupLexicalEntry--makeLink");
                for (; loop ; loop = mach->NextAnswer() ) {
                    signs.push_back(reduced_sign.Serialize());
                    lserial lsn = link_vec.size();
                    links.push_back(lsn);
                    link_vec.push_back(Link(lsn, lexent_fom + static_cast<double>( fom.ReadFloat() )));
                    link_vec.back().setWordId(word_id);
                    sign_plus.Serialize(link_vec.back().getSignPlus());
                    lexentFsp.Serialize(link_vec.back().getLexName());
                }
            }
#endif // 0
            {
                P2("GrammarFom::lookupLexicalEntry--fomTerminal");
                if (! fomTerminal(lexentFsp, reduced_sign, sign_plus, fom) ) return false;
            }
            {
                P2("GrammarFom::lookupLexicalEntry--makeLink");
                    signs.push_back(reduced_sign.Serialize());
                    lserial lsn = link_vec.size();
                    links.push_back(lsn);
                    link_vec.push_back(Link(lsn, lexent_fom + static_cast<double>( fom.ReadFloat() )));
                    link_vec.back().setWordId(word_id);
                    sign_plus.Serialize(link_vec.back().getSignPlus());
                    lexentFsp.Serialize(link_vec.back().getLexName());
            }
            return true;
        }

        bool applyIdSchemaUnary(eserial esn, std::vector<Fs*>& signs, std::vector<lserial>& links,
                                EdgeVec& edge_vec, LinkVec& link_vec)
        {
            P2("Grammar::applyIdSchemaUnary");
            lilfes::IPTrailStack iptrail(mach);

            lilfes::FSP name(mach);
            lilfes::FSP dtr(mach, edge_vec[esn].getSign());
            lilfes::FSP mother(mach);
            //lilfes::FSP dcp(mach);
            lilfes::FSP reduced_mother(mach);
            lilfes::FSP sign_plus(mach);
        
            // Schema application + dcp call
            lilfes::FSP unarySchemaPred = makeUnarySchemaFSP(name, dtr, mother);
            if (! lilfes::builtin::lilfes_call(*mach, unarySchemaPred) || mach->GetIP() == NULL) return true;;

            for (bool loop = mach->Execute(mach->GetIP()); loop ; loop = mach->NextAnswer()) {
                if ( ! reduceSign(mother, reduced_mother, sign_plus) ) continue;

                lilfes::FSP dfom(mach);
                {
                    P2("Grammar::applyIdSchemaUnary--fomUnary");
                    if ( ! fomUnary(name, dtr, reduced_mother, sign_plus, dfom) ) continue;
                }
                {
                    P2("Grammar::applyIdSchemaUnary--makeLink");
                    double fom = dfom.ReadFloat() + edge_vec[esn].getFom();
                    signs.push_back(reduced_mother.Serialize());
                    lserial lsn = link_vec.size();
                    links.push_back(lsn);
                    link_vec.push_back(Link(lsn, esn, fom));
                    sign_plus.Serialize(link_vec.back().getSignPlus());
                    name.Serialize(link_vec.back().getAppliedSchema());
                }
            }

            //CHANGE 2011-09-30 matuzaki//bool loop = idSchemaUnary(name, dtr, mother, dcp);
            //CHANGE 2011-09-30 matuzaki//for(; loop ; loop = mach->NextAnswer() ) {
            //CHANGE 2011-09-30 matuzaki//    lilfes::IPTrailStack iptrail2(mach);
            //CHANGE 2011-09-30 matuzaki//    if(! lilfes::builtin::lilfes_call(*mach, dcp) || mach->GetIP() == NULL ) continue;
            //CHANGE 2011-09-30 matuzaki//    bool loop2 = mach->Execute(mach->GetIP());
            //CHANGE 2011-09-30 matuzaki//    for(; loop2 ; loop2 = mach->NextAnswer() ) {
            //CHANGE 2011-09-30 matuzaki//        if ( ! reduceSign(mother, reduced_mother, sign_plus) ) continue;

            //CHANGE 2011-09-30 matuzaki//        lilfes::FSP dfom(mach);
            //CHANGE 2011-09-30 matuzaki//        {
            //CHANGE 2011-09-30 matuzaki//            P2("Grammar::applyIdSchemaUnary--fomUnary");
            //CHANGE 2011-09-30 matuzaki//            if ( ! fomUnary(name, dtr, reduced_mother, sign_plus, dfom) ) continue;
            //CHANGE 2011-09-30 matuzaki//        }
            //CHANGE 2011-09-30 matuzaki//        {
            //CHANGE 2011-09-30 matuzaki//            P2("Grammar::applyIdSchemaUnary--makeLink");
            //CHANGE 2011-09-30 matuzaki//            double fom = dfom.ReadFloat() + edge_vec[esn].getFom();
            //CHANGE 2011-09-30 matuzaki//            signs.push_back(reduced_mother.Serialize());
            //CHANGE 2011-09-30 matuzaki//            lserial lsn = link_vec.size();
            //CHANGE 2011-09-30 matuzaki//            links.push_back(lsn);
            //CHANGE 2011-09-30 matuzaki//            link_vec.push_back(Link(lsn, esn, fom));
            //CHANGE 2011-09-30 matuzaki//            sign_plus.Serialize(link_vec.back().getSignPlus());
            //CHANGE 2011-09-30 matuzaki//            name.Serialize(link_vec.back().getAppliedSchema());
            //CHANGE 2011-09-30 matuzaki//        }
            //CHANGE 2011-09-30 matuzaki//    }
            //CHANGE 2011-09-30 matuzaki//}

            return true;
        }
        
        bool applyIdSchemaBinary(eserial xsn, eserial ysn, lilfes::FSP xfs, lilfes::FSP yfs,
                                 std::vector<Fs*>& signs, std::vector<lserial>& links,
                                 EdgeVec& edge_vec, LinkVec& link_vec, int n)
        {
            P2("Grammar::applyIdSchemaBinary");
        
            if (isQuickCheck()) {
                return applyIdSchemaBinaryQc(xsn, ysn, xfs, yfs, signs, links, edge_vec, link_vec, n);
            }
            else {
                return applyIdSchemaBinaryNoQc(xsn, ysn, xfs, yfs, signs, links, edge_vec, link_vec);
            }
        }

        void clearQuickCheckArray()
        {
	        P2("Grammar::clearQuickCheckArray");
            if (! quick_check_path.empty()) {
                qc.clear();
            }
        }

        bool isQuickCheck() const { return is_quick_check; }

        void setQuickCheckPath(lilfes::FSP paths)
        {
            quick_check_path.clear();
            paths.Serialize(quick_check_path);
        }

        Fs getQuickCheckPath() const { return quick_check_path; }

        bool enableQuickCheck()
        {
            if (quick_check_path.empty()) return false;
            if (! qc.setQuickCheckPath(lilfes::FSP(mach, quick_check_path))) return false;
            is_quick_check = true;
            return true;
        }

        void disableQuickCheck() { is_quick_check = false; }

    private:
        // comma_pred(name, xfs, yfs, mother) :- id_schema_binary(name, xfs, yfs, mother, dcp), call(dcp).
        lilfes::FSP makeBinarySchemaFSP(lilfes::FSP name, lilfes::FSP xfs, lilfes::FSP yfs,
                                        lilfes::FSP mother)
        {
            lilfes::FSP dcp(mach);

            lilfes::FSP id_schema_binary_pred(mach);
            id_schema_binary_pred.Coerce(t_id_schema_binary);
            id_schema_binary_pred.Follow(lilfes::f_arg[1]).Unify(name);
            id_schema_binary_pred.Follow(lilfes::f_arg[2]).Unify(xfs);
            id_schema_binary_pred.Follow(lilfes::f_arg[3]).Unify(yfs);
            id_schema_binary_pred.Follow(lilfes::f_arg[4]).Unify(mother);
            id_schema_binary_pred.Follow(lilfes::f_arg[5]).Unify(dcp);

            lilfes::FSP comma_pred(mach);
            comma_pred.Coerce(lilfes::t_comma);
            comma_pred.Follow(lilfes::f_arg[1]).Unify(id_schema_binary_pred);
            comma_pred.Follow(lilfes::f_arg[2]).Unify(dcp);

            return comma_pred;
        }

        // comma_pred(name, dtr, mother) :- id_schema_unary(name, dtr, mother, dcp), call(dcp).
        lilfes::FSP makeUnarySchemaFSP(lilfes::FSP name, lilfes::FSP dtr, lilfes::FSP mother)
        {
            lilfes::FSP dcp(mach);

            lilfes::FSP id_schema_unary_pred(mach);
            id_schema_unary_pred.Coerce(t_id_schema_unary);
            id_schema_unary_pred.Follow(lilfes::f_arg[1]).Unify(name);
            id_schema_unary_pred.Follow(lilfes::f_arg[2]).Unify(dtr);
            id_schema_unary_pred.Follow(lilfes::f_arg[3]).Unify(mother);
            id_schema_unary_pred.Follow(lilfes::f_arg[4]).Unify(dcp);

            lilfes::FSP comma_pred(mach);
            comma_pred.Coerce(lilfes::t_comma);
            comma_pred.Follow(lilfes::f_arg[1]).Unify(id_schema_unary_pred);
            comma_pred.Follow(lilfes::f_arg[2]).Unify(dcp);

            return comma_pred;
        }

        bool applyIdSchemaBinaryNoQc(eserial xsn, eserial ysn, lilfes::FSP xfs, lilfes::FSP yfs,
                                     std::vector<Fs*>& signs, std::vector<lserial>& links,
                                     EdgeVec& edge_vec, LinkVec& link_vec)
        {
            lilfes::IPTrailStack iptrail(mach);

            lilfes::FSP name(mach);
            lilfes::FSP mother(mach);
            //lilfes::FSP dcp(mach);
            lilfes::FSP reduced_mother(mach);
            lilfes::FSP sign_plus(mach);

            // Schema application + dcp call
            lilfes::FSP binarySchemaPred = makeBinarySchemaFSP(name, xfs, yfs, mother);
            if (! lilfes::builtin::lilfes_call(*mach, binarySchemaPred) || mach->GetIP() == NULL) return true;

            for (bool loop = mach->Execute(mach->GetIP()); loop ; loop = mach->NextAnswer()) {
                if (! reduceSign(mother, reduced_mother, sign_plus)) continue;
                lilfes::FSP dfom(mach);
                {
                    P2("Grammar::applyIdSchemaBinary--fomBinary");
                    if (! fomBinary(name, xfs, yfs, reduced_mother, sign_plus, dfom)) continue;
                }
                {
                    P2("Grammar::applyIdSchemaBinary--makeLink");
                    double fom = dfom.ReadFloat() + edge_vec[xsn].getFom() + edge_vec[ysn].getFom();
                    signs.push_back(reduced_mother.Serialize());
                    lserial lsn = link_vec.size();
                    links.push_back(lsn);
                    link_vec.push_back(Link(lsn, xsn, ysn, fom));
                    sign_plus.Serialize(link_vec.back().getSignPlus());
                    name.Serialize(link_vec.back().getAppliedSchema());
                }
            }

            //CHANGE 2011-09-30 matuzaki//bool loop = idSchemaBinary(name, xfs, yfs, mother, dcp);
            //CHANGE 2011-09-30 matuzaki//for ( ; loop ; loop = mach->NextAnswer()) {
            //CHANGE 2011-09-30 matuzaki//    lilfes::IPTrailStack iptrail2(mach);
            //CHANGE 2011-09-30 matuzaki//    if (! lilfes::builtin::lilfes_call(*mach, dcp) || mach->GetIP() == NULL) continue;
            //CHANGE 2011-09-30 matuzaki//    bool loop2 = mach->Execute(mach->GetIP());
            //CHANGE 2011-09-30 matuzaki//    for ( ; loop2 ; loop2 = mach->NextAnswer()) {
            //CHANGE 2011-09-30 matuzaki//        if (! reduceSign(mother, reduced_mother, sign_plus)) continue;
            //CHANGE 2011-09-30 matuzaki//        lilfes::FSP dfom(mach);
            //CHANGE 2011-09-30 matuzaki//        {
            //CHANGE 2011-09-30 matuzaki//            P2("Grammar::applyIdSchemaBinary--fomBinary");
            //CHANGE 2011-09-30 matuzaki//            if (! fomBinary(name, xfs, yfs, reduced_mother, sign_plus, dfom)) continue;
            //CHANGE 2011-09-30 matuzaki//        }
            //CHANGE 2011-09-30 matuzaki//        {
            //CHANGE 2011-09-30 matuzaki//            P2("Grammar::applyIdSchemaBinary--makeLink");
            //CHANGE 2011-09-30 matuzaki//            double fom = dfom.ReadFloat() + edge_vec[xsn].getFom() + edge_vec[ysn].getFom();
            //CHANGE 2011-09-30 matuzaki//            signs.push_back(reduced_mother.Serialize());
            //CHANGE 2011-09-30 matuzaki//            lserial lsn = link_vec.size();
            //CHANGE 2011-09-30 matuzaki//            links.push_back(lsn);
            //CHANGE 2011-09-30 matuzaki//            link_vec.push_back(Link(lsn, xsn, ysn, fom));
            //CHANGE 2011-09-30 matuzaki//            sign_plus.Serialize(link_vec.back().getSignPlus());
            //CHANGE 2011-09-30 matuzaki//            name.Serialize(link_vec.back().getAppliedSchema());
            //CHANGE 2011-09-30 matuzaki//        }
            //CHANGE 2011-09-30 matuzaki//    }
            //CHANGE 2011-09-30 matuzaki//}

            return true;
        }

        bool applyIdSchemaBinaryQc(eserial xsn, eserial ysn, lilfes::FSP xfs, lilfes::FSP yfs,
                                   std::vector<Fs*>& signs, std::vector<lserial>& links,
                                   EdgeVec& edge_vec, LinkVec& link_vec, int n)
        {
            if (! qc.isValid(xsn)) {
                qc.makeQca(xsn, &(edge_vec[xsn].getSign()));
            }

            if (qc.getIdSchemaQca(xsn).size() > 0 && ! qc.isValid(ysn)) {
                qc.makeQca(ysn, &(edge_vec[ysn].getSign()), (edge_vec[ysn].getRightPosition() == n));
            }

            std::vector<std::pair<Fs*, qc::QCA*> >& idschema_qca = qc.getIdSchemaQca(xsn);

            std::vector<std::pair<Fs*, qc::QCA*> >::iterator it = idschema_qca.begin();
            std::vector<std::pair<Fs*, qc::QCA*> >::iterator last = idschema_qca.end();

            for (; it != last ; ++it) {

                qc::QCA* qcx = it->second;
                qc::QCA* qcy = qc.getEdgeQca(ysn);
                if (! qc.quickCheck(qcx, qcy)) continue;

                lilfes::IPTrailStack iptrail(mach);
                lilfes::FSP name(mach, it->first);
                lilfes::FSP mother(mach);
                //lilfes::FSP dcp(mach);
                lilfes::FSP reduced_mother(mach);
                lilfes::FSP sign_plus(mach);

                // Schema application + dcp call
                lilfes::FSP binarySchemaPred = makeBinarySchemaFSP(name, xfs, yfs, mother);
                if (! lilfes::builtin::lilfes_call(*mach, binarySchemaPred) || mach->GetIP() == NULL) continue;

                for (bool loop = mach->Execute(mach->GetIP()); loop; loop = mach->NextAnswer()) {
                    lilfes::FSP dfom(mach);
                    {
                        P2("Grammar::applyIdSchemaBinary -- reduceSign");
                        if ( ! reduceSign(mother, reduced_mother, sign_plus) ) continue;
                    }
                    {
                        P2("Grammar::applyIdSchemaBinary -- fomBinary");
                        if ( ! fomBinary(name, xfs, yfs, reduced_mother, sign_plus, dfom) ) continue;
                    }
                    {
                        P2("Grammar::applyIdSchemaBinary -- makeLink");
                        double fom = dfom.ReadFloat() + edge_vec[xsn].getFom() + edge_vec[ysn].getFom();
                        signs.push_back(reduced_mother.Serialize());
                        lserial lsn = link_vec.size();
                        links.push_back(lsn);
                        link_vec.push_back(Link(lsn, xsn, ysn, fom));
                        sign_plus.Serialize(link_vec.back().getSignPlus());
                        name.Serialize(link_vec.back().getAppliedSchema());
                    }
                }

                //CHANGE 2011-09-30 matuzaki//{
                //CHANGE 2011-09-30 matuzaki//    P2("Grammar::applyIdSchemaBinary--idSchemaBinary");
                //CHANGE 2011-09-30 matuzaki//    bool loop = idSchemaBinary(name, xfs, yfs, mother, dcp);
                //CHANGE 2011-09-30 matuzaki//    for (; loop ; loop = mach->NextAnswer()) {
                //CHANGE 2011-09-30 matuzaki//        lilfes::IPTrailStack iptrail2(mach);
                //CHANGE 2011-09-30 matuzaki//        {
                //CHANGE 2011-09-30 matuzaki//            P2("Grammar::applyIdSchemaBianry -- dcp call");
                //CHANGE 2011-09-30 matuzaki//            if (! lilfes::builtin::lilfes_call(*mach, dcp) || mach->GetIP() == NULL) continue;
                //CHANGE 2011-09-30 matuzaki//            bool loop2 = mach->Execute(mach->GetIP());
                //CHANGE 2011-09-30 matuzaki//            for (; loop2 ; loop2 = mach->NextAnswer()) {
                //CHANGE 2011-09-30 matuzaki//                lilfes::FSP dfom(mach);
                //CHANGE 2011-09-30 matuzaki//                {
                //CHANGE 2011-09-30 matuzaki//                    P2("Grammar::applyIdSchemaBinary -- reduceSign");
                //CHANGE 2011-09-30 matuzaki//                    if ( ! reduceSign(mother, reduced_mother, sign_plus) ) continue;
                //CHANGE 2011-09-30 matuzaki//                }
                //CHANGE 2011-09-30 matuzaki//                {
                //CHANGE 2011-09-30 matuzaki//                    P2("Grammar::applyIdSchemaBinary -- fomBinary");
                //CHANGE 2011-09-30 matuzaki//                    if ( ! fomBinary(name, xfs, yfs, reduced_mother, sign_plus, dfom) ) continue;
                //CHANGE 2011-09-30 matuzaki//                }
                //CHANGE 2011-09-30 matuzaki//                {
                //CHANGE 2011-09-30 matuzaki//                    P2("Grammar::applyIdSchemaBinary -- makeLink");
                //CHANGE 2011-09-30 matuzaki//                    double fom = dfom.ReadFloat() + edge_vec[xsn].getFom() + edge_vec[ysn].getFom();
                //CHANGE 2011-09-30 matuzaki//                    signs.push_back(reduced_mother.Serialize());
                //CHANGE 2011-09-30 matuzaki//                    lserial lsn = link_vec.size();
                //CHANGE 2011-09-30 matuzaki//                    links.push_back(lsn);
                //CHANGE 2011-09-30 matuzaki//                    link_vec.push_back(Link(lsn, xsn, ysn, fom));
                //CHANGE 2011-09-30 matuzaki//                    sign_plus.Serialize(link_vec.back().getSignPlus());
                //CHANGE 2011-09-30 matuzaki//                    name.Serialize(link_vec.back().getAppliedSchema());
                //CHANGE 2011-09-30 matuzaki//                }
                //CHANGE 2011-09-30 matuzaki//            }
                //CHANGE 2011-09-30 matuzaki//        }
                //CHANGE 2011-09-30 matuzaki//    }
                //CHANGE 2011-09-30 matuzaki//}
            }

            return true;
        }
    };
}

#endif // __Grammar_h

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.8  2009/04/06 06:12:03  yusuke
 *
 * bug fix: is_quick_check was not initialized
 *
 * Revision 1.7  2009/01/06 11:35:31  matuzaki
 *
 * fix for parsing with up
 *
 * fix for making 'up -nofom' work
 *
 * Revision 1.6  2008/12/11 02:49:03  yusuke
 *
 * partial support for MacOS X (enju works)
 *
 * Revision 1.5  2008/06/05 20:24:10  matuzaki
 *
 * Merge mogura
 *
 * Merge mogura, separation of Supertagger
 *
 * addtion: lexmerge
 * update: derivtoxml
 *
 * separation of supertagger
 *
 * separation of supertagger
 *
 * Separation of Supertagger
 *
 * Revision 1.4.6.1  2008/04/28 06:30:52  matuzaki
 *
 * initial version of mogura parser
 *
 * initial version of mogura parser
 *
 *
 * Separate supertagger class
 *
 * Separate supertagger class
 *
 * Revision 1.4  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
