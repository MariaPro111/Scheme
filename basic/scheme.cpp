#include <sstream>
#include "scheme.h"
#include "error.h"

auto ReadFullStr(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};

    auto obj = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Tokenizer did not go to end");
    }
    return obj;
}

std::string Interpreter::Run(const std::string& str) {
    FunctionDict d;
    auto obj = ReadFullStr(str);
    if (obj == nullptr) {
        throw RuntimeError("Null expression");
    }
    auto new_obj = obj->Calculate();
    if (new_obj == nullptr) {
        return "()";
    }
    return new_obj->ConvertToString();
}

