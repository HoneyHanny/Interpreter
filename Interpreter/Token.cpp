#include "Token.h"

std::unordered_map<std::string, TokenType> keywords = {
    {"FUNCTION", FUNCTION},
    {"BEGIN", BEGIN},
    {"END", END},
    {"CODE", CODE},
    {"TRUE", TRUE},
    {"FALSE", FALSE},
    {"IF", IF},
    {"ELSE", ELSE},
    {"DISPLAY", DISPLAY},
    {"RETURN", RETURN},
    {"INT", INT},
    {"CHAR", CHAR},
    {"BOOL", BOOL},
    {"FLOAT", FLOAT},
    {"STRING", STRING},
    {"VOID", VOID},
};
