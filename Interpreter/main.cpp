#include <iostream>
#include "lexer_test.h"
#include "parser_test.h"
#include "Repl.h"
#include <string.h>
#include <string>
#include <vector>

static void displayHelp() {
    // TODO(hans): Print more help
    static bool done = false;
    if (!done) {
        std::cout << " Help:" << std::endl;
        std::cout << " Github: https://github.com/HoneyHanny/Interpreter" << std::endl;
        std::cout << " Usage: code <filename|flags> " << std::endl;
        std::cout << " -h | --help : Display help" << std::endl;
        std::cout << " -v | --verbose : Display verbose" << std::endl;
        done = true;
    }
}

static void displayVerbose() {
    // TODO(hans): implement verbose
    static bool done = false;
    if (!done) {
        std::cerr << "Verbose not implemented" << std::endl;
        done = true;
    }
}

int main(int argc, char* argv[]) {

    std::vector<std::string> files;

    // Process command arguments
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') { // Compiler flag
            // Help flag
            if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
                displayHelp();
            }
            // Verbose flag
            if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
                displayVerbose();
            }
        } else { // File input
            // NOTE(hans): Assume only one file for now
            files.push_back(std::string(argv[i]));
            if (files.size() > 1) {
                std::cerr << "Please input only 1 file" << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    // Testing for tokenizer - uncomment to test tokenizer or add more test cases at lexer_test
    /*std::cout << "Test 1: " << std::endl;
    TestNextToken();

    std::cout << "Test 2: " << std::endl;
    TestNextToken2();

    std::cout << "Test 3: " << std::endl;
    TestNextToken3();*/

    // Testing for parser - uncomment to test parser or add more test cases at parser_test
    TestTypedDeclStatements();

    if (files.size() == 0) { // No files specified run repl
        std::cout << "Hello! This is the CODE programming language!\n";
        std::cout << "Feel free to type in commands\n";

        Repl repl;
        repl.Start(std::cin, std::cout);
    } else { // Input files specified interpret source code
        // TODO(hans): Read and process file
        std::cerr << "File input not implemented" << std::endl;
    }

    return 0;
}

// IMPORTANT DATES:
//   Date   | Inc | Feature
// ---------|-----|--------------
// March 28 |  2  | Basic
// April 04 |  2  | Basic
// April 11 |  3  | Basic
// April 25 |  4  | Conditional
//   May 02 |  5  | Loop
//   May 06 |  6  | Final

// CODE SPECS:
// Basic
// - # - Comment line
// - BEGIN CODE - Start of the program
// - INT, CHAR, BOOL, FLOAT - Datatypes
// - $ - New line
// - END CODE - End of program
// - DISPLAY: - Print
// - SCAN: - Allow the user to input a value to a data type | syntax: SCAN: <varName>[,<varName>]*
// - All variable declaration must be before than the rest of code
// - All variable names are case sensitive and starts with
//   a letter or _ and followed by _, digits, letters
// - Everyline contains a single statement
// - All reserved words are in all caps and can't be used as variable names
// - & Concatenator
// - [] - escaped code
// - (, ), *, /, %, +, -, >, <, >=, <=, ==, <>, AND, OR, NOT - Operators
// - +, - - Unary operator
//
// Control flow
// - Conditional
//      - if selection
//      - if-else selection
//      - if-else with multiple alternatives
// - Loop
//      - while
