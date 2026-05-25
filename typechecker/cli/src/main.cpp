#include <iostream>
#include <iterator>
#include <loguru.hpp>
#include <string>

#include "stella/parser.hpp"
#include "stella/typecheck/check.hpp"
#include "stella/typecheck/error.hpp"

int main(int argc, const char* argv[]) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            // Включаем все логи, включая INFO и DEBUG
            loguru::g_stderr_verbosity = loguru::Verbosity_MAX;
        }
    }

    try {
        std::string input{std::istreambuf_iterator<char>(std::cin),
                          std::istreambuf_iterator<char>()};

        auto program = stella::ParseProgramText(std::string_view{input});

        stella::typecheck::CheckProgram(*program);

        std::cout << "Well-typed" << std::endl;
    } catch (const stella::typecheck::TypeCheckError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}