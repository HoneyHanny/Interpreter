#pragma once
#include "tests/lexer_test.h"
#include "tests/parser_test.h"
#include "tests/ast_test.h"
#include "tests/evaluator_test.h"

void ExecuteTestCases() {

    // Testing for tokenizer
    //std::cout << "Test 1: " << std::endl;
    //TestNextToken();

    //std::cout << "Test 2: " << std::endl;
    //TestNextToken2();

    //std::cout << "Test 3: " << std::endl;
    //TestNextToken3();

    // Testing for parser
    //TestTypedDeclStatements();
    //TestReturnStatements();
    //TestIdentifierExpression();
    //TestNumericalLiteralExpression();
    //TestParsingPrefixExpressions();
    //TestParsingInfixExpressions();
    //TestOperatorPrecedenceParsing();
    //TestBooleanExpression();
    //TestIfExpression();
    //TestNestedIfExpression();
    //TestNestedIfInElseExpression();
    //TestIfElseExpression();
    //TestFunctionLiteralParsing();
    //TestFunctionParameterParsing();
    //TestCallExpressionParsing();

    // Testing for AST
    //TestString();

    // Testing for evaluator
    //TestEvalNumericalExpression();
    //TestEvalBooleanExpression();
    //TestBangOperator();
    //TestIfElseExpressions();
    //TestReturnStatementsV2();
    //TestErrorHandling();
    //TestFunctionObject();
    //TestFunctionApplication();
    TestClosures();
}