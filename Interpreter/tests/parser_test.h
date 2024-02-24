#pragma once

#include <iostream>
#include <cstdlib>
#include "../Parser.h"
#include "../Lexer.h"
#include <memory> 
#include <variant>

using LiteralTypeValues = std::variant<int64_t, std::string, bool>;

void TestTypedDeclStatements();
void TestReturnStatements();
void TestIdentifierExpression();
void TestNumericalLiteralExpression();
void TestParsingPrefixExpressions();
void TestParsingInfixExpressions();
void TestOperatorPrecedenceParsing();
void TestBooleanExpression();
void TestIfExpression();
void TestIfElseExpression();

bool testIdentifier(const std::unique_ptr<Expression>& exp, const std::string& value);
bool testLiteralExpression(const std::unique_ptr<Expression>& exp, const LiteralTypeValues& expected);
bool testInfixExpression(const std::unique_ptr<Expression>& exp, const LiteralTypeValues& left, const std::string& operator_, const LiteralTypeValues& right);
bool testBooleanLiteral(const std::unique_ptr<Expression>& exp, bool value);