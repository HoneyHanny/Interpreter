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

void TestEvalNumericalExpression();
void TestEvalBooleanExpression();
void TestBangOperator();
void TestIfElseExpressions();
void TestReturnStatementsV2();
void TestErrorHandling();
void TestFunctionObject();
void TestFunctionApplication();
void TestClosures();
void TestStringLiteral();
void TestStringConcatenation();