#include "parser_test.h"

#include <iostream>
#include <cstdlib>
#include "../Parser.h"
#include "../Lexer.h"
#include <memory> 

static void assertEqual(const std::string& actual, const std::string& expected, const std::string& message) {
    if (actual != expected) {
        std::cerr << "Assertion failed: " << message << "\n"
            << "Expected: " << expected << ", Actual: " << actual << std::endl;
        std::exit(EXIT_FAILURE); // Exit the program with a failure status
    }
}

static void checkParserErrors(const Parser& parser) {
    const auto& errors = parser.Errors();
    if (errors.empty()) {
        return;
    }

    std::cerr << "CODE - " << errors.size() << " error/s occured:" << std::endl;
    for (const auto& msg : errors) {
        std::cerr << "Error: " << msg << std::endl;
    }

    std::exit(EXIT_FAILURE);
}

bool testTypedDeclStatement(Statement* s, const std::string& name);

void TestTypedDeclStatements() {
    std::string input = R"(
INT x = 5
FLOAT y = 10
CHAR foobar = 10
)";
        

    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    checkParserErrors(parser);

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
    std::string tokenLiteral = s->TokenLiteral();
    if (tokenLiteral != "INT" && tokenLiteral != "FLOAT" && tokenLiteral != "BOOL" && tokenLiteral != "CHAR") {
        std::cerr << "s.TokenLiteral not one of 'INT', 'FLOAT', 'BOOL', 'CHAR'. got=" << tokenLiteral << std::endl;
        return false;
    }

    TypedDeclStatement* typedDeclStmt = dynamic_cast<TypedDeclStatement*>(s);
    if (typedDeclStmt == nullptr) {
        std::cerr << "s not TypedStatement. got=" << typeid(*s).name() << std::endl;
        return false;
    }

    assertEqual(typedDeclStmt->Name->Value, name, "typedDeclStmt.Name.Value not '" + name + "'.");
    assertEqual(typedDeclStmt->Name->TokenLiteral(), name, "typedDeclStmt.Name.TokenLiteral not '" + name + "'.");

    return true;
}


bool testReturnStatement(Statement* s, const std::string& name);

void TestReturnStatements() {
    std::string input = R"(
RETURN 5
RETURN 10
RETURN 99999
)";


    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 3) {
        std::cerr << "program.Statements does not contain 3 statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<std::string> tests = { "x", "y", "foobar" };

    for (size_t i = 0; i < tests.size(); ++i) {
        if (!testReturnStatement(program->Statements[i].get(), tests[i])) {
            std::exit(EXIT_FAILURE);
        }
    }


    std::cout << "All tests passed!" << std::endl;

}

bool testReturnStatement(Statement* s, const std::string& name) {
    std::cerr << s->String() << std::endl;
    std::string tokenLiteral = s->TokenLiteral();
    if (tokenLiteral != "RETURN") {
        std::cerr << "s.TokenLiteral not RETURN'. got=" << tokenLiteral << std::endl;
        return false;
    }

    ReturnStatement* returnStmt = dynamic_cast<ReturnStatement*>(s);
    if (returnStmt == nullptr) {
        std::cerr << "s not ReturnStatement. got=" << typeid(*s).name() << std::endl;
        return false;
    }

    return true;
}

void TestIdentifierExpression() {
    std::string input = "foobar";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser); 

    if (program->Statements.size() != 1) {
        std::cerr << "program has not enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "program.Statements[0] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto ident = dynamic_cast<Identifier*>(stmt->Expression.get());
    if (!ident) {
        std::cerr << "exp not Identifier." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (ident->Value != "foobar") {
        std::cerr << "ident.Value not 'foobar'. got=" << ident->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (ident->TokenLiteral() != "foobar") {
        std::cerr << "ident.TokenLiteral not 'foobar'. got=" << ident->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestIdentifierExpression passed." << std::endl;
}

void TestIntegerLiteralExpression() {
    std::string input = "5";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 1) {
        std::cerr << "TestIntegerLiteralExpression failed: program has not enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "TestIntegerLiteralExpression failed: program.Statements[0] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto literal = dynamic_cast<IntegerLiteral*>(stmt->Expression.get());
    if (!literal) {
        std::cerr << "TestIntegerLiteralExpression failed: exp not IntegerLiteral." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->Value != 5) {
        std::cerr << "TestIntegerLiteralExpression failed: literal.Value not 5. got=" << literal->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->TokenLiteral() != "5") {
        std::cerr << "TestIntegerLiteralExpression failed: literal.TokenLiteral not '5'. got=" << literal->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestIntegerLiteralExpression passed." << std::endl;
}