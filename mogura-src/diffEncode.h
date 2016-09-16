#ifndef diffEncode_h__
#define diffEncode_h__

#include <iostream>

namespace mogura {
namespace cfg {
    class Grammar;
    void loadDiffEncodedCfgFile(std::istream &ist, Grammar &grammar);
    void writeDiffEncodedCfgFile(const Grammar &grammar, std::ostream &ost);
    
} /// namespace cfg {
} /// namespace mogura {

#endif // diffEncode_h__
