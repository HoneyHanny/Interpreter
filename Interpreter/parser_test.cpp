#include "parser_test.h"

#include <iostream>
#include <cstdlib>
#include "Parser.h"
#include "Lexer.h"
#include <memory> 

void assertEqual(const std::string& actual, const std::string& expected, const std::string& message) {
    if (actual != expected) {
        std::cerr << "Assertion failed: " << message << "\n"
            << "Expected: " << expected << ", Actual: " << actual << std::endl;
        std::exit(EXIT_FAILURE); // Exit the program with a failure status
    }
}

bool testTypedDeclStatement(Statement* s, const std::string& name);

void TestTypedDeclStatements() {
    std::string input = R"(
INT x = 5;
INT y = 10;
INT foobar = 838383;
)";
        

    auto lexer = std::make_unique<Lexer>(input);
    Parser p(std::move(lexer));

    auto program = p.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    

    if (program->Statements.size() != 3) {
        std::cerr << "program.Statements does not contain 3 statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<std::string> tests = { "x", "y", "foobar" };

    for (size_t i = 0; i < tests.size(); ++i) {
        if (!testTypedDeclStatement(program->Statements[i].get(), tests[i])) {
            std::exit(EXIT_FAILURE);
        }
    }

    std::cout << "All tests passed!" << std::endl;
}

bool testTypedDeclStatement(Statement* s, const std::string& name) {
    assertEqual(s->TokenLiteral(), "INT", "s.TokenLiteral not 'INT'.");

    TypedDeclStatement* typedDeclStmt = dynamic_cast<TypedDeclStatement*>(s);
    if (typedDeclStmt == nullptr) {
        std::cerr << "s not TypedStatement. got=" << typeid(*s).name() << std::endl;
        return false;
    }

    assertEqual(typedDeclStmt->Name->Value, name, "typedDeclStmt.Name.Value not '" + name + "'.");
    assertEqual(typedDeclStmt->Name->TokenLiteral(), name, "typedDeclStmt.Name.TokenLiteral not '" + name + "'.");

    return true;
}
