#pragma once
#include <string>
#include <iostream>
#include "Token.h"


class Lexer {
public:
    static int lexerCurrentLine;
    static std::vector<int> commentLinePositions;

    Lexer(const std::string& input); 

    void readChar(); // Reads the next character and advances the position
    char peekChar(); // Retrieves the next character in advance without incrementing the position (for ==, <>, etc..)
    Token NextToken(); // Tokenizer


private:
    std::string input; // Input string being parsed
    int position;      // Current position in input (points to current char)
    int readPosition;  // Current reading position in input (after current char)
    char ch;           // Current char under examination

    std::string readIdentifier(); // Check if keyword or identifier
    std::string readNumber(); // Check if number
    std::string readString(); // Read strings
    std::string readCharString(); // Read char 
    void skipComment();
    void skipWhitespace(); // Skips whitespace in code
    std::string escapeString(); // Escape string syntax
    TokenType lookupIdent(const std::string& ident); // Distinguishes between keywords and identifiers

    static bool isLetter(char ch) {
        return std::isalpha(ch)
            || ch == '_';
    }

    static bool isDigit(char ch) {
        return '0' <= ch && ch <= '9';
    }

};


