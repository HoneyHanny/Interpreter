#pragma once

#include <memory> 
#include <functional>
#include <iostream>
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
        std::cout << "Initialize Parser" << std::endl;

        nextToken();
        nextToken();

        setupPrefixParseFns();
        setupInfixParseFns();
    }

    bool isFirstIfExpression = true; // Flag to handle multiple IF statements
    bool isFirstElseExpression = true; // Flag to handle multiple ELSE statements
    bool isFirstFunctionLiteral = true; // Flag to handle multiple FUNCTION statements

    std::unique_ptr<Program> ParseProgram();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<TypedDeclStatement> parseTypedDeclStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();
    std::unique_ptr<BlockStatement> parseBlockStatement();
    std::vector<std::unique_ptr<TypedDeclStatement>> parseFunctionParameters();

    std::unique_ptr<Expression> parseExpression(Precedence precedence);
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseNumericalLiteral();
    std::unique_ptr<Expression> parsePrefixExpression();
    std::unique_ptr<Expression> parseInfixExpression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parseBoolean();
    std::unique_ptr<Expression> parseGroupedExpression();
    std::unique_ptr<Expression> parseIfExpression();
    std::unique_ptr<Expression> parseFunctionLiteral();

    // Helper functions

    bool curTokenIs(const TokenType& t) const;
    bool peekTokenIs(const TokenType& t) const;
    bool expectPeek(const TokenType& t);
    Precedence peekPrecedence() const;
    Precedence curPrecedence() const;

    // Error handling

    std::vector<std::string> Errors() const;
    void peekError(const TokenType& expected);
    void noPrefixParseFnError(TokenType t);

    // Token <--> Parser functions - mapping

    std::unordered_map<TokenType, prefixParseFn> prefixParseFns;
    std::unordered_map<TokenType, infixParseFn> infixParseFns;

    void registerPrefix(TokenType type, prefixParseFn fn);
    void registerInfix(TokenType type, infixParseFn fn);

    std::unordered_map<TokenType, Precedence> precedences = {
        {EQ, Precedence::EQUALS},
        {NEQ, Precedence::EQUALS},
        {LT, Precedence::LESSGREATER},
        {GT, Precedence::LESSGREATER},
        {PLUS, Precedence::SUM},
        {MINUS, Precedence::SUM},
        {SLASH, Precedence::PRODUCT},
        {ASTERISK, Precedence::PRODUCT},
    };
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

        registerPrefix(BANG, [this]() -> std::unique_ptr<Expression> {
            return this->parsePrefixExpression();
        });

        registerPrefix(MINUS, [this]() -> std::unique_ptr<Expression> {
            return this->parsePrefixExpression();
        });

        registerPrefix(TRUE, [this]() -> std::unique_ptr<Expression> {
            return this->parseBoolean();
        });

        registerPrefix(FALSE, [this]() -> std::unique_ptr<Expression> {
            return this->parseBoolean();
        });

        registerPrefix(LPAREN, [this]() -> std::unique_ptr<Expression> {
            return this->parseGroupedExpression();
        });

        registerPrefix(IF, [this]() -> std::unique_ptr<Expression> {
            return this->parseIfExpression();
        }); 

        registerPrefix(FUNCTION, [this]() -> std::unique_ptr<Expression> {
            return this->parseFunctionLiteral();
        });
    }

    void setupInfixParseFns() {
        registerInfix(PLUS, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(MINUS, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(SLASH, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(ASTERISK, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(EQ, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(NEQ, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(LT, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
        registerInfix(GT, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
            });
    }

};
