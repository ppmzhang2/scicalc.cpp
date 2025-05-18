// Integer Calculator (enum-based Token with int values)

#include <unordered_map>

#include "expr.hpp"

namespace {

    static constexpr float kFNan = std::numeric_limits<float>::quiet_NaN();
    static constexpr float kFDummy = 0.0f;

    // binding power delta for parenthesis
    static constexpr int8_t kBpDelta = 10;

    // start of helpers
    static constexpr uint8_t kMinSignHelper = 1;
    // start of constants
    static constexpr uint8_t kMinSignConst = 21;
    // start of operators (unary / binary associative)
    static constexpr uint8_t kMinSignOp = 101;
    static constexpr uint8_t kOpSize = 7;

    // Both operatoers (left, right and infix) and helpers (parentheses)
    // used by `Atom.value`
    //
    // NOTE:
    // - when adding new operators, make sure to update
    //   - `kMapOp2Fn`
    //   - `kMapOp2Bp`
    // - adding constants: update `kMapConst2Real`
    enum class Sign : uint8_t {
        NONE = 0,
        // helpers
        PAL = kMinSignHelper, // (
        PAR,                  // )
        // constants
        PI = kMinSignConst, // pi
        E,                  // e
        // operators, size = kOpSize
        FCT = kMinSignOp, // factorial (left associative)
        LOG,              // log (right associative)
        ADD,              // +
        SUB,              // -
        MUL,              // *
        DIV,              // /
        EXP,              // ^
    };

    enum class SignType : uint8_t {
        NONE = 0,
        CON = 1, // constant
        OPL = 2, // left associative unary operator e.g. !
        OPR = 3, // right associative unary operator e.g. ln
        OPI = 4, // infix operator
    };

    // map constant signs
    static constexpr std::array<float, 2> kMapConst2Real{
        3.14159265358979323846f, // PI (kMinSignConst)
        2.71828182845904523536f, // E
    };

    // map Operator to function
    static const std::array<float (*)(const float, const float), kOpSize>
        kMapOp2Fn{
            [](float a, float) { return std::tgamma(a + 1); }, // kMinSignOpU
            [](float a, float) { return std::log(a); },        // LOG
            [](float a, float b) { return a + b; }, // ADD (kMinSignOpI)
            [](float a, float b) { return a - b; }, // SUB
            [](float a, float b) { return a * b; }, // MUL
            [](float a, float b) { return a / b; }, // DIV
            [](float a, float b) { return std::pow(a, b); }, // EXP
        };

    static constexpr std::array<std::pair<uint8_t, uint8_t>, kOpSize> kMapOp2Bp{
        std::make_pair(6, 0), // factorial (kMinSignOp)
        std::make_pair(0, 5), // log
        std::make_pair(1, 1), // +
        std::make_pair(1, 1), // -
        std::make_pair(2, 2), // *
        std::make_pair(2, 2), // /
        std::make_pair(4, 3), // left-skewed
    };

    // Binding power for operators
    static std::pair<uint8_t, uint8_t> get_bp(uint8_t op) {
        try {
            return kMapOp2Bp.at(op - kMinSignOp);
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Unknown operator");
        }
    }

    static constexpr SignType sign2optype(const uint8_t op) {
        if (op >= kMinSignOp) {
            auto [bpl, bpr] = get_bp(op);

            if (bpl == 0 && bpr == 0)
                return SignType::NONE;
            if (bpl == 0)
                return SignType::OPR;
            if (bpr == 0)
                return SignType::OPL;
            return SignType::OPI;
        }

        if (op >= kMinSignConst)
            return SignType::CON;
        return SignType::NONE;
    }

    float digit2int(const char *str) {
        float value = 0;
        float decimal = 0.1f;
        bool is_decimal = false;

        while (*str) {
            if (*str == '.') {
                is_decimal = true;
                str++;
            }

            if (is_decimal) {
                value += (*str - '0') * decimal;
                decimal *= 0.1f;
            } else {
                value = value * 10 + (*str - '0');
            }
            str++;
        }
        return value;
    }

    uint8_t alpha2sign(const char *str) {
        if (strncmp(str, "ln", 2) == 0) {
            return static_cast<uint8_t>(Sign::LOG);
        } else if (strncmp(str, "pi", 2) == 0) {
            return static_cast<uint8_t>(Sign::PI);
        } else if (strncmp(str, "e", 1) == 0) {
            return static_cast<uint8_t>(Sign::E);
        }
        throw std::runtime_error("Unknown function");
    }

    static const std::unordered_map<char, Sign> kMapChar2Sign{
        {'!', Sign::FCT}, {'+', Sign::ADD}, {'-', Sign::SUB}, {'*', Sign::MUL},
        {'/', Sign::DIV}, {'^', Sign::EXP}, {'(', Sign::PAL}, {')', Sign::PAR},
    };

    uint8_t char2sign(const char c) {
        try {
            return static_cast<uint8_t>(kMapChar2Sign.at(c));
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Unknown operator");
        }
    }
} // namespace

// Split a string into substrings, each containing a single token, either a
// number or an operator
// NOTE: free() the tokens after use
std::vector<char *> expr::split_str(const char *str) {
    std::vector<char *> tokens;
    const char *start = str;

    while (*start) {
        while (isspace(*start))
            start++; // Skip whitespace
        if (*start == '\0')
            break;

        const char *end = start;

        if (isdigit(*start) || (*start == '.' && isdigit(*(start + 1)))) {
            // Parse number
            while (isdigit(*end) || *end == '.')
                end++;
        } else if (isalpha(*start)) {
            // Parse identifier (e.g., ln, sin)
            while (isalpha(*end))
                end++;
        } else {
            // Single character symbol (e.g., +, -, *, /, ^, !, (, ))
            end++;
        }

        // Allocate and copy token
        size_t len = end - start;
        char *token = (char *)malloc(len + 1);
        strncpy(token, start, len);
        token[len] = '\0';
        tokens.push_back(token);

        start = end;
    }

    return tokens;
}

// Free the memory allocated for the strings in the vector
void expr::free_chrs(std::vector<char *> &chrs) {
    for (auto &chr : chrs) {
        free(chr);
    }
    // Clear the vector
    chrs.clear();
}

std::vector<expr::Atom> expr::chrs2atoms(const std::vector<char *> &chrs) {
    std::vector<expr::Atom> atoms;

    for (const auto &token : chrs) {
        if (isdigit(*token)) {
            int val = digit2int(token);
            atoms.emplace_back(false, static_cast<float>(val));
        } else if (isalpha(*token)) {
            uint8_t val = alpha2sign(token);
            atoms.emplace_back(true, static_cast<float>(val));
        } else {
            uint8_t val = char2sign(*token);
            atoms.emplace_back(true, static_cast<float>(val));
        }
    }

    return atoms;
}

// Convert a string to a vector of tokens using str2int and char2op
std::vector<expr::Token>
expr::atoms2tokens(const std::vector<expr::Atom> &pairs) {
    std::vector<expr::Token> tokens;
    uint8_t lpar = 0; // left parenthesis count

    for (const auto &pair : pairs) {
        if (!pair.sign) {
            tokens.emplace_back(pair.value);
        } else if (pair.sign && sign2optype(static_cast<uint8_t>(pair.value)) ==
                                    SignType::CON) {
            // Treat nullary operators as constants
            const float val = kMapConst2Real[pair.value - kMinSignConst];
            tokens.emplace_back(val);
        } else if (pair.sign && pair.value == static_cast<int>(Sign::PAL)) {
            ++lpar;
        } else if (pair.sign && pair.value == static_cast<int>(Sign::PAR)) {
            if (lpar == 0)
                throw std::runtime_error("Unmatched right parenthesis");
            --lpar;
        } else {
            auto [bpl, bpr] = get_bp(pair.value);
            // OPR will always have 0 left binding power
            bpl = (bpl == 0) ? 0 : bpl + lpar * kBpDelta;
            // OPL will always have 0 right binding power
            bpr = (bpr == 0) ? 0 : bpr + lpar * kBpDelta;
            tokens.emplace_back(pair.value, bpl, bpr);
        }
    }
    if (lpar > 0)
        throw std::runtime_error("Unmatched left parenthesis");
    return tokens;
}

std::shared_ptr<expr::Chain>
expr::tokens2chain(std::vector<expr::Token> &tokens,
                   const std::shared_ptr<expr::Chain> &zipper,
                   const bool done) {
    if (tokens.empty())
        return zipper;
    auto tkn = tokens.back();
    tokens.pop_back();

    // 1. For left associative operators, always create a new zipper,
    // regardless of the `done` flag
    if (tkn.isop && sign2optype(tkn.op.v) == SignType::OPL) {
        const auto z =
            expr::Chain(tkn.op.v, tkn.op.lbp, tkn.op.rbp, kFNan, zipper);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(z), false);
    }

    // either OPR or OPI
    if (done && tkn.isop) {
        const auto z =
            expr::Chain(tkn.op.v, tkn.op.lbp, tkn.op.rbp, kFNan, zipper);
        // 2. if tkn.op is right associative, this zipper is done
        // 3. if tkn.op is infix, fill the lhs in the next round
        bool done_ = (sign2optype(tkn.op.v) == SignType::OPR) ? true : false;
        return tokens2chain(tokens, std::make_shared<expr::Chain>(z), done_);
    }

    // 4. initial call i.e. add the last token (num)
    if (done && !tkn.isop) {
        const auto z = expr::Chain(static_cast<uint8_t>(Sign::NONE), 0, 0,
                                   tkn.num, zipper);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(z), true);
    }

    // 5. add the lhs of an infix / left associative operator
    if (!done && !tkn.isop) {
        zipper->num = tkn.num;
        return tokens2chain(tokens, zipper, true);
    }

    throw std::runtime_error("Invalid token");
}

std::string expr::Chain::ToStr() const {
    std::string str;
    str += "Chain(";
    if (op == static_cast<uint8_t>(Sign::NONE)) {
        str += "num=" + std::to_string(num) + ")";
    } else {
        str += "op=" + std::to_string(op) + ", num=" + std::to_string(num) +
               ", LBP=" + std::to_string(lbp) + ", RBP=" + std::to_string(rbp);
        if (next) {
            str += ", next=" + next->ToStr();
        } else {
            str += ", next=null";
        }
    }
    return str;
}

// Evaluate the expression of the chain head
float expr::Chain::Step() const {
    if (next == nullptr && sign2optype(op) == SignType::OPL) {
        return kMapOp2Fn[op - kMinSignOp](num, kFDummy);
    }
    if (rbp >= next->lbp && sign2optype(op) == SignType::OPR &&
        !std::isnan(next->num)) {
        return kMapOp2Fn[op - kMinSignOp](next->num, kFDummy);
    }
    if (rbp >= next->lbp && sign2optype(op) == SignType::OPI &&
        !std::isnan(num) && !std::isnan(next->num)) {
        return kMapOp2Fn[op - kMinSignOp](num, next->num);
    }
    if (lbp >= 0 && sign2optype(op) == SignType::OPL && !std::isnan(num)) {
        return kMapOp2Fn[op - kMinSignOp](num, kFDummy);
    }
    throw std::runtime_error("Invalid chain: cannot step");
}

std::shared_ptr<expr::Chain>
expr::reduce(const std::shared_ptr<expr::Chain> &zp) {
    if (zp->next == nullptr && sign2optype(zp->op) == SignType::NONE) {
        return zp;
    }
    if (zp->next == nullptr && sign2optype(zp->op) == SignType::OPL) {
        return std::make_shared<expr::Chain>(
            expr::Chain(0, 0, 0, zp->Step(), nullptr));
    }
    auto z_nxt = zp->next;
    try {
        z_nxt->num = zp->Step();
        return z_nxt;
    } catch (const std::runtime_error &) {
        auto z_ = expr::Chain(zp->op, zp->lbp, zp->rbp, zp->num, reduce(z_nxt));
        return std::make_shared<expr::Chain>(z_);
    }
}

float expr::eval(const std::shared_ptr<Chain> &zp) {
    auto z = zp;
    while (true) {
        if (z->next == nullptr && sign2optype(z->op) == SignType::NONE) {
            break;
        }
        z = reduce(z);
    }
    return z->num;
}
