#include <iostream>

#include "interpreter.h"

int main(int argc, char* argv[])
{
    std::vector<std::vector<std::string>> lines = Parse(argv[1]);

    for (int i = 0; i < lines.size(); i++)
    {
        std::cout << "Line " << i + 1 << ": ";
        for (int j = 0; j < lines.at(i).size(); j++)
        {
            std::cout << "[" << lines.at(i).at(j) << "], ";
        }
        std::cout << "\n";
    }

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
