#ifndef __DataConv_h__
#define __DataConv_h__

#include "Grammar.h"
#include "ParseStatus.h"
#include "Extent.h"

// data conversion between C++ <=> lilfes
namespace up { namespace conv {

    lilfes::type* convParseStatus(up::ParseStatus status);


    bool convLexEntLattice(lilfes::machine* mach, Grammar* grammar, lilfes::FSP latticeFsp,
                           LexEntLattice& lexent_lattice, Brackets& brackets);

}} // namespace conv; namespace up

#endif // __DataConv_h__
