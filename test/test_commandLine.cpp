
#include <catch2/catch.hpp>

#include "Configuration.h"

TEST_CASE("Command line", "[command_line]") {

    SECTION("happy path") {
        int argc = 8;
        const char* argv[] = {"cli_client", "-a", "file.wav", "-b", "file.bnf", "-l pt-BR", "-T", "file.token"};

    }

}
