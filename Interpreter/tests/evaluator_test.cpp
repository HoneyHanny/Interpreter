#include "evaluator_test.h"


static std::unique_ptr<Object> testEval(const std::string& input) {
    auto lexer = std::make_unique<Lexer>(input);
    Parser parser(std::move(lexer));
    
    auto program = parser.ParseProgram();

    return Eval(program.get());
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
    END IF)", 10},
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
