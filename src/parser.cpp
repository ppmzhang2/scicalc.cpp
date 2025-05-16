// Integer Calculator (enum-based Token with int values)
#include <functional>

#include "parser.hpp"

namespace {

    // binding power delta for parenthesis
    static constexpr int8_t K_BP_DELTA = 10;

    enum class OpType : uint8_t {
        NONE = 0,
        OPN = 1, // nullary operator e.g. pi, e
        OPL = 2, // left associative unary operator e.g. !
        OPR = 3, // right associative unary operator e.g. ln
        OPI = 4, // infix operator
    };

    // map Sign to OpType
    static const std::vector<uint8_t> map_sign2type{
        0, // NONE
        0, // PAL
        0, // PAR
        1, // PI
        1, // E
        2, // FCT
        3, // LN
        4, // ADD
        4, // SUB
        4, // MUL
        4, // DIV
        4, // EXP
    };

    // map nullary Operator to function
    static const std::vector<std::function<float()>> map_op2func0{
        nullptr,                                  // NONE
        nullptr,                                  // PAL
        nullptr,                                  // PAR
        []() { return 3.14159265358979323846f; }, // PI
        []() { return 2.71828182845904523536f; }, // E
        nullptr,                                  // FCT
        nullptr,                                  // LN
        nullptr,                                  // ADD
        nullptr,                                  // SUB
        nullptr,                                  // MUL
        nullptr,                                  // DIV
        nullptr,                                  // EXP
    };

    // map unary Operator to function
    static const std::vector<std::function<float(const float)>> map_op2func1{
        nullptr,                                    // NONE
        nullptr,                                    // PAL
        nullptr,                                    // PAR
        nullptr,                                    // PI
        nullptr,                                    // E
        [](float a) { return std::tgamma(a + 1); }, // FCT
        [](float a) { return std::log(a); },        // LN
        nullptr,                                    // ADD
        nullptr,                                    // SUB
        nullptr,                                    // MUL
        nullptr,                                    // DIV
        nullptr,                                    // EXP
    };

    // map infix Operator to function
    static const std::vector<std::function<float(const float, const float)>>
        map_op2func2{
            nullptr,                                         // NONE
            nullptr,                                         // PAL
            nullptr,                                         // PAR
            nullptr,                                         // PI
            nullptr,                                         // E
            nullptr,                                         // FCT
            nullptr,                                         // LN
            [](float a, float b) { return a + b; },          // ADD
            [](float a, float b) { return a - b; },          // SUB
            [](float a, float b) { return a * b; },          // MUL
            [](float a, float b) { return a / b; },          // DIV
            [](float a, float b) { return std::pow(a, b); }, // EXP
        };

    static constexpr uint8_t sign2optype(const parser::Sign op) {
        return map_sign2type[static_cast<uint8_t>(op)];
    }

    // Binding power for operators
    std::pair<uint8_t, uint8_t> get_bp(parser::Sign op) {
        switch (op) {
        case parser::Sign::E:
        case parser::Sign::PI:
            return {0, 0};
        case parser::Sign::ADD:
        case parser::Sign::SUB:
            return {1, 1};
        case parser::Sign::MUL:
        case parser::Sign::DIV:
            return {2, 2};
        case parser::Sign::EXP:
            return {4, 3};
        case parser::Sign::LN:
            return {0, 5};
        case parser::Sign::FCT:
            return {6, 0};

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

    uint8_t str2func(char *&str) {
        if (strncmp(str, "e", 1) == 0) {
            str += 1;
            return static_cast<uint8_t>(parser::Sign::E);
        } else if (strncmp(str, "pi", 2) == 0) {
            str += 2;
            return static_cast<uint8_t>(parser::Sign::PI);
        } else if (strncmp(str, "ln", 2) == 0) {
            str += 2;
            return static_cast<uint8_t>(parser::Sign::LN);
        }
        throw std::runtime_error("Unknown function");
    }

    uint8_t char2op(char c) {
        switch (c) {
        case '!':
            return static_cast<uint8_t>(parser::Sign::FCT);
        case '+':
            return static_cast<uint8_t>(parser::Sign::ADD);
        case '-':
            return static_cast<uint8_t>(parser::Sign::SUB);
        case '*':
            return static_cast<uint8_t>(parser::Sign::MUL);
        case '/':
            return static_cast<uint8_t>(parser::Sign::DIV);
        case '^':
            return static_cast<uint8_t>(parser::Sign::EXP);
        case '(':
            return static_cast<uint8_t>(parser::Sign::PAL);
        case ')':
            return static_cast<uint8_t>(parser::Sign::PAR);
        default:
            throw std::runtime_error(std::string("Unknown operator ") + c);
        }
    }

    parser::Token next(std::vector<parser::Token> &tokens) {
        if (tokens.empty())
            return parser::Token();
        parser::Token tok = tokens.back();
        tokens.pop_back();
        return tok;
    }

    parser::Token peek(const std::vector<parser::Token> &tokens) {
        if (tokens.empty())
            return parser::Token();
        return tokens.back();
    }

} // namespace

float parser::Expr::eval() const {
    if (op == Sign::NONE)
        return value;

    const auto op_t = static_cast<OpType>(sign2optype(op));

    switch (op_t) {
    case OpType::OPN:
        return map_op2func0[static_cast<uint8_t>(op)]();
    // unary operators: always compute on lhs
    case OpType::OPL:
        return map_op2func1[static_cast<uint8_t>(op)](lhs->eval());
    case OpType::OPR:
        return map_op2func1[static_cast<uint8_t>(op)](lhs->eval());
    // infix operator
    case OpType::OPI:
        return map_op2func2[static_cast<uint8_t>(op)](lhs->eval(), rhs->eval());
    default:
        throw std::runtime_error("Invalid operator type");
    }
}

std::vector<std::pair<parser::TknType, int>>
parser::str2pairs(const char *str) {
    std::vector<std::pair<parser::TknType, int>> tokens;
    char *p = const_cast<char *>(str);
    while (*p) {
        if (isspace(*p)) {
            ++p;
            continue;
        } else if (isdigit(*p)) {
            int value = str2int(p);
            tokens.emplace_back(parser::TknType::NUM, value);
        } else if (isalpha(*p)) {
            auto op = str2func(p);
            tokens.emplace_back(parser::TknType::OP, op);
        } else {
            auto op = char2op(*p);
            tokens.emplace_back(parser::TknType::OP, op);
            ++p;
        }
    }
    return tokens;
}

// Convert a string to a vector of tokens using str2int and char2op
std::vector<parser::Token>
parser::pairs2tokens(const std::vector<std::pair<parser::TknType, int>> &pairs) {
    std::vector<parser::Token> tokens;
    uint8_t lpar = 0; // left parenthesis count

    for (const auto &pair : pairs) {
        if (pair.first == parser::TknType::NUM) {
            tokens.emplace_back(pair.second);
        } else if (pair.first == parser::TknType::OP &&
                   pair.second == static_cast<int>(parser::Sign::PAL)) {
            ++lpar;
        } else if (pair.first == parser::TknType::OP &&
                   pair.second == static_cast<int>(parser::Sign::PAR)) {
            if (lpar == 0)
                throw std::runtime_error("Unmatched right parenthesis");
            --lpar;
        } else {
            Sign op = static_cast<parser::Sign>(pair.second);
            auto [bpl, bpr] = get_bp(op);
            tokens.emplace_back(static_cast<int>(op), bpl + lpar * K_BP_DELTA,
                                bpr + lpar * K_BP_DELTA);
        }
    }
    if (lpar > 0)
        throw std::runtime_error("Unmatched left parenthesis");
    return tokens;
}

std::shared_ptr<parser::Expr>
parser::parse(std::vector<parser::Token> & tokens, const uint8_t min_bp) {
    // std::reverse(tokens.begin(), tokens.end());
    std::shared_ptr<parser::Expr> lhs, rhs;
    parser::Token num = next(tokens);
    lhs = std::make_shared<parser::Expr>(num.value);

    while (true) {
        parser::Token tok = peek(tokens);
        if (tok.type == parser::TknType::NONE)
            break;

        if (tok.type != parser::TknType::OP)
            throw std::runtime_error("Expected operator");

        parser::Sign op = static_cast<parser::Sign>(tok.value);
        uint8_t l_bp = tok.lbp;
        uint8_t r_bp = tok.rbp;
        if (l_bp <= min_bp)
            break;

        next(tokens); // consume the operator
        rhs = parse(tokens, r_bp);
        lhs = std::make_shared<parser::Expr>(op, lhs, rhs);
    }

    return lhs;
}
