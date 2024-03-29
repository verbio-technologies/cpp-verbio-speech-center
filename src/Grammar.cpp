#include "Grammar.h"
#include <fstream>

Grammar::Grammar() : type{NONE}, content{} {}

Grammar::Grammar(const GrammarType type, const std::string content) : type(type), content(content) {
    if (type == COMPILED) {
        std::ifstream input(content, std::ios::binary);
        compiledBytes = std::vector<char> (
                (std::istreambuf_iterator<char>(input)),
                (std::istreambuf_iterator<char>()));
        input.close();
    }
}

Grammar::~Grammar() = default;

GrammarType Grammar::getType() const {
    return type;
}

std::string Grammar::getContent() const {
    return content;
}

std::vector<char> Grammar::getCompiledBytes() const {
    return compiledBytes;
}