#pragma once

#include <cstdint>
#include <vector>

namespace parser {

    enum class TokenType : uint8_t {
        NONE = 0,
        NUM,
        OP,
    };

    enum class Operator : uint8_t {
        NONE = 0,
        ADD, // +
        SUB, // -
        MUL, // *
        DIV, // /
        EXP, // ^
        PAL,
        PAR,
    };

    struct Token {
        int value;
        TokenType type;
        uint8_t lbp;
        uint8_t rbp;
        uint8_t padding;

        Token() : value(0), type(TokenType::NONE), lbp(0), rbp(0), padding(0) {}
        Token(int v)
            : value(v), type(TokenType::NUM), lbp(0), rbp(0), padding(0) {}
        Token(int v, uint8_t lbp, uint8_t rbp)
            : value(v), type(TokenType::OP), lbp(lbp), rbp(rbp), padding(0) {}
    };

    struct Expr {
        Operator op;
        int value;
        std::shared_ptr<Expr> lhs;
        std::shared_ptr<Expr> rhs;

        Expr(int v)
            : op(Operator::NONE), value(v), lhs(nullptr), rhs(nullptr) {}

        Expr(Operator o, std::shared_ptr<Expr> l, std::shared_ptr<Expr> r)
            : op(o), lhs(std::move(l)), rhs(std::move(r)) {}

        float eval() const {
            if (op == Operator::NONE)
                return value;

            float l = lhs->eval();
            float r = rhs->eval();

            switch (op) {
            case Operator::ADD:
                return l + r;
            case Operator::SUB:
                return l - r;
            case Operator::MUL:
                return l * r;
            case Operator::DIV:
                return l / r;
            case Operator::EXP:
                return std::pow(l, r);
            default:
                throw std::runtime_error("Unknown operator in eval");
            }
        }
    };

    class Raw {
      private:
        std::vector<Token> tokens;

      public:
        explicit Raw(char *str);

        Token next() {
            if (tokens.empty())
                return Token();
            Token tok = tokens.back();
            tokens.pop_back();
            return tok;
        }

        Token peek() const {
            if (tokens.empty())
                return Token();
            return tokens.back();
        }

        std::shared_ptr<Expr> parse(uint8_t min_bp);
    };

} // namespace parser
