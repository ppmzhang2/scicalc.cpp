// Integer Calculator (enum-based Token with int values)
#include <functional>

#include "parser.hpp"

namespace {

    static constexpr float kFNan = std::numeric_limits<float>::quiet_NaN();

    // binding power delta for parenthesis
    static constexpr int8_t kBpDelta = 10;

    // start of helpers
    static constexpr uint8_t kMinSignHelper = 1;
    // start of constants
    static constexpr uint8_t kMinSignConst = 21;
    // start of (unary) operators (left associative)
    static constexpr uint8_t kMinSignOpL = 101;
    // start of unary operators (right associative)
    static constexpr uint8_t kMinSignOpR = 121;
    // start of infix operators
    static constexpr uint8_t kMinSignOpI = 201;

    // Both operatoers (left, right and infix) and helpers (parentheses)
    // used by `Atom.value`
    enum class Sign : uint8_t {
        NONE = 0,
        // helpers
        PAL = kMinSignHelper, // (
        PAR,                  // )
        // constants
        PI = kMinSignConst, // pi
        E,                  // e
        // unary operators
        FCT = kMinSignOpL, // factorial (left associative)
        LOG = kMinSignOpR, // log (right associative)
        // infix operators
        ADD = kMinSignOpI, // +
        SUB,               // -
        MUL,               // *
        DIV,               // /
        EXP,               // ^
    };

    enum class SignType : uint8_t {
        NONE = 0,
        CON = 1, // constant
        OPL = 2, // left associative unary operator e.g. !
        OPR = 3, // right associative unary operator e.g. ln
        OPI = 4, // infix operator
    };

    static const std::unordered_map<char, Sign> kMapChar2Sign{
        {'!', Sign::FCT}, {'+', Sign::ADD}, {'-', Sign::SUB}, {'*', Sign::MUL},
        {'/', Sign::DIV}, {'^', Sign::EXP}, {'(', Sign::PAL}, {')', Sign::PAR},
    };

    // map constant signs
    static constexpr std::array<float, 2> kMapConst2Real{
        3.14159265358979323846f, // PI (kMinSignConst)
        2.71828182845904523536f, // E
    };

    // map unary Operator (left associative) to function
    static const std::array<std::function<float(const float)>, 1> kMapOpl2Fn{
        [](const float a) { return std::tgamma(a + 1); }, // FCT (kMinSignOpL)
    };

    // map unary Operator (right associative) to function
    static const std::array<std::function<float(const float)>, 1> kMapOpr2Fn{
        [](const float a) { return std::log(a); }, // LOG (kMinSignOpR)
    };

    // map infix Operator to function
    static const std::array<std::function<float(const float, const float)>, 5>
        kMapOpi2Fn{
            [](float a, float b) { return a + b; }, // ADD (kMinSignOpI)
            [](float a, float b) { return a - b; }, // SUB
            [](float a, float b) { return a * b; }, // MUL
            [](float a, float b) { return a / b; }, // DIV
            [](float a, float b) { return std::pow(a, b); }, // EXP
        };

    static const std::unordered_map<Sign, std::pair<uint8_t, uint8_t>>
        kMapOp2Bp{
            // infix operators
            {Sign::ADD, {1, 1}}, // +
            {Sign::SUB, {1, 1}}, // -
            {Sign::MUL, {2, 2}}, // *
            {Sign::DIV, {2, 2}}, // /
            {Sign::EXP, {4, 3}}, // left-skewed
            {Sign::LOG, {0, 5}}, // right associative
            {Sign::FCT, {6, 0}}, // left associative
        };

    static constexpr SignType sign2optype(const uint8_t op) {
        if (op >= kMinSignOpI)
            return SignType::OPI;
        if (op >= kMinSignOpR)
            return SignType::OPR;
        if (op >= kMinSignOpL)
            return SignType::OPL;
        if (op >= kMinSignConst)
            return SignType::CON;
        return SignType::NONE;
    }

    // Binding power for operators
    std::pair<uint8_t, uint8_t> get_bp(Sign op) {
        try {
            return kMapOp2Bp.at(op);
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Unknown operator");
        }
    }

    int digit2int(char *&str) {
        int value = 0;
        while (*str && isdigit(*str)) {
            value = value * 10 + (*str - '0');
            ++str;
        }
        return value;
    }

    Sign alpha2sign(char *&str) {
        if (strncmp(str, "e", 1) == 0) {
            str += 1;
            return Sign::E;
        } else if (strncmp(str, "pi", 2) == 0) {
            str += 2;
            return Sign::PI;
        } else if (strncmp(str, "ln", 2) == 0) {
            str += 2;
            return Sign::LOG;
        }
        throw std::runtime_error("Unknown function");
    }

    Sign char2sign(char c) {
        try {
            return kMapChar2Sign.at(c);
        } catch (const std::out_of_range &) {
            throw std::runtime_error("Unknown operator");
        }
    }
} // namespace

std::vector<parser::Atom> parser::str2atoms(const char *str) {
    std::vector<parser::Atom> tokens;
    char *p = const_cast<char *>(str);
    int val;
    while (*p) {
        if (isspace(*p)) {
            ++p;
            continue;
        } else if (isdigit(*p)) {
            val = digit2int(p);
            tokens.emplace_back(false, val);
        } else if (isalpha(*p)) {
            val = static_cast<int>(alpha2sign(p));
            tokens.emplace_back(true, val);
        } else {
            val = static_cast<int>(char2sign(*p));
            tokens.emplace_back(true, val);
            ++p;
        }
    }
    return tokens;
}

// Convert a string to a vector of tokens using str2int and char2op
std::vector<parser::Token>
parser::atoms2tokens(const std::vector<parser::Atom> &pairs) {
    std::vector<parser::Token> tokens;
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
            Sign op = static_cast<Sign>(pair.value);
            auto [bpl, bpr] = get_bp(op);
            // OPR will always have 0 left binding power
            bpl = (bpl == 0) ? 0 : bpl + lpar * kBpDelta;
            // OPL will always have 0 right binding power
            bpr = (bpr == 0) ? 0 : bpr + lpar * kBpDelta;
            tokens.emplace_back(static_cast<int>(op), bpl, bpr);
        }
    }
    if (lpar > 0)
        throw std::runtime_error("Unmatched left parenthesis");
    return tokens;
}

std::shared_ptr<parser::Chain>
parser::tokens2chain(std::vector<parser::Token> &tokens,
                     const std::shared_ptr<parser::Chain> &zipper,
                     const bool done) {
    if (tokens.empty())
        return zipper;
    auto tkn = tokens.back();
    tokens.pop_back();

    // 1. For left associative operators, always create a new zipper,
    // regardless of the `done` flag
    if (tkn.isop && sign2optype(tkn.op.v) == SignType::OPL) {
        const auto z =
            parser::Chain(tkn.op.v, tkn.op.lbp, tkn.op.rbp, kFNan, zipper);
        return tokens2chain(tokens, std::make_shared<parser::Chain>(z), false);
    }

    // either OPR or OPI
    if (done && tkn.isop) {
        const auto z =
            parser::Chain(tkn.op.v, tkn.op.lbp, tkn.op.rbp, kFNan, zipper);
        // 2. if tkn.op is right associative, this zipper is done
        // 3. if tkn.op is infix, fill the lhs in the next round
        bool done_ = (sign2optype(tkn.op.v) == SignType::OPR) ? true : false;
        return tokens2chain(tokens, std::make_shared<parser::Chain>(z), done_);
    }

    // 4. initial call i.e. add the last token (num)
    if (done && !tkn.isop) {
        const auto z = parser::Chain(static_cast<uint8_t>(Sign::NONE), 0, 0,
                                     tkn.num, zipper);
        return tokens2chain(tokens, std::make_shared<parser::Chain>(z), true);
    }

    // 5. add the lhs of an infix / left associative operator
    if (!done && !tkn.isop) {
        zipper->num = tkn.num;
        return tokens2chain(tokens, zipper, true);
    }

    throw std::runtime_error("Invalid token");
}

std::string parser::Chain::ToStr() const {
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

std::shared_ptr<parser::Chain>
parser::reduce(const std::shared_ptr<parser::Chain> &zp) {
    if (zp->next == nullptr && sign2optype(zp->op) == SignType::NONE) {
        return std::make_shared<parser::Chain>(
            parser::Chain(0, 0, 0, zp->num, nullptr));
    }
    if (zp->next == nullptr && sign2optype(zp->op) == SignType::OPL) {
        return std::make_shared<parser::Chain>(parser::Chain(
            0, 0, 0, kMapOpl2Fn[zp->op - kMinSignOpL](zp->num), nullptr));
    }
    auto z_nxt = zp->next;
    if (zp->rbp >= z_nxt->lbp && sign2optype(zp->op) == SignType::OPR &&
        !std::isnan(z_nxt->num)) {
        z_nxt->num = kMapOpr2Fn[zp->op - kMinSignOpR](z_nxt->num);
        return z_nxt;
    }
    if (zp->rbp >= z_nxt->lbp && sign2optype(zp->op) == SignType::OPI &&
        !std::isnan(zp->num) && !std::isnan(z_nxt->num)) {
        z_nxt->num = kMapOpi2Fn[zp->op - kMinSignOpI](zp->num, z_nxt->num);
        return z_nxt;
    }
    if (zp->lbp >= 0 && sign2optype(zp->op) == SignType::OPL &&
        !std::isnan(zp->num)) {
        z_nxt->num = kMapOpl2Fn[zp->op - kMinSignOpL](zp->num);
        return z_nxt;
    }
    auto z_ = parser::Chain(zp->op, zp->lbp, zp->rbp, zp->num, reduce(z_nxt));
    return std::make_shared<parser::Chain>(z_);
}

float parser::eval(const std::shared_ptr<Chain> &zp) {
    auto z = zp;
    while (true) {
        if (z->next == nullptr && sign2optype(z->op) == SignType::NONE) {
            break;
        }
        z = reduce(z);
    }
    return z->num;
}
