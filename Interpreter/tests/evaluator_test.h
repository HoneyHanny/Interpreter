#pragma once

#include "../Lexer.h"
#include "../Parser.h"
#include "../Object.h"
#include "../Evaluator.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <cassert>
#include <variant>

void TestEvalNumericalExpression();
void TestEvalBooleanExpression();
void TestBangOperator();
void TestIfElseExpressions();
void TestReturnStatementsV2();
void TestErrorHandling();
void TestTypedDeclStatementsV2();
void TestFunctionObject();
void TestFunctionApplication();
void TestClosures();
void TestStringLiteral();
void TestStringConcatenation();
void TestCharLiteral();
void TestBuiltinFunctions();