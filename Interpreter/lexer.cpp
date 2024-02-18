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
    case '+':
        tok = { PLUS, std::string(1, ch) };
        break;
    case '-':
        tok = { MINUS, std::string(1, ch) };
        break;
    case '*':
        tok = { ASTERISK, std::string(1, ch) };
        break;
    case '/':
        tok = { SLASH, std::string(1, ch) };
        break;
    case '<':
        tok = { LT, std::string(1, ch) };
        break;
    case '>':
        tok = { GT, std::string(1, ch) };
        break;
    case ',':
        tok = { COMMA, std::string(1, ch) };
        break;
    case ':':
        tok = { COLON, std::string(1, ch) };
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
    case '{':
        tok = { LBRACE, std::string(1, ch) };
        break;
    case '}':
        tok = { RBRACE, std::string(1, ch) };
        break;
    case '[':
        tok = { LSQBRACE, std::string(1, ch) };
        break;
    case ']':
        tok = { RSQBRACE, std::string(1, ch) };
        break;
    case '#':
        tok = { HASH, std::string(1, ch) };
        break;
    case '.':
        tok = { DOT, std::string(1, ch) };
        break;
    case '\'':
        tok = { S_QUOTE, std::string(1, ch) };
        break;
    case '"':
        tok = { D_QUOTE, std::string(1, ch) };
        break;
    case '$':
        tok = { DOLLAR_SIGN, std::string(1, ch) };
        break;
    case '&':
        tok = { AMPERSAND, std::string(1, ch) };
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
            tok.Type = NUM; // Use INT type for numbers
            return tok;
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

    // Verify if the first index is a letter
    if (isLetter(ch)) {
        readChar();
    }

    // Continue reading
    while (isLetter(ch) || isDigit(ch)) {
        readChar();
    }

    return std::string(input.substr(startPosition, position - startPosition));
}

void Lexer::skipWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        readChar();
    }
}

TokenType Lexer::lookupIdent(const std::string& ident) { 
    auto it = keywords.find(ident);
    if (it != keywords.end()) {
        return it->second;
    }
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