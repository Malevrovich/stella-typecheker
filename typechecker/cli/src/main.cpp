#include <iostream>

#include "stella/parser.hpp"

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file>" << std::endl;
        return 1;
    }

    try {
        std::string filename = argv[1];
        auto program = stella::ParseProgramFile(filename);

        std::cout << *program << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}