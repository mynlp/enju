#ifndef __TagggedTokenSplitter_h__
#define __TagggedTokenSplitter_h__

#include <stack>
#include "LexAnalyzerPipe.h"

#ifdef _MSC_VER
#include <ctype.h>
#endif

namespace up {

    // For tagged, space-tokenized sentence
    class TaggedTokenSplitter : public PosTagger {
    public:
        void analyze(const std::string& sentence,
                     std::vector<TokenRegion>& tokens,
                     std::vector<BracketRegion>& bregions)
        {
            tokens.clear();
            bregions.clear();

            std::stack<unsigned> bracket_stack;

            std::string::const_iterator ch = sentence.begin();
            std::string::const_iterator end = sentence.end();
            while (ch != end) {
#ifdef _MSC_VER
                if (::isspace(*ch)) {
#else
                if (std::isspace(*ch)) {
#endif
                    ++ch;
                    continue;
                }

                std::string::const_iterator begin = ch;
#ifdef _MSC_VER
                while (ch != end && ! ::isspace(*ch)) {
#else
                while (ch != end && ! std::isspace(*ch)) {
#endif
                  ++ch;
                }

                unsigned b = std::distance(sentence.begin(), begin);
                unsigned e = std::distance(sentence.begin(), ch);
                std::string t(begin, ch);
                if (t == "(") {
                    bracket_stack.push(b);
                }
                else if (t == ")") {
                    if (bracket_stack.empty()) {
                        throw std::runtime_error("unbalanced brackets in the input");
                    }
                    BracketRegion bracket;
                    bracket.begin_offset = bracket_stack.top();
                    bracket.end_offset   = b;
                    bregions.push_back(bracket);

                    bracket_stack.pop();
                }
                else {
                    TokenRegion token;
                    token.begin_offset = b;
                    token.end_offset   = e;
                    splitTaggedWord(std::string(begin, ch), token.word, token.tag);
                    token.prob = 0;
                    tokens.push_back(token);
                }
            }
        }

    private:
        void splitTaggedWord(const std::string& word_tag, std::string& word, std::string& tag) const
        {
            
#if 0       // This re-implements corresponding lilfes predicates in enju:preproc, but doesn't work well
            // with the convention used in *.tagged file
            std::string::const_iterator slash = std::find(word_tag.begin(), word_tag.end(), '/');
            while (slash != word_tag.end()) {
                if (slash == word_tag.begin()) { // empty word
                    break;
                }

                if (slash + 1 == word_tag.end()) { // empty tag
                    break;
                }

                if (*(slash - 1) != '\\') {
                    word = std::string(word_tag.begin(), slash);
                    tag = std::string(slash + 1, word_tag.end());
                    return;
                }

                // search for next '/'
                slash = std::find(slash, word_tag.end(), '/');
            }

            throw std::runtime_error("input word format error: " + word_tag);
#endif
            std::string::size_type slash = word_tag.rfind('/');
            if (slash == std::string::npos || slash == 0 || slash == word_tag.size() - 1) {
                throw std::runtime_error("TaggedTokenSplitter: input word format error: " + word_tag);
            }
            word = word_tag.substr(0, slash);
            tag  = word_tag.substr(slash + 1);
        }
    };
} // namespace up

#endif // __TagggedTokenSplitter_h__
