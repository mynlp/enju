/*
 * $Id: ParserBuiltin.h,v 1.5 2009-12-03 23:33:28 matuzaki Exp $
 *
 *  Copyright (c) 2005, Tsujii Laboratory, The University of Tokyo.
 *  All rights reserved.
 *
 */

#ifndef __ParserBuiltin_h__
#define __ParserBuiltin_h__

namespace lilfes {
    // To link with up-builtin predicates
    extern void *UP_BUILTIN_PRED_SYMBOLS[];
}

namespace up {
    class Parser;
    class LexAnalyzer;
    class Grammar;
    
    class Builtin {
    private:
        static Grammar* grammar;
        static Parser* parser;
        static LexAnalyzer* lex_analyzer;
        
    public:
        static Grammar* getGrammar() { return grammar; }
        static void setGrammar(Grammar* g) { grammar = g; }

        static Parser* getParser() { return parser; }
        static void setParser(Parser* p) { parser = p; }

        static LexAnalyzer* getLexAnalyzer() { return lex_analyzer; }
        static void setLexAnalyzer(LexAnalyzer* la) { lex_analyzer = la; }
    };
    
} // end of namespace up

#endif // __ParserBuiltin_h__

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2008/07/14 22:46:54  matuzaki
 *
 * UP_BUILTIN_PRED_SYMBOL array was added to link built-in predicates with programs
 *
 * Revision 1.3  2007/09/07 00:23:53  ninomi
 * grammar-refine-1 is merged to trunk.
 *
 */
