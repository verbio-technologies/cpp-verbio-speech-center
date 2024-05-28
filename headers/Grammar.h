#ifndef SPEECHCENTER_GRAMMAR_H
#define SPEECHCENTER_GRAMMAR_H

#include <string>
#include <vector>

enum GrammarType {
    NONE,
    INLINE,
    URI,
    COMPILED
};

class Grammar {
public:
    Grammar();

    Grammar(const GrammarType type, const std::string content);

    ~Grammar();

    GrammarType getType() const;

    std::string getContent() const;

    std::vector<char> getCompiledBytes() const;

private:
    void readCompiledGrammar();

    GrammarType type;
    std::string content;
    std::vector<char> compiledBytes;
};


#endif //SPEECHCENTER_GRAMMAR_H
