#include "parser_test.h"



static void assertEqual(const std::string& actual, const std::string& expected, const std::string& message) {
    if (actual != expected) {
        std::cerr << "Assertion failed: " << message << "\n"
            << "Expected: " << expected << ", Actual: " << actual << std::endl;
        std::exit(EXIT_FAILURE); // Exit the program with a failure status
    }
}

static void checkParserErrors(const Parser& parser) {
    const auto& errors = parser.Errors();
    if (errors.empty()) {
        return;
    }

    std::cerr << "CODE - " << errors.size() << " error/s occured:" << std::endl;
    for (const auto& msg : errors) {
        std::cerr << "Error: " << msg << std::endl;
    }

    std::exit(EXIT_FAILURE);
}

bool testTypedDeclStatement(Statement* s, const std::string& name) {
    std::string tokenLiteral = s->TokenLiteral();
    if (tokenLiteral != "INT" && tokenLiteral != "FLOAT" && tokenLiteral != "BOOL" && tokenLiteral != "CHAR" && tokenLiteral != "STRING") {
        std::cerr << "s.TokenLiteral not one of 'INT', 'FLOAT', 'BOOL', 'CHAR', 'STRING'. got=" << tokenLiteral << std::endl;
        return false;
    }

    TypedDeclStatement* typedDeclStmt = dynamic_cast<TypedDeclStatement*>(s);
    if (typedDeclStmt == nullptr) {
        std::cerr << "s not TypedStatement. got=" << typeid(*s).name() << std::endl;
        return false;
    }

    assertEqual(typedDeclStmt->Name->Value, name, "typedDeclStmt.Name.Value not '" + name + "'.");
    assertEqual(typedDeclStmt->Name->TokenLiteral(), name, "typedDeclStmt.Name.TokenLiteral not '" + name + "'.");

    return true;
}

void TestTypedDeclStatements() {
    std::string input = R"(
INT x = 5
FLOAT y = 10.101
CHAR foobar = 'a'
INT test
STRING str = "string"
)";
        

    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    checkParserErrors(parser);

    std::cout << program->Statements.size() << std::endl;
    if (program->Statements.size() != 5) {
        std::cerr << "program.Statements does not contain 5 statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<std::string> tests = {"x", "y", "foobar", "test", "str"};

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Typed Statement: " << program->Statements[i].get()->String() << std::endl;
        if (!testTypedDeclStatement(program->Statements[i].get(), tests[i])) {
            std::exit(EXIT_FAILURE);
        }
    }

    
    std::cout << "All tests passed!" << std::endl;
    
}

void TestTypedDeclStatementsV2_1() {
    std::string input = R"(
INT test
test = 10
)";

    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    std::cout << program->Statements.size() << std::endl;
    if (program->Statements.size() != 2) {
        std::cerr << "program.Statements does not contain 2 statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<TypedDeclStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "program.Statements[0] is not TypedDeclStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt1 = dynamic_cast<ExpressionStatement*>(program->Statements[1].get());
    if (!stmt1) {
        std::cerr << "program.Statements[1] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    std::cout << stmt1->token.Type << " | " << stmt1->token.Literal << std::endl;
    std::cout << stmt1->Expression_->String() << " | " << stmt1->token.Literal << std::endl;

    auto stmt2 = dynamic_cast<AssignExpression*>(stmt1->Expression_.get());
    if (!stmt2) {
        std::cerr << "stmt1->Expression_ is not AssignExpression." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt3 = dynamic_cast<Identifier*>(stmt2->name.get());
    if (!stmt3) {
        std::cerr << "stmt2->name is not Identifier." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt4 = dynamic_cast<IntegerLiteral*>(stmt2->Expression_.get());
    if (!stmt4) {
        std::cerr << "stmt2->Expression_ is not IntegerLiteral. Got: " << stmt2->Expression_->String() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "All tests passed!" << std::endl;

}

bool testReturnStatement(Statement* s, const std::string& name) {
    std::cerr << s->String() << std::endl;
    std::string tokenLiteral = s->TokenLiteral();
    if (tokenLiteral != "RETURN") {
        std::cerr << "s.TokenLiteral not RETURN'. got=" << tokenLiteral << std::endl;
        return false;
    }

    ReturnStatement* returnStmt = dynamic_cast<ReturnStatement*>(s);
    if (returnStmt == nullptr) {
        std::cerr << "s not ReturnStatement. got=" << typeid(*s).name() << std::endl;
        return false;
    }

    return true;
}

void TestReturnStatements() {
    std::string input = R"(
RETURN 5
RETURN 10
RETURN 99999
)";


    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 3) {
        std::cerr << "program.Statements does not contain 3 statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<std::string> tests = { "x", "y", "foobar" };

    for (size_t i = 0; i < tests.size(); ++i) {
        if (!testReturnStatement(program->Statements[i].get(), tests[i])) {
            std::exit(EXIT_FAILURE);
        }
    }


    std::cout << "All tests passed!" << std::endl;

}

void TestIdentifierExpression() {
    std::string input = "foobar";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser); 

    if (program->Statements.size() != 1) {
        std::cerr << "program has not enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "program.Statements[0] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto ident = dynamic_cast<Identifier*>(stmt->Expression_.get());
    if (!ident) {
        std::cerr << "exp not Identifier." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (ident->Value != "foobar") {
        std::cerr << "ident.Value not 'foobar'. got=" << ident->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (ident->TokenLiteral() != "foobar") {
        std::cerr << "ident.TokenLiteral not 'foobar'. got=" << ident->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestIdentifierExpression passed." << std::endl;
}

void TestIntegerLiteralExpression() {
    std::string input = "5";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 1) {
        std::cerr << "TestIntegerLiteralExpression failed: program has not enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "TestIntegerLiteralExpression failed: program.Statements[0] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto literal = dynamic_cast<IntegerLiteral*>(stmt->Expression_.get());
    if (!literal) {
        std::cerr << "TestIntegerLiteralExpression failed: exp not IntegerLiteral." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->Value != 5) {
        std::cerr << "TestIntegerLiteralExpression failed: literal.Value not 5. got=" << literal->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->TokenLiteral() != "5") {
        std::cerr << "TestIntegerLiteralExpression failed: literal.TokenLiteral not '5'. got=" << literal->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestIntegerLiteralExpression passed." << std::endl;
}

bool testIntegerLiteral(const std::unique_ptr<Expression>& nl, int64_t value) {
    auto integ = dynamic_cast<IntegerLiteral*>(nl.get());
    if (!integ) {
        std::cerr << "nl not ast::IntegerLiteral." << std::endl;
        return false;
    }

    if (integ->Value != value) {
        std::cerr << "integ.Value not " << value << ". got=" << integ->Value << std::endl;
        return false;
    }

    if (integ->TokenLiteral() != std::to_string(value)) {
        std::cerr << "integ.TokenLiteral not " << value << ". got=" << integ->TokenLiteral() << std::endl;
        return false;
    }

    return true;
}

void TestParsingPrefixExpressions() {
    struct PrefixTest {
        std::string input;
        std::string op;
        int64_t integerValue;
    };

    std::vector<PrefixTest> prefixTests = {
        {"!5", "!", 5},
        {"-15", "-", 15},
    };

    for (const auto& tt : prefixTests) {
        auto lexer = std::make_unique<Lexer>(tt.input);
        Parser parser(std::move(lexer));
        auto program = parser.ParseProgram();
        checkParserErrors(parser);

        if (program->Statements.size() != 1) {
            std::cerr << "program.Statements does not contain 1 statements. got=" << program->Statements.size() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
        if (!stmt) {
            std::cerr << "program.Statements[0] is not ExpressionStatement." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        auto exp = dynamic_cast<PrefixExpression*>(stmt->Expression_.get());
        if (!exp) {
            std::cerr << "stmt is not PrefixExpression." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (exp->Operator != tt.op) {
            std::cerr << "exp.Operator is not '" << tt.op << "'. got=" << exp->Operator << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (!testIntegerLiteral(exp->Right, tt.integerValue)) {
            std::exit(EXIT_FAILURE);
        }
    }

    std::cout << "TestParsingPrefixExpressions passed." << std::endl;
}

void TestParsingInfixExpressions() {
    struct InfixTest {
        std::string input;
        const LiteralTypeValues leftValue;
        std::string op;
        const LiteralTypeValues rightValue;
    };

    std::vector<InfixTest> infixTests = {
        {"5 + 5\n", 5, "+", 5}, 
        {"5 - 5\n", 5, "-", 5}, 
        {"5 * 5\n", 5, "*", 5}, 
        {"5 / 5\n", 5, "/", 5}, 
        {"5 > 5\n", 5, ">", 5}, 
        {"5 < 5\n", 5, "<", 5}, 
        {"5 == 5\n", 5, "==", 5}, 
        {"5 <> 5\n", 5, "<>", 5},
        {"TRUE == FALSE", true, "==", false}, 
        {"TRUE <> FALSE", true, "<>", false}, 
        {"FALSE == FALSE", false, "==", false},
    };

    for (const auto& tt : infixTests) {
        auto lexer = std::make_unique<Lexer>(tt.input);
        Parser parser(std::move(lexer));
        auto program = parser.ParseProgram();
        checkParserErrors(parser);

        if (program->Statements.size() != 1) {
            std::cerr << "TestParsingInfixExpressions failed: program.Statements does not contain 1 statements. got=" << program->Statements.size() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
        if (!stmt) {
            std::cerr << "TestParsingInfixExpressions failed: program.Statements[0] is not ExpressionStatement." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        auto exp = dynamic_cast<InfixExpression*>(stmt->Expression_.get());
        if (!exp) {
            std::cerr << "TestParsingInfixExpressions failed: exp is not InfixExpression." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (!testLiteralExpression(exp->Left, tt.leftValue)) {
            std::cerr << "Left: " << std::endl;
            continue;
        }

        if (exp->Operator != tt.op) {
            std::cerr << "TestParsingInfixExpressions failed: exp.Operator is not '" << tt.op << "'. got=" << exp->Operator << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (!testLiteralExpression(exp->Right, tt.rightValue)) {
            std::cerr << "Right" << std::endl;
            continue;
        }

        /*if (!testIntegerLiteral(exp->Left, tt.leftValue)) {
            continue; 
        }

        

        if (!testIntegerLiteral(exp->Right, tt.rightValue)) {
            continue; 
        }*/
    }

    std::cout << "TestParsingInfixExpressions passed." << std::endl;
}

void TestOperatorPrecedenceParsing() {
    struct TestCase {
        std::string input;
        std::string expected;
    };

    std::vector<TestCase> tests = {
        {"-a * b", "((-a) * b)"},
        {"!-a", "(!(-a))"},
        {"a + b + c", "((a + b) + c)"},
        {"a + b - c", "((a + b) - c)"},
        {"a * b * c", "((a * b) * c)"},
        {"a * b / c", "((a * b) / c)"},
        {"a + b / c", "(a + (b / c))"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
        {"3 + 4\n -5 * 5", "(3 + 4)((-5) * 5)"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
        {"5 < 4 <> 3 > 4", "((5 < 4) <> (3 > 4))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        { "TRUE", "TRUE", }, 
        { "FALSE", "FALSE", }, 
        { "3 > 5 == FALSE", "((3 > 5) == FALSE)", }, 
        { "3 < 5 == FALSE", "((3 < 5) == FALSE)", },
        { "1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)", },
        { "(5 + 5) * 2", "((5 + 5) * 2)", }, 
        { "2 / (5 + 5)", "(2 / (5 + 5))", }, 
        { "-(5 + 5)", "(-(5 + 5))", }, 
        { "!(TRUE == TRUE)", "(!(TRUE == TRUE))", },
    };

    for (const auto& test : tests) {
        auto lexer = std::make_unique<Lexer>(test.input);
        Parser parser(std::move(lexer));
        auto program = parser.ParseProgram();
        checkParserErrors(parser);

        auto actual = program->String(); 
        if (actual != test.expected) {
            std::cerr << "TestOperatorPrecedenceParsing failed for input: " << test.input << "\n"
                << "expected=" << test.expected << ", got=" << actual << std::endl;
        }
        else {
            std::cout << "Test passed for: " << test.input << std::endl;
        }
    }
}

void TestBooleanExpression() {
    std::string input = "TRUE";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 1) {
        std::cerr << "program has not enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "program.Statements[0] is not Boolean." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto b = dynamic_cast<Boolean*>(stmt->Expression_.get());
    if (!b) {
        std::cerr << "exp not boolean." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (b->Value != true) {
        std::cerr << "ident.value not 'foobar'. got=" << b->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (b->TokenLiteral() != "TRUE") {
        std::cerr << "ident.tokenliteral not 'foobar'. got=" << b->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "testBoolean passed." << std::endl;
}

void TestIfExpression() {
    std::string input = R"(
    IF(x < y) 
        BEGIN IF 
            x 
        END IF)";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    auto program = parser.ParseProgram();
    checkParserErrors(parser); // Implement this based on your error handling

    if (program->Statements.size() != 1) {
        std::cerr << "Program does not contain 1 statement. Got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "Program's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << stmt->Expression_.get()->String() << std::endl;

    auto exp = dynamic_cast<IfExpression*>(stmt->Expression_.get());
    if (!exp) {
        std::cerr << "Statement's expression is not IfExpression." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!testInfixExpression(exp->Condition, "x", "<", "y")) {
        std::cerr << "Condition test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (exp->Consequence->Statements.size() != 1) {
        std::cerr << "Consequence does not contain 1 statement. Got=" << exp->Consequence->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto consequence = dynamic_cast<ExpressionStatement*>(exp->Consequence->Statements[0].get());
    if (!consequence) {
        std::cerr << "Consequence's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!testIdentifier(consequence->Expression_, "x")) {
        std::cerr << "Identifier test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (exp->Alternative != nullptr) {
        std::cerr << "Expected no alternative, but one was found." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestIfStatement passed." << std::endl;
}


void TestNestedIfExpression() {
    std::string input = R"(
    IF(x < y) 
        BEGIN IF 
            IF (x) 
                BEGIN IF 
                    x 
                END IF
        END IF
    )";

    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    auto program = parser.ParseProgram();
    checkParserErrors(parser); // Assuming this function checks for and reports any parsing errors

    // Verifying that there is exactly one statement in the program
    if (program->Statements.size() != 1) {
        std::cerr << "Program does not contain 1 statement. Got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\nExpressionStatement: \n" << program->Statements[0].get()->String() << std::endl;

    // Verifying that the statement is an IfExpression
    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "Program's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\nIF: \n" << stmt->Expression_.get()->String() << std::endl;

    auto exp = dynamic_cast<IfExpression*>(stmt->Expression_.get());
    if (!exp) {
        std::cerr << "Statement's expression is not IfExpression." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Verifying the condition of the outer IF expression
    if (!testInfixExpression(exp->Condition, "x", "<", "y")) {
        std::cerr << "Outer condition test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Verifying the structure of the nested IF expression
    if (exp->Consequence->Statements.size() != 1) {
        std::cerr << "Consequence does not contain 1 statement for the nested IF. Got=" << exp->Consequence->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\nNested IF: \n" << exp->Consequence->Statements[0].get()->String() << std::endl;

    auto stmtInner = dynamic_cast<ExpressionStatement*>(exp->Consequence->Statements[0].get());
    if (!stmtInner) {
        std::cerr << "Consequence's statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto nestedStmt = dynamic_cast<IfExpression*>(stmtInner->Expression_.get());
    if (!nestedStmt) {
        std::cerr << "Consequence's statement is not an IfExpression for nested IF." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\nInner Condition: \n" << nestedStmt->Condition->String() << std::endl;

    // Assuming testIdentifier verifies the condition of the nested IF and its consequence
    if (!testIdentifier(nestedStmt->Condition, "x")) {
        std::cerr << "Nested IF condition test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (nestedStmt->Consequence->Statements.size() != 1) {
        std::cerr << "Nested IF's consequence does not contain 1 statement. Got=" << nestedStmt->Consequence->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto nestedConsequence = dynamic_cast<ExpressionStatement*>(nestedStmt->Consequence->Statements[0].get());
    if (!nestedConsequence || !testIdentifier(nestedConsequence->Expression_, "x")) {
        std::cerr << "Nested IF's consequence first statement is not ExpressionStatement or identifier test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Verifying there is no alternative block in the outer IF
    if (exp->Alternative != nullptr) {
        std::cerr << "Expected no alternative for the outer IF, but one was found." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestNestedIfExpression passed." << std::endl;
}

void TestNestedIfInElseExpression() {
    std::string input = R"(
    IF(x < y) 
        BEGIN IF 
            x 
        END IF
    ELSE 
        BEGIN ELSE
            IF (x) 
                BEGIN IF 
                    x
                END IF
        END ELSE
    )";

    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    auto program = parser.ParseProgram();
    checkParserErrors(parser); // Assuming this function checks for and reports any parsing errors

    // Verifying that there is exactly one statement in the program
    if (program->Statements.size() != 1) {
        std::cerr << "Program does not contain 1 statement. Got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Verifying that the statement is an IfExpression
    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "Program's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto exp = dynamic_cast<IfExpression*>(stmt->Expression_.get());
    if (!exp) {
        std::cerr << "Statement's expression is not IfExpression." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Verifying the condition of the outer IF expression
    if (!testInfixExpression(exp->Condition, "x", "<", "y")) {
        std::cerr << "Outer condition test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Verifying the structure of the nested IF expression within the ELSE block
    if (!exp->Alternative) {
        std::cerr << "Expected an alternative block, but none was found." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "\nNested IF: \n" << exp->Alternative->Statements[0].get()->String() << std::endl;

    auto stmtInner = dynamic_cast<ExpressionStatement*>(exp->Alternative->Statements[0].get());
    if (!stmtInner) {
        std::cerr << "Consequence's statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto elseStmt = dynamic_cast<IfExpression*>(stmtInner->Expression_.get());
    if (!elseStmt) {
        std::cerr << "Alternative block is not an IfExpression." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (elseStmt->Consequence->Statements.size() != 1) {
        std::cerr << "Nested IF in ELSE's consequence does not contain 1 statement. Got=" << elseStmt->Consequence->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto nestedConsequence = dynamic_cast<ExpressionStatement*>(elseStmt->Consequence->Statements[0].get());
    if (!nestedConsequence || !testIdentifier(nestedConsequence->Expression_, "x")) {
        std::cerr << "Nested IF in ELSE's consequence first statement is not ExpressionStatement or identifier test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestNestedIfInElseExpression passed." << std::endl;
}


void TestIfElseExpression() {
    std::string input = R"(
    IF(x < y) 
        BEGIN IF 
        x 
        END IF
    ELSE
        BEGIN ELSE
        some_var
        END ELSE)";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    auto program = parser.ParseProgram();
    checkParserErrors(parser); // Implement this based on your error handling

    if (program->Statements.size() != 1) {
        std::cerr << "Program does not contain 1 statement. Got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "Program's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto exp = dynamic_cast<IfExpression*>(stmt->Expression_.get());
    if (!exp) {
        std::cerr << "Statement's expression is not IfExpression." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!testInfixExpression(exp->Condition, "x", "<", "y")) {
        std::cerr << "Condition test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (exp->Consequence->Statements.size() != 1) {
        std::cerr << "Consequence does not contain 1 statement. Got=" << exp->Consequence->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto consequence = dynamic_cast<ExpressionStatement*>(exp->Consequence->Statements[0].get());
    if (!consequence) {
        std::cerr << "Consequence's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!testIdentifier(consequence->Expression_, "x")) {
        std::cerr << "Identifier test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (exp->Alternative == nullptr) {
        std::cerr << "Expected an alternative, but none was found." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto alternative = dynamic_cast<ExpressionStatement*>(exp->Alternative->Statements[0].get());
    if (!alternative) {
        std::cerr << "Alternative's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (!testIdentifier(alternative->Expression_, "some_var")) {
        std::cerr << "Identifier test failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestIfElseStatement passed." << std::endl;
}

bool testIdentifier(const std::unique_ptr<Expression>& exp, const std::string& value) {
    auto ident = dynamic_cast<Identifier*>(exp.get());
    if (!ident) {
        std::cerr << "Expression not Identifier. got=" << typeid(*exp).name() << std::endl;
        return false;
    }

    if (ident->Value != value) {
        std::cerr << "ident.Value not " << value << ". got=" << ident->Value << std::endl;
        return false;
    }

    if (ident->TokenLiteral() != value) {
        std::cerr << "ident.TokenLiteral not " << value << ". got=" << ident->TokenLiteral() << std::endl;
        return false;
    }

    return true;
}

bool testLiteralExpression(const std::unique_ptr<Expression>& exp, const LiteralTypeValues& expected) {
    if (std::holds_alternative<int64_t>(expected)) {
        return testIntegerLiteral(exp, std::get<int64_t>(expected));
    }
    else if (std::holds_alternative<std::string>(expected)) {
        return testIdentifier(exp, std::get<std::string>(expected));
    }
    else if (std::holds_alternative<bool>(expected)) {
        return testBooleanLiteral(exp, std::get<bool>(expected));
    }
    else {
        std::cerr << "Type of expected not handled." << std::endl;
        return false;
    }
}

bool testInfixExpression(const std::unique_ptr<Expression>& exp, const LiteralTypeValues& left, const std::string& operator_, const LiteralTypeValues& right) {
    auto opExp = dynamic_cast<InfixExpression*>(exp.get());
    if (!opExp) {
        std::cerr << "Expression is not InfixExpression. got=" << typeid(*exp).name() << std::endl;
        return false;
    }

    if (!testLiteralExpression(opExp->Left, left)) {
        return false;
    }

    if (opExp->Operator != operator_) {
        std::cerr << "Operator is not '" << operator_ << "'. got=" << opExp->Operator << std::endl;
        return false;
    }

    if (!testLiteralExpression(opExp->Right, right)) {
        return false;
    }

    return true;
}

bool testBooleanLiteral(const std::unique_ptr<Expression>& exp, bool value) {
    auto bo = dynamic_cast<Boolean*>(exp.get());
    if (!bo) {
        std::cerr << "Expression is not ast::Boolean. got=" << typeid(*exp).name() << std::endl;
        return false;
    }

    if (bo->Value != value) {
        std::cerr << "Boolean value not " << std::boolalpha << value << ". got=" << std::boolalpha << bo->Value << std::endl;
        return false;
    }

    std::string valueStr = value ? "TRUE" : "FALSE";
    if (bo->TokenLiteral() != valueStr) {
        std::cerr << "TokenLiteral not " << valueStr << ". got=" << bo->TokenLiteral() << std::endl;
        return false;
    }

    return true;
}

void TestFunctionLiteralParsing() {
    std::string input = R"(
FUNCTION foo(INT x, INT y) INT:
BEGIN FUNCTION
x + y
END FUNCTION)";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    
    auto program = parser.ParseProgram();
    checkParserErrors(parser); // Assume this function prints errors and stops execution if any

    if (program->Statements.size() != 1) {
        std::cerr << "Program does not contain 1 statement. Got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "Program's first statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto function = dynamic_cast<FunctionLiteral*>(stmt->Expression_.get());
    if (!function) {
        std::cerr << "Statement's expression is not FunctionLiteral." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (function->Parameters.size() != 2) {
        std::cerr << "Function literal parameters wrong. Want 2, got=" << function->Parameters.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (function->Body->Statements.size() != 1) {
        std::cerr << "Function body does not contain 1 statement. Got=" << function->Body->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto bodyStmt = dynamic_cast<ExpressionStatement*>(function->Body->Statements[0].get());
    if (!bodyStmt) {
        std::cerr << "Function body's statement is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    testInfixExpression(bodyStmt->Expression_, "x", "+", "y");

    std::cout << "TestFunctionLiteralParsing passed." << std::endl;
}

void TestFunctionParameterParsing() {
    struct Test {
        std::string input;
        std::vector<std::string> expectedParams;
    };

    std::vector<Test> tests = {
        {R"(
FUNCTION foo() VOID:
BEGIN FUNCTION
    
END FUNCTION
)", {}},
        {R"(
FUNCTION foo(INT x) VOID: 
BEGIN FUNCTION

END FUNCTION
)", {"INT x"}},
        {R"(
FUNCTION foo(INT x, INT y, INT z) VOID: 
BEGIN FUNCTION

END FUNCTION
)", {"INT x", "INT y", "INT z"}},
    };

    for (const auto& tt : tests) {
        auto lexer = std::make_unique<Lexer>(tt.input);
        Parser parser(std::move(lexer));
        auto program = parser.ParseProgram();
        checkParserErrors(parser);

        auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements.at(0).get());
        if (!stmt) {
            continue; 
            std::cerr << "Statement is not an ExpressionStatement.\n"; 
        }

        auto function = dynamic_cast<FunctionLiteral*>(stmt->Expression_.get());
        if (!function) {
            std::cerr << "Expression is not a FunctionLiteral.\n";
            continue;
        }

        if (function->Parameters.size() != tt.expectedParams.size()) {
            std::cerr << "Length of parameters wrong. Wanted " << tt.expectedParams.size() << ", got " << function->Parameters.size() << ".\n";
            continue;
        }

        for (size_t i = 0; i < tt.expectedParams.size(); ++i) {
            if (function->Parameters[i]->TokenLiteral() + " " + function->Parameters[i]->Name->Value != tt.expectedParams[i]) {
                std::cerr << "Parameter mismatch. Wanted " << tt.expectedParams[i] << ", got " << function->Parameters[i]->TokenLiteral() + " " + function->Parameters[i]->Name->Value << ".\n";
            }
        }
    }

    std::cout << "TestFunctionParameterParsing passed." << std::endl;
}

void TestCallExpressionParsing() {
    std::string input = "add: 1, 2 * 3, 4 + 5";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    auto program = parser.ParseProgram();
    checkParserErrors(parser); // Assume this function prints errors and stops execution if any

    if (program->Statements.size() != 1) {
        std::cerr << "Program does not contain 1 statement. Got=" << program->Statements.size() << "\n";
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements.at(0).get());
    if (!stmt) {
        std::cerr << "Statement is not an ExpressionStatement.\n";
        std::exit(EXIT_FAILURE);
    }

    auto exp = dynamic_cast<CallExpression*>(stmt->Expression_.get());
    if (!exp) {
        std::cerr << "Expression is not a CallExpression.\n";
        std::exit(EXIT_FAILURE);
    }

    if (!testIdentifier(exp->Function, "add")) {
        std::cerr << "Function identifier is not 'add'.\n";
        return; // Assuming testIdentifier prints its own error message
    }

    if (exp->Arguments.size() != 3) {
        std::cerr << "Wrong length of arguments. Got=" << exp->Arguments.size() << "\n";
        std::exit(EXIT_FAILURE);
    }

    testLiteralExpression(exp->Arguments[0], 1);
    testInfixExpression(exp->Arguments[1], 2, "*", 3);
    testInfixExpression(exp->Arguments[2], 4, "+", 5);

    std::cout << "TestCallExpressionParsing passed." << std::endl;
}

void TestStringLiteralExpression() {
    std::string input = R"("hello world")";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 1) {
        std::cerr << "TestStringLiteralExpression failed: program has not enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "TestStringLiteralExpression failed: program.Statements[0] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto literal = dynamic_cast<StringLiteral*>(stmt->Expression_.get());
    if (!literal) {
        std::cerr << "TestStringLiteralExpression failed: exp not StringLiteral." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->Value != "hello world") {
        std::cerr << "TestStringLiteralExpression failed: literal.Value not 'hello world'. got=" << literal->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->TokenLiteral() != "hello world") {
        std::cerr << "TestStringLiteralExpression failed: literal.TokenLiteral not 'hello world'. got=" << literal->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestStringLiteralExpression passed." << std::endl;
}

void TestCharLiteralExpression() {
    std::string input = R"('a')";
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    if (program->Statements.size() != 1) {
        std::cerr << "TestCharLiteralExpression failed: program does not contain enough statements. got=" << program->Statements.size() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto stmt = dynamic_cast<ExpressionStatement*>(program->Statements[0].get());
    if (!stmt) {
        std::cerr << "TestCharLiteralExpression failed: program.Statements[0] is not ExpressionStatement." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto literal = dynamic_cast<CharLiteral*>(stmt->Expression_.get());
    if (!literal) {
        std::cerr << "TestCharLiteralExpression failed: exp is not CharLiteral." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (literal->Value != 'a') {
        std::cerr << "TestCharLiteralExpression failed: literal.Value is not 'a'. got=" << literal->Value << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Assuming TokenLiteral returns a string representation of the character.
    if (literal->TokenLiteral() != "a") {
        std::cerr << "TestCharLiteralExpression failed: literal.TokenLiteral is not 'a'. got=" << literal->TokenLiteral() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "TestCharLiteralExpression passed." << std::endl;
}


ParseState state = ParseState::BeginCode;

//std::unique_ptr<Expression> extractExpression(const std::unique_ptr<Statement>& stmt) {
//    // Utility function to extract an Expression from a Statement if possible.
//    const auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt.get());
//    return exprStmt ? exprStmt->Expression_->clone() : nullptr;
//}
//
//bool isBeginCodeStatement(const std::unique_ptr<Statement>& stmt) {
//    auto markerStmt = dynamic_cast<MarkerStatement*>(stmt.get());
//    return markerStmt && markerStmt->token.Literal + " " + markerStmt->codeToken.Literal == "BEGIN CODE";
//}
//
//bool isEndCodeStatement(const std::unique_ptr<Statement>& stmt) {
//    auto markerStmt = dynamic_cast<MarkerStatement*>(stmt.get());
//    return markerStmt && markerStmt->token.Literal + " " + markerStmt->codeToken.Literal == "END CODE";
//}
//
//bool isTransitionToVariableDeclarations(const std::unique_ptr<Statement>& stmt) {
//    if (auto typedDeclStmt = dynamic_cast<TypedDeclStatement*>(stmt.get())) {
//        std::cout << "Transitioning to Variable Declarations" << std::endl;
//        std::cout << "Variable Declaration." << std::endl;
//        return true;
//    }
//
//    return false; 
//}
//
//bool isTransitionToExecutableCode(const std::unique_ptr<Statement>& stmt) {
//    auto expr = extractExpression(stmt);
//    if (expr) {
//        std::cout << "Transitioning to Executable Code" << std::endl;
//        std::cout << "Executable Code." << std::endl;
//        return true;
//    }
//
//    return false; 
//}
//
//bool isAllowedInEndCode(const std::unique_ptr<Statement>& stmt) {
//    return false;
//}

void TestASTStructure() {
    std::string input = R"(
    BEGIN CODE
        FUNCTION add(INT x, INT y) INT: 
        BEGIN FUNCTION
            RETURN x + y
        END FUNCTION

        INT x = 5
        CHAR a_1 = 'n'
        BOOL t = TRUE
        STRING st = "foobar"

        a_1 = 'c' 

    
        DISPLAY: x & t & z & a_1 & [#] & "last"
    END CODE
)";

    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));

    auto program = parser.ParseProgram();
    if (program == nullptr) {
        std::cerr << "ParseProgram() returned nullptr" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    checkParserErrors(parser);

    std::cout << "--------------------------------"<< std::endl;
    for (const auto& stmt : program->Statements) {
        std::cout << stmt->String() << std::endl;

        if (state == ParseState::BeginCode) {
            if (!isBeginCodeStatement(stmt)) {
                std::cerr << "Error: Expected BEGIN CODE statement." << std::endl;
                return;
            }
            state = ParseState::FunctionDeclarations;
        }

        else if (state == ParseState::FunctionDeclarations) {
            auto expr = extractExpression(stmt);
            const auto fnlit = dynamic_cast<FunctionLiteral*>(expr.get());
            if (fnlit) {
                std::cout << "Function Declaration." << std::endl;
            }
            else if (isTransitionToVariableDeclarations(stmt->clone())) {
                state = ParseState::VariableDeclarations;
            }
            else if (isTransitionToExecutableCode(stmt)) {
                state = ParseState::ExecutableCode;
            }
            else {
                std::cerr << "Error: Unexpected statement type in Function Declarations state." << std::endl;
                return;
            }
        }

        else if (state == ParseState::VariableDeclarations) {
            if (auto typedDeclStmt = dynamic_cast<TypedDeclStatement*>(stmt.get())) {
                std::cout << "Variable Declaration." << std::endl;
            }
            else if (isTransitionToExecutableCode(stmt)) {
                state = ParseState::ExecutableCode;
            }
            else {
                std::cerr << "Error: Unexpected statement type in Variable Declarations state." << std::endl;
                return;
            }
        }

        else if (state == ParseState::ExecutableCode) {
            auto expr = extractExpression(stmt);
            const auto fnlit = dynamic_cast<FunctionLiteral*>(expr.get());
            if (fnlit) {
                std::cerr << "Error: Unexpected function declaration in Executable Code state." << std::endl;
                return;
            }

            if (auto typedDeclStmt = dynamic_cast<TypedDeclStatement*>(stmt.get())) {
                std::cerr << "Error: Unexpected variable declaration in Executable Code state." << std::endl;
                return;
            }

            if (isEndCodeStatement(stmt)) {
                state = ParseState::EndCode;
            }

            else {
                std::cout << "Executable Code." << std::endl;
                // Process executable code statement
            }
        }

        else if (state == ParseState::EndCode) {
            if (!isAllowedInEndCode(stmt)) {
                std::cerr << "Error: No statements are expected after END CODE." << std::endl;
                return;
            }
        }

        std::cout << "--------------------------------" << std::endl;
    }
}