#include <iostream>
#include <loguru.hpp>
#include <string>

#include "stella/parser.hpp"
#include "stella/typecheck/check.hpp"
#include "stella/typecheck/error.hpp"

int main(int argc, const char* argv[]) {
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

    bool verbose = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
            // Включаем все логи, включая INFO и DEBUG
            loguru::g_stderr_verbosity = loguru::Verbosity_MAX;
        } else if (arg[0] != '-') {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] <input-file>" << std::endl;
        std::cerr << "\nOptions:" << std::endl;
        std::cerr << "  -v, --verbose    Enable verbose debug output" << std::endl;
        return 1;
    }

    try {
        auto program = stella::ParseProgramFile(filename);

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