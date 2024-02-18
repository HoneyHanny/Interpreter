#include "Lexer.h"

Lexer::Lexer(const std::string& input)
    : input(input), position(0), readPosition(0), ch(0) {
    readChar(); // Initialize the first character
}

// Public function definitions
void Lexer::readChar() {
    if (readPosition >= input.size()) {
        ch = 0; // Use 0 as EOF marker
    }
    else {
        ch = input[readPosition];
    }
    position = readPosition;
    ++readPosition;
}   

Token Lexer::NextToken() {
    Token tok;

    skipWhitespace();

    switch (ch) {
    case '=':
        tok = { ASSIGN, std::string(1, ch) };
        break;
    case ';':
        tok = { SEMICOLON, std::string(1, ch) };
        break;
    case '(':
        tok = { LPAREN, std::string(1, ch) };
        break;
    case ')':
        tok = { RPAREN, std::string(1, ch) };
        break;
    case ',':
        tok = { COMMA, std::string(1, ch) };
        break;
    case '+':
        tok = { PLUS, std::string(1, ch) };
        break;
    case '{':
        tok = { LBRACE, std::string(1, ch) };
        break;
    case '}':
        tok = { RBRACE, std::string(1, ch) };
        break;
    case 0: // End of file / input
        tok.Type = EOF_TOKEN;
        tok.Literal = "";
        break;
    default:
        if (isLetter(ch)) {
            tok.Literal = readIdentifier();
            tok.Type = lookupIdent(tok.Literal); // Determine if it's a keyword or just an IDENT
            return tok;
        }
        else if (isDigit(ch)) {
            tok.Literal = readNumber();
            tok.Type = INT; // Use INT type for numbers
            return tok; // Early return for numbers
        }
        else {
            tok = { ILLEGAL, std::string(1, ch) };
        }
        break;
    }
    readChar(); // Move to the next character
    return tok;
}

// Private function definitions
std::string Lexer::readIdentifier() {
    int startPosition = position;
    while (isLetter(ch)) {
        readChar();
    }
    return std::string(input.substr(startPosition, position - startPosition));
}

void Lexer::skipWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        readChar();
    }
}

std::string_view Lexer::lookupIdent(const std::string & ident) { // Implement lookupIdent here to distinguish between keywords and identifiers
    // Example implementation
    if (ident == "fn") return FUNCTION;
    if (ident == "let") return LET;
    // Add more keywords as needed
    return IDENT;
}

std::string Lexer::readNumber() {
    int startPosition = position;
    while (isDigit(ch)) {
        readChar();
    }
    // Since input is a std::string_view, we directly return a std::string part of it
    return std::string(input.substr(startPosition, position - startPosition));
}