#pragma once

#include <memory> 
#include "Lexer.h" 
#include "Token.h" 
#include "AST.h"  

class Parser {
public:
    Parser(std::unique_ptr<Lexer> lexer);

    std::unique_ptr<Program> ParseProgram();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<TypedDeclStatement> parseTypedDeclStatement();

    bool curTokenIs(const TokenType& t) const;
    bool peekTokenIs(const TokenType& t) const;
    bool expectPeek(const TokenType& t);

private:
    std::unique_ptr<Lexer> lexer; 
    Token curToken;
    Token peekToken;

    void nextToken();
};
