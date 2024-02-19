#include <iostream>
//#include <vector>
//#include <string>
#include "lexer_test.h"

int main(int argc, char* argv[]) {
    std::cout << "Test 1: " << std::endl;
    TestNextToken();

    std::cout << "Test 2: " << std::endl;
    TestNextToken2();

    std::cout << "Test 3: " << std::endl;
    TestNextToken3();
    return 0;
}

// IMPORTANT DATES:
//   Date   | Inc | Feature
// ------------------------------
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
