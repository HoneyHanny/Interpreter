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

bool Environment::Exists(const std::string& name) const {
    if (store.find(name) != store.end()) {
        return true;
    }
    else if (outer) {
        return outer->Exists(name);
    }
    return false;

}

//void Environment::Set(const std::string& name, Token type, std::shared_ptr<Object> val) {
//    store[name] = std::make_tuple(type, name, val);
//    reverseStore[val] = name;
//}

void Environment::Set(const std::string& name, Token type, std::shared_ptr<Object> val) {
    if (Exists(name)) {
        // If the variable exists anywhere in the chain, find the closest definition and update it.
        Environment* env = this;
        while (env->outer != nullptr && !env->store.count(name)) {
            env = env->outer.get();
        }
        env->store[name] = std::make_tuple(type, name, val);  
    }
    else {
        // If it does not exist, define it in the current environment
        store[name] = std::make_tuple(type, name, val);
    }
    reverseStore[val] = name;  // Update reverse lookup in the current environment (in case we need it)
}


std::string Environment::GetNameByObject(const std::shared_ptr<Object>& obj) const {
    auto it = reverseStore.find(obj);
    if (it != reverseStore.end()) {
        return it->second;
    }
    else if (outer != nullptr) {
        return outer->GetNameByObject(obj);
    }
    return ""; 
}

