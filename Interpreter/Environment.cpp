#include "Environment.h"

std::shared_ptr<Object> Environment::Get(const std::string& name) const {
    auto iter = store.find(name);
    if (iter != store.end()) {
        return iter->second;
    }
    else if (outer != nullptr) {    
        return outer->Get(name);
    }
    return nullptr;
}

void Environment::Set(const std::string& name, std::shared_ptr<Object> val) {
    store[name] = val; 
}