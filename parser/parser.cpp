#include <parser.h>
#include <error.h>

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    Token token = tokenizer->GetToken();
    if (tokenizer->IsEnd()) {
        throw SyntaxError("error");
    } else if (ConstantToken* cont = std::get_if<ConstantToken>(&token)) {
        std::shared_ptr<Number> ptr = std::make_shared<Number>(cont->value);
        tokenizer->Next();
        return ptr;
    } else if (SymbolToken* symt = std::get_if<SymbolToken>(&token)) {
        std::shared_ptr<Symbol> ptr = std::make_shared<Symbol>(symt->name);
        tokenizer->Next();
        return ptr;
    } else if (BracketToken* brt = std::get_if<BracketToken>(&token)) {
        if (*brt == BracketToken::OPEN) {
            return ReadList(tokenizer);
        } else {
            throw SyntaxError("error1");
        }
    } else if (DotToken* dt = std::get_if<DotToken>(&token)) {
        tokenizer->Next();
        return Read(tokenizer);
    } else {
        throw SyntaxError("error2");
    }
};

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    std::shared_ptr<Cell> root = nullptr;
    std::shared_ptr<Cell> cur_root = root;
    std::shared_ptr<Cell> pred = root;
    bool flag = true;
    tokenizer->Next();
    bool if_end = false;
    while (flag) {
        Token token = tokenizer->GetToken();
        if (tokenizer->IsEnd()) {
            throw SyntaxError("error3");
        } else if (std::holds_alternative<BracketToken>(token)) {
            BracketToken* brt = std::get_if<BracketToken>(&token);
            if (*brt == BracketToken::CLOSE) {
                tokenizer->Next();
                return root;
            } else {
                std::shared_ptr<Cell> new_node = std::make_shared<Cell>();
                new_node->first_ = Read(tokenizer);
                new_node->second_ = nullptr;
                if (root == nullptr) {
                    root = new_node;
                    pred = new_node;
                } else {
                    pred->second_ = new_node;
                    pred = new_node;
                }
            }
        } else if (if_end) {
            throw SyntaxError("error4");
        } else if (std::holds_alternative<DotToken>(token)) {
            tokenizer->Next();
            Token token1 = tokenizer->GetToken();
            if ((std::holds_alternative<ConstantToken>(token1) ||
                 (std::holds_alternative<SymbolToken>(token1) ||
                  std::holds_alternative<BracketToken>(token1))) &&
                pred != nullptr) {
                std::shared_ptr new_node = Read(tokenizer);
                pred->second_ = new_node;
                if_end = true;
            } else {
                throw SyntaxError("error4");
            }

        } else {
            std::shared_ptr<Cell> new_node = std::make_shared<Cell>();
            new_node->first_ = Read(tokenizer);
            new_node->second_ = nullptr;
            if (root == nullptr) {
                root = new_node;
                pred = new_node;
            } else {
                pred->second_ = new_node;
                pred = new_node;
            }
        }
    }
};

