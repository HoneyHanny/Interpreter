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
    Parser(std::unique_ptr<Lexer> lexer) : lexer(std::move(lexer)) {
        // Call nextToken twice to initialize currToken and peekToken
        nextToken();
        nextToken();

        setupPrefixParseFns();
    }

    std::unique_ptr<Program> ParseProgram();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<TypedDeclStatement> parseTypedDeclStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();

    std::unique_ptr<Expression> parseExpression(Precedence precedence);
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseNumericalLiteral();

    bool curTokenIs(const TokenType& t) const;
    bool peekTokenIs(const TokenType& t) const;
    bool expectPeek(const TokenType& t);

    std::vector<std::string> Errors() const;
    void peekError(const TokenType& expected);

    std::unordered_map<TokenType, prefixParseFn> prefixParseFns;
    std::unordered_map<TokenType, infixParseFn> infixParseFns;

    void registerPrefix(TokenType type, prefixParseFn fn);
    void registerInfix(TokenType type, infixParseFn fn);

private:
    std::unique_ptr<Lexer> lexer; 
    std::vector<std::string> errors;
    Token curToken;
    Token peekToken;


    void nextToken();

    // Setup the prefix parse functions
    void setupPrefixParseFns() {
        // Registering the parser functions for the tokens
        registerPrefix(IDENT, [this]() -> std::unique_ptr<Expression> {
            return this->parseIdentifier();
        });

        registerPrefix(NUM, [this]() -> std::unique_ptr<Expression> {
            return this->parseNumericalLiteral();
        });
    }
};
