#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::shared_ptr<Object> Calculate();
    virtual std::string ConvertToString();
    virtual std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec);
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
        return std::make_shared<Symbol>(name_);
    }
    std::string ConvertToString() override {
        return name_;
    }

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
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

class Plus : public Object {
public:
    Plus(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Minus : public Object {
public:
    Minus(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Multiplies : public Object {
public:
    Multiplies(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Divides : public Object {
public:
    Divides(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Greater : public Object {
public:
    Greater(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Less : public Object {
public:
    Less(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Equal : public Object {
public:
    Equal(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class GreaterEqual : public Object {
public:
    GreaterEqual(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class LessEqual : public Object {
public:
    LessEqual(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Max : public Object {
public:
    Max(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Min : public Object {
public:
    Min(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Abs : public Object {
public:
    Abs(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class IsNumber : public Object {
public:
    IsNumber(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class IsBoolean : public Object {
public:
    IsBoolean(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Not : public Object {
public:
    Not(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class And : public Object {
public:
    And(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Or : public Object {
public:
    Or(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class IsPair : public Object {
public:
    IsPair(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class IsNull : public Object {
public:
    IsNull(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class IsList : public Object {
public:
    IsList(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class List : public Object {
public:
    List(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class ListRef : public Object {
public:
    ListRef(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class ListTail : public Object {
public:
    ListTail(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Cons : public Object {
public:
    Cons(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Car : public Object {
public:
    Car(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

class Cdr : public Object {
public:
    Cdr(){};
    std::shared_ptr<Object> Apply(std::vector<std::shared_ptr<Object>> vec) override;
};

struct FunctionDict {
public:
    FunctionDict();
    std::unordered_map<std::string, std::shared_ptr<Object>> function_dict;
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

