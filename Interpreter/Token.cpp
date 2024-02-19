#include "Token.h"

std::unordered_map<std::string, TokenType> keywords = {
    {"FN", FUNCTION},
    {"BEGIN", BEGIN},
    {"END", END},
    {"CODE", CODE},
    {"TRUE", TRUE},
    {"FALSE", FALSE},
    {"IF", IF},
    {"ELSE", ELSE},
    {"DISPLAY", DISPLAY},
    {"INT", INT},
    {"CHAR", CHAR},
    {"BOOL", BOOL},
    {"FLOAT", FLOAT},
};
