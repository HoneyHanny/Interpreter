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