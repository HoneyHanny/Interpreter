#include "ast_test.h"

#include <iostream>
#include "../AST.h" 

void TestString() {
    // Create a program with one TypedDeclStatement
    auto program = std::make_unique<Program>();
    program->Statements.push_back(std::make_unique<TypedDeclStatement>(
        Token{ INT, "INT" }, 
        std::make_unique<Identifier>(Token{ IDENT, "myVar" }, "myVar"),
        std::make_unique<Identifier>(Token{ IDENT, "anotherVar" }, "anotherVar")
    ));

    std::string expected = "INT myVar = anotherVar";
    if (program->String() != expected) {
        std::cerr << "TestString failed: expected '" << expected << "', got '" << program->String() << "'" << std::endl;
        std::exit(EXIT_FAILURE); 
    }
    else {
        std::cout << "TestString passed." << std::endl;
    }
}

