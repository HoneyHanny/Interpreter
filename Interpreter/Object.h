#pragma once
#include <iostream>
#include <string>

using ObjectType = std::string;

enum class ObjectType_ {
    INTEGER_OBJ,
    BOOLEAN_OBJ,
    NULL_OBJ,
    // Add other types as needed
};

class Object {
public:
    virtual ~Object() = default; 

    virtual ObjectType Type() const = 0; 
    virtual std::string Inspect() const = 0; 
};

inline std::string ObjectTypeToString(ObjectType_ type) {
    switch (type) {
    case ObjectType_::INTEGER_OBJ: return "INTEGER";
    case ObjectType_::BOOLEAN_OBJ: return "BOOLEAN";
    case ObjectType_::NULL_OBJ: return "NULL";
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
};