#pragma once

#include <iostream>
#include <string>
#include "Lexer.h"
#include "Token.h" 
#include "Parser.h"

class Repl {
public:
    void Start(std::istream& in, std::ostream& out);
};
