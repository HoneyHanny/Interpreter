#include <iostream>
#include "Tests.h"
#include "Repl.h"
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <memory>

static void displayHelp() {
    // TODO(hans): Print more help
    static bool done = false;
    if (!done) {
        std::cout <<"\
     ________  ________  ________  _______\n\
    |\\   ____\\|\\   __  \\|\\   ___ \\|\\  ___ \\\n\
    \\ \\  \\___|\\ \\  \\|\\  \\ \\  \\_|\\ \\ \\   __/|    \n\
     \\ \\  \\    \\ \\  \\\\\\  \\ \\  \\ \\\\ \\ \\  \\_|/__  \n\
      \\ \\  \\____\\ \\  \\\\\\  \\ \\  \\_\\\\ \\ \\  \\_|\\ \\ \n\
       \\ \\_______\\ \\_______\\ \\_______\\ \\_______\\ \n\
        \\|_______|\\|_______|\\|_______|\\|_______|" << std::endl;
                                            
        std::cout << "\n Help:" << std::endl;
        std::cout << " Github: https://github.com/HoneyHanny/Interpreter" << std::endl;
        std::cout << " Usage: code <filename|flags> " << std::endl;
        std::cout << " -h | --help : Display help" << std::endl;
        std::cout << " -v | --verbose : Display verbose" << std::endl;
        done = true;
    }
}

static void displayVerbose() {
    // TODO(hans): Implement verbose
    static bool done = false;
    if (!done) {
        std::cerr << "Verbose not implemented" << std::endl;
        done = true;
    }
}

std::string readFile(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    fileStream.close();
    return buffer.str();
}



int main(int argc, char* argv[]) {

    std::vector<std::string> files;

    //ExecuteTestCases(); // Comment out in actual

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

    if (files.size() == 0) { // No files specified run repl
        std::cout << "Hello! This is the CODE programming language!\n";
        std::cout << "Feel free to type in commands\n";

        Repl repl;
        repl.Start(std::cin, std::cout);
    } else { // Input files specified interpret source code
        std::string fileContent = readFile(files[0]);
        if (!fileContent.empty()) {
            //std::cout << "File content:\n" << fileContent << std::endl;
            auto lexer = std::make_unique<Lexer>(fileContent);
            Parser parser(std::move(lexer));

            auto program = parser.ParseProgram();
            //parser.enforcedStructure = false;
            // Enforce Structure
            if (parser.enforcedStructure) {
                for (const auto& stmt : program->Statements) {
                    if (mainstate == ParseState::BeginCode) {
                        if (!isBeginCodeStatement(stmt)) {
                            std::cerr << "Structure Error: Expected BEGIN CODE statement." << std::endl;
                            //std::exit(EXIT_FAILURE);
                        }
                        mainstate = ParseState::FunctionDeclarations;
                    }

                    else if (mainstate == ParseState::FunctionDeclarations) {
                        auto expr = extractExpression(stmt);
                        const auto fnlit = dynamic_cast<FunctionLiteral*>(expr.get());
                        if (fnlit) {
                            // Pass
                        }
                        else if (isTransitionToVariableDeclarations(stmt)) {
                            mainstate = ParseState::VariableDeclarations;
                        }
                        else if (isTransitionToExecutableCode(stmt)) {
                            mainstate = ParseState::ExecutableCode;
                        }
                        else if (isEndCodeStatement(stmt)) {
                            mainstate = ParseState::EndCode;
                        }
                        else {
                            std::cerr << "Structure Error: Unexpected statement type in Function Declarations state." << std::endl;
                            //std::exit(EXIT_FAILURE);
                        }
                    }

                    else if (mainstate == ParseState::VariableDeclarations) {
                        bool transitionFlag = false;
                        if (auto exprstmt = dynamic_cast<ExpressionStatement*>(stmt.get())) {
                            //std::cout << exprstmt->TokenLiteral() << std::endl;
                            if (exprstmt->TokenLiteral() == "FUNCTION") {
                                transitionFlag = true;
                            }
                        }
                        if (auto typedDeclStmt = dynamic_cast<TypedDeclStatement*>(stmt.get())) {
                            // Pass
                        }
                        else if (isTransitionToExecutableCode(stmt)) {
                            if (!transitionFlag) {
                                mainstate = ParseState::ExecutableCode;
                            }
                        }
                        else if (isEndCodeStatement(stmt)) {
                            mainstate = ParseState::EndCode;
                        }
                        else {
                            std::cerr << "Structure Error: Unexpected statement type in Variable Declarations state." << std::endl;
                            //std::exit(EXIT_FAILURE);
                        }
                    }

                    else if (mainstate == ParseState::ExecutableCode) {
                        auto expr = extractExpression(stmt);
                        const auto fnlit = dynamic_cast<FunctionLiteral*>(expr.get());
                        if (fnlit) {
                            std::cerr << "Structure Error: Unexpected function declaration in Executable Code state." << std::endl;
                            //std::exit(EXIT_FAILURE);
                        }

                        else if (auto typedDeclStmt = dynamic_cast<TypedDeclStatement*>(stmt.get())) {
                            std::cerr << "Structure Error: Unexpected variable declaration in Executable Code state." << std::endl;
                            //std::exit(EXIT_FAILURE);
                        }

                        else if (isEndCodeStatement(stmt)) {
                            mainstate = ParseState::EndCode;
                        }

                        else {
                            // Pass
                        }
                    }

                    else if (mainstate == ParseState::EndCode) {
                        if (!isAllowedInEndCode(stmt)) {
                            std::cerr << "Structure Error: No statements are expected after END CODE." << std::endl;
                            //std::exit(EXIT_FAILURE);
                        }
                    }
                }
            }

            // Display if any other parser errors
            for (auto errors : parser.Errors()) {
                std::cout << errors << std::endl;
            }

           /* if (!enforceProgramStructure(clonedStatements)) {
                std::exit(EXIT_FAILURE);
            }*/
            auto env = std::make_shared<Environment>();

            auto res = Eval(program.get(), env);

            // Display if any evaluation errors
            if (auto displayObject = dynamic_cast<ErrorObject*>(res.get())) {
                std::cout << displayObject->Message << std::endl;
            }
        }
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
