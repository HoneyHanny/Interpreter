#include "Evaluator.h"

static std::unique_ptr<Object> evalStatements(const std::vector<std::unique_ptr<Statement>>& stmts) {
    std::unique_ptr<Object> result;
    for (const auto& statement : stmts) {
        // Dynamic cast to Node* for Eval
        const Node* node = dynamic_cast<const Node*>(statement.get());
        if (node) {
            result = Eval(node);
        }
        else {
            throw std::runtime_error("Failed to cast Statement to Node");
        }
    }
    return result;
}

static std::unique_ptr<Object> evalBangOperatorExpression(std::unique_ptr<Object> right) {
    if (auto booleanRight = dynamic_cast<BooleanObject*>(right.get())) {
        return std::make_unique<BooleanObject>(!booleanRight->Value);
    }
    else if (dynamic_cast<NullObject*>(right.get())) {
        // NULL is considered falsy, so negating it returns TRUE
        return std::make_unique<BooleanObject>(true);
    }
    // For all other object types, consider them truthy, thus negation returns FALSE
    else {
        return std::make_unique<BooleanObject>(false);
    }
}

static std::unique_ptr<Object> evalMinusPrefixOperatorExpression(std::unique_ptr<Object> right) {
    IntegerObject* integerRight = dynamic_cast<IntegerObject*>(right.get());

    if (!integerRight) {
        return std::make_unique<NullObject>();
    }

    return std::make_unique<IntegerObject>(-integerRight->Value);
}

static std::unique_ptr<Object> evalPrefixExpression(const std::string& operator_, std::unique_ptr<Object> right) {
    if (operator_ == "!") {
        return evalBangOperatorExpression(std::move(right));
    }
    if (operator_ == "-") {
        return evalMinusPrefixOperatorExpression(std::move(right));
    }
    // Default case returns a unique pointer to a new NullObject
    return std::make_unique<NullObject>();
}

static std::unique_ptr<Object> evalIntegerInfixExpression(
    const std::string& operator_,
    std::unique_ptr<Object> left,
    std::unique_ptr<Object> right) {

    auto leftVal = dynamic_cast<IntegerObject*>(left.get())->Value;
    auto rightVal = dynamic_cast<IntegerObject*>(right.get())->Value;

    if (operator_ == "+") {
        return std::make_unique<IntegerObject>(leftVal + rightVal);
    }
    else if (operator_ == "-") {
        return std::make_unique<IntegerObject>(leftVal - rightVal);
    }
    else if (operator_ == "*") {
        return std::make_unique<IntegerObject>(leftVal * rightVal);
    }
    else if (operator_ == "/") {
        if (rightVal == 0) {
            // TODO: Maybe throw an error here
            return std::make_unique<NullObject>(); 
        }
        return std::make_unique<IntegerObject>(leftVal / rightVal);
    }
    else if (operator_ == "<") {
        return std::make_unique<BooleanObject>(leftVal < rightVal);
    }
    else if (operator_ == ">") {
        return std::make_unique<BooleanObject>(leftVal > rightVal);
    }
    else if (operator_ == "==") {
        return std::make_unique<BooleanObject>(leftVal == rightVal);
    }
    else if (operator_ == "<>") {
        return std::make_unique<BooleanObject>(leftVal != rightVal);
    }
    else {
        // TODO: Maybe throw an error here
        // For unsupported operators, return NullObject
        return std::make_unique<NullObject>(); 
    }
}

static std::unique_ptr<Object> evalInfixExpression(
    const std::string& operator_,
    std::unique_ptr<Object> left,
    std::unique_ptr<Object> right) {


    if (left->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) && 
        right->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ)) {
        return evalIntegerInfixExpression(operator_, std::move(left), std::move(right));
    }
    else if (operator_ == "==") {
        auto leftVal = dynamic_cast<BooleanObject*>(left.get())->Value;
        auto rightVal = dynamic_cast<BooleanObject*>(right.get())->Value;
        return std::make_unique<BooleanObject>(leftVal == rightVal);
    }
    else if (operator_ == "<>") {
        auto leftVal = dynamic_cast<BooleanObject*>(left.get())->Value;
        auto rightVal = dynamic_cast<BooleanObject*>(right.get())->Value;
        return std::make_unique<BooleanObject>(leftVal != rightVal);
    }
    // Handle other types and combinations...
    else {
        return std::make_unique<NullObject>();
    }
}

std::unique_ptr<Object> Eval(const Node* node) {
    // Numerical Literal -> Integer Object
    if (auto programNode = dynamic_cast<const Program*>(node)) {
        return evalStatements(programNode->Statements);
    }
    else if (auto exprStmtNode = dynamic_cast<const ExpressionStatement*>(node)) {
        return Eval(exprStmtNode->Expression.get());
    } 
    else if (auto numLit = dynamic_cast<const NumericalLiteral*>(node)) {
        return std::make_unique<IntegerObject>(numLit->Value);
    }
    else if (auto bl = dynamic_cast<const Boolean*>(node)) {
        return std::make_unique<BooleanObject>(bl->Value);
    }
    else if (auto prefixExpr = dynamic_cast<const PrefixExpression*>(node)) {
        auto right = Eval(prefixExpr->Right.get());
        return evalPrefixExpression(prefixExpr->Operator, std::move(right));
    }
    else if (auto infixExpr = dynamic_cast<const InfixExpression*>(node)) {
        auto left = Eval(infixExpr->Left.get());
        auto right = Eval(infixExpr->Right.get());
        return evalInfixExpression(infixExpr->Operator, std::move(left), std::move(right));
    }
    return nullptr;
}