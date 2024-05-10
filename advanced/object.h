#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include "error.h"

class Context;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::shared_ptr<Object> Calculate();
    virtual std::string ConvertToString();
    virtual std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec);
    virtual void SetContext(std::shared_ptr<Context> context){};
    virtual std::shared_ptr<Context> GetContext(){};
    virtual std::string GetKey() {
    }
    virtual void SetKey(std::string str){};
    virtual std::shared_ptr<Object> Copy(){};
};

class Context {
public:
    Context() {
    }
    std::shared_ptr<Context> parent_context_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<Object>> variables_dict_;
    std::shared_ptr<Context> child_context_ = nullptr;
};

class Number : public Object {
public:
    Number(int value) : value_(value) {
    }
    int GetValue() const {
        return value_;
    };
    std::shared_ptr<Object> Calculate() override {
        return std::make_shared<Number>(value_);
    };
    std::string ConvertToString() override {
        return std::to_string(value_);
    }
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> GetContext() override {
        return context_;
    };
    std::string GetKey() override {
        return key_;
    }
    void SetKey(std::string key) override {
        key_ = key;
    }
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override {
        return std::make_shared<Number>(value_);
    };

    std::shared_ptr<Object> Copy() override {
        auto res = std::make_shared<Number>(value_);
        res->context_ = context_;
        res->key_ = key_;
        return res;
    };
    std::shared_ptr<Context> context_;
    std::string key_;

private:
    int value_;
};
class Symbol : public Object {
public:
    Symbol(std::string name) : name_(name) {
    }
    const std::string& GetName() const {
        return name_;
    };
    std::shared_ptr<Object> Calculate() override {
        if (context_->variables_dict_.find(name_) != context_->variables_dict_.end()) {
            context_->variables_dict_[name_]->SetKey(name_);
            return context_->variables_dict_[name_];
        } else if (context_->parent_context_ != nullptr &&
                   context_->parent_context_->variables_dict_.find(name_) !=
                       context_->parent_context_->variables_dict_.end()) {
            return context_->parent_context_->variables_dict_[name_];
        }
        if (context_->variables_dict_.find("special arg") != context_->variables_dict_.end()) {
            context_->variables_dict_["z"] = context_->variables_dict_["special arg"];
            context_->variables_dict_["z"]->SetKey("z");
            return context_->variables_dict_["z"];
        }

        if (name_ == "#t" || name_ == "#f") {
            auto res = std::make_shared<Symbol>(name_);
            res->SetContext(context_);
            return res;
        }
        throw NameError("One symbol");
    }
    std::string ConvertToString() override {
        if (context_ != nullptr &&
            context_->variables_dict_.find(name_) != context_->variables_dict_.end()) {
            context_->variables_dict_[name_]->SetKey(name_);
            return context_->variables_dict_[name_]->ConvertToString();
        }

        return name_;
    }
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> GetContext() override {
        return context_;
    };
    std::string GetKey() override {
        return key_;
    }
    void SetKey(std::string key) override {
        key_ = key;
    }
    std::shared_ptr<Object> Copy() override {
        auto res = std::make_shared<Symbol>(name_);
        res->context_ = context_;
        res->key_ = key_;
        return res;
    };
    std::shared_ptr<Context> context_;
    std::string key_;

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell() : first_(nullptr), second_(nullptr) {
    }
    std::shared_ptr<Object> GetFirst() const {
        return first_;
    };
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    };
    std::shared_ptr<Object> Calculate() override;
    std::string ConvertToString() override;
    std::string CellToList();
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> GetContext() override {
        return context_;
    };
    std::string GetKey() override {
        return key_;
    }
    void SetKey(std::string key) override {
        key_ = key;
    }
    std::shared_ptr<Object> Copy() override {
        std::shared_ptr<Cell> res = std::make_shared<Cell>();
        res->first_ = first_;
        if (second_ != nullptr) {
            res->second_ = second_->Copy();
        }
        if (first_ != nullptr) {
            res->first_ = first_->Copy();
        }
        res->context_ = context_;
        res->key_ = key_;
        return res;
    };
    std::shared_ptr<Object> first_ = nullptr;
    std::shared_ptr<Object> second_ = nullptr;
    std::shared_ptr<Context> context_;
    std::string key_;
};

class Plus : public Object {
public:
    Plus(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Minus : public Object {
public:
    Minus(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Multiplies : public Object {
public:
    Multiplies(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Divides : public Object {
public:
    Divides(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Greater : public Object {
public:
    Greater(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Less : public Object {
public:
    Less(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Equal : public Object {
public:
    Equal(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class GreaterEqual : public Object {
public:
    GreaterEqual(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class LessEqual : public Object {
public:
    LessEqual(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Max : public Object {
public:
    Max(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Min : public Object {
public:
    Min(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Abs : public Object {
public:
    Abs(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class IsNumber : public Object {
public:
    IsNumber(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class IsBoolean : public Object {
public:
    IsBoolean(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Not : public Object {
public:
    Not(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class And : public Object {
public:
    And(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Or : public Object {
public:
    Or(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class IsPair : public Object {
public:
    IsPair(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class IsNull : public Object {
public:
    IsNull(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class IsSymbol : public Object {
public:
    IsSymbol(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class IsList : public Object {
public:
    IsList(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class List : public Object {
public:
    List(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class ListRef : public Object {
public:
    ListRef(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class ListTail : public Object {
public:
    ListTail(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Cons : public Object {
public:
    Cons(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Car : public Object {
public:
    Car(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> context_;
};

class Cdr : public Object {
public:
    Cdr(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> context_;
};

class SetCar : public Object {
public:
    SetCar(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> context_;
};

class SetCdr : public Object {
public:
    SetCdr(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> context_;
};

class Define : public Object {
public:
    Define(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class Set : public Object {
public:
    Set(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    std::shared_ptr<Context> context_;
};

class If : public Object {
public:
    If(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> GetContext() override {
        return context_;
    };
    std::shared_ptr<Context> context_;
};

class Lambda : public Object {
public:
    Lambda(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Context> context_;
    bool if_count;
};

class LambdaFunc : public Object {
public:
    LambdaFunc(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
    void SetContext(std::shared_ptr<Context> context) override {
        context_ = context;
    }
    std::shared_ptr<Object> Copy() override {
        auto res = std::make_shared<LambdaFunc>();
        res->parameters = parameters;
        res->func = func->Copy();
        res->context_ = context_;
        return res;
    };
    std::string ConvertToString() override {
        std::vector<std::shared_ptr<Object>> vec;
        return Apply(vec)->ConvertToString();
    };
    std::shared_ptr<Context> context_;
    std::vector<std::string> parameters;
    std::shared_ptr<Object> func;
    std::shared_ptr<Context> par_context_;
};

struct FunctionDict {
public:
    FunctionDict();
    std::unordered_map<std::string, std::shared_ptr<Object>> function_dict;
    std::shared_ptr<Context> context_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
};

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    if (As<T>(obj) != nullptr) {
        return true;
    } else {
        return false;
    }
};


