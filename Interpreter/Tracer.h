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
		for (int i = 0; i < indentLevel; i++)
			std::cout << "| ";
        std::cout << message;
    }
};

int Tracer::indentLevel = 0;
bool Tracer::trace = true; // Set to true for function tracing;
