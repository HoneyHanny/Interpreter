#pragma once

#include <iostream>
#include <string>
#include "Lexer.h"
#include "Token.h" 

class Repl {
public:
    void Start(std::istream& in, std::ostream& out);
};
