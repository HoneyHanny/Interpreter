#pragma once

#include <sstream>
#include <string>
#include <memory>
#include "AST.h"
#include "Object.h"
#include "Environment.h"

std::unique_ptr<Object> Eval(const Node* node, const std::shared_ptr<Environment>& env);