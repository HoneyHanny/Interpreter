#include "Lexer.h"

Lexer::Lexer(const std::string& input)
    : input(input), position(0), readPosition(0), ch(0) {
    readChar(); // Initialize the first character
}

// Public function definitions

// Reads the next character and advances the position
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

// Retrieves the next character in advance without incrementing the position (for ==, <>, etc..)
char Lexer::peekChar() {
    if (readPosition >= static_cast<int>(input.size())) {
        return '\0';
    }
    else {
        return input[readPosition];
    }
}

// Tokenizer
Token Lexer::NextToken() {
    Token tok;

    skipWhitespace();

    switch (ch) {
    case '=':
        if (peekChar() == '=') {
            readChar();
            tok.Type = EQ; 
            tok.Literal = "==";
        }
        else {
            tok = { ASSIGN, std::string(1, ch) };
        }
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
    case '!':
        tok = { BANG, std::string(1, ch) };
        break;
    case '<':
        if (peekChar() == '>') {
            readChar();
            tok.Type = NEQ;
            tok.Literal = "<>";
        }
        else if (peekChar() == '=') {
            readChar();
            tok.Type = LEQT;
            tok.Literal = "<=";
        }
        else {
            tok = { LT, std::string(1, ch) };
        }
        break;
    case '>':
        if (peekChar() == '=') {
            readChar();
            tok.Type = GEQT;
            tok.Literal = ">=";
        }
        else {
            tok = { GT, std::string(1, ch) };
        }
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
    case '\n':
        tok = { NEWLINE, std::string(1, ch) };
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
            tok.Type = NUM; 
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

// Check if keyword or identifier
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

// Skips whitespace in code
void Lexer::skipWhitespace() {
    while (ch == ' ' 
        //|| ch == '\t' 
        //|| ch == '\n' 
        //|| ch == '\r' // Commented out to treat newlines as a token by the Lexer
) {
        readChar();
    }
}

// Distinguishes between keywords and identifiers
TokenType Lexer::lookupIdent(const std::string& ident) { 
    auto it = keywords.find(ident);
    if (it != keywords.end()) {
        return it->second;
    }
    return IDENT;
}

// Check if number
std::string Lexer::readNumber() {
    int startPosition = position;
    while (isDigit(ch)) {
        readChar();
    }
    // Since input is a std::string_view, we directly return a std::string part of it
    return std::string(input.substr(startPosition, position - startPosition));
}