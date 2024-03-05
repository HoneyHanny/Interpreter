#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

// Token Type Definition
using TokenType = std::string_view;

struct Token {
    TokenType Type;
    std::string Literal;
};

// Token Value Definitions
// Special Identifiers
constexpr TokenType ILLEGAL = "ILLEGAL";
constexpr TokenType EOF_TOKEN = "EOF"; // EOF is a macro in C++, so we use EOF_TOKEN

// Operators
constexpr TokenType ASSIGN = "=";
constexpr TokenType PLUS = "+";
constexpr TokenType MINUS = "-";
constexpr TokenType ASTERISK = "*";
constexpr TokenType SLASH = "/";
constexpr TokenType BANG = "!";

constexpr TokenType LT = "<";
constexpr TokenType GT = ">";

constexpr TokenType EQ = "==";
constexpr TokenType NEQ = "<>";
constexpr TokenType LEQT = "<=";
constexpr TokenType GEQT = ">=";

// Delimiters
constexpr TokenType COMMA = ",";
constexpr TokenType COLON = ":";
constexpr TokenType SEMICOLON = ";";
constexpr TokenType LPAREN = "(";
constexpr TokenType RPAREN = ")";
constexpr TokenType LBRACE = "{";
constexpr TokenType RBRACE = "}";
constexpr TokenType LSQBRACE = "[";
constexpr TokenType RSQBRACE = "]";
constexpr TokenType HASH = "#";
constexpr TokenType DOT = ".";
constexpr TokenType S_QUOTE = "'";
constexpr TokenType D_QUOTE = "\"";
constexpr TokenType DOLLAR_SIGN = "$";
constexpr TokenType AMPERSAND = "&";
constexpr TokenType NEWLINE = "\n";

// Identifiers 
constexpr TokenType IDENT = "IDENT"; // varName, foobar, x, y, ...
constexpr TokenType NUM = "NUM"; // 0123456789

// Keywords
// Literals, Conditionals, etc...
constexpr TokenType FUNCTION = "FUNCTION";
constexpr TokenType BEGIN = "BEGIN";
constexpr TokenType END = "END";
constexpr TokenType CODE = "CODE";
constexpr TokenType TRUE = "TRUE";
constexpr TokenType FALSE = "FALSE";
constexpr TokenType IF = "IF";
constexpr TokenType ELSE = "ELSE";
//constexpr TokenType DISPLAY = "DISPLAY";
constexpr TokenType RETURN = "RETURN";

// Types
constexpr TokenType INT = "INT";
constexpr TokenType CHAR = "CHAR";
constexpr TokenType BOOL = "BOOL";
constexpr TokenType FLOAT = "FLOAT";
constexpr TokenType STRING = "STRING";
constexpr TokenType VOID = "VOID";

extern std::unordered_map<std::string, std::string_view> keywords;

