#include "Evaluator.h"

int evaluatorCurrentLine = 0;
static size_t evaluatorCommentIndex;

static void incrementEvaluatorLine() {
    evaluatorCurrentLine++;
    // Continue to skip comment lines without modifying the vector
    while (evaluatorCommentIndex < Lexer::commentLinePositions.size() &&
        evaluatorCurrentLine == Lexer::commentLinePositions[evaluatorCommentIndex]) {
        evaluatorCurrentLine++;
        evaluatorCommentIndex++;  // Move to the next comment line index
    }
}

std::unordered_map<std::string, std::shared_ptr<Builtin>> builtins = {
    {"LEN", std::make_shared<Builtin>(
        [](const std::vector<std::shared_ptr<Object>>& args, std::shared_ptr<Environment> env) -> std::shared_ptr<Object> {
            if (args.size() != 1) {
                return std::make_shared<ErrorObject>("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
            }

            if (auto str = std::dynamic_pointer_cast<String>(args[0])) {
                return std::make_shared<IntegerObject>(static_cast<int64_t>(str->Value.length()));
            } else {
                return std::make_shared<ErrorObject>("argument to `LEN` not supported, got " + args[0]->Type());
            }
        }
    )},
    {"DISPLAY", std::make_shared<Builtin>(
        [](const std::vector<std::shared_ptr<Object>>& args, std::shared_ptr<Environment> env) -> std::shared_ptr<Object> {
            if (args.size() != 1) {
                return std::make_shared<ErrorObject>("wrong number of arguments. got=" + std::to_string(args.size()) + ", want=1");
            }
            // Return 1 for success code
            if (auto str = std::dynamic_pointer_cast<String>(args[0])) {
                std::cout << str->Value << std::endl;
                return std::make_shared<IntegerObject>(1);  
                //return str;
            }
            if (auto num = std::dynamic_pointer_cast<IntegerObject>(args[0])) {
                std::cout << num->Value << std::endl;
                return std::make_shared<IntegerObject>(1);
                //return num;
            }
            if (auto bl = std::dynamic_pointer_cast<BooleanObject>(args[0])) {
                std::string display = bl->Value ? "TRUE" : "FALSE";
                std::cout << display << std::endl;
                return std::make_shared<IntegerObject>(1);
                //return num;
            }
            if (auto c = std::dynamic_pointer_cast<Char>(args[0])) {
                std::cout << c->Value << std::endl;
                return std::make_shared<IntegerObject>(1);
                //return num;
            }
            if (auto f = std::dynamic_pointer_cast<FloatObject>(args[0])) {
                std::cout << f->Value << std::endl;
                return std::make_shared<IntegerObject>(1);
            }
            else {
              return std::make_shared<ErrorObject>("argument to `DISPLAY` not supported, got " + args[0]->Type());
            }
        }
    )},
    {"SCAN", std::make_shared<Builtin>(
        [](const std::vector<std::shared_ptr<Object>>& args, std::shared_ptr<Environment> env) -> std::shared_ptr<Object> {
            
            for (const auto& arg : args) {
                //std::cout << "SCAN INSPECT: " << arg->Inspect() << std::endl;
                std::string varName = env->GetNameByObject(arg);
                //std::cout << "SCAN INSPECT: " << varName << std::endl;
                //std::cout << "SCAN INSPECT: " << env->GetType(varName).Type << std::endl;

                auto varType = env->GetType(varName);
                //std::cout << "Got " << varType.Type << " type." << std::endl;

                if (varType.Type != INT && varType.Type != BOOL && varType.Type != STRING && varType.Type != CHAR) {
                    return std::make_shared<ErrorObject>("Unsupported type for SCAN: " + varType.Literal);
                }

                std::string input;
                //std::cout << "Enter input for " << varName << ": ";
                std::getline(std::cin, input);

                
                if (varType.Type == INT) {
                    try {
                        int value = std::stoi(input);
                        env->Set(varName, varType, std::make_shared<IntegerObject>(value));
                    }
                    catch (const std::invalid_argument& e) {
                        return std::make_shared<ErrorObject>("Invalid input for type " + varType.Literal);
                    }
                    catch (const std::out_of_range& e) {
                        return std::make_shared<ErrorObject>("Input out of range for type " + varType.Literal);
                    }
                }
                else if (varType.Type == STRING) {
                    env->Set(varName, varType, std::make_shared<String>(input));
                }
                else if (varType.Type == BOOL) {
                    bool value = (input == "TRUE" || input == "1");
                    env->Set(varName, varType, std::make_shared<BooleanObject>(value));
                }
                else if (varType.Type == CHAR) {
                    if (input.length() != 1) {
                        return std::make_shared<ErrorObject>("Invalid input size for CHAR type. Expected a single character.");
                    }
                    char value = input.front();
                    env->Set(varName, varType, std::make_shared<Char>(value));
                }
            }
            // Return 1 for success code
            return std::make_shared<IntegerObject>(1);
        }
    )},
};


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
static void formatMessage(std::ostringstream& message, int line, const std::string& format, Args... args) {
    //message << "Error on line " << line << ": " << format;
    message << "Error: " << format;
    handleArgs(message, std::forward<Args>(args)...);
}

template<typename... Args>
static std::shared_ptr<ErrorObject> newError(const std::string& format, Args... args) {
    std::ostringstream message;
    formatMessage(message, evaluatorCurrentLine, format, std::forward<Args>(args)...);
    return std::make_unique<ErrorObject>(message.str());
}

static bool isError(const std::shared_ptr<Object>& obj) {
    if (obj) {
        return obj->Type() == ObjectTypeToString(ObjectType_::ERROR_OBJ);
    }
    return false;

}

static void logError(const std::string& errorMessage) {
    std::cout << "Error: " << errorMessage << std::endl;
}

static std::shared_ptr<Object> evalProgram(const std::vector<std::unique_ptr<Statement>>& stmts, const std::shared_ptr<Environment>& env) {
    std::shared_ptr<Object> result;
    for (const auto& statement : stmts) {
        // Dynamic cast to Node* for Eval
        const Node* node = dynamic_cast<const Node*>(statement.get());
        if (node) {
            result = Eval(node, env);
            if (auto returnValue = dynamic_cast<ReturnValue*>(result.get())) {
                return std::shared_ptr<Object>(returnValue->TakeValue());
            }
            else if (auto error = dynamic_cast<ErrorObject*>(result.get())) {
                std::cout << error->Message << std::endl;
                //return result;
            }
        }
        else {
            throw std::runtime_error("Failed to cast Statement to Node");
        }
    }
    return result; // Default return path
}

static std::shared_ptr<Object> evalBangOperatorExpression(std::shared_ptr<Object> right) {
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

static std::shared_ptr<Object> evalMinusPrefixOperatorExpression(std::shared_ptr<Object> right) {
    IntegerObject* integerRight = dynamic_cast<IntegerObject*>(right.get());

    if (!integerRight) {
        return newError("Unknown operator: -", right->Type());
    }

    return std::make_unique<IntegerObject>(-integerRight->Value);
}

static std::shared_ptr<Object> evalPrefixExpression(const std::string& operator_, std::shared_ptr<Object> right) {
    if (operator_ == "!") {
        return evalBangOperatorExpression(std::move(right));
    }
    if (operator_ == "-") {
        return evalMinusPrefixOperatorExpression(std::move(right));
    }
    // Default case returns a unique pointer to a new NullObject
    return newError("Unknown operator: ", operator_, right->Type());
}

static std::shared_ptr<Object> evalIntegerInfixExpression(
    const std::string& operator_,
    std::shared_ptr<Object> left,
    std::shared_ptr<Object> right) {

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
    return newError("Unknown operator: ", left->Type(), operator_, right->Type());  
}

static std::shared_ptr<Object> evalFloatInfixExpression(
    const std::string& operator_,
    std::shared_ptr<Object> left,
    std::shared_ptr<Object> right) {

    auto leftVal = (left->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ)) ?
        static_cast<float>(dynamic_cast<IntegerObject*>(left.get())->Value) :
        dynamic_cast<FloatObject*>(left.get())->Value;

    auto rightVal = (right->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ)) ?
        static_cast<float>(dynamic_cast<IntegerObject*>(right.get())->Value) :
        dynamic_cast<FloatObject*>(right.get())->Value;


    if (operator_ == "+") {
        return std::make_shared<FloatObject>(leftVal + rightVal);
    }
    else if (operator_ == "-") {
        return std::make_shared<FloatObject>(leftVal - rightVal);
    }
    else if (operator_ == "*") {
        return std::make_shared<FloatObject>(leftVal * rightVal);
    }
    else if (operator_ == "/") {
        if (rightVal == 0) {
            return std::make_shared<ErrorObject>("Division by zero");
        }
        return std::make_shared<FloatObject>(leftVal / rightVal);
    }
    else if (operator_ == "<") {
        return std::make_shared<BooleanObject>(leftVal < rightVal);
    }
    else if (operator_ == ">") {
        return std::make_shared<BooleanObject>(leftVal > rightVal);
    }
    else if (operator_ == "==") {
        return std::make_shared<BooleanObject>(leftVal == rightVal);
    }
    else if (operator_ == "<>") {
        return std::make_shared<BooleanObject>(leftVal != rightVal);
    }
    return std::make_shared<ErrorObject>("unknown operator: " + left->Type() + " " + operator_ + " " + right->Type());
}

//static std::shared_ptr<Object> evalStringInfixExpression(
//    const std::string& operator_,
//    std::shared_ptr<Object> left,
//    std::shared_ptr<Object> right) {
//
//    std::string leftVal;
//    if (left->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ)) {
//        leftVal = dynamic_cast<String*>(left.get())->Value;
//    }
//    else if (left->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)) {
//        leftVal = std::string(1, dynamic_cast<Char*>(left.get())->Value);
//    }
//    else {
//        return std::make_shared<ErrorObject>("left-hand operand of & must be a string or char, got " + left->Type());
//    }
//
//    std::string rightVal;
//    if (right->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ)) {
//        rightVal = dynamic_cast<String*>(right.get())->Value;
//    }
//    else if (right->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)) {
//        rightVal = std::string(1, dynamic_cast<Char*>(right.get())->Value);
//    }
//    else {
//        return std::make_shared<ErrorObject>("right-hand operand of & must be a string or char, got " + right->Type());
//    }
//
//    if (operator_ == "&") {
//        return std::make_shared<String>(leftVal + rightVal);
//    }
//    return std::make_shared<ErrorObject>("unknown operator: " + operator_ + " for types " + left->Type() + " and " + right->Type());
//}

static std::shared_ptr<Object> evalStringInfixExpression(
    const std::string& operator_,
    std::shared_ptr<Object> left,
    std::shared_ptr<Object> right) {

    auto objectToString = [](const std::shared_ptr<Object>& obj) -> std::string {
        if (auto str = dynamic_cast<String*>(obj.get())) {
            return str->Value;
        }
        else if (auto ch = dynamic_cast<Char*>(obj.get())) {
            return std::string(1, ch->Value);
        }
        else if (auto num = dynamic_cast<IntegerObject*>(obj.get())) {
            return std::to_string(num->Value);
        }
        else if (auto flt = dynamic_cast<FloatObject*>(obj.get())) {
            return std::to_string(flt->Value);
        }
        else if (auto bol = dynamic_cast<BooleanObject*>(obj.get())) {
            return bol->Value ? "TRUE" : "FALSE";
        }
        else {
            throw std::runtime_error("Unsupported type for string concatenation.");
        }
        };

    try {
        std::string leftVal = objectToString(left);
        std::string rightVal = objectToString(right);

        if (operator_ == "&") {
            return std::make_shared<String>(leftVal + rightVal);
        }
    }
    catch (const std::runtime_error& e) {
        return std::make_shared<ErrorObject>(e.what());
    }
    return std::make_shared<ErrorObject>("unknown operator: " + operator_ + " for types " + left->Type() + " and " + right->Type());
}

std::string getObjectStringValue(std::shared_ptr<Object> obj) {
    if (obj->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ)) {
        return std::to_string(dynamic_cast<IntegerObject*>(obj.get())->Value);
    }
    else if (obj->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ)) {
        return std::to_string(dynamic_cast<FloatObject*>(obj.get())->Value);
    }
    else if (obj->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ)) {
        return dynamic_cast<String*>(obj.get())->Value;
    }
    else if (obj->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)) {
        return std::string(1, dynamic_cast<Char*>(obj.get())->Value);
    }
    return "";
}

static std::shared_ptr<Object> evalBoolInfixExpression(
    const std::string& operator_,
    std::shared_ptr<Object> left,
    std::shared_ptr<Object> right) {

    std::string leftVal;
    if (left->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ)) {
        leftVal = dynamic_cast<BooleanObject*>(left.get())->Value ? "TRUE" : "FALSE";
    }
    else {
        leftVal = getObjectStringValue(left);
        if (leftVal.empty()) {
            return std::make_shared<ErrorObject>("left-hand operand of & must be boolean, integer, float, string, or char, got " + left->Type());
        }
    }

    std::string rightVal;
    if (right->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ)) {
        rightVal = dynamic_cast<BooleanObject*>(right.get())->Value ? "TRUE" : "FALSE";
    }
    else {
        rightVal = getObjectStringValue(right);
        if (rightVal.empty()) {
            return std::make_shared<ErrorObject>("right-hand operand of & must be boolean, integer, float, string, or char, got " + right->Type());
        }
    }

    if (operator_ == "&") {
        return std::make_shared<String>(leftVal + rightVal);
    }
    return std::make_shared<ErrorObject>("unknown operator: " + operator_ + " for types " + left->Type() + " and " + right->Type());
}

static std::shared_ptr<Object> evalInfixExpression(
    const std::string& operator_,
    std::shared_ptr<Object> left,
    std::shared_ptr<Object> right) {

    //std::cout << "Infix" << std::endl;

    if (left->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) && 
        right->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ)) {
        return evalIntegerInfixExpression(operator_, std::move(left), std::move(right));
    }
    // Int | Float vs Float
    else if ((left->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) &&
        right->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ)) ||
        (left->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ) &&
            right->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ)) ||
        (left->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ) &&
            right->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ))) {
        return evalFloatInfixExpression(operator_, std::move(left), std::move(right));
    }
    //// String vs String | Char
    //else if (
    //    (left->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) &&
    //        (right->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
    //        right->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ))) ||
    //    (left->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ) &&
    //        (right->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
    //        right->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)))
    //    ) {
    //    return evalStringInfixExpression(operator_, std::move(left), std::move(right));
    //}
    // String vs Any Primitive Type
    else if (
        (left->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
            left->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)) &&
        (right->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
            right->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ) ||
            right->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) ||
            right->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ) ||
            right->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ)) ||
        (right->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
            right->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)) &&
        (left->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) ||
            left->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ) ||
            left->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ))
        ) {
        return evalStringInfixExpression(operator_, std::move(left), std::move(right));
    }
    // Primitive Type vs Bool
    else if (
        (left->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ) && 
            right->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ)) ||
        (left->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ) &&
            (right->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) ||
                right->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ) ||
                right->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
                right->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ))) ||
        (right->Type() == ObjectTypeToString(ObjectType_::BOOLEAN_OBJ) &&
            (left->Type() == ObjectTypeToString(ObjectType_::INTEGER_OBJ) ||
                left->Type() == ObjectTypeToString(ObjectType_::FLOAT_OBJ) ||
                left->Type() == ObjectTypeToString(ObjectType_::STRING_OBJ) ||
                left->Type() == ObjectTypeToString(ObjectType_::CHAR_OBJ)))
        ) {
        return evalBoolInfixExpression(operator_, std::move(left), std::move(right));
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
        return newError("Type mismatch: ", left->Type(), operator_, right->Type());
    }
    else {
        return newError("Unknown operator: ", left->Type(), operator_, right->Type());
    }
}

static bool isTruthy(const std::shared_ptr<Object>& obj) {
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

static std::shared_ptr<Object> evalIfExpression(const IfExpression* ie, const std::shared_ptr<Environment>& env) {
    auto condition = Eval(ie->Condition.get(), env);
    if (isError(condition)) {
        return condition;
    }

    incrementEvaluatorLine();
    if (isTruthy(condition)) {
        return Eval(ie->Consequence.get(), env);
    }
    else if (ie->Alternative) {
        incrementEvaluatorLine();
        return Eval(ie->Alternative.get(), env);
    }
    else {
        incrementEvaluatorLine();
        return std::make_unique<NullObject>();
    }
}

static std::shared_ptr<Object> evalWhileExpression(const WhileExpression* we, const std::shared_ptr<Environment>& env) {

    while (true) {
        auto loopEnv = std::make_shared<Environment>(env);

        auto condition = Eval(we->Condition.get(), loopEnv);
        if (isError(condition)) {
            return condition;
        }

        // Break if condition is false
        if (!isTruthy(condition)) {
            break; 
        }

        // Evaluate the loop body 
        auto result = Eval(we->Body.get(), loopEnv); 
        if (isError(result)) {
            return result;
        }

        loopEnv.reset();

        // TODO: Maybe implement a break
    }

    return std::make_shared<NullObject>(); 
}


static std::shared_ptr<Object> evalBlockStatement(const BlockStatement* block, const std::shared_ptr<Environment>& env) {
    std::shared_ptr<Object> result = nullptr;

    for (const auto& statement : block->Statements) {
        incrementEvaluatorLine();
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

static std::shared_ptr<Object> evalIdentifier(const Identifier* node, const std::shared_ptr<Environment>& env) {
    auto val = env->GetObject(node->Value);
    if (val) {
        return val; 
    }

    auto it = builtins.find(node->Value);
    if (it != builtins.end()) {
        return it->second; 
    }

    return newError("Identifier not found '" + node->Value + "'");
}

static std::vector<std::shared_ptr<Object>> evalExpressions(const std::vector<std::unique_ptr<Expression>>& exps, const std::shared_ptr<Environment>& env) {
    std::vector<std::shared_ptr<Object>> results;

    for (const auto& expr : exps) {
        auto evaluated = Eval(expr.get(), env);
        if (isError(evaluated)) {
            // Immediately return a vector containing just this error
            std::vector<std::shared_ptr<Object>> result;
            result.push_back(std::move(evaluated));

            return result;
        }
        results.push_back(std::move(evaluated));
    }

    return results;
}

static std::shared_ptr<Environment> extendFunctionEnv(const Function& fn, const std::vector<std::shared_ptr<Object>>& args) {
    auto extendedEnv = std::make_shared<Environment>(fn.Env);

    for (size_t i = 0; i < fn.Parameters.size(); ++i) {
        extendedEnv->Set(fn.Parameters[i]->Name->Value, Token(FUNCTION, "FUNCTION"), args[i]);
    }

    return extendedEnv;
}

static std::shared_ptr<Object> unwrapReturnValue(std::shared_ptr<Object>& obj) {
    if (auto returnValue = dynamic_cast<ReturnValue*>(obj.get())) {
        return returnValue->TakeValue();
    }
    return obj;
}

static std::shared_ptr<Object> applyFunction(std::shared_ptr<Object>& fn, const std::vector<std::shared_ptr<Object>>& args, std::shared_ptr<Environment> env) {
    if (auto function = std::dynamic_pointer_cast<Function>(fn)) {
        auto extendedEnv = extendFunctionEnv(*function, args);
        auto evaluated = Eval(function->Body.get(), extendedEnv);
        return unwrapReturnValue(evaluated);
    }
    else if (auto builtin = std::dynamic_pointer_cast<Builtin>(fn)) {
        return builtin->Fn(args, env); // Call the builtin function with args
    }
    else {
        return std::make_shared<ErrorObject>("not a function: " + fn->Inspect());
    }
}

static bool isTypeCompatible(const TokenType declaredType, const std::shared_ptr<Object>& value) {
    if (!value) return false; // null check for safety
    if (declaredType == INT && dynamic_cast<IntegerObject*>(value.get())) return true;
    if (declaredType == FLOAT && dynamic_cast<FloatObject*>(value.get())) return true;
    if (declaredType == CHAR && dynamic_cast<Char*>(value.get())) return true;
    if (declaredType == BOOL && dynamic_cast<BooleanObject*>(value.get())) return true;
    if (declaredType == STRING && dynamic_cast<String*>(value.get())) return true;
    return false;
}

std::shared_ptr<Object> Eval(const Node* node, const std::shared_ptr<Environment>& env) {
    if (auto programNode = dynamic_cast<const Program*>(node)) {
        return evalProgram(programNode->Statements, env);
    }
    else if (auto markerStmt = dynamic_cast<const MarkerStatement*>(node)) {
        incrementEvaluatorLine();
    }
    else if (auto exprStmtNode = dynamic_cast<const ExpressionStatement*>(node)) { // Must increment line number
        if (exprStmtNode->token.Type == FUNCTION && exprStmtNode->name) {
            auto val = Eval(exprStmtNode->Expression_.get(), env);
            if (isError(val)) {
                return val;
            }
            env->Set(exprStmtNode->name->TokenLiteral(), exprStmtNode->token, val);
        }
        else {
            incrementEvaluatorLine();
            return Eval(exprStmtNode->Expression_.get(), env);
        }
    } 
    else if (auto intLit = dynamic_cast<const IntegerLiteral*>(node)) {
        return std::make_unique<IntegerObject>(intLit->Value);
    }
    else if (auto floatLit = dynamic_cast<const FloatLiteral*>(node)) {
        return std::make_unique<FloatObject>(floatLit->Value);
    }
    else if (auto bl = dynamic_cast<const Boolean*>(node)) {
        return std::make_unique<BooleanObject>(bl->Value);
    }
    else if (auto str = dynamic_cast<const StringLiteral*>(node)) {
        return std::make_unique<String>(str->Value);
    }
    else if (auto ch = dynamic_cast<const CharLiteral*>(node)) {
        return std::make_unique<Char>(ch->Value);
    }
    else if (auto prefixExpr = dynamic_cast<const PrefixExpression*>(node)) {
        auto right = Eval(prefixExpr->Right.get(), env);
        if (isError(right)) {
            return right;
        }
        return evalPrefixExpression(prefixExpr->Operator, right);
    }
    else if (auto infixExpr = dynamic_cast<const InfixExpression*>(node)) {
        auto left = Eval(infixExpr->Left.get(), env);
        //std::cout << left->Inspect() << std::endl;
        if (isError(left)) {
            return left;
        }
        auto right = Eval(infixExpr->Right.get(), env);
        //std::cout << right->Inspect() << std::endl;
        if (isError(right)) {
            return right;
        }
        return evalInfixExpression(infixExpr->Operator, left, right);
    }
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(node)) {
        return evalBlockStatement(blockStmt, env);
    }
    else if (auto ifExpr = dynamic_cast<const IfExpression*>(node)) {
        return evalIfExpression(ifExpr, env);
    }
    else if (auto whileExpr = dynamic_cast<const WhileExpression*>(node)) {
        return evalWhileExpression(whileExpr, env);
    }
    else if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(node)) {
        auto val = Eval(returnStmt->ReturnValue.get(), env);
        if (isError(val)) {
            return val;
        }
        return std::make_unique<ReturnValue>(val);
    }
    else if (auto multiStmt = dynamic_cast<const MultiTypedDeclStatement*>(node)) {
        incrementEvaluatorLine();
        //std::cout << multiStmt->token.Literal << std::endl;
        for (size_t i = 0; i < multiStmt->Names.size(); ++i) {
            auto& name = multiStmt->Names[i];
            std::shared_ptr<Object> value;

            // Check if there is a corresponding value for this name
            if (i < multiStmt->Values.size() && multiStmt->Values[i]) {
                value = Eval(multiStmt->Values[i].get(), env); // Evaluate the expression associated with the name
                if (!isTypeCompatible(multiStmt->token.Literal, value)) {
                    //return std::make_shared<ErrorObject>("Type mismatch for '" + name->Value + "': expected " + multiStmt->token.Literal + ", got " + value->Type());
                    logError("Type mismatch for '" + name->Value + "': expected " + multiStmt->token.Literal + ", got " + value->Type());
                    continue;
                }
                if (isError(value)) {
                    return value; // Propagate error
                }
            }
            else {
                value = std::make_shared<NullObject>(); // Default to Null if no value provided
            }

            // Assuming the name is an Identifier and has a string value field
            if (name) {
                env->Set(name->Value, multiStmt->token, value);
            }
            else {
                // Error handling if name is somehow null
                return std::make_shared<ErrorObject>("Invalid variable name in declaration.");
            }
        }
    }

    else if (const auto* typedDeclStmt = dynamic_cast<const TypedDeclStatement*>(node)) {
        incrementEvaluatorLine();
        if (typedDeclStmt->Value) {
            auto val = Eval(typedDeclStmt->Value.get(), env);
            if (isError(val)) {
                return val;
            }
            if (!isTypeCompatible(typedDeclStmt->token.Type, val)) {
                return std::make_shared<ErrorObject>("Type mismatch: Expected " + typedDeclStmt->token.Literal + " but got " + val->Type());
            }
            env->Set(typedDeclStmt->Name->Value, typedDeclStmt->token, val);
        }
        else {
            env->Set(typedDeclStmt->Name->Value, typedDeclStmt->token, std::make_unique<NullObject>());
        }
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

        auto fnObject = std::make_shared<Function>(funcLit->type, std::move(funcLit->CallName->clone()), std::move(params), std::move(body), env);
        
        env->Set(fnObject->CallName->TokenLiteral(), Token(FUNCTION, "FUNCTION"), fnObject);

        return fnObject;
    }
    else if (const auto* callExpr = dynamic_cast<const CallExpression*>(node)) {
        auto function = Eval(callExpr->Function.get(), env);
        
        if (isError(function)) {
            std::cout << function->Inspect() << std::endl;
            return function;
        }
        
        auto args = evalExpressions(callExpr->Arguments, env);
        if (!args.empty() && isError(args.front())) {
            return std::move(args.front());
        }

        auto result = applyFunction(function, args, env);

        return result;
    }
    else if (const auto* assignExpr = dynamic_cast<const AssignExpression*>(node)) {
        if (assignExpr->names.empty()) {
            return newError("Assignment without a variable name.");
        }
        auto value = Eval(assignExpr->value.get(), env);
        if (isError(value)) {
            return value;
        }

        for (auto it = assignExpr->names.rbegin(); it != assignExpr->names.rend(); ++it) {
            auto identifier = (*it)->TokenLiteral();
            auto expectedType = env->GetType(identifier);  // Retrieve expected type for the identifier

            // If the identifier exists but types don't match, return an error
            if ((
                expectedType.Literal == INT   || 
                expectedType.Literal == CHAR  ||
                expectedType.Literal == FLOAT ||
                expectedType.Literal == BOOL  ||
                expectedType.Literal == STRING 
                ) && !isTypeCompatible(expectedType.Literal, value)) {
                return newError("Type mismatch for '" + identifier + "': expected " + expectedType.Literal + ", got " + value->Type());
            }

            // If the identifier does not exist in the environment, return an error
            if (!(
                expectedType.Literal == INT   ||
                expectedType.Literal == CHAR  ||
                expectedType.Literal == FLOAT ||
                expectedType.Literal == BOOL  ||
                expectedType.Literal == STRING
                )) {
                return newError("Unknown Identifier: '" + identifier + "'");
            }

            // If types match, or no type information is required, set the new value
            //env->Set(identifier, Token("", ""), value);
            env->Set(identifier, expectedType, value);
        }
        return value;
    }
    return nullptr;
}