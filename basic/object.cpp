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
}

std::vector<std::shared_ptr<Object>> GiveArgs(std::shared_ptr<Object> args) {
    FunctionDict d;
    std::vector<std::shared_ptr<Object>> res_vec;
    if (args == nullptr) {
        return res_vec;
    }
    std::shared_ptr<Object> cur_cell = args;
    while (true) {
        if (Is<Symbol>(cur_cell) || Is<Number>(cur_cell)) {
            res_vec.push_back(cur_cell);
            break;
        } else if (Is<Symbol>(As<Cell>(cur_cell)->first_) &&
                   d.function_dict.find(As<Symbol>(As<Cell>(cur_cell)->first_)->GetName()) !=
                       d.function_dict.end()) {
            cur_cell = cur_cell->Calculate();
        } else if (Is<Symbol>(As<Cell>(cur_cell)->first_) &&
                   (As<Symbol>(As<Cell>(cur_cell)->first_)->GetName() == "quote" ||
                    As<Symbol>(As<Cell>(cur_cell)->first_)->GetName() == "'")) {
            res_vec.push_back(As<Cell>(cur_cell)->second_);
            break;
        } else if (Is<Cell>(As<Cell>(cur_cell)->first_)) {
            std::vector<std::shared_ptr<Object>> vec_f = GiveArgs(As<Cell>(cur_cell)->first_);
            for (auto& x : vec_f) {
                res_vec.push_back(x);
            }
            std::vector<std::shared_ptr<Object>> vec_s = GiveArgs(As<Cell>(cur_cell)->second_);
            for (auto& y : vec_s) {
                res_vec.push_back(y);
            }
            return res_vec;
        } else {
            res_vec.push_back(As<Cell>(cur_cell)->first_);
            if (As<Cell>(cur_cell)->second_ == nullptr) {
                break;
            } else {
                cur_cell = As<Cell>(cur_cell)->second_;
            }
        }
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
        } else {
            throw RuntimeError("Invalid args in IntOperations");
        }
    }
    return std::make_shared<Number>(res_value);
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
    throw RuntimeError("error in Object_ConvertToString");
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
    if (vec.size() != 1 || !Is<Cell>(vec[0])) {
        throw RuntimeError("Invalid Cdr args");
    }
    return As<Cell>(vec[0])->second_;
}

std::shared_ptr<Object> Car::Apply(std::vector<std::shared_ptr<Object>> vec) {
    if (vec.size() != 1 || !Is<Cell>(vec[0])) {
        throw RuntimeError("Invalid Cdr args");
    }
    return As<Cell>(vec[0])->first_;
}

std::shared_ptr<Object> Cell::Calculate() {
    FunctionDict d;
    if (Is<Symbol>(first_)) {
        std::string operation = As<Symbol>(first_)->GetName();
        if (operation == "quote" || operation == "'") {
            return second_;
        }
        if (d.function_dict.find(operation) != d.function_dict.end()) {
            return d.function_dict[operation]->Apply(GiveArgs(second_));
        }
    }
    throw RuntimeError("Error in cell calculate");
}
