#include "Environment.h"

std::shared_ptr<Object> Environment::GetObject(const std::string& name) const {
    auto iter = store.find(name);
    if (iter != store.end()) {
        return std::get<2>(iter->second); // Return the Object part of the tuple
    }
    else if (outer != nullptr) {
        return outer->GetObject(name);
    }
    return nullptr;
}

Token Environment::GetType(const std::string& name) const {
    auto iter = store.find(name);
    if (iter != store.end()) {
        return std::get<0>(iter->second);
    }
    else if (outer != nullptr) {
        return outer->GetType(name);
    }
    // Return a default Token if not found
    return { "","" };
}

void Environment::Set(const std::string& name, Token type, std::shared_ptr<Object> val) {
    store[name] = std::make_tuple(type, name, val);
    reverseStore[val] = name;
}

std::string Environment::GetNameByObject(const std::shared_ptr<Object>& obj) const {
    auto it = reverseStore.find(obj);
    if (it != reverseStore.end()) {
        return it->second;
    }
    return ""; 
}