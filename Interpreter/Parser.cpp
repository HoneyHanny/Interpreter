#include "Parser.h"
#include "Tracer.h"

void Parser::nextToken() {
    curToken = peekToken;
    peekToken = lexer->NextToken();
}   

std::unique_ptr<Program> Parser::ParseProgram() {
    Tracer tracer("parseProgram");

    auto program = std::make_unique<Program>();

    while (curToken.Type != EOF_TOKEN) {
        auto stmt = parseStatement();
        if (stmt != nullptr) {
            program->Statements.push_back(std::move(stmt));
        }
        nextToken();
    }

    return program;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    Tracer tracer("parseStatement");

    // Typed Declaration Statements
    if (curToken.Type == INT 
        || curToken.Type == CHAR 
        || curToken.Type == BOOL
        || curToken.Type == FLOAT
        || curToken.Type == STRING) 
    {

       /* if (peekTokenIs(NEWLINE)) {
            std::cout << "FN statement" << std::endl;
            return nullptr;
        }*/

        return parseTypedDeclStatement();
    }
    // Return Statements
    else if (curToken.Type == RETURN) {
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

// Subtree structure: <TYPE> <IDENT> <ASSIGN> <EXPRESSION>
std::unique_ptr<TypedDeclStatement> Parser::parseTypedDeclStatement() {
    Tracer tracer("parseTypedDeclStatement");

    auto stmt = std::make_unique<TypedDeclStatement>(curToken);

    if (!expectPeek(IDENT)) {
        return nullptr;
    }

    stmt->Name = std::make_unique<Identifier>(curToken, curToken.Literal);

    // Typed statement is either a declaration w/out value or a function parameter:
    // NEWLINE: Statement is a variable declaration (e.g. INT x) 
    // COMMA and RPAREN: Statement is part of a function arguement
    if (peekTokenIs(COMMA, NEWLINE, RPAREN)) { 
        return stmt;
    }

    if (!peekTokenIs(ASSIGN)) {
        return stmt;
    }

    nextToken();
    nextToken();

    stmt->Value = parseExpression(Precedence::LOWEST);

    while (!curTokenIs(NEWLINE) && !curTokenIs(EOF_TOKEN)) {
        nextToken();
    }

    return stmt;
}

// Subtree structure: <RETURN> <EXPRESSION>
std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    Tracer tracer("parseReturnStatement");

    auto stmt = std::make_unique<ReturnStatement>(curToken);

    nextToken();

    stmt->ReturnValue = parseExpression(Precedence::LOWEST);

    while (!curTokenIs(NEWLINE) && !curTokenIs(EOF_TOKEN)) {
        nextToken();
    }

    return stmt;
}

// Subtree structure: <EXPRESSION>
std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() { 
    Tracer tracer("parseExpressionStatement");

    std::unique_ptr<ExpressionStatement> stmt = std::make_unique<ExpressionStatement>(curToken);

    stmt->Expression_ = parseExpression(Precedence::LOWEST);

    if (currentParsedType.Type == FUNCTION) {
        std::cout << "Creating a function!  - currentParsedType: " << currentParsedType.Type << std::endl;
        stmt->token = currentParsedType;

        if (currentParsedCallName) {
            std::cout << "Cloning into exprstmt " << stmt->token.Type << " name: " << currentParsedCallName->String() << std::endl;
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
        && (peekTokenIs(IF, ELSE, FUNCTION))) // ... END (IF || ELSE || FUNCTION)
        && !curTokenIs(EOF_TOKEN)) { 
        auto stmt = parseStatement();   
        if (stmt != nullptr) {
            block->Statements.push_back(std::move(stmt));
        }
        nextToken();
    }

    if (curTokenIs(END)) { // Advance tokens by two to account for positioning. Adjust later if needed.
        nextToken();
        nextToken();
    }
    return block;
}

std::vector<std::unique_ptr<TypedDeclStatement>> Parser::parseFunctionParameters() {
    Tracer tracer("parseFunctionParameters");

    std::vector<std::unique_ptr<TypedDeclStatement>> typedDeclStmts;

    if (peekTokenIs(RPAREN)) {
        nextToken(); 
        return typedDeclStmts;
    }

    do {
        nextToken(); // Move to the first parameter or next parameter after ','

        // Assuming parseTypedDeclStatement parses a parameter declaration and returns a unique_ptr<TypedDeclStatement>
        auto typedDeclStmt = parseTypedDeclStatement();
        if (typedDeclStmt == nullptr) {
            std::cerr << "Expected a typed declaration statement." << std::endl;
            return {}; // Return an empty vector to indicate failure
        }

        typedDeclStmts.push_back(std::move(typedDeclStmt));

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
        std::cerr << "Cant find " << curToken.Type << std::endl;
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
    //// Flag guards
    //if (!isFirstIfExpression) {
    //    // Skip parsing this IF expression if it's done parsing the original expression.
    //    std::cerr << "Skipping IF expression as it's not the first." << std::endl;
    //    return nullptr;
    //}

    //if (!isFirstElseExpression) {
    //    // Skip parsing this IF expression if it's done parsing the original expression.
    //    std::cerr << "Skipping ELSE expression as it's not the first." << std::endl;
    //    return nullptr;
    //}

    //isFirstIfExpression = false; // Set flag to false so that it skips other IFs

    auto token = curToken; 
    auto expression = std::make_unique<IfExpression>(token);

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

    if (!expectPeek(IF)) {
        std::cerr << expression->TokenLiteral() << std::endl;
        std::cerr << "Failed to find IF, peekToken: " << peekToken.Type << std::endl;
        return nullptr;
    }

    expression->Consequence = parseBlockStatement();

    // Check for an 'else' part
    if (peekTokenIs(ELSE)) {

        nextToken();

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

        if (!expectPeek(ELSE)) {
            std::cerr << expression->TokenLiteral() << std::endl;
            std::cerr << "Failed to find ELSE, peekToken: " << peekToken.Type << std::endl;
            return nullptr;
        }

        expression->Alternative = parseBlockStatement();
    }

    //// Set flags to true as it's done parsing the entire control flow.
    //isFirstIfExpression = true; 
    //isFirstElseExpression = true;

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
    nextToken();
    auto assignExp = std::make_unique<AssignExpression>(std::move(name));
    assignExp->Expression_ = parseExpression(Precedence::LOWEST);
    if (peekTokenIs(NEWLINE, EOF_TOKEN)) {
        nextToken();
    }
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

// For expecting multiple peekToken types at once
template<typename... TokenTypes> 
bool Parser::expectPeek(TokenTypes... types) { 
    if (peekTokenIs(types...)) {
        nextToken();
        return true;
    }
    else {
        std::string expectedTypes = "";
        peekError(expectedTypes);
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

void Parser::peekError(const TokenType& expected) { // Handle custom errors here
    std::string msg = "Expected next token to be " + std::string(expected) +
        ", got " + std::string(peekToken.Type) + " instead.";
    errors.push_back(msg);
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
