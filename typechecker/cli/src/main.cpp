#include <iostream>

#include "stella/parser.hpp"
#include "stella/typecheck/check.hpp"
#include "stella/typecheck/error.hpp"

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file>" << std::endl;
        return 1;
    }

    try {
        std::string filename = argv[1];
        auto program = stella::ParseProgramFile(filename);

        stella::typecheck::CheckProgram(*program);

        std::cout << "Well-typed" << std::endl;
    } catch (const stella::typecheck::TypeCheckError& e) {
        std::cerr << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}