#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Token.h"

class Node {
public:
    virtual ~Node() = default;
    virtual std::string TokenLiteral() const = 0;
};

class Statement : public Node {
public:
    virtual void statementNode() = 0;
};

class Expression : public Node {
public:
    virtual void expressionNode() = 0;
};

class Program : public Node {
public:
    std::vector<std::shared_ptr<Statement>> Statements;

    std::string TokenLiteral() const override {
        if (!Statements.empty()) {
            return Statements[0]->TokenLiteral();
        }
        return "";
    }
};

class Identifier : public Expression {
public:
    Token token;
    std::string Value;

    Identifier(const Token& token, const std::string& value)
        : token(token), Value(value) {}

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};


class TypedDeclStatement : public Statement {
public:
    Token token;
    std::shared_ptr<Identifier> Name;
    std::shared_ptr<Expression> Value;

    TypedDeclStatement(const Token& token)
        : token(token) {}

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

