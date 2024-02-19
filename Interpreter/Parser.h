#pragma once

#include <memory> 
#include <functional>
#include "Lexer.h" 
#include "Token.h" 
#include "AST.h"  

enum class Precedence : int {
    LOWEST = 1,
    EQUALS,      // == 
    LESSGREATER, // > or <
    SUM,         // + 
    PRODUCT,     // *
    PREFIX,      // -X or !X 
    CALL         // myFunction(X
};


using prefixParseFn = std::function<std::unique_ptr<Expression>()>;
using infixParseFn = std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)>;

class Parser {
public:
    Parser(std::unique_ptr<Lexer> lexer);

    std::unique_ptr<Program> ParseProgram();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<TypedDeclStatement> parseTypedDeclStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();

    std::unique_ptr<Expression> parseExpression(Precedence precedence);
    std::unique_ptr<Expression> parseIdentifier();

    bool curTokenIs(const TokenType& t) const;
    bool peekTokenIs(const TokenType& t) const;
    bool expectPeek(const TokenType& t);

    std::vector<std::string> Errors() const;
    void peekError(const TokenType& expected);

    std::unordered_map<TokenType, prefixParseFn> prefixParseFns;
    std::unordered_map<TokenType, infixParseFn> infixParseFns;

    void registerPrefix(TokenType type, prefixParseFn fn);
    void registerInfix(TokenType type, infixParseFn fn);

    void setupPrefixParseFns() {
        // Registering the parseIdentifier function for IDENT tokens
        registerPrefix(IDENT, [this]() -> std::unique_ptr<Expression> {
            return this->parseIdentifier();
        });

        // Add more registrations
    }
private:
    std::unique_ptr<Lexer> lexer; 
    std::vector<std::string> errors;
    Token curToken;
    Token peekToken;


    void nextToken();
};
