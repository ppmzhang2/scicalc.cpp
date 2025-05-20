#pragma once

#include <memory>
#include <vector>

namespace expr {

    struct Atom {
        bool sign;
        float value; // IMP: enum class Sign
    };

    struct Token {
        struct Op {
            uint8_t v;   // IMP: enum class Sign
            uint8_t lbp; // left binding power
            uint8_t rbp; // right binding power
            char padding[1];
        };

        bool isop;
        union {
            float num; // number
            Op op;     // operator
        };

        Token(float v) : isop(false), num(v) {}
        Token(uint8_t v, uint8_t lbp, uint8_t rbp)
            : isop(true), op{v, lbp, rbp, {0}} {}

        std::string ToStr() const {
            if (isop)
                return std::string("Op: " + std::to_string(op.v) + " LBP" +
                                   std::to_string(op.lbp) + ", " + " RBP" +
                                   std::to_string(op.rbp));
            return std::string("Num: " + std::to_string(num));
        }
    };

    struct Chain {
        uint8_t state; // IMP: enum class ChainState
        uint8_t op;    // IMP: enum class Sign
        uint8_t lbp;
        uint8_t rbp;
        float lhs;
        std::shared_ptr<Chain> rhs;

        Chain(uint8_t s, uint8_t o, uint8_t lbp, uint8_t rbp, float n,
              std::shared_ptr<Chain> z)
            : state(s), op(o), lbp(lbp), rbp(rbp), lhs(n), rhs(std::move(z)) {}

        std::string ToStr() const;

        float Step() const;
    };

    std::vector<char *> split_str(const char *);

    void free_chrs(std::vector<char *> &);

    std::vector<Atom> chrs2atoms(const std::vector<char *> &);

    std::vector<Token> atoms2tokens(const std::vector<Atom> &);

    std::shared_ptr<Chain> tokens2chain(std::vector<Token> &,
                                        const std::shared_ptr<Chain> &);

    std::shared_ptr<Chain> reduce(const std::shared_ptr<Chain> &);

    float eval(const std::shared_ptr<Chain> &);

    float eval(const char *str);
} // namespace expr
