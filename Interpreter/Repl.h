#pragma once

#include <iostream>
#include <string>
#include "Lexer.h"
#include "Token.h" 
#include "Parser.h"
#include "Evaluator.h"

class Repl {
public:
    void Start(std::istream& in, std::ostream& out);
};
