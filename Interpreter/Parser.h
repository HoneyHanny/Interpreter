#pragma once

#include <memory> 
#include <functional>
#include <iostream>
#include <initializer_list>
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
    CALL         // myFunction: x, ...
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
        setupInfixParseFns();
    }
    
    Token currentParsedType = {"", ""};
    std::unique_ptr<Expression> currentParsedCallName = nullptr;

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
    std::unique_ptr<Expression> parseStringLiteral();
    std::unique_ptr<Expression> parsePrefixExpression();
    std::unique_ptr<Expression> parseInfixExpression(std::unique_ptr<Expression> left);
    std::unique_ptr<Expression> parseBoolean();
    std::unique_ptr<Expression> parseGroupedExpression();
    std::unique_ptr<Expression> parseIfExpression();
    std::unique_ptr<Expression> parseFunctionLiteral(Token fnToken, std::unique_ptr<Expression> CallName);
    std::unique_ptr<Expression> parseCallExpression(std::unique_ptr<Expression> function);
    std::vector<std::unique_ptr<Expression>> parseCallArguments();
    std::unique_ptr<Expression> parseAssignExpression(std::unique_ptr<Expression> name);

    // Helper functions

    bool curTokenIs(const TokenType& t) const;
    bool peekTokenIs(const TokenType& t) const;

    template<typename... TokenTypes>
    bool peekTokenIs(TokenTypes... types);

    bool expectPeek(const TokenType& t);

    template<typename... TokenTypes>
    bool expectPeek(TokenTypes... types);

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
        {AMPERSAND, Precedence::SUM},
        {PLUS, Precedence::SUM},
        {MINUS, Precedence::SUM},
        {SLASH, Precedence::PRODUCT},
        {ASTERISK, Precedence::PRODUCT},
        {COLON, Precedence::CALL},
        {ASSIGN, Precedence::CALL},
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
            std::cout << "calling function stmt parser" << std::endl;
            currentParsedType = {FUNCTION, "FUNCTION"};
            auto stmt = std::make_unique<ExpressionStatement>(curToken);
            Token fnToken = curToken;

            nextToken();

            currentParsedCallName = parseIdentifier();

            //if (peekTokenIs(LPAREN)) {
            //    return parseFunctionLiteral(fnToken, std::move(callName));
            //}
            if (peekTokenIs(ASSIGN)) {
                nextToken();
                nextToken();

                //stmt->name = std::move(callName);
                return parseExpression(Precedence::LOWEST);
            }
            std::cout << "returned function" << std::endl;
            std::cout << "Call name: " << currentParsedCallName->String() << std::endl;

            auto fnexp = parseFunctionLiteral(fnToken, std::move(currentParsedCallName));
            return fnexp;
        });

        registerPrefix(STRING, [this]() -> std::unique_ptr<Expression> {
            return this->parseStringLiteral();
        });
    }

    void setupInfixParseFns() {
        registerInfix(PLUS, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
        });

        registerInfix(MINUS, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseInfixExpression(std::move(left));
        });

        registerInfix(AMPERSAND, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
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

        registerInfix(COLON, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseCallExpression(std::move(left));
        });

        registerInfix(ASSIGN, [this](std::unique_ptr<Expression> left) -> std::unique_ptr<Expression> {
            return this->parseAssignExpression(std::move(left));
        });
    }

};
