#pragma once

#include <iostream>
#include <string>

class Tracer {
public:
    static int indentLevel;

    Tracer(const std::string& functionName) : funcName(functionName) {
        printIndent("Entering " + funcName + "\n");
        ++indentLevel; 
    }

    ~Tracer() {
        --indentLevel; 
        printIndent("Exiting " + funcName + "\n");
    }

private:
    std::string funcName;

    void printIndent(const std::string& message) {
        std::cout << std::string(indentLevel * 2, ' ') << message;
    }
};

int Tracer::indentLevel = 0;

