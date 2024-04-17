#pragma once

#include <memory> 
#include <functional>
#include <iostream>
#include <initializer_list>
#include <variant>
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

enum class ParseState {
    BeginCode,
    FunctionDeclarations,
    VariableDeclarations,
    ExecutableCode,
    EndCode
};

static ParseState mainstate = ParseState::BeginCode;

using prefixParseFn = std::function<std::unique_ptr<Expression>()>;
using infixParseFn = std::function<std::unique_ptr<Expression>(std::unique_ptr<Expression>)>;

class DeclStatementWrapper {
public:
    // Can hold either a single TypedDeclStatement or a MultiTypedDeclStatement
    std::variant<std::unique_ptr<TypedDeclStatement>, std::unique_ptr<MultiTypedDeclStatement>> content;

    DeclStatementWrapper(std::unique_ptr<TypedDeclStatement> stmt) : content(std::move(stmt)) {}
    DeclStatementWrapper(std::unique_ptr<MultiTypedDeclStatement> stmt) : content(std::move(stmt)) {}
};

class Parser {
public:
    Parser(std::unique_ptr<Lexer> lexer) : lexer(std::move(lexer)) {
        // Call nextToken twice to initialize currToken and peekToken
        nextToken();
        nextToken();

        setupPrefixParseFns();
        setupInfixParseFns();
    }
    
    bool enforcedStructure = true;
    int currentLine = 1;

    Token currentParsedType = {"", ""};
    std::unique_ptr<Expression> currentParsedCallName = nullptr;

    bool isTypedDeclStatementStart() const;

    std::unique_ptr<Program> ParseProgram();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<MarkerStatement> parseMarkerStatement(Token type, Token codeToken);
    std::vector<DeclStatementWrapper> parseTypedDeclStatements();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();
    std::unique_ptr<BlockStatement> parseBlockStatement();
    std::vector<std::unique_ptr<TypedDeclStatement>> parseFunctionParameters();

    std::unique_ptr<Expression> parseExpression(Precedence precedence);
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseNumericalLiteral();
    std::unique_ptr<Expression> parseStringLiteral();
    std::unique_ptr<Expression> parseCharLiteral();
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
    bool customExpectPeek(const std::string& errorMessage, const TokenType& t);

    template<typename... TokenTypes>
    bool expectPeek(TokenTypes... types);

    template<typename... TokenTypes>
    bool customExpectPeek(const std::string& errorMessage, TokenTypes... types);

    void printCurTokenLiteral();

    Precedence peekPrecedence() const;
    Precedence curPrecedence() const;

    // Error handling

    std::vector<std::string> Errors() const;
    void peekError(const TokenType& expected);
    void peekError(const std::string& message);
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

    size_t commentIndex = 0; 

    void incrementLine() {
        currentLine++;
        // Continue to skip comment lines without modifying the vector
        while (commentIndex < Lexer::commentLinePositions.size() && currentLine == Lexer::commentLinePositions[commentIndex]) {
            currentLine++;
            commentIndex++;  // Move to the next comment line index
        }
    }

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

            auto fnexp = parseFunctionLiteral(fnToken, std::move(currentParsedCallName));
            return fnexp;
        });

        registerPrefix(STRING_LITERAL, [this]() -> std::unique_ptr<Expression> {
            return this->parseStringLiteral();
        });

        registerPrefix(CHAR_LITERAL, [this]() -> std::unique_ptr<Expression> {
            return this->parseCharLiteral();
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

static std::unique_ptr<Expression> extractExpression(const std::unique_ptr<Statement>& stmt) {
    // Utility function to extract an Expression from a Statement if possible.
    const auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt.get());
    if (exprStmt && exprStmt->Expression_) { // Ensure exprStmt and its Expression_ are not nullptr
        auto clonedExpr = exprStmt->Expression_->clone();
        if (clonedExpr) { // Ensure clonedExpr is not nullptr
            return clonedExpr;
        }
    }
    return nullptr; // Return nullptr if any condition fails
}

static bool isBeginCodeStatement(const std::unique_ptr<Statement>& stmt) {
    auto markerStmt = dynamic_cast<MarkerStatement*>(stmt.get());
    return markerStmt && markerStmt->token.Literal + " " + markerStmt->codeToken.Literal == "BEGIN CODE";
}

static bool isEndCodeStatement(const std::unique_ptr<Statement>& stmt) {
    auto markerStmt = dynamic_cast<MarkerStatement*>(stmt.get());
    return markerStmt && markerStmt->token.Literal + " " + markerStmt->codeToken.Literal == "END CODE";
}

static bool isTransitionToVariableDeclarations(const std::unique_ptr<Statement>& stmt) {
    if (auto typedDeclStmt = dynamic_cast<TypedDeclStatement*>(stmt.get())) {
        //std::cout << "Transition to var decl state" << std::endl;
        return true;
    }
    else if (auto multiTypedDeclStmt = dynamic_cast<MultiTypedDeclStatement*>(stmt.get())) {
        //std::cout << "Transition to var decl state" << std::endl;
        return true;
    }
    return false;
}

static bool isTransitionToExecutableCode(const std::unique_ptr<Statement>& stmt) {
    auto expr = extractExpression(stmt);
    if (expr) {
        //std::cout << "Transition to exec state" << std::endl;
        return true;
    }

    return false;
}

static bool isAllowedInEndCode(const std::unique_ptr<Statement>& stmt) {
    return false;
}