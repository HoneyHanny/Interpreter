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
    virtual std::unique_ptr<Statement> clone() const = 0;
};

class Expression : public Node {
public:
    virtual void expressionNode() = 0;
    virtual std::unique_ptr<Expression> clone() const = 0;
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

    Identifier(const Token& token)
        : token(token) {}

    Identifier(const Token& token, const std::string& value)
        : token(token), Value(value) {}

    std::string String() const override { return Value; }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Identifier>(token, Value);
    }
};

// Subtree structure: <TYPE> <IDENT> <ASSIGN> <EXPRESSION>
class TypedDeclStatement : public Statement {
public:
    Token token = {"NONE", ""}; // <TYPE>
    std::unique_ptr<Identifier> Name; // <IDENT>
    std::unique_ptr<Expression> Value; // <EXPRESSION>

    TypedDeclStatement(const Token& token)
        : token(token) {}

    TypedDeclStatement(const Token& token, std::unique_ptr<Identifier> name)
        : token(token), Name(std::move(name)) {}

    //TypedDeclStatement(std::unique_ptr<Identifier> name, std::unique_ptr<Identifier> value)
    //    : Name(std::move(name)), Value(std::move(value)) {}

    TypedDeclStatement(const Token& token, std::unique_ptr<Identifier> name, std::unique_ptr<Identifier> value)
        : token(token), Name(std::move(name)), Value(std::move(value)) {}

    TypedDeclStatement(const Token& token, std::unique_ptr<Identifier> name, std::unique_ptr<Expression> value)
        : token(token), Name(std::move(name)), Value(std::move(value)) {}

    std::string String() const override {
        std::ostringstream out;

        if (token.Type != "NONE") {
            out << TokenLiteral();
        }

        out << " ";
        out << Name->String(); 
        
        if (Value) {
            out << " = ";
            out << Value->String();
        }

        return out.str();
    }

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Statement> clone() const override {
        auto nameClone = std::make_unique<Identifier>(*Name);
        auto valueClone = Value ? std::unique_ptr<Expression>(static_cast<Expression*>(Value->clone().release())) : nullptr;
        return std::make_unique<TypedDeclStatement>(token, std::move(nameClone), std::move(valueClone));
    }
};

// Subtree structure: <RETURN> <EXPRESSION>
class ReturnStatement : public Statement {
public:
    Token token; // <RETURN>
    std::unique_ptr<Expression> ReturnValue; // <EXPRESSION>

    ReturnStatement(const Token& token)
        : token(token) {}

    ReturnStatement(const Token& token, std::unique_ptr<Expression> returnValue)
        : token(token), ReturnValue(std::move(returnValue)) {}

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

    std::unique_ptr<Statement> clone() const override {
        auto returnValueClone = ReturnValue ? std::unique_ptr<Expression>(static_cast<Expression*>(ReturnValue->clone().release())) : nullptr;
        return std::make_unique<ReturnStatement>(token, std::move(returnValueClone));
    }
};

// Subtree structure: <EXPRESSION> ... <EXPRESSION> 
class ExpressionStatement : public Statement {
public:
    Token token = {"NONE", ""}; // <First token of the expression>
    std::unique_ptr<Expression> name;
    std::unique_ptr<Expression> Expression_; // <EXPRESSION>

    ExpressionStatement(const Token& token)
        : token(token) {}

    std::string String() const override {
        std::ostringstream out;

        if (name) {
            out << token.Literal << " " << name->String() << " ";
        }

        if (Expression_ != nullptr) {
            out << Expression_->String();
        }
        else {
            out << "";
        }

        return out.str();
    }

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Statement> clone() const override {
        auto clonedExpression = std::unique_ptr<Expression>(static_cast<Expression*>(Expression_->clone().release()));
        auto clonedStmt = std::make_unique<ExpressionStatement>(token);
        clonedStmt->Expression_ = std::move(clonedExpression);
        return clonedStmt;
    }
};

// Subtree structure: <EXPRESSION>
class IntegerLiteral : public Expression {
public:
    Token token; 
    int Value = 0; //  Default value = 0;

    IntegerLiteral(const Token& token)
        : token(token) {}

    IntegerLiteral(const Token& token, const int value)
        : token(token), Value(value) {}

    std::string String() const override { return token.Literal; }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<IntegerLiteral>(token, Value);
    }
};

// Subtree structure: <EXPRESSION>
class StringLiteral : public Expression {
public:
    Token token;
    std::string Value;

    StringLiteral(const Token& token, const std::string& value)
        : token(token), Value(value) {}

    std::string String() const override { return "\"" + Value + "\""; }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<StringLiteral>(token, Value);
    }
};

class FloatLiteral : public Expression {
public:
    Token token;
    double Value = 0.0;

    FloatLiteral(const Token& token, double value)
        : token(token), Value(value) {}

    std::string String() const override { return token.Literal; }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<FloatLiteral>(token, Value);
    }
};

// Subtree structure: <EXPRESSION>
class CharLiteral : public Expression {
public:
    Token token;
    char Value; // Changed from std::string to char to represent a CHAR

    CharLiteral(const Token& token, char value)
        : token(token), Value(value) {}

    std::string String() const override { return "'" + std::string(1, Value) + "'"; } // Adjusted for char

    void expressionNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<CharLiteral>(token, Value);
    }
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

    std::unique_ptr<Expression> clone() const override {
        auto clonedRight = std::unique_ptr<Expression>(static_cast<Expression*>(Right->clone().release()));
        auto clonedExpr = std::make_unique<PrefixExpression>(token, Operator);
        clonedExpr->Right = std::move(clonedRight);
        return clonedExpr;
    }
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

    std::unique_ptr<Expression> clone() const override {
        auto clonedLeft = std::unique_ptr<Expression>(static_cast<Expression*>(Left->clone().release()));
        auto clonedRight = std::unique_ptr<Expression>(static_cast<Expression*>(Right->clone().release()));
        auto clonedExpr = std::make_unique<InfixExpression>(token, std::move(clonedLeft), Operator, std::move(clonedRight));
        return clonedExpr;
    }
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

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Boolean>(token, Value);
    }
};

class BlockStatement : public Statement {
public:
    Token token; 
    std::vector<std::unique_ptr<Statement>> Statements;

    BlockStatement(const Token& tok) : token(tok) {}

    void statementNode() override {}

    std::string TokenLiteral() const override {
        return token.Literal;
    }

    std::string String() const override {
        std::ostringstream out;
        for (const auto& stmt : Statements) {
            out << stmt->String();
        }
        return out.str();
    }

    std::unique_ptr<Statement> clone() const override {
        auto clonedBlock = std::make_unique<BlockStatement>(token);
        for (const auto& stmt : Statements) {
            clonedBlock->Statements.push_back(std::unique_ptr<Statement>(stmt->clone().release()));
        }
        return clonedBlock;
    }
};

class IfExpression : public Expression {
public:
    Token token; 
    std::unique_ptr<Expression> Condition;
    std::unique_ptr<BlockStatement> Consequence;
    std::unique_ptr<BlockStatement> Alternative;

    IfExpression(const Token& tok) : token(tok) {}

    void expressionNode() override {}

    std::string TokenLiteral() const override {
        return token.Literal;
    }

    std::string String() const override {
        std::ostringstream out;
        out << "IF ";
        out << Condition->String();
        out << "\nBEGIN IF\n";
        out << Consequence->String();
        out << "\nEND IF";

        if (Alternative) {
            out << "ELSE " << Alternative->String();
        }

        return out.str();
    }

    std::unique_ptr<Expression> clone() const override {
        auto clonedCondition = std::unique_ptr<Expression>(static_cast<Expression*>(Condition->clone().release()));
        auto clonedConsequence = std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(Consequence->clone().release()));
        auto clonedAlternative = Alternative ? std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(Alternative->clone().release())) : nullptr;

        auto clonedIfExpr = std::make_unique<IfExpression>(token);
        clonedIfExpr->Condition = std::move(clonedCondition);
        clonedIfExpr->Consequence = std::move(clonedConsequence);
        clonedIfExpr->Alternative = std::move(clonedAlternative);

        return clonedIfExpr;
    }
};

class FunctionLiteral : public Expression {
public:
    Token token;
    Token type;
    std::unique_ptr<Expression> CallName;
    std::vector<std::unique_ptr<TypedDeclStatement>> Parameters;
    std::unique_ptr<BlockStatement> Body;

    FunctionLiteral(const Token& tok) : token(tok) {}

    FunctionLiteral(const Token& tok, const Token& type) : token(tok), type(type) {}

    FunctionLiteral(const Token& tok, const Token& typ, std::unique_ptr<Expression> callName, std::vector<std::unique_ptr<TypedDeclStatement>>&& params, std::unique_ptr<BlockStatement>&& body)
        : token(tok), type(typ), CallName(std::move(callName)), Parameters(std::move(params)), Body(std::move(body)) {}

    void expressionNode() override {}

    std::string TokenLiteral() const override {
        return token.Literal;
    }

    std::string String() const override {
        std::ostringstream out;
        std::vector<std::string> params;

        for (const auto& p : Parameters) {
            params.push_back(p->String());
        }

        out << TokenLiteral() << " " << CallName->TokenLiteral() << " (";
        for (size_t i = 0; i < params.size(); ++i) {
            out << params[i];
            if (i < params.size() - 1) {
                out << ", ";
            }
        }
        out << ") " << type.Type << ":\n";
        out << "BEGIN FUNCTION\n";
        out << Body->String();
        out << "\nEND FUNCTION\n";

        return out.str();
    }

    std::unique_ptr<Expression> clone() const override {
        std::vector<std::unique_ptr<TypedDeclStatement>> clonedParams;
        for (const auto& param : Parameters) {
            clonedParams.push_back(std::unique_ptr<TypedDeclStatement>(static_cast<TypedDeclStatement*>(param->clone().release())));
        }
        auto clonedBody = std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(Body->clone().release()));
        auto clonedCallName = CallName ? std::unique_ptr<Expression>(static_cast<Expression*>(CallName->clone().release())) : nullptr;

        return std::make_unique<FunctionLiteral>(token, type, std::move(clonedCallName), std::move(clonedParams), std::move(clonedBody));
    }
};

class CallExpression : public Expression {
public:
    Token token;
    std::unique_ptr<Expression> Function; // Can be Identifier or FunctionLiteral
    std::vector<std::unique_ptr<Expression>> Arguments;

    CallExpression(const Token& tok) : token(tok) {}

    void expressionNode() override {}

    std::string TokenLiteral() const override {
        return token.Literal;
    }

    std::string String() const override {
        std::ostringstream out;
        std::vector<std::string> args;

        for (const auto& arg : Arguments) {
            args.push_back(arg->String());
        }

        out << Function->String();
        out << ": ";
        for (size_t i = 0; i < args.size(); ++i) {
            out << args[i];
            if (i < args.size() - 1) {
                out << ", ";
            }
        }

        return out.str();
    }

    std::unique_ptr<Expression> clone() const override {
        auto clonedFunction = std::unique_ptr<Expression>(static_cast<Expression*>(Function->clone().release()));
        std::vector<std::unique_ptr<Expression>> clonedArgs;
        for (const auto& arg : Arguments) {
            clonedArgs.push_back(std::unique_ptr<Expression>(static_cast<Expression*>(arg->clone().release())));
        }

        auto clonedCallExpr = std::make_unique<CallExpression>(token);
        clonedCallExpr->Function = std::move(clonedFunction);
        clonedCallExpr->Arguments = std::move(clonedArgs);

        return clonedCallExpr;
    }
};

class AssignExpression : public Expression {
public:
    std::unique_ptr<Expression> name;
    std::unique_ptr<Expression> Expression_; 

    AssignExpression(std::unique_ptr<Expression> name)
        : name(std::move(name)) {}

    std::string String() const override {
        std::ostringstream out;

        out << name->String() << " = ";

        if (Expression_ != nullptr) {
            out << Expression_->String();
        }
        else {
            out << "";
        }

        return out.str();
    }

    void expressionNode() override {}
    std::string TokenLiteral() const override { return ""; }

    std::unique_ptr<Expression> clone() const override {
        auto clonedName = std::unique_ptr<Expression>(static_cast<Expression*>(name->clone().release()));
        auto clonedExpression = std::unique_ptr<Expression>(static_cast<Expression*>(Expression_->clone().release()));
        auto clonedExpr = std::make_unique<AssignExpression>(std::move(clonedName));
        clonedExpr->Expression_ = std::move(clonedExpression);
        return clonedExpr;
    }
};

class MarkerStatement : public Statement {
public:
    Token token;
    Token codeToken;

    MarkerStatement(Token token, Token codeToken) : token(token), codeToken(codeToken) {}

    std::string String() const override {
        std::ostringstream out;

        out << token.Literal << " " << codeToken.Literal;

        return out.str();
    }

    void statementNode() override {}
    std::string TokenLiteral() const override { return token.Literal; }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<MarkerStatement>(token, codeToken);
    }
};