#pragma once

#include "Object.h"
#include <string>
#include <unordered_map>
#include <memory>

class Environment {
private:
    using StoreValue = std::tuple<Token, std::string, std::shared_ptr<Object>>;
    std::unordered_map<std::string, StoreValue> store;
    std::unordered_map<std::shared_ptr<Object>, std::string> reverseStore;
    std::shared_ptr<Environment> outer;

public:
    Environment() : outer(nullptr) {}
    explicit Environment(std::shared_ptr<Environment> outerEnv) : outer(std::move(outerEnv)) {}

    std::shared_ptr<Object> GetObject(const std::string& name) const;
    Token GetType(const std::string& name) const;
    bool Exists(const std::string& name) const;
    void Set(const std::string& name, Token type, std::shared_ptr<Object> val);
    std::string GetNameByObject(const std::shared_ptr<Object>& obj) const;
};
