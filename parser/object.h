#pragma once

#include <memory>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
public:
    Number(int value) : value_(value) {
    }
    int GetValue() const {
        return value_;
    };

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
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
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
