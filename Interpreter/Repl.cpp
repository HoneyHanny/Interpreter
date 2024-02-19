#include "Repl.h"

void Repl::Start(std::istream& in, std::ostream& out) {
    const std::string PROMPT = ">> ";

    while (true) {
        out << PROMPT;
        std::string line;
        if (!std::getline(in, line)) {
            break; // Exit loop if input stream is closed or an error occurs
        }

        Lexer l(line);

        for (Token tok = l.NextToken(); tok.Type != "EOF"; tok = l.NextToken()) {
            out << "Type: " << tok.Type << ", Literal: " << tok.Literal << std::endl;
        }
    }
}