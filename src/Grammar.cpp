#include "Grammar.h"

Grammar::Grammar() {
    type = NONE;
    content = "";
}

Grammar::Grammar(const GrammarType type, const std::string content) {
    this->type = type;
    this->content = content;
}

Grammar::~Grammar() = default;

GrammarType Grammar::getType() const {
    return type;
}

std::string Grammar::getContent() const {
    return content;
}