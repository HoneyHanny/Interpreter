#include <iostream>
#include <vector>
#include "Lexer_test.h"
#include "Lexer.h"

static void assertEqual(const std::string_view& actual, const std::string_view& expected, const std::string& message) {
    if (actual != expected) {
        std::cerr << " Assertion failed: " << message << "\n"
            << "Expected: " << expected << ", Actual: " << actual << std::endl;
        std::exit(EXIT_FAILURE); // Exit the program with a failure status
    }
}

void TestNextToken() {
    std::string input = "=+(){},;";
    Lexer l(input);

    std::vector<std::pair<TokenType, std::string>> tests = {
        {ASSIGN, "="},
        {PLUS, "+"},
        {LPAREN, "("},
        {RPAREN, ")"},
        {LBRACE, "{"},
        {RBRACE, "}"},
        {COMMA, ","},
        {SEMICOLON, ";"},
        {EOF_TOKEN, ""}, 
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        Token tok = l.NextToken();
        std::cout << "Testing token: " << tok.Literal << " ";
        assertEqual(tok.Type, tests[i].first, "Test[" + std::to_string(i) + "] - TokenType wrong.");
        assertEqual(tok.Literal, tests[i].second, "Test[" + std::to_string(i) + "] - Literal wrong.");
        std::cout << "Test passed!" << std::endl;
    }

    std::cout << "All tests passed!" << std::endl;
}

void TestNextToken2() {
    std::string input = R"(let five = 5;
let ten = 10;

let add = fn(x, y) {
	x + y;
};

let result = add(five, ten);
)";

    Lexer l(input);

    std::vector<std::pair<TokenType, std::string>> tests = {
        {LET, "let"},
        {IDENT, "five"},
        {ASSIGN, "="},
        {INT, "5"},
        {SEMICOLON, ";"},
        {LET, "let"},
        {IDENT, "ten"},
        {ASSIGN, "="},
        {INT, "10"},
        {SEMICOLON, ";"},
        {LET, "let"},
        {IDENT, "add"},
        {ASSIGN, "="},
        {FUNCTION, "fn"},
        {LPAREN, "("},
        {IDENT, "x"},
        {COMMA, ","},
        {IDENT, "y"},
        {RPAREN, ")"},
        {LBRACE, "{"},
        {IDENT, "x"},
        {PLUS, "+"},
        {IDENT, "y"},
        {SEMICOLON, ";"},
        {RBRACE, "}"},
        {SEMICOLON, ";"},
        {LET, "let"},
        {IDENT, "result"},
        {ASSIGN, "="},
        {IDENT, "add"},
        {LPAREN, "("},
        {IDENT, "five"},
        {COMMA, ","},
        {IDENT, "ten"},
        {RPAREN, ")"},
        {SEMICOLON, ";"},
        {EOF_TOKEN, ""},
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        Token tok = l.NextToken();
        std::cout << "Testing token: " << tok.Literal;
        assertEqual(tok.Type, tests[i].first, "Test[" + std::to_string(i) + "] - TokenType wrong.");
        assertEqual(tok.Literal, tests[i].second, "Test[" + std::to_string(i) + "] - Literal wrong.");
        std::cout << " Test passed!" << std::endl;
    }

    std::cout << "All tests passed!" << std::endl;
}