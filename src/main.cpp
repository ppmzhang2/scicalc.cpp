#include <algorithm>
#include <atomic>
#include <csignal>
#include <iostream>

#include "exam.hpp"
#include "expr.hpp"

static constexpr float EPSILON = 1e-3f;

std::atomic<bool> flag_int(false);

void handle_sigint(int) { flag_int = true; }

std::string sanitize_input(const std::string &input) {
    std::string ret;
    std::copy_if(input.begin(), input.end(), std::back_inserter(ret),
                 [](unsigned char c) { return c >= 32 && c <= 126; });
    return ret;
}

// Returns false if interrupted, true if input is successfully read
bool safe_getline(std::string &out, std::atomic<bool> &interrupted) {
    if (!std::getline(std::cin, out)) {
        if (interrupted) {
            std::cin.clear();
            std::cout << std::endl;
            interrupted = false;
            return false; // Interrupted, caller should retry
        } else {
            std::cout << "\nExiting..." << std::endl;
            std::exit(0); // Clean exit on Ctrl+D
        }
    }
    return true;
}

// Function to fetch an integer with validation and retry support
float get_num(const std::string &prompt,
              const std::function<bool(float)> &validate,
              std::atomic<bool> &interrupted) {
    std::string input;
    while (true) {
        std::cout << prompt;
        if (!safe_getline(input, interrupted)) {
            std::cout << "Input interrupted. Try again.\n";
            continue; // Retry prompt if interrupted
        }
        try {
            float value = std::stof(input);
            if (validate(value)) {
                return value;
            } else {
                std::cerr << "Invalid value. Try again.\n";
            }
        } catch (std::exception &ex) {
            std::cerr << "Invalid input: " << ex.what() << ". Try again.\n";
        }
    }
}

int main() {
    std::string input;

    // Register signal handler using sigaction
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    // SA_RESTART can be omitted to allow read() to be interrupted
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    while (true) {
        std::cout << ">> ";
        std::cout.flush();

        if (!safe_getline(input, flag_int)) {
            continue; // retry prompt if interrupted
        }

        if (input == "exit" || input == "quit")
            // print "bye" and exit
            std::cout << "bye" << std::endl, exit(0);

        if (input == "clear") {
            std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear screen
            continue;
        }

        else if (input == "exam") {
            std::string s_op;

            // Prompt for operators
            std::cout
                << "Enter operators separated by commas (e.g., +, -, *): ";
            if (!safe_getline(s_op, flag_int)) {
                std::cout << "Aborted exam setup." << std::endl;
                continue;
            }

            // Prompt for number of quizzes
            const uint8_t n = get_num(
                "Enter number of quizzes: ", [](int v) { return v >= 1; },
                flag_int);
            // Prompt for number of operands
            const uint8_t n_opd = get_num(
                "Enter number of operands (2 or more): ",
                [](int v) { return v >= 2; }, flag_int);
            const int min_opd = get_num(
                "Enter minimum operand value: ", [](int v) { return v >= 2; },
                flag_int);
            const int max_opd = get_num(
                "Enter maximum operand value: ",
                [min_opd](int v) { return v > min_opd; }, flag_int);

            std::vector<std::optional<float>> arr_ansusr(n);
            std::vector<float> arr_ansexp(n);
            std::vector<std::string> arr_expr(n);

            // Generate expressions and store correct answers
            for (int i = 0; i < n; ++i) {
                std::string s_expr =
                    exam::rand_expr(s_op, n_opd, min_opd, max_opd);
                arr_expr[i] = s_expr;
                arr_ansexp[i] = expr::eval(s_expr.c_str());
            }

            // Get user answers
            for (int i = 0; i < n;) {
                float ans_usr = get_num(
                    "Quiz " + std::to_string(i + 1) + ": " + arr_expr[i] +
                        " = ",
                    [](int) { return true; }, flag_int);
                arr_ansusr[i] = ans_usr;
                ++i;
            }

            // Display results after all answers are collected
            int cnt_correct = 0;
            std::cout << "\nExam Results:\n";
            for (int i = 0; i < n; ++i) {

                std::cout << "Quiz " << (i + 1) << ": " << arr_expr[i] << " = ";
                if (arr_ansusr[i].has_value()) {
                    const float ans_usr = arr_ansusr[i].value();
                    std::cout << ans_usr;
                    if (std::abs(ans_usr - arr_ansexp[i]) < EPSILON) {
                        std::cout << " ✅" << std::endl;
                        ++cnt_correct;
                    } else {
                        std::cout << " ❌. Correct answer " << arr_ansexp[i]
                                  << std::endl;
                    }
                } else {
                    std::cout << "No answer provided." << std::endl;
                }
            }

            std::cout << "You got " << cnt_correct << " out of " << n
                      << " questions." << std::endl;
            continue;
        }

        // Avoid evaluating an empty expression
        if (input.empty()) {
            continue;
        }

        std::string safe_input = sanitize_input(input);
        if (!safe_input.empty()) {
            try {
                float ans = expr::eval(safe_input.c_str());
                std::cout << ans << std::endl;
            } catch (std::exception &ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
            }
        } else {
            std::cerr << "Error: invalid characters." << std::endl;
        }
    }

    return 0;
}
