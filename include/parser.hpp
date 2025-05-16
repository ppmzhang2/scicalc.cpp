#pragma once

#include <vector>

namespace parser {

    enum class TknType : uint8_t {
        NONE = 0,
        NUM,
        OP,
    };

    // Both operatoers (left, right and infix) and helpers (parentheses)
    enum class Sign : uint8_t {
        NONE = 0,
        PAL, // (, helper only
        PAR, // ), helper only
        PI,  // pi
        E,   // e
        FCT, // !
        LN,  // ln
        ADD, // +
        SUB, // -
        MUL, // *
        DIV, // /
        EXP, // ^
    };

    struct Token {
        int value; // either a number or a sign
        TknType type;
        uint8_t lbp;
        uint8_t rbp;
        uint8_t padding;

        Token() : value(0), type(TknType::NONE), lbp(0), rbp(0), padding(0) {}
        Token(int v)
            : value(v), type(TknType::NUM), lbp(0), rbp(0), padding(0) {}
        Token(int v, uint8_t lbp, uint8_t rbp)
            : value(v), type(TknType::OP), lbp(lbp), rbp(rbp), padding(0) {}

        std::string ToStr() const {
            switch (type) {
            case TknType::NUM:
                return std::string("Num: " + std::to_string(value));
            case TknType::OP:
                return std::string("Op: " + std::to_string(value) + " LBP" +
                                   std::to_string(lbp) +
                                   ", "
                                   " RBP" +
                                   std::to_string(rbp));
            default:
                return std::string("");
            }
        }
    };

    struct Expr {
        char padding[3];
        Sign op;
        int value;
        std::shared_ptr<Expr> lhs;
        std::shared_ptr<Expr> rhs;

        Expr(int v)
            : padding{0}, op(Sign::NONE), value(v), lhs(nullptr), rhs(nullptr) {
        }

        Expr(Sign o)
            : padding{0}, op(o), value(0), lhs(nullptr), rhs(nullptr) {}

        Expr(Sign o, std::shared_ptr<Expr> l)
            : padding{0}, op(o), value(0), lhs(std::move(l)), rhs(nullptr) {}

        Expr(Sign o, std::shared_ptr<Expr> l, std::shared_ptr<Expr> r)
            : padding{0}, op(o), value(0), lhs(std::move(l)),
              rhs(std::move(r)) {}

        float eval() const;
    };

    std::vector<std::pair<parser::TknType, int>> str2pairs(const char *);

    std::vector<parser::Token>
    pairs2tokens(const std::vector<std::pair<parser::TknType, int>> &);

    std::shared_ptr<Expr> parse(std::vector<Token> &, const uint8_t);

} // namespace parser
