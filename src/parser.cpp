// Integer Calculator (enum-based Token with int values)
#include <vector>

#include "parser.hpp"

namespace {

    static constexpr int8_t K_BP_DELTA =
        10; // binding power delta for parenthesis

    // Binding power for infix operators
    std::pair<uint8_t, uint8_t> get_bp_infix(parser::Operator op) {
        switch (op) {
        case parser::Operator::ADD:
        case parser::Operator::SUB:
            return {1, 2};
        case parser::Operator::MUL:
        case parser::Operator::DIV:
            return {3, 4};
        case parser::Operator::EXP:
            return {6, 5};
        default:
            throw std::runtime_error("Unknown operator for binding power");
        }
    }

    int str2int(char *&str) {
        int value = 0;
        while (*str && isdigit(*str)) {
            value = value * 10 + (*str - '0');
            ++str;
        }
        return value;
    }

    uint8_t char2op(char c) {
        switch (c) {
        case '+':
            return static_cast<uint8_t>(parser::Operator::ADD);
        case '-':
            return static_cast<uint8_t>(parser::Operator::SUB);
        case '*':
            return static_cast<uint8_t>(parser::Operator::MUL);
        case '/':
            return static_cast<uint8_t>(parser::Operator::DIV);
        case '^':
            return static_cast<uint8_t>(parser::Operator::EXP);
        case '(':
            return static_cast<uint8_t>(parser::Operator::PAL);
        case ')':
            return static_cast<uint8_t>(parser::Operator::PAR);
        default:
            throw std::runtime_error(std::string("Unknown operator ") + c);
        }
    }

    // Convert a string to a vector of tokens using str2int and char2op
    std::vector<parser::Token> str2tokens(char *str) {
        std::vector<parser::Token> tokens;
        uint8_t lpar = 0; // left parenthesis count
        while (*str) {
            if (isspace(*str)) {
                ++str;
                continue;
            }
            if (isdigit(*str)) {
                int value = str2int(str);
                tokens.emplace_back(value);
            } else {
                auto op = static_cast<parser::Operator>(char2op(*str));

                if (op == parser::Operator::PAL) {
                    ++lpar;
                    ++str;
                    continue;
                } else if (op == parser::Operator::PAR) {
                    if (lpar == 0)
                        throw std::runtime_error("Unmatched right parenthesis");
                    --lpar;
                    ++str;
                    continue;
                } else {
                    auto [bpl, bpr] = get_bp_infix(op);
                    tokens.emplace_back(static_cast<int>(op),
                                        bpl + lpar * K_BP_DELTA,
                                        bpr + lpar * K_BP_DELTA);
                    ++str;
                }
            }
        }
        if (lpar > 0)
            throw std::runtime_error("Unmatched left parenthesis");
        return tokens;
    }
} // namespace

parser::Raw::Raw(char *str) {
    tokens = str2tokens(str);
    // Reverse the order for easier parsing
    std::reverse(tokens.begin(), tokens.end());
}

std::shared_ptr<parser::Expr> parser::Raw::parse(uint8_t min_bp) {
    std::shared_ptr<parser::Expr> lhs, rhs;
    parser::Token tok = this->next();
    lhs = std::make_shared<parser::Expr>(tok.value);

    while (true) {
        parser::Token next = peek();
        if (next.type == parser::TokenType::NONE)
            break;

        if (next.type != parser::TokenType::OP)
            throw std::runtime_error("Expected operator");

        parser::Operator op = static_cast<parser::Operator>(next.value);
        uint8_t l_bp = next.lbp;
        uint8_t r_bp = next.rbp;
        if (l_bp < min_bp)
            break;

        this->next(); // consume operator
        rhs = parse(r_bp);
        lhs = std::make_shared<parser::Expr>(op, lhs, rhs);
    }

    return lhs;
}
