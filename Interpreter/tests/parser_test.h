#pragma once

#include <iostream>
#include <cstdlib>
#include "../Parser.h"
#include "../Lexer.h"
#include <memory> 
#include <variant>

void TestTypedDeclStatements();
void TestReturnStatements();
void TestIdentifierExpression();
void TestNumericalLiteralExpression();
void TestParsingPrefixExpressions();
void TestParsingInfixExpressions();
void TestOperatorPrecedenceParsing();

bool testIdentifier(const std::unique_ptr<Expression>& exp, const std::string& value);
bool testLiteralExpression(const std::unique_ptr<Expression>& exp, const std::variant<int64_t, std::string>& expected);
bool testInfixExpression(const std::unique_ptr<Expression>& exp, const std::variant<int64_t, std::string>& left, const std::string& operator_, const std::variant<int64_t, std::string>& right);

void TestBooleanExpression();