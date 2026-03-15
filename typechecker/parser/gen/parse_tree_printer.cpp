#include <fstream>
#include <iostream>

#include <antlr4-runtime.h>

#include "StellaLexer.h"
#include "StellaParser.h"

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file '" << argv[1] << "'" << std::endl;
        return 1;
    }

    try {
        antlr4::ANTLRInputStream stream(file);

        antlr4_stella::StellaLexer lexer(&stream);

        antlr4::CommonTokenStream tokens(&lexer);

        antlr4_stella::StellaParser parser(&tokens);

        antlr4_stella::StellaParser::Start_ProgramContext* tree = parser.start_Program();

        std::cout << antlr4::tree::Trees::toStringTree(tree, &parser) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Parsing error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}