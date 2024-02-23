#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include "Token.h"

class Node {
public:
    virtual ~Node() = default;
    virtual std::string TokenLiteral() const = 0;
    virtual std::string String() const = 0;
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
    std::vector<std::unique_ptr<Statement>> Statements;

    std::string TokenLiteral() const override {
        if (!Statements.empty()) {
            return Statements[0]->TokenLiteral();
        }
        return "";
    }

    std::string String() const override {
        std::ostringstream out;

        for (const auto& s : Statements) {
            out << s->String();
        }

        return out.str();
    }
};

class Identifier : public Expression {
public:
    Token token;
    std::string Value;

    Identifier(const Token& token, const std::string& value)
        : token(token), Value(value) {}

    std::string String() const override { return Value; }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

// Subtree structure: <TYPE> <IDENT> <ASSIGN> <EXPRESSION>
class TypedDeclStatement : public Statement {
public:
    Token token; // <TYPE>
    std::unique_ptr<Identifier> Name; // <IDENT>
    std::unique_ptr<Expression> Value; // <EXPRESSION>

    TypedDeclStatement(const Token& token)
        : token(token) {}
    
    // Testing constructor. Do not use in actual Parser.
    TypedDeclStatement(const Token& token, std::unique_ptr<Identifier> name, std::unique_ptr<Identifier> value)
        : token(token), Name(std::move(name)), Value(std::move(value)) {}

    std::string String() const override {
        std::ostringstream out;

        out << TokenLiteral() << " ";
        out << Name->String(); 
        out << " = ";

        if (Value != nullptr) {
            out << Value->String();
        }

        return out.str();
    }

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

// Subtree structure: <RETURN> <EXPRESSION>
class ReturnStatement : public Statement {
public:
    Token token; // <RETURN>
    std::unique_ptr<Expression> ReturnValue; // <EXPRESSION>

    ReturnStatement(const Token& token)
        : token(token) {}

    std::string String() const override {
        std::ostringstream out;

        out << TokenLiteral() << " ";

        if (ReturnValue != nullptr) {
            out << ReturnValue->String();
        }

        return out.str();
    }

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

// Subtree structure: <EXPRESSION> ... <EXPRESSION> 
class ExpressionStatement : public Statement {
public:
    Token token; // <First token of the expression>
    std::unique_ptr<Expression> Expression; // <EXPRESSION>

    ExpressionStatement(const Token& token)
        : token(token) {}

    std::string String() const override {
        std::ostringstream out;

        if (Expression != nullptr) {
            out << Expression->String();
        }
        else {
            out << "";
        }

        return out.str();
    }

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

// Subtree structure: <EXPRESSION>
class NumericalLiteral : public Expression {
public:
    Token token; 
    int Value = 0; //  Default value = 0;

    NumericalLiteral(const Token& token)
        : token(token) {}

    std::string String() const override { return token.Literal; }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

// Subtree structure: <prefix operator><EXPRESSION>;
class PrefixExpression : public Expression {
public:
    Token token;
    std::string Operator;
    std::unique_ptr<Expression> Right;

    PrefixExpression(const Token& token, const std::string Operator)
        : token(token), Operator(Operator) {}

    std::string String() const override {
        std::ostringstream out;

        out << "(";
        out << Operator;
        out << Right->String();
        out << ")";

        return out.str();
    }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }
};

class InfixExpression : public Expression {
public:
    Token token;
    std::unique_ptr<Expression> Left;
    std::string Operator;
    std::unique_ptr<Expression> Right;

    InfixExpression(const Token& token, std::unique_ptr<Expression> left, const std::string& op, std::unique_ptr<Expression> right)
        : token(token), Left(std::move(left)), Operator(op), Right(std::move(right)) {}

    std::string TokenLiteral() const override {
        return token.Literal;
    }

    std::string String() const override {
        std::ostringstream out;
        out << "(";
        out << Left->String();
        out << " " << Operator << " ";
        out << Right->String();
        out << ")";
        return out.str();
    }

    void expressionNode() override {}
};

class Boolean : public Expression {
public:
    Token token; 
    bool Value = true; // Default value = true  

    Boolean(const Token& token)
        : token(token) {}

    Boolean(const Token& token, bool value)
        : token(token), Value(value) {}

    void expressionNode() override {}

    std::string TokenLiteral() const override {
        return token.Literal;
    }

    std::string String() const override {
        return token.Literal; 
    }
};
