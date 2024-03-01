#pragma once

#include "Object.h"
#include <string>
#include <unordered_map>
#include <memory>

class Environment {
private:
    std::unordered_map<std::string, std::shared_ptr<Object>> store;
    std::shared_ptr<Environment> outer;

public:
    Environment() : outer(nullptr) {}

    explicit Environment(std::shared_ptr<Environment> outerEnv) : outer(std::move(outerEnv)) {}

    std::shared_ptr<Object> Get(const std::string& name) const;
    void Set(const std::string& name, std::shared_ptr<Object> val);
};