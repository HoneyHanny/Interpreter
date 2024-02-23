#pragma once

#include <iostream>
#include <string>

class Tracer {
public:
    static int indentLevel;
    static bool trace;

    Tracer(const std::string& functionName) : funcName(functionName) {
        if (trace) {
            printIndent("Entering " + funcName + "\n");
            ++indentLevel; 
        }
    }

    ~Tracer() {
        if (trace) {
            --indentLevel; 
            printIndent("Exiting " + funcName + "\n");
        }
    }

private:
    std::string funcName;

    void printIndent(const std::string& message) {
        std::cout << std::string(indentLevel * 2, ' ') << message;
    }
};

int Tracer::indentLevel = 0;
bool Tracer::trace = false; // Set to true for test function tracing;
