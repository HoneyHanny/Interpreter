#pragma once

class Environment;

#include "AST.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <functional>
#include <unordered_map>

using ObjectType = std::string;

enum class ObjectType_ {
    INTEGER_OBJ,
    BOOLEAN_OBJ,
    NULL_OBJ,
    RETURN_VALUE_OBJ,
    ERROR_OBJ,
    FUNCTION_OBJ,
    STRING_OBJ,
    BUILTIN,
    // Add other types as needed
};

class Object {
public:
    virtual ~Object() = default; 

    virtual ObjectType Type() const = 0; 
    virtual std::string Inspect() const = 0; 
    //virtual std::shared_ptr<Object> clone() const = 0;
};

inline std::string ObjectTypeToString(ObjectType_ type) {
    switch (type) {
        case ObjectType_::INTEGER_OBJ: return "INTEGER";
        case ObjectType_::BOOLEAN_OBJ: return "BOOLEAN";
        case ObjectType_::NULL_OBJ: return "NULL";
        case ObjectType_::RETURN_VALUE_OBJ: return "RETURN";
        case ObjectType_::ERROR_OBJ: return "ERROR";
        case ObjectType_::FUNCTION_OBJ: return "FUNCTION";
        case ObjectType_::STRING_OBJ: return "STRING";
            // Add cases for other types
        default: return "UNKNOWN";
    }
}

class IntegerObject : public Object {
public:
    int Value;

    IntegerObject(int value) : Value(value) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::INTEGER_OBJ);
    }

    std::string Inspect() const override {
        return std::to_string(Value);
    }

    //std::shared_ptr<Object> clone() const override {
    //    return std::make_unique<IntegerObject>(Value);
    //}
};

class BooleanObject : public Object {
public:
    bool Value;

    BooleanObject(bool value) : Value(value) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::BOOLEAN_OBJ);
    }

    std::string Inspect() const override {
        return std::to_string(Value);
    }

    //std::shared_ptr<Object> clone() const override {
    //    return std::make_unique<BooleanObject>(Value);
    //}
};

class NullObject : public Object {

public:
    NullObject() {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::NULL_OBJ);
    }

    std::string Inspect() const override {
        return "NULL";
    }

    //std::shared_ptr<Object> clone() const override {
    //    return std::make_unique<NullObject>();
    //}
};

class ReturnValue : public Object {
public:
    std::shared_ptr<Object> Value;

    explicit ReturnValue(std::shared_ptr<Object> value) : Value(std::move(value)) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::RETURN_VALUE_OBJ);
    }

    std::string Inspect() const override {
        return Value->Inspect();
    }

    std::shared_ptr<Object> TakeValue() {
        return std::move(Value);
    }

    //std::shared_ptr<Object> clone() const override {
    //    return std::make_unique<ReturnValue>(Value->clone());
    //}
};

class ErrorObject : public Object {
public:
    std::string Message;

    explicit ErrorObject(std::string message) : Message(std::move(message)) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::ERROR_OBJ);
    }

    std::string Inspect() const override {
        return "CODE ERROR - " + Message;
    }

    //std::shared_ptr<Object> clone() const override {
    //    return std::make_unique<ErrorObject>(Message);
    //}
};

class Function : public Object {
public:
    Token ReturnType;
    std::unique_ptr<Expression> CallName;
    std::vector<std::unique_ptr<TypedDeclStatement>> Parameters;
    std::unique_ptr<BlockStatement> Body;
    std::shared_ptr<Environment> Env; 

    // Constructor with rvalue references for move semantics
    Function(const Token& returnType, std::unique_ptr<Expression> callName, std::vector<std::unique_ptr<TypedDeclStatement>> params, std::unique_ptr<BlockStatement> body, std::shared_ptr<Environment> env)
        : ReturnType(returnType), CallName(std::move(callName)), Parameters(std::move(params)), Body(std::move(body)), Env(std::move(env)) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::FUNCTION_OBJ);
    }

    std::string Inspect() const override {
        std::ostringstream out;

        out << "FUNCTION" << " " << CallName->TokenLiteral() << " (";
        for (size_t i = 0; i < Parameters.size(); ++i) {
            if (i > 0) out << ", ";
            out << Parameters[i].get()->String();
        }
        out << ") " << ReturnType.Type << ":\n";
        out << "BEGIN FUNCTION\n";
        out << Body->String();
        out << "\nEND FUNCTION\n";

        return out.str();
    }

    //std::shared_ptr<Object> clone() const override {
    //    std::vector<std::unique_ptr<TypedDeclStatement>> clonedParams;
    //    for (const auto& param : Parameters) {
    //        clonedParams.push_back(std::unique_ptr<TypedDeclStatement>(static_cast<TypedDeclStatement*>(param->clone().release())));
    //    }

    //    auto clonedBody = std::unique_ptr<BlockStatement>(static_cast<BlockStatement*>(Body->clone().release()));
    //    auto clonedCallName = CallName ? std::unique_ptr<Expression>(static_cast<Expression*>(CallName->clone().release())) : nullptr;

    //    return std::make_unique<Function>(ReturnType, std::move(clonedCallName), std::move(clonedParams), std::move(clonedBody), Env);
    //}
};

class String : public Object {
public:
    std::string Value;

    String(std::string value) : Value(value) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::STRING_OBJ);
    }

    std::string Inspect() const override {
        return Value;
    }
};

using BuiltinFunction = std::function<std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>>&, std::shared_ptr<Environment> env)>;

class Builtin : public Object {
public:
    BuiltinFunction Fn;

    // Constructor accepting a BuiltinFunction
    Builtin(BuiltinFunction fn) : Fn(std::move(fn)) {}

    ObjectType Type() const override {
        return ObjectTypeToString(ObjectType_::BUILTIN);
    }

    std::string Inspect() const override {
        return "builtin function";
    }
};