#pragma once
#include <string>
#include "Token.h"

class Lexer {
public:
    Lexer(const std::string& input); 

    void readChar(); // Reads the next character and advances the position
    Token NextToken(); // Tokenizer


private:
    std::string input; // Input string being parsed
    int position;      // Current position in input (points to current char)
    int readPosition;  // Current reading position in input (after current char)
    char ch;           // Current char under examination

    std::string readIdentifier();
    void skipWhitespace();
    std::string_view lookupIdent(const std::string& ident);
    std::string readNumber();

    static bool isLetter(char ch) {
        return std::isalpha(ch) || ch == '_';
    }

    static bool isDigit(char ch) {
        return '0' <= ch && ch <= '9';
    }

};


