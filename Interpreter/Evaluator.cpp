#include "Evaluator.h"

template<typename T>
static void printArg(std::ostringstream& os, T&& arg) {
    os << std::forward<T>(arg);
}

static void handleArgs(std::ostringstream& os) {
    // Base case: do nothing

}

template<typename T, typename... Args>
static void handleArgs(std::ostringstream& os, T&& firstArg, Args&&... args) {
    printArg(os, std::forward<T>(firstArg));
    if constexpr (sizeof...(args) > 0) {
        os << " "; 
        handleArgs(os, std::forward<Args>(args)...);
    }
}

template<typename... Args>
static void formatMessage(std::ostringstream& message, const std::string& format, Args... args) {
    message << format << "";
    handleArgs(message, std::forward<Args>(args)...);
}

template<typename... Args>
static std::unique_ptr<ErrorObject> newError(const std::string& format, Args... args) {
    std::ostringstream message;
    formatMessage(message, format, std::forward<Args>(args)...);
    return std::make_unique<ErrorObject>(message.str());
}

static bool isError(const std::unique_ptr<Object>& obj) {
    if (obj) {
        return obj->Type() == ObjectTypeToString(ObjectType_::ERROR_OBJ);
    }
    return false;

}
static std::unique_ptr<Object> evalProgram(const std::vector<std::unique_ptr<Statement>>& stmts, const std::shared_ptr<Environment>& env) {
    std::unique_ptr<Object> result;
    for (const auto& statement : stmts) {
        // Dynamic cast to Node* for Eval
        const Node* node = dynamic_cast<const Node*>(statement.get());
        if (node) {
            result = Eval(node, env);

            if (auto returnValue = dynamic_cast<ReturnValue*>(result.get())) {
                return std::unique_ptr<Object>(returnValue->TakeValue());
            }
            else if (auto error = dynamic_cast<ErrorObject*>(result.get())) {
                return result;
            }
        }
        else {
            throw std::runtime_error("Failed to cast Statement to Node");
        }
    }
    return result; // Default return path
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
        return newError("unknown operator: -", right->Type());
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
    return newError("unknown operator: ", operator_, right->Type());
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
    return newError("unknown operator: ", left->Type(), operator_, right->Type());  
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
    else if (left->Type() != right->Type()) {
        return newError("type mismatch: ", left->Type(), operator_, right->Type());
    }
    else {
        return newError("unknown operator: ", left->Type(), operator_, right->Type());
    }
}

static bool isTruthy(const std::unique_ptr<Object>& obj) {
    if (dynamic_cast<NullObject*>(obj.get()) != nullptr) {
        return false;
    }
    else if (auto boolVal = dynamic_cast<BooleanObject*>(obj.get())) {
        if (boolVal->Value == true) {
            return true;
        }
        else if (boolVal->Value == false) {
            return false;
        }
    }
    // For any other object type, consider it truthy
    return true;
}

static std::unique_ptr<Object> evalIfExpression(const IfExpression* ie, const std::shared_ptr<Environment>& env) {
    auto condition = Eval(ie->Condition.get(), env);
    if (isError(condition)) {
        return condition;
    }

    if (isTruthy(condition)) {
        return Eval(ie->Consequence.get(), env);
    }
    else if (ie->Alternative) {
        return Eval(ie->Alternative.get(), env);
    }
    else {
        return std::make_unique<NullObject>();
    }
}

static std::unique_ptr<Object> evalBlockStatement(const BlockStatement* block, const std::shared_ptr<Environment>& env) {
    std::unique_ptr<Object> result = nullptr;

    for (const auto& statement : block->Statements) {
        result = Eval(statement.get(), env);

        if (result != nullptr) {
            std::string rt = result->Type();
            if (rt == ObjectTypeToString(ObjectType_::RETURN_VALUE_OBJ) || rt == ObjectTypeToString(ObjectType_::ERROR_OBJ)) {
                return result;
            }
        }
    }

    return result;
}

static std::unique_ptr<Object> evalIdentifier(const Identifier* node, const std::shared_ptr<Environment>& env) {
    auto val = env->Get(node->Value);
    if (!val) {
        return newError("identifier not found: " + node->Value);
    }

    return val->clone();
}

static std::vector<std::unique_ptr<Object>> evalExpressions(const std::vector<std::unique_ptr<Expression>>& exps, const std::shared_ptr<Environment>& env) {
    std::vector<std::unique_ptr<Object>> results;

    for (const auto& expr : exps) {
        auto evaluated = Eval(expr.get(), env);
        if (isError(evaluated)) {
            // Immediately return a vector containing just this error
            std::vector<std::unique_ptr<Object>> result;
            result.push_back(std::move(evaluated));

            return result;
        }
        results.push_back(std::move(evaluated));
    }

    return results;
}

static std::shared_ptr<Environment> extendFunctionEnv(const Function& fn, const std::vector<std::unique_ptr<Object>>& args) {
    auto extendedEnv = std::make_shared<Environment>(fn.Env);

    for (size_t i = 0; i < fn.Parameters.size(); ++i) {
        extendedEnv->Set(fn.Parameters[i]->Name->Value, args[i]->clone());
    }

    return extendedEnv;
}

static std::unique_ptr<Object> unwrapReturnValue(std::unique_ptr<Object>& obj) {
    if (auto returnValue = dynamic_cast<ReturnValue*>(obj.get())) {
        return returnValue->TakeValue();
    }
    return obj->clone();
}

static std::unique_ptr<Object> applyFunction(std::unique_ptr<Object>& fn, const std::vector<std::unique_ptr<Object>>& args) {
    auto function = dynamic_cast<Function*>(fn.get());
    if (!function) {
        return std::make_unique<ErrorObject>("Not a function: " + fn->Type());
    }

    auto extendedEnv = extendFunctionEnv(*function, args);
    auto evaluated = Eval(function->Body.get(), extendedEnv);
    return unwrapReturnValue(evaluated);
}


std::unique_ptr<Object> Eval(const Node* node, const std::shared_ptr<Environment>& env) {
    // Numerical Literal -> Integer Object
    if (auto programNode = dynamic_cast<const Program*>(node)) {
        return evalProgram(programNode->Statements, env);
    }
    else if (auto exprStmtNode = dynamic_cast<const ExpressionStatement*>(node)) {
        return Eval(exprStmtNode->Expression_.get(), env);
    } 
    else if (auto numLit = dynamic_cast<const NumericalLiteral*>(node)) {
        return std::make_unique<IntegerObject>(numLit->Value);
    }
    else if (auto bl = dynamic_cast<const Boolean*>(node)) {
        return std::make_unique<BooleanObject>(bl->Value);
    }
    else if (auto prefixExpr = dynamic_cast<const PrefixExpression*>(node)) {
        auto right = Eval(prefixExpr->Right.get(), env);
        if (isError(right)) {
            return right;
        }
        return evalPrefixExpression(prefixExpr->Operator, std::move(right));
    }
    else if (auto infixExpr = dynamic_cast<const InfixExpression*>(node)) {
        auto left = Eval(infixExpr->Left.get(), env);
        if (isError(left)) {
            return left;
        }
        auto right = Eval(infixExpr->Right.get(), env);
        if (isError(right)) {
            return right;
        }
        return evalInfixExpression(infixExpr->Operator, std::move(left), std::move(right));
    }
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(node)) {
        return evalBlockStatement(blockStmt, env);
    }
    else if (auto ifExpr = dynamic_cast<const IfExpression*>(node)) {
        return evalIfExpression(ifExpr, env);
    }
    else if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(node)) {
        auto val = Eval(returnStmt->ReturnValue.get(), env);
        if (isError(val)) {
            return val;
        }
        return std::make_unique<ReturnValue>(std::move(val));
    }
    else if (const auto* typedDeclStmt = dynamic_cast<const TypedDeclStatement*>(node)) {
        auto val = Eval(typedDeclStmt->Value.get(), env);
        if (isError(val)) {
            return val;
        }
        env->Set(typedDeclStmt->Name->Value, std::move(val));
    }
    else if (const auto* ident = dynamic_cast<const Identifier*>(node)) {
        return evalIdentifier(ident, env);
    }
    else if (const auto* funcLit = dynamic_cast<const FunctionLiteral*>(node)) {
        std::vector<std::unique_ptr<TypedDeclStatement>> params;
        for (const auto& p : funcLit->Parameters) {
            params.push_back(std::make_unique<TypedDeclStatement>(p->token, std::move(p->Name)));
        }

        auto body = std::make_unique<BlockStatement>(funcLit->Body->token);
        body->Statements = std::move(funcLit->Body->Statements);

        auto fnObject = std::make_unique<Function>(funcLit->type, std::move(funcLit->CallName->clone()), std::move(params), std::move(body), env);
        
        env->Set(fnObject->CallName->TokenLiteral(), fnObject->clone());

        return fnObject;
    }
    else if (const auto* callExpr = dynamic_cast<const CallExpression*>(node)) {
        auto function = Eval(callExpr->Function.get(), env);
        
        //if (isError(function)) {
        //    std::cout << function->Inspect() << std::endl;
        //    return function;
        //}
        
        auto args = evalExpressions(callExpr->Arguments, env);
        if (!args.empty() && isError(args.front())) {
            return std::move(args.front());
        }

        auto result = applyFunction(function, args);

        return result;
    }
    return nullptr;
}