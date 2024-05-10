#include <sstream>

#include "scheme.h"
#include "error.h"
#include "tokenizer.h"
#include "parser.h"

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
    if (context_ == nullptr) {
        context_ = std::make_shared<Context>();
    }
    FunctionDict d;
    auto obj = ReadFullStr(str);
    if (obj == nullptr) {
        throw RuntimeError("Null expression");
    }
    obj->SetContext(context_);
    auto new_obj = obj->Calculate();

    if (new_obj == nullptr) {
        return "()";
    }
    if (Is<Cell>(obj) && Is<Symbol>(As<Cell>(obj)->first_) &&
        As<Symbol>(As<Cell>(obj)->first_)->GetName() == "define") {
        return "define";
    }
    return new_obj->ConvertToString();
}


