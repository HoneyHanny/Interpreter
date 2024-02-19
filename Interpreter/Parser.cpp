#include "Parser.h"

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
        return false;
    }
}