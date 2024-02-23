#include "Parser.h"
#include <iostream>

void Parser::nextToken() {
    curToken = peekToken;
    peekToken = lexer->NextToken();
}   

std::unique_ptr<Program> Parser::ParseProgram() {
    auto program = std::make_unique<Program>();

    while (curToken.Type != EOF_TOKEN) {
        auto stmt = parseStatement();
        if (stmt != nullptr) {
            program->Statements.push_back(std::move(stmt));
        }
        nextToken();
    }

    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    // Typed Declaration Statements
    if (curToken.Type == INT 
        || curToken.Type == CHAR 
        || curToken.Type == BOOL
        || curToken.Type == FLOAT) {
        return parseTypedDeclStatement();
    }
    // Return Statements
    else if (curToken.Type == RETURN) {
        return parseReturnStatement();
    }
    // Expression Statements
    else if (curToken.Type != NEWLINE){
        return parseExpressionStatement();
    }
    return nullptr;
}

// Subtree structure: <TYPE> <IDENT> <ASSIGN> <EXPRESSION>
std::unique_ptr<TypedDeclStatement> Parser::parseTypedDeclStatement() {
    auto stmt = std::make_unique<TypedDeclStatement>(curToken);

    if (!expectPeek(IDENT)) {
        return nullptr;
    }

    stmt->Name = std::make_unique<Identifier>(curToken, curToken.Literal);

    if (!expectPeek(ASSIGN)) {
        return nullptr;
    }

    // TODO: We're skipping the expressions until we encounter a newline or EOF
    while (!curTokenIs(NEWLINE) && !curTokenIs(EOF_TOKEN)) {
        nextToken();
    }

    return stmt;
}

// Subtree structure: <RETURN> <EXPRESSION>
std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    auto stmt = std::make_unique<ReturnStatement>(curToken);

    nextToken();

    // TODO: We're skipping the expressions until we encounter a newline or EOF
    while (!curTokenIs(NEWLINE) && !curTokenIs(EOF_TOKEN)) {
        nextToken();
    }

    return stmt;
}

// Subtree structure: <EXPRESSION>
std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto stmt = std::make_unique<ExpressionStatement>(curToken);

    stmt->Expression = parseExpression(Precedence::LOWEST);

    if (peekTokenIs(SEMICOLON)) {
        nextToken();
    }

    return stmt;
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence) {
    auto prefixIt = prefixParseFns.find(curToken.Type);
    if (prefixIt == prefixParseFns.end()) {
        noPrefixParseFnError(curToken.Type);
        return nullptr;
    }

    auto& prefixFn = prefixIt->second;
    std::unique_ptr<Expression> leftExp = prefixFn();

    // TODO: Handle explicitly forcing a new expression to be on a new line,
    // current implementation doesn't handle this
    while (!peekTokenIs(NEWLINE) && precedence < peekPrecedence()) {
        auto infixIt = infixParseFns.find(peekToken.Type);
        if (infixIt == infixParseFns.end()) {
            return leftExp;
        }

        auto& infixFn = infixIt->second;
        nextToken();

        leftExp = infixFn(std::move(leftExp));
    }

    return leftExp;
}

// Prefix parsers

std::unique_ptr<Expression> Parser::parseIdentifier() {
    return std::make_unique<Identifier>(curToken, curToken.Literal);
}

std::unique_ptr<Expression> Parser::parseNumericalLiteral() {
    auto lit = std::make_unique<NumericalLiteral>(curToken);

    char* end;
    // strtol can convert a string to a long int, considering the base
    long int value = std::strtol(curToken.Literal.c_str(), &end, 10); // Using base 10 for conversion

    if (*end != '\0') { // Check if the entire string was converted
        std::string msg = "could not parse " + curToken.Literal + " as integer";
        errors.push_back(msg);
        return nullptr;
    }

    lit->Value = value;

    return lit;
}

std::unique_ptr<Expression> Parser::parsePrefixExpression() {
    auto expression = std::make_unique<PrefixExpression>(curToken, curToken.Literal);

    nextToken();

    expression->Right = parseExpression(Precedence::PREFIX);

    return expression;
}

std::unique_ptr<Expression> Parser::parseInfixExpression(std::unique_ptr<Expression> left) {
    auto token = curToken; // Copy current token (assuming Token is a lightweight object)
    auto op = curToken.Literal;
    auto precedence = curPrecedence();

    nextToken(); // Move to the next token (the right-hand side expression)

    auto right = parseExpression(precedence);

    return std::make_unique<InfixExpression>(token, std::move(left), op, std::move(right));
}


// Helper functions

bool Parser::curTokenIs(const TokenType& t) const {
    return curToken.Type == t;
}

bool Parser::peekTokenIs(const TokenType& t) const {
    return peekToken.Type == t;
}

bool Parser::expectPeek(const TokenType& t) {
    if (peekTokenIs(t)) {
        nextToken();
        return true;
    }
    else {
        peekError(t);
        return false;
    }
}

Precedence Parser::peekPrecedence() const {
    auto it = precedences.find(peekToken.Type);
    if (it != precedences.end()) {
        return it->second;
    }
    return Precedence::LOWEST;
}

Precedence Parser::curPrecedence() const {
    auto it = precedences.find(curToken.Type);
    if (it != precedences.end()) {
        return it->second;
    }
    return Precedence::LOWEST;
}

// Error handling

std::vector<std::string> Parser::Errors() const {
    return errors;
}

void Parser::peekError(const TokenType& expected) {
    std::string msg = "Expected next token to be " + std::string(expected) +
        ", got " + std::string(peekToken.Type) + " instead.";
    errors.push_back(msg);
}

void Parser::noPrefixParseFnError(TokenType t) {
    std::string msg = "no prefix parse function for " + std::string(t) + " found";
    errors.push_back(msg);
}

// Token <--> Parser functions - mapping

void Parser::registerPrefix(TokenType type, prefixParseFn fn) {
    prefixParseFns[type] = fn;
}

void Parser::registerInfix(TokenType type, infixParseFn fn) {
    infixParseFns[type] = fn;
}
