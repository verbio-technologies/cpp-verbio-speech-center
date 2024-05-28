#include "Grammar.h"

#include <fstream>
#include <filesystem>

Grammar::Grammar() : type{NONE}, content{} {}

Grammar::Grammar(const GrammarType type, const std::string content) : type(type), content(content) {
    if (type == COMPILED) {
        readCompiledGrammar();
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

void Grammar::readCompiledGrammar() {
    if(!std::filesystem::exists(content)) {
        throw std::invalid_argument("Compiled grammar file '" + content + "' does not exist.");
    }
    const auto kXzExtension = std::filesystem::path(content).extension();
    const auto kFileNameWithoutXzExtension = std::filesystem::path(content).stem();
    const auto kTarExtension = std::filesystem::path(kFileNameWithoutXzExtension).extension();
    if(kXzExtension != ".xz" || kTarExtension != ".tar") {
        throw std::invalid_argument("Compiled grammar file '" + content + "' extension is not .tar.xz.");
    }
    std::ifstream input(content, std::ios::binary);
    compiledBytes = std::vector<char> (
            (std::istreambuf_iterator<char>(input)),
            (std::istreambuf_iterator<char>()));
    input.close();
}