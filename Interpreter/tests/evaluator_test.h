#pragma once

#include "../Lexer.h"
#include "../Parser.h"
#include "../Object.h"
#include "../Evaluator.h"

#include <iostream>
#include <vector>
#include <cassert>

void TestEvalNumericalExpression();
void TestEvalBooleanExpression();
void TestBangOperator();