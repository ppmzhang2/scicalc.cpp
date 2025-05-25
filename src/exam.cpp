#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "exam.hpp"

namespace {

    enum class OpInfix {
        NONE = 0,
        ADD,
        SUB,
        MUL,
        DIV,
        EXP,
    };

    enum class OpUnary {
        NONE = 0,
        LOG,
        FCT,
    };

    static constexpr uint8_t kNOpUnary = 3;

    static const std::unordered_map<char, OpInfix> kMapChr2Opi = {
        {'+', OpInfix::ADD}, {'-', OpInfix::SUB}, {'*', OpInfix::MUL},
        {'/', OpInfix::DIV}, {'^', OpInfix::EXP},
    };

    static const std::unordered_map<std::string, OpUnary> kMapStr2Opu = {
        {"ln", OpUnary::LOG},
        {"!", OpUnary::FCT},
    };

    // Unary operator pool: 'none' is index 0
    static constexpr std::array<std::string (*)(const std::string &), kNOpUnary>
        kArrOpu = {
            [](const std::string &s) { return s; }, // No unary op (trivial)
            [](const std::string &s) { return "ln(" + s + ")"; }, // log
            [](const std::string &s) { return "(" + s + ")!"; },  // factorial
    };

    // Unary operator pool for the 1st round
    static constexpr std::array<std::string (*)(const std::string &), kNOpUnary>
        kArrOpu1 = {
            [](const std::string &s) { return s; }, // No unary op (trivial)
            [](const std::string &s) { return "ln " + s; }, // log
            [](const std::string &s) { return s + "!"; },   // factorial
    };

    const std::vector<std::string> ops_infix = {"+", "-", "*", "/"};

    std::pair<std::vector<OpInfix>, std::vector<OpUnary>>
    get_op_pool(const std::string &op_str) {
        std::vector<OpInfix> ops_infix;
        std::vector<OpUnary> ops_unary;

        std::istringstream iss(op_str);
        std::string token;
        while (std::getline(iss, token, ',')) {
            // Remove any leading/trailing whitespace
            token.erase(0, token.find_first_not_of(" \t\n\r"));
            token.erase(token.find_last_not_of(" \t\n\r") + 1);

            if (token.size() == 1 && kMapChr2Opi.count(token[0])) {
                ops_infix.push_back(kMapChr2Opi.at(token[0]));
            } else if (kMapStr2Opu.count(token)) {
                ops_unary.push_back(kMapStr2Opu.at(token));
            } else {
                throw std::invalid_argument("Unknown operator: " + token);
            }
        }

        return {ops_infix, ops_unary};
    }

    std::pair<std::vector<std::string (*)(const std::string &)>,
              std::vector<std::string (*)(const std::string &)>>
    pool_fn_opu(const std::vector<OpUnary> &ops_unary,
                const uint8_t n_none = 5) {
        static constexpr auto fn_pa = [](const std::string &s) {
            return "(" + s + ")";
        };
        // Unary operator function pool indexed by OpUnary enum values
        std::vector<std::string (*)(const std::string &)> fns_opu, fns_opu1;

        // 1. Generate the unary operator pool
        // Always include None for `n_none` times
        for (uint8_t i = 0; i < n_none; ++i) {
            fns_opu.push_back(kArrOpu[static_cast<int>(OpUnary::NONE)]);
        }
        // Add the unary operators
        for (const auto &op : ops_unary) {
            fns_opu.push_back(kArrOpu[static_cast<int>(op)]);
        }
        // Finally add the trivial parentheses operator
        fns_opu.push_back(fn_pa);

        // 2. Generate the unary operator pool for the 1st round
        // Always include None for `n_none` times
        for (uint8_t i = 0; i < n_none; ++i) {
            fns_opu1.push_back(kArrOpu1[static_cast<int>(OpUnary::NONE)]);
        }
        // Add the unary operators
        for (const auto &op : ops_unary) {
            fns_opu1.push_back(kArrOpu1[static_cast<int>(op)]);
        }

        return {fns_opu, fns_opu1};
    }

    // Generate a complex random arithmetic expression string from a given
    // number of operands.
    //
    // The expression includes:
    //   - Randomly generated integers within a specified range
    //   - Random infix operators: +, -, *, /
    //   - Random unary operators: log(x), (x)!, (x) [trivial parenthesis]
    //
    // Algorithm Steps:
    //
    // 1. Generate `n` random integer operands within [min_operand,
    // max_operand],
    //    stored as strings.
    //
    // 2. For each operand, randomly apply a unary operator:
    //    - A random integer `u` is drawn from [0, m], where m is the number of
    //      available unary ops.
    //    - If `u == 0`, no unary op is applied (i.e., the operand is kept
    //    as-is).
    //    - If `u > 0`, apply the corresponding unary op: log, !, or
    //    parenthesis.
    //
    // 3. Attempt to apply unary operators to **pairs of operands or
    //    sub-expressions**, recursively:
    //    - Iterate through the operands from left to right, index `i = 1`.
    //    - For each adjacent pair (i-1, i), draw a random unary operator `u`
    //    from
    //      [0, m].
    //    - If `u > 0`, apply the unary operator to a combined expression of the
    //      form:
    //         unary_op(operand[i-1] <infix> operand[i])
    //      - Replace operand[i-1] and operand[i] with this new combined
    //      expression.
    //      - Reset `i = 1` to re-check from the start of the updated operand
    //      list.
    //    - Continue until `i == current operand count`.
    //
    // 4. When no more unary combinations are applied, randomly assign infix
    //    operators between the remaining operands:
    //    - Randomly select infix operators from {+, -, *, /}.
    //    - Construct a flat expression in the form:
    //        operand[0] op1 operand[1] op2 operand[2] ...
    //
    // The result is a syntactically valid expression string that includes
    // varying degrees of unary and infix operator complexity.
    //
    // @param n         Number of operands (before any combinations)
    // @param min_opd   Minimum value of operands
    // @param max_opd   Maximum value of operands
    // @return          A complex expression string suitable for evaluation
    std::string
    fns2str(const uint8_t n, const int min_opd, const int max_opd,
            const std::vector<std::string (*)(const std::string &)> &fns_opu1,
            const std::vector<std::string (*)(const std::string &)> &fns_opu) {
        if (n < 1)
            throw std::invalid_argument("Number of operands must be >= 1");

        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> dist_opd(min_opd, max_opd);
        std::uniform_int_distribution<> idx_u1(0, fns_opu1.size() - 1);
        std::uniform_int_distribution<> idx_unary(0, fns_opu.size() - 1);
        std::uniform_int_distribution<> idx_infix(0, ops_infix.size() - 1);

        // Step 1: Generate operands
        std::vector<std::string> operands;
        for (int i = 0; i < n; ++i) {
            operands.push_back(std::to_string(dist_opd(gen)));
        }

        // Step 2.1: First unary application to each operand
        for (std::string &op : operands) {
            int choice = idx_u1(gen);
            op = fns_opu1[choice](op);
        }

        // Step 2.2: Iteratively combine expressions using unary ops
        for (int i = 1; i < static_cast<int>(operands.size()); ++i) {
            int choice = idx_unary(gen);
            if (choice != 0) { // apply unary to group
                std::string combined = fns_opu[choice](
                    operands[i - 1] + " " + ops_infix[idx_infix(gen)] + " " +
                    operands[i]);
                operands.erase(operands.begin() + i - 1,
                               operands.begin() + i + 1);
                operands.insert(operands.begin() + i - 1, combined);
                i = 1; // Reset i to 2 on next loop (i++ at end)
            }
        }

        // Step 3: Final infix chaining
        std::ostringstream ss_expr;
        ss_expr << operands[0];
        for (size_t i = 1; i < operands.size(); ++i) {
            ss_expr << " " << ops_infix[idx_infix(gen)] << " " << operands[i];
        }

        return ss_expr.str();
    }

} // namespace

std::string exam::rand_expr(const std::string &s, const uint8_t n_opd,
                            const int min_opd, const int max_opd) {
    auto [ops_infix, ops_unary] = get_op_pool(s);
    auto [fns_opu, fns_opu1] = pool_fn_opu(ops_unary);
    return fns2str(n_opd, min_opd, max_opd, fns_opu1, fns_opu);
}
