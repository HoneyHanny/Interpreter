#pragma once
#include "tests/lexer_test.h"
#include "tests/parser_test.h"
#include "tests/ast_test.h"

void ExecuteTestCases() {

    // Testing for tokenizer - uncomment to test tokenizer or add more test cases at lexer_test
    //std::cout << "Test 1: " << std::endl;
    //TestNextToken();

    //std::cout << "Test 2: " << std::endl;
    //TestNextToken2();

    //std::cout << "Test 3: " << std::endl;
    //TestNextToken3();

    // Testing for parser - uncomment to test parser or add more test cases at parser_test
    TestTypedDeclStatements();
    TestReturnStatements();
    TestIdentifierExpression();
    TestNumericalLiteralExpression();
    TestParsingPrefixExpressions();
    TestParsingInfixExpressions();
    TestOperatorPrecedenceParsing();
    TestBooleanExpression();
    TestIfExpression();
    TestIfElseExpression();
    TestFunctionLiteralParsing();
    TestFunctionParameterParsing();
    TestCallExpressionParsing();

    // Testing for AST - uncomment to test parser or add more test cases at ast_test
    //TestString();
}