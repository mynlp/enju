/*
 * $Id: Qc.cc,v 1.4 2009-12-03 23:33:28 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#include "Qc.h"
#include "Grammar.h"

namespace up {
    using namespace lilfes;

    void QcManager::makeQca(eserial esn, Fs* x, bool rightmost) {
        P2("QcManager::makeQca");

        if(esn >= edge_qca.size()) {
            edge_qca.resize(esn+1, 0);
            idschema_qca.resize(esn+1);
        }
        lilfes::IPTrailStack iptrail(mach);
        lilfes::FSP left(mach, x);
        edge_qca[esn] = qcam->MakeQCA(left);

        if(!rightmost) {
            lilfes::FSP name(mach); lilfes::FSP right(mach); lilfes::FSP mother(mach); lilfes::FSP dcp(mach);

            bool loop = grammar->idSchemaBinary(name, left, right, mother, dcp);
            for(; loop ; loop = mach->NextAnswer()) {
                idschema_qca[esn].push_back(std::make_pair(name.Serialize(),
                            qcam->MakeQCA(right)));
            }
        }
    }
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
