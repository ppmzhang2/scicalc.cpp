#pragma once

#include <cstdint>
#include <vector>

namespace parser {

    enum class TokenType : uint8_t {
        NONE = 0,
        NUM,
        OP,
    };

    enum class Sign : uint8_t {
        NONE = 0,
        ADD, // +
        SUB, // -
        MUL, // *
        DIV, // /
        EXP, // ^
        PAL, // (
        PAR, // )
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
        Sign op;
        int value;
        std::shared_ptr<Expr> lhs;
        std::shared_ptr<Expr> rhs;

        Expr(int v) : op(Sign::NONE), value(v), lhs(nullptr), rhs(nullptr) {}

        Expr(Sign o, std::shared_ptr<Expr> l, std::shared_ptr<Expr> r)
            : op(o), lhs(std::move(l)), rhs(std::move(r)) {}

        float eval() const {
            if (op == Sign::NONE)
                return value;

            float l = lhs->eval();
            float r = rhs->eval();

            switch (op) {
            case Sign::ADD:
                return l + r;
            case Sign::SUB:
                return l - r;
            case Sign::MUL:
                return l * r;
            case Sign::DIV:
                return l / r;
            case Sign::EXP:
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
