#include "object.h"
#include "error.h"
#include <functional>
#include <sstream>
#include "tokenizer.h"
#include "parser.h"

FunctionDict::FunctionDict() {
    function_dict["+"] = std::make_shared<Plus>();
    function_dict["-"] = std::make_shared<Minus>();
    function_dict["*"] = std::make_shared<Multiplies>();
    function_dict["/"] = std::make_shared<Divides>();
    function_dict[">"] = std::make_shared<Greater>();
    function_dict["<"] = std::make_shared<Less>();
    function_dict["="] = std::make_shared<Equal>();
    function_dict[">="] = std::make_shared<GreaterEqual>();
    function_dict["<="] = std::make_shared<LessEqual>();
    function_dict["max"] = std::make_shared<Max>();
    function_dict["min"] = std::make_shared<Min>();
    function_dict["abs"] = std::make_shared<Abs>();
    function_dict["number?"] = std::make_shared<IsNumber>();
    function_dict["boolean?"] = std::make_shared<IsBoolean>();
    function_dict["not"] = std::make_shared<Not>();
    function_dict["and"] = std::make_shared<And>();
    function_dict["or"] = std::make_shared<Or>();
    function_dict["pair?"] = std::make_shared<IsPair>();
    function_dict["null?"] = std::make_shared<IsNull>();
    function_dict["list?"] = std::make_shared<IsList>();
    function_dict["list"] = std::make_shared<List>();
    function_dict["list-ref"] = std::make_shared<ListRef>();
    function_dict["list-tail"] = std::make_shared<ListTail>();
    function_dict["cons"] = std::make_shared<Cons>();
    function_dict["car"] = std::make_shared<Car>();
    function_dict["cdr"] = std::make_shared<Cdr>();
    function_dict["define"] = std::make_shared<Define>();
    function_dict["set!"] = std::make_shared<Set>();
    function_dict["symbol?"] = std::make_shared<IsSymbol>();
    function_dict["if"] = std::make_shared<If>();
    function_dict["set-car!"] = std::make_shared<SetCar>();
    function_dict["set-cdr!"] = std::make_shared<SetCdr>();
    function_dict["lambda"] = std::make_shared<Lambda>();
}

std::vector<std::shared_ptr<Object>> GiveArgs(std::shared_ptr<Object> args) {
    FunctionDict d;
    std::vector<std::shared_ptr<Object>> res_vec;
    if (args == nullptr) {
        return res_vec;
    }
    auto contex = args->GetContext();
    auto cur_cell = args;
    while (true) {
        if (Is<LambdaFunc>(cur_cell)) {
            std::vector<std::shared_ptr<Object>> v;
            res_vec.push_back(cur_cell);
            break;
        }
        if (Is<Symbol>(cur_cell) || Is<Number>(cur_cell)) {
            if (Is<Symbol>(cur_cell) && As<Symbol>(cur_cell)->GetName() == "lambda") {
                res_vec.push_back(cur_cell->Calculate());
            } else {
                cur_cell->SetContext(contex);
                res_vec.push_back(cur_cell);
            }
            break;
        } else if (Is<Symbol>(As<Cell>(cur_cell)->first_) &&
                   d.function_dict.find(As<Symbol>(As<Cell>(cur_cell)->first_)->GetName()) !=
                       d.function_dict.end()) {
            cur_cell->SetContext(contex);
            cur_cell = cur_cell->Calculate();
        } else if (Is<Symbol>(As<Cell>(cur_cell)->first_) &&
                   (As<Symbol>(As<Cell>(cur_cell)->first_)->GetName() == "quote" ||
                    As<Symbol>(As<Cell>(cur_cell)->first_)->GetName() == "'")) {
            cur_cell->SetContext(contex);
            res_vec.push_back(As<Cell>(cur_cell)->second_);
            break;
        } else if (Is<Cell>(As<Cell>(cur_cell)->first_)) {
            As<Cell>(cur_cell)->first_->SetContext(contex);
            std::vector<std::shared_ptr<Object>> vec_f = GiveArgs(As<Cell>(cur_cell)->first_);
            for (auto& x : vec_f) {
                if (x == nullptr) {
                    res_vec.push_back(nullptr);
                    return res_vec;
                }
                x->SetContext(contex);
                res_vec.push_back(x);
            }
            if (As<Cell>(cur_cell)->second_ != nullptr) {
                As<Cell>(cur_cell)->second_->SetContext(contex);
                std::vector<std::shared_ptr<Object>> vec_s = GiveArgs(As<Cell>(cur_cell)->second_);
                for (auto& y : vec_s) {
                    y->SetContext(contex);
                    res_vec.push_back(y);
                }
            }

            return res_vec;
        } else {
            std::shared_ptr<Object> x;
            if (As<Cell>(cur_cell)->first_ != nullptr) {
                try {
                    As<Cell>(cur_cell)->first_->SetContext(contex);
                    x = As<Cell>(cur_cell)->first_->Calculate();
                } catch (...) {
                    x = As<Cell>(cur_cell)->first_;
                }
            } else {
                x = nullptr;
            }
            if (x) {
                x->SetContext(contex);
            }
            res_vec.push_back(x);
            if (As<Cell>(cur_cell)->second_ == nullptr) {
                break;
            } else {
                cur_cell = As<Cell>(cur_cell)->second_;
            }
        }
    }
    return res_vec;
}
void Reset(std::shared_ptr<Context> context, std::string key) {
    if (!context->child_context_) {
        context->child_context_ = std::make_shared<Context>();
        context->child_context_->variables_dict_ = context->variables_dict_;
    }
    context->child_context_->variables_dict_[key] = context->variables_dict_[key];
    if (context->parent_context_ != nullptr &&
        context->parent_context_->variables_dict_.find(key) !=
            context->parent_context_->variables_dict_.end()) {
        if (!context->parent_context_->child_context_) {
            context->parent_context_->child_context_ = std::make_shared<Context>();
            context->child_context_->variables_dict_ = context->variables_dict_;
        }
        context->parent_context_->child_context_->variables_dict_[key] =
            context->parent_context_->variables_dict_[key];
        context->parent_context_->variables_dict_[key] = context->variables_dict_[key];
        Reset(context->parent_context_, key);
    }
}

std::vector<std::shared_ptr<Object>> GiveNotCalcArgs(std::shared_ptr<Object> args) {
    std::vector<std::shared_ptr<Object>> res_vec;
    if (args == nullptr || !Is<Cell>(args)) {
        throw SyntaxError("Invalid If args1");
    }
    auto context = args->GetContext();
    std::shared_ptr<Cell> cur_cell = As<Cell>(args);
    while (true) {
        cur_cell->first_->SetContext(context);
        if (Is<Cell>(cur_cell->first_)) {
            auto x = cur_cell->first_->Calculate();
            res_vec.push_back(x);
        } else {
            res_vec.push_back(cur_cell->first_);
        }
        if (cur_cell->second_ == nullptr) {
            break;
        }
        if (!Is<Cell>(cur_cell->second_)) {
            throw SyntaxError("Invalid If args2");
        }
        cur_cell = As<Cell>(cur_cell->second_);
    }
    return res_vec;
}

std::vector<std::shared_ptr<Object>> GiveNotCalcArgs1(std::shared_ptr<Object> args) {
    std::vector<std::shared_ptr<Object>> res_vec;
    if (args == nullptr || !Is<Cell>(args)) {
        throw SyntaxError("Invalid If args1");
    }
    auto context = args->GetContext();
    std::shared_ptr<Cell> cur_cell = As<Cell>(args);
    while (true) {
        cur_cell->first_->SetContext(context);
        res_vec.push_back(cur_cell->first_);
        if (cur_cell->second_ == nullptr) {
            break;
        }
        if (!Is<Cell>(cur_cell->second_)) {
            throw SyntaxError("Invalid If args2");
        }
        cur_cell = As<Cell>(cur_cell->second_);
    }
    return res_vec;
}

template <class Func>
std::shared_ptr<Object> ApplyIntOperations(Func f, std::vector<std::shared_ptr<Object>> vec) {
    if ((std::is_same<Func, std::minus<int>>::value ||
         std::is_same<Func, std::divides<int>>::value) &&
        vec.empty()) {
        throw RuntimeError("- or / have zero args");
    }
    int res_value = 0;
    if (std::is_same<Func, std::multiplies<int>>::value) {
        res_value = 1;
    }
    for (int i = 0; i < vec.size(); ++i) {
        std::shared_ptr<Number> arg = As<Number>(vec[i]);
        if (arg != nullptr) {
            if (i == 0) {
                res_value = arg->GetValue();
            } else {
                res_value = f(res_value, arg->GetValue());
            }
        } else if (Is<LambdaFunc>(vec[i])) {
            return Check(f, vec);
        } else if (vec[i] != nullptr && vec[i]->GetContext() != nullptr &&
                   vec[i]->GetContext()->parent_context_ != nullptr) {
            return CheckArg(f, vec, i);
        } else {
            throw RuntimeError("Invalid args in IntOperations");
        }
    }
    return std::make_shared<Number>(res_value);
}

template <class T>
std::shared_ptr<Object> Check(T f, std::vector<std::shared_ptr<Object>> vec) {
    auto lamb = vec[0];
    std::vector<std::shared_ptr<Object>> res;
    int i = 0;
    while (i < vec.size()) {
        if (Is<Number>(vec[i])) {
            res.push_back(vec[i]);
            ++i;
        } else {
            auto l = vec[i];
            std::vector<std::shared_ptr<Object>> v;
            ++i;
            while (i < vec.size()) {
                if (Is<Number>(vec[i])) {
                    v.push_back(vec[i]);
                    ++i;
                } else {
                    break;
                }
            }
            res.push_back(l->Apply(v));
        }
    }
    return ApplyIntOperations(f, res);
}

template <class T>
std::shared_ptr<Object> CheckArg(T f, std::vector<std::shared_ptr<Object>> vec, int i) {
    if (vec[i]->GetContext()->parent_context_->variables_dict_.find("special arg") !=
        vec[i]->GetContext()->parent_context_->variables_dict_.end()) {
        std::vector<std::shared_ptr<Object>> v =
            GiveArgs(vec[i]->GetContext()->parent_context_->variables_dict_["special arg"]);
        std::vector<std::shared_ptr<Object>> new_v;
        for (int j = 0; j < i; ++j) {
            new_v.push_back(vec[j]);
        }
        for (auto& x : v) {
            new_v.push_back(x);
        }
        return ApplyIntOperations(f, new_v);
    } else {
        throw RuntimeError("Invalid args in IntOperations");
    }
}

template <class Func>
std::shared_ptr<Object> ApplyIntComparison(Func f, std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty()) {
        return std::make_shared<Symbol>("#t");
    }
    for (int i = 0; i < vec.size() - 1; ++i) {
        std::shared_ptr<Number> arg1 = As<Number>(vec[i]);
        std::shared_ptr<Number> arg2 = As<Number>(vec[i + 1]);
        if (arg1 != nullptr && arg2 != nullptr) {
            if (!f(arg1->GetValue(), arg2->GetValue())) {
                return std::make_shared<Symbol>("#f");
            }
        } else {
            throw RuntimeError("Invalid args in IntComparison");
        }
    }
    return std::make_shared<Symbol>("#t");
}

std::shared_ptr<Object> ApplyMaxMin(std::string command, std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty()) {
        throw RuntimeError("Zero ApplyMaxMin args");
    }
    int res = 0;
    for (int i = 0; i < vec.size(); ++i) {
        std::shared_ptr<Number> arg = As<Number>(vec[i]);
        if (arg != nullptr) {
            if (i == 0) {
                res = arg->GetValue();
            } else {
                if (command == "max") {
                    res = std::max(res, arg->GetValue());
                } else {
                    res = std::min(res, arg->GetValue());
                }
            }
        } else {
            throw RuntimeError("Invalid args in IntComparison");
        }
    }
    return std::make_shared<Number>(res);
}

std::shared_ptr<Object> ApplyAbs(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1) {
        throw RuntimeError("Invalid ApplyAbs args");
    }
    std::shared_ptr<Number> arg = As<Number>(vec[0]);
    int res = 0;
    if (arg != nullptr) {
        res = abs(arg->GetValue());
        return std::make_shared<Number>(res);
    } else {
        throw RuntimeError("Invalid args in Abs");
    }
}

std::shared_ptr<Object> ApplyIsNumber(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1) {
        throw RuntimeError("Invalid ApplyIsNumber args");
    }
    std::shared_ptr<Number> arg = As<Number>(vec[0]);
    if (arg != nullptr) {
        return std::make_shared<Symbol>("#t");
    } else {
        return std::make_shared<Symbol>("#f");
    }
}

std::shared_ptr<Object> ApplyIsBoolean(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1) {
        return std::make_shared<Symbol>("#f");
    }
    std::shared_ptr<Symbol> arg = As<Symbol>(vec[0]);
    if (arg != nullptr && (arg->GetName() == "#f" || arg->GetName() == "#t")) {
        return std::make_shared<Symbol>("#t");
    } else {
        return std::make_shared<Symbol>("#f");
    }
}

std::shared_ptr<Object> ApplyNot(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1) {
        throw RuntimeError("Error in ApplyNot");
    }
    std::shared_ptr<Symbol> arg = As<Symbol>(vec[0]);
    if (arg != nullptr) {
        if (arg->GetName() == "#f") {
            return std::make_shared<Symbol>("#t");
        }
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> ApplyAnd(std::vector<std::shared_ptr<Object>> vec) {
    for (int i = 0; i < vec.size(); ++i) {
        std::shared_ptr<Symbol> arg = As<Symbol>(vec[i]);
        if (i == vec.size() - 1) {
            if (arg == nullptr) {
                return vec[i];
            } else if (arg->GetName() != "#f") {
                return vec[i];
            }
        }
        if (arg != nullptr && arg->GetName() == "#f") {
            return std::make_shared<Symbol>("#f");
        }
    }
    return std::make_shared<Symbol>("#t");
}

std::shared_ptr<Object> ApplyOr(std::vector<std::shared_ptr<Object>> vec) {
    for (int i = 0; i < vec.size(); ++i) {
        std::shared_ptr<Symbol> arg = As<Symbol>(vec[i]);
        if (i == vec.size() - 1) {
            if (arg == nullptr) {
                return vec[i];
            } else if (arg->GetName() == "#t") {
                return vec[i];
            }
        }
        if (arg == nullptr || arg->GetName() == "#t") {
            return std::make_shared<Symbol>("#t");
        }
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> Object::Apply(std::vector<std::shared_ptr<Object>> vec) {
    throw RuntimeError("error in Object_Apply");
}

std::shared_ptr<Object> Object::Calculate() {
    throw RuntimeError("error in Object_Calculate");
}

std::string Object::ConvertToString() {
    return "";
}

std::string Cell::CellToList() {
    std::string res = "(";
    res += first_->ConvertToString();
    auto cur_cell = second_;
    while (true) {
        if (cur_cell == nullptr) {
            break;
        } else if (Is<Number>(cur_cell)) {
            res += " . ";
            res += As<Number>(cur_cell)->ConvertToString();
            break;
        } else if (Is<Symbol>(cur_cell)) {
            res += " ";
            res += As<Symbol>(cur_cell)->ConvertToString();
            break;
        } else {
            res += " ";
            res += As<Cell>(cur_cell)->first_->ConvertToString();
            cur_cell = As<Cell>(cur_cell)->second_;
        }
    }
    res += ")";
    return res;
}

std::string Cell::ConvertToString() {
    if (first_ == nullptr && second_ == nullptr) {
        return "(())";
    }
    if (Is<Number>(first_) || Is<Symbol>(first_)) {
        return CellToList();
    } else {
        if (second_ == nullptr) {
            return first_->ConvertToString();
        }
    }

    std::string res = "(";
    if (first_ != nullptr) {
        res += first_->ConvertToString();
    }
    if (second_ != nullptr) {
        res += " ";
        res += second_->ConvertToString();
    }
    res += ")";
    return res;
}

std::shared_ptr<Object> Plus::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::plus<int> f;
    return ApplyIntOperations(f, vec);
}

std::shared_ptr<Object> Minus::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::minus<int> f;
    return ApplyIntOperations(f, vec);
}

std::shared_ptr<Object> Multiplies::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::multiplies<int> f;
    return ApplyIntOperations(f, vec);
}

std::shared_ptr<Object> Divides::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::divides<int> f;
    return ApplyIntOperations(f, vec);
}

std::shared_ptr<Object> Greater::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::greater<int> f;
    return ApplyIntComparison(f, vec);
}

std::shared_ptr<Object> Less::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::less<int> f;
    return ApplyIntComparison(f, vec);
}

std::shared_ptr<Object> GreaterEqual::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::greater_equal<int> f;
    return ApplyIntComparison(f, vec);
}

std::shared_ptr<Object> LessEqual::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::less_equal<int> f;
    return ApplyIntComparison(f, vec);
}

std::shared_ptr<Object> Equal::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::equal_to<int> f;
    return ApplyIntComparison(f, vec);
}

std::shared_ptr<Object> Max::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyMaxMin("max", vec);
}

std::shared_ptr<Object> Min::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyMaxMin("min", vec);
}

std::shared_ptr<Object> Abs::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyAbs(vec);
}

std::shared_ptr<Object> IsNumber::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyIsNumber(vec);
}

std::shared_ptr<Object> IsBoolean::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyIsBoolean(vec);
}

std::shared_ptr<Object> Not::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyNot(vec);
}

std::shared_ptr<Object> And::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyAnd(vec);
}

std::shared_ptr<Object> Or::Apply(std::vector<std::shared_ptr<Object>> vec) {
    return ApplyOr(vec);
}

std::shared_ptr<Object> IsPair::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() == 1 && Is<Cell>(vec[0])) {
        auto arg = As<Cell>(vec[0]);
        if (arg->first_ != nullptr && arg->second_ != nullptr) {
            return std::make_shared<Symbol>("#t");
        } else {
            return std::make_shared<Symbol>("#f");
        }
    } else {
        return std::make_shared<Symbol>("#f");
    }
}

std::shared_ptr<Object> IsNull::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty()) {
        return std::make_shared<Symbol>("#t");
    }
    if (vec.size() == 1) {
        auto arg = As<Cell>(vec[0]);
        if (arg == nullptr || (arg->first_ == nullptr && arg->second_ == nullptr)) {
            return std::make_shared<Symbol>("#t");
        }
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> IsList::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec[0] == nullptr) {
        return std::make_shared<Symbol>("#t");
    }
    auto arg = As<Cell>(vec[0]);
    auto cur_arg = arg;
    while (true) {
        if (cur_arg->second_ == nullptr) {
            return std::make_shared<Symbol>("#t");
        } else if (!Is<Cell>(cur_arg->second_)) {
            return std::make_shared<Symbol>("#f");
        } else {
            cur_arg = As<Cell>(cur_arg->second_);
        }
    }
}

std::shared_ptr<Object> IsSymbol::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1 || vec[0] == nullptr) {
        return std::make_shared<Symbol>("#f");
    }
    if (Is<Symbol>(vec[0])) {
        return std::make_shared<Symbol>("#t");
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> List::Apply(std::vector<std::shared_ptr<Object>> vec) {
    std::string res = "(";
    for (auto& x : vec) {
        res += " ";
        res += x->ConvertToString();
    }
    res += ")";
    std::stringstream ss{res};
    Tokenizer tokenizer{&ss};

    auto obj = Read(&tokenizer);
    return obj;
}

std::shared_ptr<Object> ListRef::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() < 2) {
        throw RuntimeError("Invalid ListTail arg1");
    }
    std::vector<std::shared_ptr<Object>> list = GiveArgs(vec[0]);
    if (!Is<Number>(vec[1])) {
        throw RuntimeError("Invalid ListTail arg2");
    }
    if (As<Number>(vec[1])->GetValue() < list.size()) {
        int val = As<Number>(list[As<Number>(vec[1])->GetValue()])->GetValue();
        return std::make_shared<Number>(val);
    } else {
        throw RuntimeError("Invalid ListTail args");
    }
}

std::shared_ptr<Object> ListTail::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() < 2) {
        throw RuntimeError("Invalid ListTail arg1");
    }
    std::vector<std::shared_ptr<Object>> list = GiveArgs(vec[0]);
    if (!Is<Number>(vec[1])) {
        throw RuntimeError("Invalid ListTail arg2");
    }
    if (As<Number>(vec[1])->GetValue() <= list.size()) {
        std::string res = "(";
        for (int i = As<Number>(vec[1])->GetValue(); i < list.size(); ++i) {
            res += " ";
            res += list[i]->ConvertToString();
        }
        res += ")";
        std::stringstream ss{res};
        Tokenizer tokenizer{&ss};

        auto obj = Read(&tokenizer);
        return obj;
    } else {
        throw RuntimeError("Invalid ListTail args");
    }
}

std::shared_ptr<Object> Cons::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() == 2 && Is<LambdaFunc>(vec[0]) && Is<LambdaFunc>(vec[1])) {  ////////
        auto cell = std::make_shared<Cell>();
        cell->first_ = vec[0];
        cell->second_ = vec[1];
        return cell;
    }
    std::string res = "(";
    for (int i = 0; i < vec.size(); ++i) {
        if (i == vec.size() - 1 && i != 0) {
            res += " . ";
        } else if (i != 0) {
            res += " ";
        }
        res += vec[i]->ConvertToString();
    }
    res += ")";
    std::stringstream ss{res};
    Tokenizer tokenizer{&ss};

    auto obj = Read(&tokenizer);
    return obj;
}

std::shared_ptr<Object> Cdr::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1 || !Is<Cell>(vec[0]) || vec[0] == nullptr) {
        throw RuntimeError("Invalid Cdr args");
    }
    if (As<Cell>(vec[0])->second_ == nullptr) {
        vec[0] = As<Cell>(vec[0])->first_;
    }
    vec[0]->SetContext(context_);
    auto x = vec[0]->Calculate();
    if (x == nullptr) {
        throw RuntimeError("Invalid Cdr args");
    }
    if (Is<Symbol>(x)) {
        x = x->Calculate();
        return As<Cell>(x)->second_;
    }
    if (Is<Cell>(x)) {
        return As<Cell>(x)->second_;
    }
    return nullptr;
}

std::shared_ptr<Object> Car::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1 || !Is<Cell>(vec[0]) || vec[0] == nullptr) {
        throw RuntimeError("Invalid Car args");
    }
    if (As<Cell>(vec[0])->second_ == nullptr) {
        vec[0] = As<Cell>(vec[0])->first_;
    }
    vec[0]->SetContext(context_);
    auto x = vec[0]->Calculate();
    if (x == nullptr) {
        throw RuntimeError("Invalid Car args");
    }
    if (Is<Symbol>(x)) {
        x = x->Calculate();
        return As<Cell>(x)->first_;
    }
    if (Is<Cell>(x)) {
        return As<Cell>(x)->first_;
    }
    return x;
}

std::shared_ptr<Object> SetCdr::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty()) {
        throw RuntimeError("Error in SetCar");
    }
    vec = GiveNotCalcArgs(vec[0]);
    if (vec.size() != 2) {
        throw RuntimeError("Error in SetCar");
    }
    auto x = vec[0]->Calculate();
    auto res = std::make_shared<Cell>();
    auto y = vec[1];
    if (Is<Symbol>(x)) {
        x = x->Calculate();
    }
    As<Cell>(x)->second_ = y;
    return nullptr;
}

std::shared_ptr<Object> SetCar::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty()) {
        throw RuntimeError("Error in SetCar");
    }
    vec = GiveNotCalcArgs(vec[0]);
    if (vec.size() != 2) {
        throw RuntimeError("Error in SetCar");
    }
    auto x = vec[0]->Calculate();
    auto res = std::make_shared<Cell>();
    auto y = vec[1];
    if (Is<Symbol>(x)) {
        x = x->Calculate();
    }
    As<Cell>(x)->first_ = y;
    return nullptr;
}

std::shared_ptr<Object> Define::Apply(std::vector<std::shared_ptr<Object>> vec) {
    FunctionDict d;
    std::shared_ptr<Object> res;
    auto cell = As<Cell>(vec[0]);
    if (cell == nullptr || cell->first_ == nullptr) {
        throw RuntimeError("Invalid Define args1");
    }
    if (Is<Cell>(cell->first_) && cell->second_ == nullptr) {
        cell->first_->SetContext(cell->context_);
        cell = As<Cell>(cell->first_);
    }
    auto x = As<Symbol>(cell->first_);
    if (x == nullptr) {
        if (Is<Cell>(cell->first_)) {
            auto new_cell = std::make_shared<Cell>();
            auto l = std::make_shared<Symbol>("lambda");
            new_cell->first_ = l;
            auto new_cell3 = std::make_shared<Cell>();
            new_cell3->first_ = As<Cell>(cell->first_)->second_;
            new_cell3->second_ = As<Cell>(cell->second_)->first_->Copy();
            new_cell->second_ = new_cell3;
            auto new_cell2 = std::make_shared<Cell>();
            new_cell2->first_ = As<Cell>(cell->first_)->first_->Copy();
            auto tmp = std::make_shared<Cell>();
            tmp->first_ = new_cell;
            tmp->second_ = nullptr;
            new_cell2->second_ = tmp;
            new_cell2->SetContext(vec[0]->GetContext());
            std::vector<std::shared_ptr<Object>> vec;
            vec.push_back(new_cell2);
            res = d.function_dict["define"]->Apply(vec);
            return d.function_dict["define"]->Apply(vec);
        }
        throw SyntaxError("Invalid Define args2");
    }
    if (Is<Cell>(cell->second_) && As<Cell>(cell->second_)->second_ == nullptr) {
        As<Cell>(cell->second_)->first_->SetContext(cell->context_);
        auto key = As<Symbol>(As<Cell>(cell->second_)->first_);
        if (key != nullptr && d.function_dict.find(key->GetName()) != d.function_dict.end()) {
            cell->context_->variables_dict_[x->GetName()] = d.function_dict[key->GetName()];
            Reset(cell->context_, x->GetName());
        } else {
            try {
                cell->context_->variables_dict_[x->GetName()] =
                    As<Cell>(cell->second_)->first_->Calculate();
                Reset(cell->context_, x->GetName());
                res = cell->context_->variables_dict_[x->GetName()];
            } catch (NameError) {
                throw NameError("Invalid Define args3");
            } catch (...) {
                throw SyntaxError("Invalid Define args4");
            }
        }
    } else {
        cell->second_->SetContext(cell->context_);
        try {
            cell->context_->variables_dict_[x->GetName()] = cell->second_->Calculate();
            Reset(cell->context_, x->GetName());
            res = cell->context_->variables_dict_[x->GetName()];
        } catch (NameError) {
            throw NameError("Invalid Define args5");
        } catch (...) {
            throw SyntaxError("Invalid Define args6");
        }
    }
    return res;
}

std::shared_ptr<Object> Set::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty()) {
        throw SyntaxError("Invalid Set args");
    }
    auto cell = As<Cell>(vec[0]);
    if (cell == nullptr || cell->first_ == nullptr) {
        throw SyntaxError("Invalid Set args");
    }
    if (Is<Cell>(cell->first_) && cell->second_ == nullptr) {
        cell->first_->SetContext(cell->context_);
        cell = As<Cell>(cell->first_);
    }
    auto x = As<Symbol>(cell->first_);
    if (x == nullptr) {
        throw SyntaxError("Invalid Set args");
    }
    std::shared_ptr<Object> cell2;
    if (Is<Cell>(cell->second_) && As<Cell>(cell->second_)->second_ == nullptr) {
        As<Cell>(cell->second_)->first_->SetContext(cell->context_);
        cell2 = As<Cell>(cell->second_)->first_;
    } else {
        cell->second_->SetContext(cell->context_);
        cell2 = cell->second_;
    }

    try {
        cell2 = cell2->Calculate();
    } catch (...) {
        throw SyntaxError("Invalid Set args1");
    }

    auto cont = cell->context_;
    bool f = false;
    while (true) {
        if (cont == nullptr) {
            f = false;
            break;
        }
        if (cont->variables_dict_.find(x->GetName()) != cont->variables_dict_.end()) {
            cont->variables_dict_[x->GetName()] = cell2;
            Reset(cont, x->GetName());
            f = true;
            break;
        }
        cont = cont->parent_context_;
    }
    if (!f) {
        throw NameError("Invalid Set args");
    }

    return cell2;
}

std::shared_ptr<Object> If::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.empty() || vec[0] == nullptr) {
        throw SyntaxError("Invalid If args");
    }
    // auto context = vec[0]->GetContext();
    vec = GiveNotCalcArgs1(vec[0]);
    if (vec.size() > 3 || vec.size() < 2) {
        throw SyntaxError("Invalid If args");
    }
    vec[0]->SetContext(context_);
    auto arg1 = vec[0]->Calculate();
    if (!Is<Symbol>(arg1) || As<Symbol>(arg1)->GetName() != "#f") {
        vec[1]->SetContext(context_);
        auto res = vec[1]->Calculate();
        res->SetContext(context_);
        return res;
    } else {
        if (vec.size() == 2) {
            return nullptr;
        } else {
            vec[2]->SetContext(context_);
            auto res = vec[2]->Calculate();
            res->SetContext(context_);
            return res;
        }
    }
}

std::shared_ptr<Object> Lambda::Apply(std::vector<std::shared_ptr<Object>> vec) {
    // context_ = std::make_shared<Context>();
    auto root = As<Cell>(vec[0]);
    std::shared_ptr<Cell> arg = As<Cell>(root->first_);
    std::shared_ptr<Object> func = root->second_;
    if (func == nullptr || (!Is<Cell>(func) && !Is<Number>(root->second_))) {
        throw SyntaxError("Error in Lambda args");
    }
    std::shared_ptr<LambdaFunc> res = std::make_shared<LambdaFunc>();
    res->par_context_ = context_;
    if (arg != nullptr) {
        arg->SetContext(context_);
    }
    std::vector<std::shared_ptr<Object>> parameters = GiveArgs(arg);
    res->func = func;
    res->SetContext(context_);
    for (auto& x : parameters) {
        if (Is<Number>(x) || Is<Cell>(x)) {
            res->parameters.push_back(x->GetKey());
        } else {
            res->parameters.push_back(As<Symbol>(x)->GetName());
        }
    }
    return res;
}

std::shared_ptr<Object> LambdaFunc::Apply(std::vector<std::shared_ptr<Object>> vec) {
    context_ = std::make_shared<Context>();
    if (par_context_->variables_dict_.find("special arg") != par_context_->variables_dict_.end()) {
        if (par_context_->child_context_ != nullptr) {
            par_context_->child_context_->variables_dict_["special arg"] =
                par_context_->variables_dict_["special arg"];
        }
    }
    if (par_context_->child_context_ != nullptr) {
        context_->parent_context_ = par_context_->child_context_;
    } else {
        context_->parent_context_ = par_context_;
    }
    if (vec.size() != parameters.size()) {
        for (int i = 0; i < vec.size(); ++i) {
            if (!Is<Symbol>((vec[i]))) {
                throw RuntimeError("Error in lambda");
            } else if (context_->parent_context_->variables_dict_.find(
                           As<Symbol>((vec[i]))->GetName()) !=
                       context_->parent_context_->variables_dict_.end()) {
                context_->variables_dict_[parameters[i]] =
                    context_->parent_context_->variables_dict_[As<Symbol>((vec[i]))->GetName()];
            } else {
                throw RuntimeError("Error in lambda");
            }
        }
    }
    func->SetContext(context_);
    if (vec.empty() && parameters.empty()) {
        context_->variables_dict_ = context_->parent_context_->variables_dict_;
    }
    for (int i = 0; i < vec.size(); ++i) {
        context_->variables_dict_[parameters[i]] = vec[i];
    }
    // func->SetContext(context_);
    if (Is<Number>(func)) {
        return func;
    }
    if (Is<Cell>(As<Cell>(func)->first_)) {
        As<Cell>(func)->first_->SetContext(context_);
        if (As<Cell>(func)->second_ != nullptr) {
            As<Cell>(func)->second_->SetContext(context_);
        }
        auto res1 = As<Cell>(func)->first_->Calculate();

        auto res = As<Cell>(func)->second_;
        if (res == nullptr) {
            return res1;
        }
        res->SetContext(context_);
        res = res->Calculate();
        return res;
    }
    auto res = func->Calculate();
    return res;
}

std::shared_ptr<Object> Cell::Calculate() {
    FunctionDict d;
    if (Is<Symbol>(first_)) {
        std::string operation = As<Symbol>(first_)->GetName();
        if (operation == "quote" || operation == "'") {
            if (Is<Cell>(second_) && As<Cell>(second_)->first_ == nullptr) {
                return second_;
            }
            if (Is<Cell>(second_) && As<Cell>(second_)->second_ == nullptr &&
                Is<Symbol>(As<Cell>(second_)->first_)) {
                return As<Cell>(second_)->first_;
                ;
            }
            if (Is<Cell>(second_) && As<Cell>(second_)->second_ == nullptr &&
                Is<Cell>(As<Cell>(second_)->first_)) {
                return As<Cell>(second_)->first_;
            }
            return second_;
        }
        if (operation == "define" || operation == "set!" || operation == "if" ||
            operation == "set-cdr!" || operation == "set-car!" || operation == "cdr" ||
            operation == "car" || operation == "lambda") {
            if (second_ == nullptr) {
                throw SyntaxError("Zero args in define");
            }
            second_->SetContext(context_);
            std::vector<std::shared_ptr<Object>> vec;
            vec.push_back(second_);
            d.function_dict[operation]->SetContext(context_);
            return d.function_dict[operation]->Apply(vec);
        }
        if (context_->variables_dict_.find(operation) != context_->variables_dict_.end()) {
            std::vector<std::shared_ptr<Object>> vec;
            context_->variables_dict_[operation]->SetContext(context_);
            vec.push_back(second_);
            if (second_) {
                vec[0]->SetContext(context_);
            }
            return context_->variables_dict_[operation]->Apply(GiveArgs(vec[0]));
        } else {
            auto cont = context_;
            while (true) {
                if (cont == nullptr) {
                    break;
                }
                if (cont->variables_dict_.find(operation) != cont->variables_dict_.end()) {
                    std::vector<std::shared_ptr<Object>> vec;
                    vec.push_back(second_);
                    if (second_) {
                        vec[0]->SetContext(context_);
                    }
                    if (Is<Number>(cont->variables_dict_[operation])) {
                        return cont->variables_dict_[operation];
                    }
                    return cont->variables_dict_[operation]->Apply(GiveArgs(vec[0]));
                }
                cont = cont->parent_context_;
            }
        }

        if (d.function_dict.find(operation) != d.function_dict.end()) {
            if (second_) {
                second_->SetContext(context_);
            }
            return d.function_dict[operation]->Apply(GiveArgs(second_));
        }

    } else {
        if (Is<Cell>(first_) && second_ == nullptr) {
            first_->SetContext(context_);
            return first_->Calculate();
        }
        if (Is<Cell>(first_) && second_ != nullptr) {
            if (Is<Cell>(second_) && As<Cell>(second_)->first_ == nullptr &&
                As<Cell>(second_)->second_ == nullptr) {
                throw RuntimeError("Error in cell calculate");
            }
            first_->SetContext(context_);
            context_->variables_dict_["special arg"] = second_;
            return first_->Calculate();
        }
    }
    throw RuntimeError("Error in cell calculate");
}

