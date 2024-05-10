#include <tokenizer.h>
#include <type_traits>
#include <iostream>
#include <error.h>

Tokenizer::Tokenizer(std::istream *in) : flow_(in) {
    symbol_ = 32;
    Next();
}

bool Tokenizer::IsEnd() {
    if (is_end_) {
        return true;
    } else {
        return false;
    }
}

bool IfStartSimbol(int x) {
    if ((x != EOF) && ((x > 64 && x < 92) || (x > 96 && x < 123) || (x > 59 && x < 63) || x == 35 ||
                       x == 42 || x == 47 || x == 93)) {
        return true;
    } else {
        return false;
    }
}

bool IfSimbol(int x) {
    if (IfStartSimbol(x) || (x < 58 && x > 47) || x == 33 || x == 45 || x == 63) {
        return true;
    } else {
        return false;
    }
}

void Tokenizer::Next() {
    if (symbol_ == EOF) {
        is_end_ = true;
    } else {
        is_end_ = false;
        if (symbol_ == 40) {
            cur_token_ = BracketToken::OPEN;
            symbol_ = 32;
        } else if (symbol_ == 41) {
            cur_token_ = BracketToken::CLOSE;
            symbol_ = 32;
        } else if (symbol_ == 39) {
            cur_token_ = QuoteToken();
            symbol_ = 32;
        } else if (symbol_ == 46) {
            cur_token_ = DotToken();
            symbol_ = 32;
        } else if (symbol_ == 32 || symbol_ == 10) {
            symbol_ = flow_->std::istream::get();
            Next();
        } else if ((symbol_ > 47 && symbol_ < 58) || symbol_ == 43 || symbol_ == 45) {
            if (symbol_ == 43) {
                symbol_ = flow_->std::istream::get();
                if (!(symbol_ > 47 && symbol_ < 58)) {
                    char ch = '+';
                    std::string s;
                    s += ch;
                    cur_token_ = SymbolToken{s};
                } else {
                    char s = static_cast<char>(symbol_);
                    int val = s - '0';
                    int res = val;
                    symbol_ = flow_->std::istream::get();
                    while (symbol_ < 58 && symbol_ > 47) {
                        s = static_cast<char>(symbol_);
                        val = s - '0';
                        res = res * 10 + val;
                        symbol_ = flow_->std::istream::get();
                    }
                    cur_token_ = ConstantToken{res};
                }
            } else if (symbol_ == 45) {
                symbol_ = flow_->std::istream::get();
                if (!(symbol_ > 47 && symbol_ < 58)) {
                    char ch = '-';
                    std::string s;
                    s += ch;
                    cur_token_ = SymbolToken{s};
                } else {
                    char s = static_cast<char>(symbol_);
                    int val = s - '0';
                    int res = val;
                    symbol_ = flow_->std::istream::get();
                    while (symbol_ < 58 && symbol_ > 47) {
                        s = static_cast<char>(symbol_);
                        val = s - '0';
                        res = res * 10 + val;
                        symbol_ = flow_->std::istream::get();
                    }
                    res = res * (-1);
                    cur_token_ = ConstantToken{res};
                }
            } else {
                char s = static_cast<char>(symbol_);
                int val = s - '0';
                int res = val;
                symbol_ = flow_->std::istream::get();
                while (symbol_ < 58 && symbol_ > 47) {
                    s = static_cast<char>(symbol_);
                    val = s - '0';
                    res = res * 10 + val;
                    symbol_ = flow_->std::istream::get();
                }
                cur_token_ = ConstantToken{res};
            }
        } else if (IfStartSimbol(symbol_)) {
            char ch = static_cast<char>(symbol_);
            std::string s;
            s += ch;
            symbol_ = flow_->std::istream::get();
            while (IfSimbol(symbol_)) {
                ch = static_cast<char>(symbol_);
                s += ch;
                symbol_ = flow_->std::istream::get();
            }
            cur_token_ = SymbolToken{s};
        } else {
            throw SyntaxError("Syntax error");
        }
    }
}

Token Tokenizer::GetToken() {
    return cur_token_;
}

bool SymbolToken::operator==(const SymbolToken &other) const {
    return name == other.name;
}

bool DotToken::operator==(const DotToken &) const {
    return true;
}

bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken &other) const {
    return value == other.value;
}

