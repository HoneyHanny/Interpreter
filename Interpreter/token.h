#pragma once
#include <string>
#include <string_view>

// Token Type Definition
using TokenType = std::string_view;

struct Token {
    TokenType Type;
    std::string Literal;
};

// Token Value Definitions
// Special Identifiers
constexpr std::string_view ILLEGAL = "ILLEGAL";
constexpr std::string_view EOF_TOKEN = "EOF"; // EOF is a macro in C++, so we use EOF_TOKEN

// Identifiers + literals
constexpr std::string_view IDENT = "IDENT"; // varName, foobar, x, y, ...
constexpr std::string_view INT = "INT"; // 1343456

// Operators
constexpr std::string_view ASSIGN = "=";
constexpr std::string_view PLUS = "+";

// Delimiters
constexpr std::string_view COMMA = ",";
constexpr std::string_view SEMICOLON = ";";
constexpr std::string_view LPAREN = "(";
constexpr std::string_view RPAREN = ")";
constexpr std::string_view LBRACE = "{";
constexpr std::string_view RBRACE = "}";

// Keywords
constexpr std::string_view FUNCTION = "FUNCTION";
constexpr std::string_view LET = "LET";
