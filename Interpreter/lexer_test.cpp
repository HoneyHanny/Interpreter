#include <iostream>
#include <vector>
#include "Lexer_test.h"
#include "Lexer.h"

static void assertEqual(const TokenType& actual, const TokenType& expected, const std::string& message) {
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
    std::string input = R"(
# this is a sample program in CODE
BEGIN CODE
    INT x, y, z = 5
    CHAR a_1 = 'n'
    BOOL t = "TRUE"
    x = y = 4 
    a_1 = 'c' 
    # this is a comment 
    DISPLAY: x & t & z & $ & a_1 & [#] & "last"
END CODE
)";

    Lexer l(input);

    std::vector<std::pair<TokenType, std::string>> tests = {
        {HASH, "#"},
        {IDENT, "this"},
        {IDENT, "is"},
        {IDENT, "a"},
        {IDENT, "sample"},
        {IDENT, "program"},
        {IDENT, "in"},
        {CODE, "CODE"},
        {BEGIN, "BEGIN"},
        {CODE, "CODE"},
        {INT, "INT"},
        {IDENT, "x"},
        {COMMA, ","},
        {IDENT, "y"},
        {COMMA, ","},
        {IDENT, "z"},
        {ASSIGN, "="},
        {NUM, "5"},
        {CHAR, "CHAR"},
        {IDENT, "a_1"},
        {ASSIGN, "="},
        {S_QUOTE, "'"},
        {IDENT, "n"},
        {S_QUOTE, "'"},
        {BOOL, "BOOL"},
        {IDENT, "t"},
        {ASSIGN, "="},
        {D_QUOTE, "\""},
        {TRUE, "TRUE"},
        {D_QUOTE, "\""},
        {IDENT, "x"},
        {ASSIGN, "="},
        {IDENT, "y"},
        {ASSIGN, "="},
        {NUM, "4"},
        {IDENT, "a_1"},
        {ASSIGN, "="},
        {S_QUOTE, "'"},
        {IDENT, "c"},
        {S_QUOTE, "'"},
        {HASH, "#"},
        {IDENT, "this"},
        {IDENT, "is"},
        {IDENT, "a"},
        {IDENT, "comment"},
        {DISPLAY, "DISPLAY"},
        {COLON, ":"},
        {IDENT, "x"},
        {AMPERSAND, "&"},
        {IDENT, "t"},
        {AMPERSAND, "&"},
        {IDENT, "z"},
        {AMPERSAND, "&"},
        {DOLLAR_SIGN, "$"},
        {AMPERSAND, "&"},
        {IDENT, "a_1"},
        {AMPERSAND, "&"},
        {LSQBRACE, "["},
        {HASH, "#"},
        {RSQBRACE, "]"},
        {AMPERSAND, "&"},
        {D_QUOTE, "\""},
        {IDENT, "last"},
        {D_QUOTE, "\""},
        {END, "END"},
        {CODE, "CODE"},
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

void TestNextToken3() {
    std::string input = R"(= == <> <= >=)";
    Lexer l(input);

    std::vector<std::pair<TokenType, std::string>> tests = {
        {ASSIGN, "="},
        {EQ, "=="},
        {NEQ, "<>"},
        {LEQT, "<="},
        {GEQT, ">="},
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
