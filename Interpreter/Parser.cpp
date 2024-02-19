#include "Parser.h"
#include <iostream>

Parser::Parser(std::unique_ptr<Lexer> lexer) : lexer(std::move(lexer)) {
    nextToken();
    nextToken();
}

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
    if (curToken.Type == INT 
        || curToken.Type == CHAR 
        || curToken.Type == BOOL
        || curToken.Type == FLOAT) {
        return parseTypedDeclStatement();
    }
    else {
        return nullptr;
    }
}

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

std::vector<std::string> Parser::Errors() const {
    return errors;
}

void Parser::peekError(const TokenType& expected) {
    std::string msg = "Expected next token to be " + std::string(expected) +
        ", got " + std::string(peekToken.Type) + " instead.";
    errors.push_back(msg);
}
