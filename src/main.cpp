#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "codegen/codegen.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }
    
    // Read input file
    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open input file" << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string source = buffer.str();
    
    try {
        // Tokenize
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        std::cout << "Tokens found:" << std::endl;
        for (const auto& token : tokens) {
            std::cout << token.toString() << std::endl;
        }
        
        // Parse and generate code for statements
        CodeGenerator codegen;
        codegen.cgpreamble();
        Parser parser(tokens);
        parser.parseStatements(codegen);
        codegen.cgpostamble();
        
        // Write assembly to output file
        std::string outputFile = std::string(argv[1]) + ".s";
        std::ofstream output(outputFile);
        if (!output.is_open()) {
            std::cerr << "Error: Could not create output file" << std::endl;
            return 1;
        }
        
        output << codegen.generateAssembly(nullptr); // Output the buffer
        std::cout << "\nAssembly written to " << outputFile << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
