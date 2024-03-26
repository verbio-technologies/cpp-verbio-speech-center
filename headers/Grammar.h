#ifndef SPEECHCENTER_GRAMMAR_H
#define SPEECHCENTER_GRAMMAR_H

#include <string>

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

private:
    GrammarType type;
    std::string content;
};


#endif//SPEECHCENTER_GRAMMAR_H
