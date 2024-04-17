#pragma once

#include <sstream>
#include <string>
#include <memory>
#include "AST.h"
#include "Object.h"
#include "Environment.h"
#include "Lexer.h"
std::shared_ptr<Object> Eval(const Node* node, const std::shared_ptr<Environment>& env);