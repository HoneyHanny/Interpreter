#include "Parser.h"
#include "Tracer.h"

void Parser::nextToken() {
    curToken = peekToken;
    if (curToken.Literal == NEWLINE) incrementLine();

    peekToken = lexer->NextToken();
}   

bool Parser::isTypedDeclStatementStart() const {
    // Check if the current token is a type keyword that can start a typed declaration statement
    return curToken.Type == INT ||
        curToken.Type == CHAR ||
        curToken.Type == BOOL ||
        curToken.Type == FLOAT ||
        curToken.Type == STRING;
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::unique_ptr<Program> Parser::ParseProgram() {
    auto program = std::make_unique<Program>();

    while (curToken.Type != EOF_TOKEN) {
        if (isTypedDeclStatementStart()) { // Check if the current token starts a typed declaration
            auto declWrappers = parseTypedDeclStatements(); // Returns a vector of DeclStatementWrapper

            // Determine whether to create a single or multiple declarations
            if (declWrappers.size() == 0) {
                // pass
            }
            else if (declWrappers.size() == 1) {
                std::visit(overloaded{
                    [&program](std::unique_ptr<TypedDeclStatement>& stmt) {
                        program->Statements.push_back(std::move(stmt));
                    },
                    [&program](std::unique_ptr<MultiTypedDeclStatement>& multiStmt) {
                        program->Statements.push_back(std::move(multiStmt));
                    }
                    }, declWrappers[0].content);
            }
            else {
                Token commonToken = std::get<std::unique_ptr<TypedDeclStatement>>(declWrappers[0].content)->token;
                auto multiStmt = std::make_unique<MultiTypedDeclStatement>(commonToken);

                for (auto& wrapper : declWrappers) {
                    std::visit(overloaded{
                        [multiStmt = multiStmt.get()](std::unique_ptr<TypedDeclStatement>& stmt) mutable {
                            multiStmt->Names.push_back(std::move(stmt->Name));
                            if (stmt->Value) {  // Assume values align with names
                                multiStmt->Values.push_back(std::move(stmt->Value));
                            }
                        },
                        [](std::unique_ptr<MultiTypedDeclStatement>&) {
                            // This case should not happen if parseTypedDeclStatements is implemented correctly
                        }
                        }, wrapper.content);
                }
                program->Statements.push_back(std::move(multiStmt));
            }
        }
        else {
            // Handle other statement types
            auto stmt = parseStatement();
            if (stmt != nullptr) {
                program->Statements.push_back(std::move(stmt));
            }
        }
        nextToken();
    }
    return program;
}




std::unique_ptr<Statement> Parser::parseStatement() {
    Tracer tracer("parseStatement");

    if (enforcedStructure) {
        if ((curToken.Type == BEGIN || curToken.Type == END) && peekToken.Type == CODE) {
            return parseMarkerStatement(curToken, peekToken);
        }
    }

    //// Typed Declaration Statements
    //if (curToken.Type == INT 
    //    || curToken.Type == CHAR 
    //    || curToken.Type == BOOL
    //    || curToken.Type == FLOAT
    //    || curToken.Type == STRING) 
    //{
    //    auto variantResult = parseTypedDeclStatement();

    //    // Use std::visit to handle each type in the variant
    //    std::unique_ptr<Statement> result = std::visit([](auto&& arg) -> std::unique_ptr<Statement> {
    //        using T = std::decay_t<decltype(arg)>;
    //        if constexpr (std::is_same_v<T, std::unique_ptr<TypedDeclStatement>> ||
    //            std::is_same_v<T, std::unique_ptr<MultiTypedDeclStatement>>) {
    //            // For TypedDeclStatement or MultiTypedDeclStatement, return as is.
    //            return std::move(arg);
    //        }
    //        else if constexpr (std::is_same_v<T, nullptr_t>) {
    //            // Handle the nullptr case.
    //            return nullptr;
    //        }
    //        else {
    //            // This should never be reached if you only have the specified types in your variant.
    //            //static_assert(always_false<T>::value, "Non-exhaustive visitor!");
    //            return nullptr;
    //        }
    //        }, variantResult);

    //    return result;
    //}
    // Return Statements
    if (curToken.Type == RETURN) {
        return parseReturnStatement();
    }

    //else if (curToken.Type == FUNCTION) {
    //    auto stmt = std::make_unique<ExpressionStatement>(curToken);
    //    auto fnToken = curToken;

    //    nextToken();

    //    auto callName = parseIdentifier();

    //    if (peekTokenIs(LPAREN)) {
    //        stmt->Expression_ = parseFunctionLiteral(fnToken, std::move(callName));

    //        if (peekTokenIs(NEWLINE)) {
    //            nextToken();
    //        }

    //    }
    //    else if (peekTokenIs(ASSIGN)) {
    //        nextToken();
    //        nextToken();

    //        stmt->name = std::move(callName);
    //        stmt->Expression_ = parseExpression(Precedence::LOWEST);

    //        if (peekTokenIs(NEWLINE)) {
    //            nextToken();
    //        }
    //    }

    //    return stmt;
    //}

    // Expression Statements
    else if (curToken.Type != NEWLINE){
        return parseExpressionStatement();
    }
    return nullptr;
}

std::unique_ptr<MarkerStatement> Parser::parseMarkerStatement(Token type, Token codeToken) {
    auto stmt = std::make_unique<MarkerStatement>(type, codeToken);

    if (!expectPeek(CODE)) { // Move on to CODE
        return nullptr;
    }
    if (!customExpectPeek("Expected BEGIN CODE statement.", NEWLINE, EOF_TOKEN)) { // Move on to NEWLINE, EOF
        while (!curTokenIs(NEWLINE) && !curTokenIs(EOF_TOKEN)) { // Ensure that statement after doesn't get processed
            nextToken();
        }
        return nullptr;
    }

    return stmt;
}


// Typed statement is either a declaration w/out value or a function parameter:
// 
// NEWLINE: Statement is a variable declaration (e.g. INT x) 
// INT i = 0
// INT x, y, z = 5 (not yet implemented)
// 
// COMMA and RPAREN: Statement is part of a function arguement
// FUNCTION add(INT x, INT y) INT:
// #[...]

std::vector<DeclStatementWrapper> Parser::parseTypedDeclStatements() {
    Tracer tracer("parseTypedDeclStatements");

    Token typeToken = curToken;
    std::vector<DeclStatementWrapper> declarations;

    do {
        std::vector<std::unique_ptr<Identifier>> names;
        std::unique_ptr<Expression> value;

        if (!customExpectPeek("Expected identifier in declaration.", IDENT)) {
            while (peekToken.Type != NEWLINE) {
                nextToken();
            }
            return {};
        }
        names.push_back(std::make_unique<Identifier>(curToken, curToken.Literal));

        // Check for assignment right after the identifier
        if (peekTokenIs(ASSIGN)) {
            nextToken(); // Consume ASSIGN
            nextToken(); // Move to the expression
            value = parseExpression(Precedence::LOWEST);
            declarations.push_back(DeclStatementWrapper(std::make_unique<TypedDeclStatement>(typeToken, std::move(names.back()), std::move(value))));
        }
        else {
            declarations.push_back(DeclStatementWrapper(std::make_unique<TypedDeclStatement>(typeToken, std::move(names.back()))));
        }

        // Check for comma to continue the loop or break if it's the end of the declaration list
        if (!peekTokenIs(COMMA)) break;
        nextToken(); // Consume COMMA

    } while (true);

    // Ensure end of declaration with NEWLINE or EOF_TOKEN
    if (!expectPeek(NEWLINE, EOF_TOKEN, "Expected end of statement after declaration.")) {
        return {};
    }

    return declarations;
}

// Subtree structure: <RETURN> <EXPRESSION>
std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    Tracer tracer("parseReturnStatement");

    auto stmt = std::make_unique<ReturnStatement>(curToken);

    nextToken();

    stmt->ReturnValue = parseExpression(Precedence::LOWEST);

    // If statement not on new line
    if (!expectPeek(NEWLINE, EOF_TOKEN)) {
        //errors.push_back("Statement not on new line.");
        return nullptr;
    }
    else {
        while (!curTokenIs(NEWLINE) && !curTokenIs(EOF_TOKEN)) {
            nextToken();
        }
        return stmt;
    }
}

// Subtree structure: <EXPRESSION>
std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() { 
    Tracer tracer("parseExpressionStatement");

    std::unique_ptr<ExpressionStatement> stmt = std::make_unique<ExpressionStatement>(curToken);

    stmt->Expression_ = parseExpression(Precedence::LOWEST);

    if (currentParsedType.Type == FUNCTION) {
        stmt->token = currentParsedType;

        if (currentParsedCallName) {
            stmt->name = currentParsedCallName->clone();
        }
    }

    if (peekTokenIs(NEWLINE)) {
        nextToken();
    }

    currentParsedType = { "", "" };

    return stmt;
}

std::unique_ptr<BlockStatement> Parser::parseBlockStatement() {
    Tracer tracer("parseBlockStatement");
    auto block = std::make_unique<BlockStatement>(curToken);
    nextToken();

    while (!(curTokenIs(END) 
        && (peekTokenIs(IF, ELSE, FUNCTION, WHILE))) // ... END (IF || ELSE || FUNCTION || WHILE)
        && !curTokenIs(EOF_TOKEN)) { 
        auto stmt = parseStatement();   
        if (stmt != nullptr) {
            block->Statements.push_back(std::move(stmt));
        }
        nextToken();
    }

    // TODO: Need to check for matching checksums 
    if (curTokenIs(END)) { // Advance tokens by two to account for positioning. Adjust later if needed.
        nextToken(); 
        nextToken();
    }
    return block;
}

std::vector<std::unique_ptr<TypedDeclStatement>> Parser::parseFunctionParameters() {
    Tracer tracer("parseFunctionParameters");
    std::cout << "Called" << std::endl;

    std::vector<std::unique_ptr<TypedDeclStatement>> typedDeclStmts;

    if (peekTokenIs(RPAREN)) {
        nextToken(); 
        return typedDeclStmts;
    }

    do {
        nextToken(); // Move to the first parameter or next parameter after ','

        if (isTypedDeclStatementStart()) {
            auto declWrappers = parseTypedDeclStatements();

            for (auto& wrapper : declWrappers) {
                std::visit(overloaded{
                    [&typedDeclStmts](std::unique_ptr<TypedDeclStatement>& stmt) {
                        typedDeclStmts.push_back(std::move(stmt));
                    },
                    [](std::unique_ptr<MultiTypedDeclStatement>& multiStmt) {
                        // MultiTypedDeclStatement is not expected in function parameters.
                        // You can log an error or handle it according to your needs.
                        // For simplicity, ignoring it here but in a real scenario, you should handle this case.
                        std::cerr << "MultiTypedDeclStatement is not allowed in function parameters." << std::endl;
                    },
                    [](auto&&) {
                        // Handle error or nullptr cases
                    }
                }, wrapper.content); // Assuming wrapper is the variant holding either TypedDeclStatement or MultiTypedDeclStatement
            }
        }


        if (!peekTokenIs(COMMA)) {
            break; // Exit if the next token is not a comma
        }
        nextToken(); 
    } while (!peekTokenIs(RPAREN, EOF_TOKEN));

    if (!expectPeek(RPAREN)) {
        return {};
    }

    return typedDeclStmts;
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence precedence) {
    Tracer tracer("parseExpression");

    auto prefixIt = prefixParseFns.find(curToken.Type);
    if (prefixIt == prefixParseFns.end()) {
        std::cerr << "Unexpected token: " << curToken.Type << std::endl;
        noPrefixParseFnError(curToken.Type);
        return nullptr;
    }

    auto& prefixFn = prefixIt->second;
    std::unique_ptr<Expression> leftExp = prefixFn();

    // TODO: Handle explicitly forcing a new expression to be on a new line,
    // current implementation doesn't handle this
    while (!peekTokenIs(NEWLINE) && precedence < peekPrecedence()) {
        auto infixIt = infixParseFns.find(peekToken.Type);
        if (infixIt == infixParseFns.end()) {
            return leftExp;
        }

        auto& infixFn = infixIt->second;
        nextToken();

        leftExp = infixFn(std::move(leftExp));
    }

    return leftExp;
}

// Prefix parsers

std::unique_ptr<Expression> Parser::parseIdentifier() {
    Tracer tracer("parseIdentifier");

    return std::make_unique<Identifier>(curToken, curToken.Literal);
}

std::unique_ptr<Expression> Parser::parseNumericalLiteral() {
    Tracer tracer("parseNumericalLiteral");

    if (peekTokenIs(DOT)) {
        // Handle float
        std::string floatLiteral = curToken.Literal;
        nextToken(); // Consume the '.'
        if (!expectPeek(NUM)) {
            std::string msg = "expected number after dot, got " + peekToken.Literal;
            errors.push_back(msg);
            return nullptr;
        }
        floatLiteral += "." + curToken.Literal;
        char* end;
        double value = std::strtod(floatLiteral.c_str(), &end);

        //if (!peekTokenIs(NEWLINE, EOF_TOKEN)) { TODO: Figure out how to enforce strict formatting for floats and ints
        //    std::string msg = "unexpected text in float literal";
        //    errors.push_back(msg);
        //    return nullptr;
        //}

        if (*end != '\0') { // Check if the entire string was converted
            std::string msg = "could not parse " + floatLiteral + " as float";
            errors.push_back(msg);
            return nullptr;
        }

        return std::make_unique<FloatLiteral>(Token(FLOAT, floatLiteral), value);
    }
    else {
        // Handle integer
        char* end;
        long int value = std::strtol(curToken.Literal.c_str(), &end, 10); // Using base 10 for conversion

        if (*end != '\0') { // Check if the entire string was converted
            std::string msg = "could not parse " + curToken.Literal + " as integer";
            errors.push_back(msg);
            return nullptr;
        }

        return std::make_unique<IntegerLiteral>(curToken, value);
    }
}

std::unique_ptr<Expression> Parser::parseStringLiteral() {
    Tracer tracer("parseStringLiteral");

    return std::make_unique<StringLiteral>(curToken, curToken.Literal);
}

std::unique_ptr<Expression> Parser::parseCharLiteral() {
    Tracer tracer("parseCharLiteral");

    if (curToken.Literal.length() > 1) {
        std::string errorMessage = "Character literals must contain only one character. Found '" + curToken.Literal + "'";
        errors.push_back(errorMessage);
        return nullptr; 
    }

    char charValue = curToken.Literal[0];
    return std::make_unique<CharLiteral>(curToken, charValue);
}

std::unique_ptr<Expression> Parser::parsePrefixExpression() {
    Tracer tracer("parsePrefixExpression");

    auto expression = std::make_unique<PrefixExpression>(curToken, curToken.Literal);

    nextToken();

    expression->Right = parseExpression(Precedence::PREFIX);

    return expression;
}

std::unique_ptr<Expression> Parser::parseInfixExpression(std::unique_ptr<Expression> left) {
    Tracer tracer("parseInfixExpression");

    Token token = curToken;
    std::string op = curToken.Literal;
    auto precedence = curPrecedence();

    nextToken(); 

    auto right = parseExpression(precedence);

    return std::make_unique<InfixExpression>(token, std::move(left), op, std::move(right));
}

std::unique_ptr<Expression> Parser::parseBoolean() {
    Tracer tracer("parseBoolean");

    bool isTrue = curToken.Type == TRUE;
    return std::make_unique<Boolean>(curToken, isTrue);
}

std::unique_ptr<Expression> Parser::parseGroupedExpression() {
    Tracer tracer("parseGroupedExpression");

    nextToken(); 

    auto exp = parseExpression(Precedence::LOWEST);

    if (!expectPeek(RPAREN)) {
        return nullptr; 
    }

    return exp;
}

std::unique_ptr<Expression> Parser::parseIfExpression() {
    Tracer tracer("parseIfExpression");

    auto token = curToken; 
    auto expression = std::make_unique<IfExpression>(token);

    if (!expectPeek(LPAREN)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find LPAREN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    nextToken();

    //expression->Condition = parseExpression(Precedence::LOWEST);
    auto ifCondition = parseExpression(Precedence::LOWEST);

    if (!expectPeek(RPAREN)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find RPAREN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(NEWLINE)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find NEWLINE, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(BEGIN)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find BEGIN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(IF)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find IF, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    //expression->Consequence = parseBlockStatement();
    auto ifConsequence = parseBlockStatement();

    // Push Branch
    expression->Branches.push_back(std::make_pair(std::move(ifCondition), std::move(ifConsequence)));

    // Check for an 'else' part
    while (peekTokenIs(ELSE)) {
        nextToken(); // CONSUME ELSE

        // ELSE IF
        if (peekTokenIs(IF)) {
            nextToken(); // CONSUME IF

            if (!expectPeek(LPAREN)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find LPAREN, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            nextToken();

            //std::pair<std::unique_ptr<Expression>, std::unique_ptr<BlockStatement>> elseIfBranch;
            auto elseIfCondition = parseExpression(Precedence::LOWEST);

            if (!expectPeek(RPAREN)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find RPAREN, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            if (!expectPeek(NEWLINE)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find NEWLINE, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            if (!expectPeek(BEGIN)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find BEGIN, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            if (!expectPeek(IF)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find ELSE, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            auto elseIfConsequence = parseBlockStatement();
            
            expression->Branches.push_back(std::make_pair(std::move(elseIfCondition), std::move(elseIfConsequence)));
        }

        // ELSE
        else {
            if (!expectPeek(NEWLINE)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find NEWLINE, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            if (!expectPeek(BEGIN)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find BEGIN, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            //if (!expectPeek(ELSE)) {
            if (!expectPeek(IF)) {
                std::cerr << expression->TokenLiteral() << std::endl;
                std::cerr << "Failed to find ELSE, peekToken: " << peekToken.Type << std::endl;
                return nullptr;
            }

            expression->Alternative = parseBlockStatement();
        }
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseWhileExpression() {
    Tracer tracer("parseWhileExpression");

    auto token = curToken;
    auto expression = std::make_unique<WhileExpression>(token);

    if (!expectPeek(LPAREN)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find LPAREN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    nextToken();
    expression->Condition = parseExpression(Precedence::LOWEST);

    if (!expectPeek(RPAREN)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find RPAREN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(NEWLINE)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find NEWLINE, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(BEGIN)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find BEGIN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(WHILE)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find IF, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    expression->Body = parseBlockStatement();

    return expression;
}

std::unique_ptr<Expression> Parser::parseFunctionLiteral(Token fnToken, std::unique_ptr<Expression> CallName) {
    Tracer tracer("parseFunctionLiteral");

    auto lit = std::make_unique<FunctionLiteral>(fnToken);

    lit->CallName = std::move(CallName);

    if (!expectPeek(LPAREN)) {
        std::cerr << lit->TokenLiteral() << std::endl;
        std::cerr << "Failed to find LPAREN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    lit->Parameters = parseFunctionParameters();
    //if (lit->Parameters.empty() && hasErrors) {
    //    return nullptr; 
    //}

    if (!(peekTokenIs(INT, CHAR, FLOAT, BOOL, VOID, STRING, FUNCTION))) { // Function must have type declaration
        std::cerr << lit->TokenLiteral() << std::endl;
        std::cerr << "Failed to find TYPE, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    nextToken();

    lit->type = curToken;

    if (!expectPeek(COLON)) {
        std::cerr << lit->TokenLiteral() << std::endl;
        std::cerr << "Failed to find COLON, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(NEWLINE)) {
        std::cerr << "Failed to find NEWLINE" << std::endl;
        return nullptr;
    }

    // Parsing function header ends here

    if (!expectPeek(BEGIN)) {
        std::cerr << lit->TokenLiteral() << std::endl;
        std::cerr << "Failed to find BEGIN, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    if (!expectPeek(FUNCTION)) {
        std::cerr << lit->TokenLiteral() << std::endl;
        std::cerr << "Failed to find ELSE, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }


    lit->Body = parseBlockStatement();

    return lit;
}

std::unique_ptr<Expression> Parser::parseCallExpression(std::unique_ptr<Expression> function) {
    Tracer tracer("parseCallExpression");
    auto exp = std::make_unique<CallExpression>(curToken);
    exp->Function = std::move(function);
    exp->Arguments = parseCallArguments();
    return exp;
}

std::vector<std::unique_ptr<Expression>> Parser::parseCallArguments() {
    Tracer tracer("parseCallArguments");

    std::vector<std::unique_ptr<Expression>> args;

    if (peekTokenIs(NEWLINE, EOF_TOKEN)) { // Arg list is empty
        nextToken(); 
        return args;
    }

    do {
        nextToken(); // Move to the first argument 

        auto arg = parseExpression(Precedence::LOWEST);
        if (!arg) {
            return {}; 
        }
        args.push_back(std::move(arg));

        while (peekTokenIs(COMMA)) {
            nextToken(); // Consume the ','
            nextToken(); // Move to the next argument

            auto arg = parseExpression(Precedence::LOWEST);
            if (!arg) {
                return {};
            }
            args.push_back(std::move(arg));
        }
    } while (!peekTokenIs(NEWLINE, EOF_TOKEN));

    if (!expectPeek(NEWLINE, EOF_TOKEN)) {
        return {};
    }

    return args;
}

std::unique_ptr<Expression> Parser::parseAssignExpression(std::unique_ptr<Expression> name) {
    Tracer tracer("parseAssignExpression");
    auto assignExp = std::make_unique<AssignExpression>();
    assignExp->addName(std::move(name));
    
    nextToken(); // Consume ASSIGN

    bool identStage = true, assignStage = false; // Start expecting an identifier after an assignment

    while (true) {        
        if (identStage) {
            if (curTokenIs(IDENT) && !peekTokenIs(NEWLINE, EOF_TOKEN, PLUS, MINUS, ASTERISK, SLASH, AMPERSAND)) {
                auto nextName = parseIdentifier();
                assignExp->addName(std::move(nextName)); 
                identStage = false;
                assignStage = true; 
            }
            else if (peekTokenIs(NEWLINE, EOF_TOKEN, PLUS, MINUS, ASTERISK, SLASH, AMPERSAND)) {
                // Reached expression part
                break;
            }
            else {
                // If we don't find an identifier when we expect one, it's an error
                return nullptr;
            }
        }
        else if (assignStage) {
            if (curTokenIs(ASSIGN)) {
                // Continue parsing, switch back to identifier stage
                identStage = true;
                assignStage = false;
            }
            else {
                return nullptr;
            }
        }

        // Move to the next token to check
        nextToken();
    }

    assignExp->setValue(parseExpression(Precedence::LOWEST));
    if (!expectPeek(NEWLINE, EOF_TOKEN)) {
        return nullptr;
    }
    /*if (peekTokenIs(NEWLINE, EOF_TOKEN)) {
        nextToken();
    }*/
    return assignExp;
}

// Helper functions

bool Parser::curTokenIs(const TokenType& t) const {
    return curToken.Type == t;
}

bool Parser::peekTokenIs(const TokenType& t) const {
    return peekToken.Type == t;
}

template<typename... TokenTypes>
bool Parser::peekTokenIs(TokenTypes... types) {
    return ((peekToken.Type == types) || ...); // Krazy C++ wizardry...
}

bool Parser::expectPeek(const TokenType& t) {
    if (peekTokenIs(t)) {
        nextToken();
        return true;
    }
    else {
        peekError(t);
        return false;
    }
}

bool Parser::customExpectPeek(const std::string& errorMessage, const TokenType& t) {
    if (peekTokenIs(t)) {
        nextToken();
        return true;
    }
    else {
        // Use the custom error message if provided, otherwise generate a generic one.
        if (!errorMessage.empty()) {
            peekError(errorMessage);
        }
        else {
            std::string msg = "Expected next token to be " + std::string(t) +
                ", got " + std::string(peekToken.Type) + " instead.";
            peekError(msg);
        }
        return false;
    }
}

// For expecting multiple peekToken types at once
template<typename... TokenTypes> 
bool Parser::expectPeek(TokenTypes... types) { 
    if (peekTokenIs(types...)) {
        nextToken();
        return true;
    }
    else {
        std::string expectedTypes = "Invalid identifier/literal";
        peekError(expectedTypes);
        return false;
    }
}

template<typename... TokenTypes>
bool Parser::customExpectPeek(const std::string& errorMessage, TokenTypes... types) {
    if (peekTokenIs(types...)) {
        nextToken();
        return true;
    }
    else {
        peekError(errorMessage);
        return false;
    }
}

Precedence Parser::peekPrecedence() const {
    auto it = precedences.find(peekToken.Type);
    if (it != precedences.end()) {
        return it->second;
    }
    return Precedence::LOWEST;
}

Precedence Parser::curPrecedence() const {
    auto it = precedences.find(curToken.Type);
    if (it != precedences.end()) {
        return it->second;
    }
    return Precedence::LOWEST;
}

// Error handling

std::vector<std::string> Parser::Errors() const {
    return errors;
}

void Parser::peekError(const TokenType& expected) {
    //std::string msg = "Error on line " + std::to_string(currentLine) + ": Expected next token to be " +
    //    std::string(expected) + ", got " + std::string(peekToken.Type) + " instead.";
    std::string msg = "Error: Expected next token to be " +
        std::string(expected) + ", got " + std::string(peekToken.Type) + " instead.";
    errors.push_back(msg);
}


void Parser::peekError(const std::string& message) {
    //errors.push_back("Error on line " + std::to_string(currentLine) + ": " + message);
    errors.push_back("Error: " + message);
}


void Parser::noPrefixParseFnError(TokenType t) {
    std::string msg = "no prefix parse function for " + std::string(t) + " found";
    errors.push_back(msg);
}

// Token <--> Parser functions - mapping

void Parser::registerPrefix(TokenType type, prefixParseFn fn) {
    prefixParseFns[type] = fn;
}

void Parser::registerInfix(TokenType type, infixParseFn fn) {
    infixParseFns[type] = fn;
}

void Parser::printCurTokenLiteral() {
    if (curToken.Literal == "\n") {
        std::cout << "\\n" << std::endl;
    }
    else {
        std::cout << curToken.Literal << std::endl;
    }
}
