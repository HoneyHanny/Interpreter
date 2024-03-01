#include "evaluator_test.h"


static std::unique_ptr<Object> testEval(const std::string& input) {
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    
    auto program = parser.ParseProgram();
    auto env = std::make_shared<Environment>();

    return Eval(program.get(), env);
}

static bool testIntegerObject(const std::unique_ptr<Object>& obj, int64_t expected) {
    auto result = dynamic_cast<IntegerObject*>(obj.get());
    if (!result) {
        std::cerr << "object is not Integer. got=" << typeid(*obj).name() << std::endl;
        return false;
    }

    if (result->Value != expected) {
        std::cerr << "object has wrong value. got=" << result->Value << ", want=" << expected << std::endl;
        return false;
    }

    return true;
}

void TestEvalNumericalExpression() {
    struct Test {
        std::string input;
        int64_t expected;
    };

    std::vector<Test> tests = {
        {"5", 5},
        {"10", 10},
        {"-5", -5},
        {"-10", -10},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"-50 + 100 + -50", 0},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"20 + 2 * -10", 0},
        {"50 / 2 * 2 + 10", 60},
        {"2 * (5 + 10)", 30},
        {"3 * 3 * 3 + 10", 37},
        {"3 * (3 * 3) + 10", 37},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };

    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);
        if (!testIntegerObject(evaluated, tt.expected)) {
            std::cerr << "Test failed for input: " << tt.input << std::endl;
        }
        std::cout << "Test passed for input: " << tt.input << std::endl;
    }

    std::cout << "TestEvalNumericalExpression passed." << std::endl;
}

static bool testBooleanObject(const std::unique_ptr<Object>& obj, bool expected) {
    auto result = dynamic_cast<BooleanObject*>(obj.get());
    if (!result) {
        std::cout << "object is not Boolean. got=" << typeid(*obj).name() << std::endl;
        return false;
    }

    if (result->Value != expected) {
        std::cout << "object has wrong value. got=" << std::boolalpha << result->Value << ", want=" << expected << std::endl;
        return false;
    }

    return true;
}

void TestEvalBooleanExpression() {
    struct TestCase {
        std::string input;
        bool expected;
    };

    std::vector<TestCase> tests = {
        {"TRUE", true},
        {"FALSE", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 1", true},
        {"1 <> 1", false},
        {"1 == 2", false},
        {"1 <> 2", true},
        {"TRUE == TRUE", true},
        {"FALSE == FALSE", true},
        {"TRUE == FALSE", false},
        {"TRUE <> FALSE", true},
        {"FALSE <> TRUE", true},
        {"(1 < 2) == TRUE", true},
        {"(1 < 2) == FALSE", false},
        {"(1 > 2) == TRUE", false},
        {"(1 > 2) == FALSE", true},
    };

    for (auto& test : tests) {
        auto evaluated = testEval(test.input);
        if (!testBooleanObject(evaluated, test.expected)) {
            std::cout << "Test failed for input: " << test.input << std::endl;
        }
        else {
            std::cout << "Test passed for input: " << test.input << std::endl;
        }
    }

    std::cout << "TestEvalBooleanExpression passed." << std::endl;
}

void TestBangOperator() {
    struct Test {
        std::string input;
        bool expected;
    };

    std::vector<Test> tests = {
        {"!TRUE", false},
        {"!FALSE", true},
        {"!5", false},
        {"!!TRUE", true},
        {"!!FALSE", false},
        {"!!5", true},
    };

    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);
        if (!testBooleanObject(evaluated, tt.expected)) {
            std::cerr << "Test failed: " << tt.input << " expected " << std::boolalpha << tt.expected << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::cout << "Test passed for input: " << tt.input << std::endl;
    }

    std::cout << "TestBangOperator passed." << std::endl;
}

static bool testNullObject(const std::unique_ptr<Object>& obj) {
    if (dynamic_cast<NullObject*>(obj.get()) == nullptr) {
        std::cerr << "Object is not a NullObject. got = " << typeid(*obj).name() << std::endl;
        return false;
    }
    return true;
}

void TestIfElseExpressions() {
    struct Test {
        std::string input;
        std::optional<int> expected;
    };

    std::vector<Test> tests = {
        {R"(
IF (TRUE) 
    BEGIN IF 
    10
    END IF)", 10},
        {R"(
IF (FALSE) 
    BEGIN IF 
    10
    END IF)", std::nullopt},
        {R"(
IF (1) 
    BEGIN IF 
    10
    END IF)", 10},
        {R"(
IF (1 < 2) 
    BEGIN IF 
    10
    END IF)", 10},
        {R"(
IF (1 > 2) 
    BEGIN IF 
    10
    END IF)", std::nullopt},
        {R"(
IF (1 > 2) 
    BEGIN IF 
    10
    END IF
ELSE
    BEGIN ELSE
    20
    END ELSE)", 20},
        {R"(
IF (1 < 2) 
    BEGIN IF 
    10
    END IF
ELSE
    BEGIN ELSE
    20
    END ELSE)", 10},
    };

    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);
        if (tt.expected.has_value()) {
            if (!testIntegerObject(evaluated, tt.expected.value())) {
                std::cerr << "Test failed for input: " << tt.input << std::endl;
                std::exit(EXIT_FAILURE);
            }
            std::cout << "Test passed for input: " << tt.input << std::endl;
        }
        else {
            if (!testNullObject(evaluated)) {
                std::cerr << "Test failed for input: " << tt.input << std::endl;
            }
        }
    }

    std::cout << "TestIfElseExpressions passed." << std::endl;
}

void TestReturnStatementsV2() {
    struct TestCase {
        std::string input;
        int64_t expected;
    };

    std::vector<TestCase> tests = {
        {R"(RETURN 10)", 10},
        {R"(
RETURN 10
9)", 10},
        {R"(
RETURN 2 * 5 
9)", 10},
        {R"(
9 
RETURN 2 * 5
9)", 10},
        {R"(
IF (10 > 1)
    BEGIN IF
    IF (10 > 1)
        BEGIN IF
        RETURN 10
        END IF
    RETURN 1
    END IF)", 10},
    };

    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);
        if (!testIntegerObject(evaluated, tt.expected)) {
            std::cerr << "Test failed for input: " << tt.input
                << ".\nExpected " << tt.expected << std::endl;
            std::exit(EXIT_FAILURE);
        }
        std::cout << "Test passed for input: " << tt.input << std::endl;
    }

    std::cout << "TestReturnStatements passed." << std::endl;
}

std::unique_ptr<Object> testEval(const std::string& input);

void TestErrorHandling() {
    struct TestCase {
        std::string input;
        std::string expectedMessage;
    };

    std::vector<TestCase> tests = {
        {"5 + TRUE", "CODE ERROR - type mismatch: INTEGER + BOOLEAN"},
        {R"(5 + TRUE
            5)", "CODE ERROR - type mismatch: INTEGER + BOOLEAN"},
        {"-TRUE", "CODE ERROR - unknown operator: -BOOLEAN"},
        {"TRUE + FALSE", "CODE ERROR - unknown operator: BOOLEAN + BOOLEAN"},
        {R"(5
            TRUE + FALSE 
            5)", "CODE ERROR - unknown operator: BOOLEAN + BOOLEAN"},
        {R"(IF (10 > 1) 
            BEGIN IF
                TRUE + FALSE
            END IF)", "CODE ERROR - unknown operator: BOOLEAN + BOOLEAN"},
        {
            R"(
                IF (10 > 1) 
                BEGIN IF
                    IF (10 > 1) 
                    BEGIN IF
                        RETURN TRUE + FALSE
                    END IF
                    RETURN 1
                END IF
            )",
            "CODE ERROR - unknown operator: BOOLEAN + BOOLEAN",
        },
        {
            "foobar",
            "CODE ERROR - identifier not found: foobar",
        },
    };

    for (const auto& tt : tests) {
        auto evaluated = testEval(tt.input);

        auto errObj = dynamic_cast<ErrorObject*>(evaluated.get());
        if (!errObj) {
            std::cerr << "no error object returned. got=" << typeid(*evaluated).name() << std::endl;
            std::exit(EXIT_FAILURE);
        }

        if (errObj->Inspect() != tt.expectedMessage) {
            std::cerr << "wrong error message. expected=\"" << tt.expectedMessage << "\", got=\"" << errObj->Inspect() << "\"" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::cout << "Test passed for input: " << tt.input << std::endl;
    }

    std::cout << "TestErrorHandling passed." << std::endl;
}


void TestTypedDeclStatementsV2() {
    struct TestCase {
        std::string input;
        int64_t expected;
    };

    std::vector<TestCase> tests = {
        {R"(INT a = 5 
            a)", 5},
        {R"(INT a = 5 * 5
            a)", 25},
        {R"(let a = 5
            let b = a
            b)", 5},
        {R"(INT a = 5
            INT b = a
            INT c = a + b + 5 
            c)", 15},
    };

    for (const auto& test : tests) {
        auto evaluated = testEval(test.input);
        if (!testIntegerObject(evaluated, test.expected)) {
            std::cerr << "Test failed for input: " << test.input << std::endl;
            std::exit(EXIT_FAILURE);
        }
        else {
            std::cout << "Test passed for input: " << test.input << std::endl;
        }
    }
}

void TestFunctionObject() {
    std::string input = R"(FUNCTION foo(INT x) INT: 
                            BEGIN FUNCTION
                            x + 2
                            END FUNCTION)";

    auto evaluated = testEval(input); // Assuming testEval returns std::unique_ptr<Object>
    Function* fn = dynamic_cast<Function*>(evaluated.get());

    if (!fn) {
        std::cerr << "TestFunctionObject failed: object is not Function." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (fn->Parameters.size() != 1) {
        std::cerr << "TestFunctionObject failed: function has wrong number of parameters. Expected 1, got "
            << fn->Parameters.size() << "." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    if (fn->Parameters[0]->String() != "INT x") {
        std::cerr << "TestFunctionObject failed: parameter is not 'x'. Got "
            << fn->Parameters[0]->String() << "." << std::endl;
        return;
    }

    std::string expectedBody = "(x + 2)";

    // Assuming BlockStatement has an Inspect method that serializes the block's content.
    if (fn->Body->String() != expectedBody) {
        std::cerr << "TestFunctionObject failed: body does not match expected. Expected "
            << expectedBody << ", got " << fn->Body->String() << "." << std::endl;
        return;
    }

    std::cout << "TestFunctionObject passed." << std::endl;
}

void TestFunctionApplication() {
    struct Test {
        std::string input;
        int64_t expected;
    };

    std::vector<Test> tests = {
        {R"(FUNCTION identity(INT x) INT: 
            BEGIN FUNCTION 
            x
            END FUNCTION 
            
            identity: 5
        )", 5},
        {R"(FUNCTION identity(INT x) INT: 
            BEGIN FUNCTION 
            RETURN x
            END FUNCTION 
            
            identity: 5
        )", 5},
        {R"(FUNCTION identity(INT x) INT: 
            BEGIN FUNCTION 
            x
            END FUNCTION 
            
            identity: 5
        )", 5},
        {R"(FUNCTION double(INT x) INT: 
            BEGIN FUNCTION 
            RETURN x * 2
            END FUNCTION 
            
            double: 5
        )", 10},
        {R"(FUNCTION add(INT x, INT y) INT: 
            BEGIN FUNCTION 
            RETURN x + y
            END FUNCTION 
            
            add: 5, 5
        )", 10},
        {R"(FUNCTION add(INT x, INT y) INT: 
            BEGIN FUNCTION 
            RETURN x + y
            END FUNCTION 
            
            add: 5 + 5, add: 5, 5   
        )", 20},
    };

    for (const auto& test : tests) {
        auto evaluated = testEval(test.input);
        std::cout << "Evaluated: " << evaluated->Inspect() << std::endl;
        if (!testIntegerObject(evaluated, test.expected)) {
            std::cerr << "Test failed for input: " << test.input << std::endl;
            std::exit(EXIT_FAILURE);
        }
        else {
            std::cout << "Test passed for input: " << test.input << std::endl;
        }
    }
}