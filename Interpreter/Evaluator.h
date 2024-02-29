#pragma once

#include <sstream>
#include <string>
#include <memory>
#include "AST.h"
#include "Object.h"

std::unique_ptr<Object> Eval(const Node* node);