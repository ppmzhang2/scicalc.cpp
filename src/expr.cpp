// Integer Calculator (enum-based Token with int values)
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
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
    static constexpr uint8_t kOpSize = 9;

    // Both operatoers (left, right and infix) and helpers (parentheses)
    // used by `Atom.value`
    //
    // NOTE:
    // - when adding new operators, make sure to update
    //   - `kOpSize`
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
        UAD,              // unary add (right associative)
        USB,              // unary sub (right associative)
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
            [](float a, float) { return a; },                // UAD
            [](float a, float) { return -a; },               // USB
        };

    static constexpr std::array<std::pair<uint8_t, uint8_t>, kOpSize> kMapOp2Bp{
        std::make_pair(6, 0), // factorial (kMinSignOp)
        std::make_pair(0, 5), // log
        std::make_pair(1, 1), // +
        std::make_pair(1, 1), // -
        std::make_pair(2, 2), // *
        std::make_pair(2, 2), // /
        std::make_pair(4, 3), // left-skewed
        std::make_pair(0, 1), // unary add
        std::make_pair(0, 1), // unary sub
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

    enum class ChainState : uint8_t {
        NUL_NUL_NUL = 0, // represent nullptr (NOMOD, expect num / OPL)

        // All `XXX_XXX_NUL` are last node in the chain, which should be
        // evaluated as merely an operand and thus can only have the following
        // three valid states

        LHS_NUL_NUL, // null RHS, only num (NOMOD | expect OPR / OPI)
        NUL_OPL_NUL, // null RHS, opl only (MOD | expect num / OPL)
        LHS_OPL_NUL, // null RHS, num + opl (NOMOD, expect OPR / OPI)

        NUL_OPL_RHS, // not-null RHS, opl only (MOD | expect num / OPL)
        NUL_OPR_RHS, // not-null RHS, opr only (NOMOD, expect OPR / OPI)
        NUL_OPI_RHS, // not-null RHS, opi only (MOD | expect num / OPL)
        LHS_OPL_RHS, // not-null RHS, opl + num (NOMOD, expect OPR / OPI)
        LHS_OPI_RHS, // not-null RHS, opi + num (NOMOD, expect OPR / OPI)
    };

    ChainState chain_state(const std::shared_ptr<expr::Chain> &head) {
        if (head == nullptr)
            return ChainState::NUL_NUL_NUL;
        return static_cast<ChainState>(head->state);
    }

    static constexpr std::array<bool, 9> kMapChain2NoMod{
        true,  // NUL
        true,  // LHS_NUL_NUL
        false, // NUL_OPL_NUL
        true,  // LHS_OPL_NUL
        false, // NUL_OPL_RHS
        true,  // NUL_OPR_RHS
        false, // NUL_OPI_RHS
        true,  // LHS_OPL_RHS
        true,  // LHS_OPI_RHS
    };

    // true if a new node is needed (NOMOD), otherwise current node can be
    // modified (MOD)
    bool chain_nomod(const std::shared_ptr<expr::Chain> &head) {
        if (head == nullptr)
            return true;
        try {
            return kMapChain2NoMod.at(head->state);
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Unknown chain state");
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
    if (chrs.empty())
        throw std::runtime_error("Empty string");

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

    // Change the starting + or - sign to unary
    if (atoms[0].sign && atoms[0].value == static_cast<float>(Sign::SUB)) {
        atoms[0].value = static_cast<float>(Sign::USB);
    } else if (atoms[0].sign &&
               atoms[0].value == static_cast<float>(Sign::ADD)) {
        atoms[0].value = static_cast<float>(Sign::UAD);
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

// NOTE:
//   the implementation should NOT create ambiguous chain nodes i.e. nodes with
//   both valid num and op values
//   - a num node should always has `op = Sign::NONE`
//   - an op node should always has `num = kFNan`
//   This ensures that the chain functions can use function `sign2optype`
//   freely without checking first either it is an operator or a number
std::shared_ptr<expr::Chain>
expr::tokens2chain(std::vector<expr::Token> &tokens,
                   const std::shared_ptr<expr::Chain> &head) {
    if (tokens.empty() && !chain_nomod(head))
        // Error 1: e.g. starting with a infix / left associative operator
        throw std::runtime_error("Incomplete expression");
    if (tokens.empty())
        return head;

    auto tkn = tokens.back();
    tokens.pop_back();

    // CASE 1: enumerate NUL_NUL_NUL, different from the other NOMOD cases
    if (chain_state(head) == ChainState::NUL_NUL_NUL && tkn.isop &&
        sign2optype(tkn.op.v) == SignType::OPL) {
        const auto c =
            expr::Chain(static_cast<uint8_t>(ChainState::NUL_OPL_NUL), tkn.op.v,
                        tkn.op.lbp, tkn.op.rbp, kFNan, nullptr);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(c));
    }
    if (chain_state(head) == ChainState::NUL_NUL_NUL && !tkn.isop) {
        const auto c =
            expr::Chain(static_cast<uint8_t>(ChainState::LHS_NUL_NUL), 0, 0, 0,
                        tkn.num, nullptr);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(c));
    }
    if (chain_state(head) == ChainState::NUL_NUL_NUL) {
        throw std::runtime_error("Unfinished expression");
    }

    // CASE 2: enumerate NOMOD
    if (chain_nomod(head) && tkn.isop &&
        sign2optype(tkn.op.v) == SignType::OPR) {
        const auto c =
            expr::Chain(static_cast<uint8_t>(ChainState::NUL_OPR_RHS), tkn.op.v,
                        tkn.op.lbp, tkn.op.rbp, kFNan, head);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(c));
    }
    if (chain_nomod(head) && tkn.isop &&
        sign2optype(tkn.op.v) == SignType::OPI) {
        const auto c =
            expr::Chain(static_cast<uint8_t>(ChainState::NUL_OPI_RHS), tkn.op.v,
                        tkn.op.lbp, tkn.op.rbp, kFNan, head);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(c));
    }
    if (chain_nomod(head)) {
        throw std::runtime_error("Dangling NUM / OPL");
    }

    // CASE 3: enumerate MOD
    if (!chain_nomod(head) && tkn.isop &&
        // all MOD can prepend opl
        sign2optype(tkn.op.v) == SignType::OPL) {
        const auto c =
            expr::Chain(static_cast<uint8_t>(ChainState::NUL_OPL_RHS), tkn.op.v,
                        tkn.op.lbp, tkn.op.rbp, kFNan, head);
        return tokens2chain(tokens, std::make_shared<expr::Chain>(c));
    }
    if (chain_state(head) == ChainState::NUL_OPL_NUL && !tkn.isop) {
        // modify LHS
        head->lhs = tkn.num;
        head->state = static_cast<uint8_t>(ChainState::LHS_OPL_NUL);
        return tokens2chain(tokens, head);
    }
    if (chain_state(head) == ChainState::NUL_OPL_RHS && !tkn.isop) {
        // modify LHS
        head->lhs = tkn.num;
        head->state = static_cast<uint8_t>(ChainState::LHS_OPL_RHS);
        return tokens2chain(tokens, head);
    }
    if (chain_state(head) == ChainState::NUL_OPI_RHS && !tkn.isop) {
        // modify LHS
        head->lhs = tkn.num;
        head->state = static_cast<uint8_t>(ChainState::LHS_OPI_RHS);
        return tokens2chain(tokens, head);
    }
    if (!chain_nomod(head))
        throw std::runtime_error("Dangling OPR / OPI");

    // CASE 4: catch all other invalid cases (if there is any)
    throw std::runtime_error("Invalid token");
}

std::string expr::Chain::ToStr() const {
    std::string str;
    str += "Chain(";
    if (op == static_cast<uint8_t>(Sign::NONE)) {
        str += "num=" + std::to_string(lhs) + ")";
    } else {
        str += "op=" + std::to_string(op) + ", num=" + std::to_string(lhs) +
               ", LBP=" + std::to_string(lbp) + ", RBP=" + std::to_string(rbp);
        if (rhs) {
            str += ", next=" + rhs->ToStr();
        } else {
            str += ", next=null";
        }
    }
    return str;
}

// Evaluate the expression of the chain head
// TODO:
// - handle the only-num case?
// - is this function necessary?
float expr::Chain::Step() const {
    if (rhs == nullptr && sign2optype(op) == SignType::OPL) {
        return kMapOp2Fn[op - kMinSignOp](lhs, kFDummy);
    }
    if (rbp >= rhs->lbp && sign2optype(op) == SignType::OPR &&
        !std::isnan(rhs->lhs)) {
        return kMapOp2Fn[op - kMinSignOp](rhs->lhs, kFDummy);
    }
    if (rbp >= rhs->lbp && sign2optype(op) == SignType::OPI &&
        !std::isnan(lhs) && !std::isnan(rhs->lhs)) {
        return kMapOp2Fn[op - kMinSignOp](lhs, rhs->lhs);
    }
    if (lbp >= 0 && sign2optype(op) == SignType::OPL && !std::isnan(lhs)) {
        return kMapOp2Fn[op - kMinSignOp](lhs, kFDummy);
    }
    throw std::runtime_error("Invalid chain: cannot step");
}

std::shared_ptr<expr::Chain>
expr::reduce(const std::shared_ptr<expr::Chain> &car) {
    if (car->rhs == nullptr && sign2optype(car->op) == SignType::NONE) {
        return car;
    }
    if (car->rhs == nullptr && sign2optype(car->op) == SignType::OPL) {
        return std::make_shared<expr::Chain>(
            expr::Chain(static_cast<uint8_t>(ChainState::LHS_NUL_NUL), 0, 0, 0,
                        car->Step(), nullptr));
    }
    auto cdr = car->rhs;
    try {
        cdr->lhs = car->Step();
        return cdr;
    } catch (const std::runtime_error &) {
        auto c = expr::Chain(car->state, car->op, car->lbp, car->rbp, car->lhs,
                             reduce(cdr));
        return std::make_shared<expr::Chain>(c);
    }
}

float expr::eval(const std::shared_ptr<Chain> &chain) {
    auto c = chain;
    while (true) {
        if (c->rhs == nullptr && sign2optype(c->op) == SignType::NONE) {
            break;
        }
        c = reduce(c);
    }
    return c->lhs;
}

float expr::eval(const char *str) {
    auto chrs = split_str(str);
    auto atoms = chrs2atoms(chrs);
    free_chrs(chrs);
    auto tokens = atoms2tokens(atoms);
    auto chain = tokens2chain(tokens, nullptr);
    return eval(chain);
}
