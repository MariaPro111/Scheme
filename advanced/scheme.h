#pragma once
#include <string>
#include "object.h"

class Interpreter {
public:
    Interpreter() {
        context_ = std::make_shared<Context>();
    }
    std::string Run(const std::string&);

private:
    std::shared_ptr<Context> context_;
};


