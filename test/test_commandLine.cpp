
#include <gtest/gtest.h>

#include "Configuration.h"


TEST(CommandLine, happy_path) {
    int argc = 8;
    const char* argv[] = {"cli_client", "-a", "file.wav", "-b", "file.bnf", "-l pt-BR", "-t", "file.token", "-T", "GENERIC"};
}