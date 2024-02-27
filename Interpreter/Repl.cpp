#include "Repl.h"

static void printParserErrors(std::ostream& out, const std::vector<std::string>& errors) {
    for (const auto& error : errors) {
        out << "Parser error: " << error << std::endl;
    }
}

void Repl::Start(std::istream& in, std::ostream& out) {
    const std::string PROMPT = ">> ";

    while (true) {
        out << PROMPT;
        std::string line;
        if (!std::getline(in, line)) {
            break; // Exit loop if input stream is closed or an error occurs
        }

        auto lexer = std::make_unique<Lexer>(line);
        Parser parser(std::move(lexer));
        auto program = parser.ParseProgram();

        if (!parser.Errors().empty()) {
            printParserErrors(std::cout, parser.Errors());
            continue;
        }

        auto evaluated = Eval(program.get());
        if (evaluated != nullptr) {
            out << evaluated->Inspect() << std::endl;
        }
    }
}