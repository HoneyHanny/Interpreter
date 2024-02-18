#include <iostream>
//#include <vector>
//#include <string>
#include "lexer_test.h"

int main(int argc, char* argv[])
{
    std::cout << "Test 1: " << std::endl;
    TestNextToken();

    std::cout << "Test 2: " << std::endl;
    TestNextToken2();
    return 0;
}

// - # - Comment line
// - BEGIN CODE - Start of the program
// - INT, CHAR, BOOL, FLOAT - Datatypes
// - $ - New line
// - END CODE - End of program
// - DISPLAY: - Print
// - All variable declaration must be before than the rest of code
// - All variable names are case sensitive and starts
//   with a letter or _ and followed by _, digits, letters
// - Everyline contains a single statement
// - All reserved words are in all caps and can't be used as variable names
// - & Concatenator
// - [] - escaped code
// - (, ), *, /, %, +, -, >, <, >=, <=, ==, <>, AND, OR, NOT - Operators
// - +, - - Unary operator
