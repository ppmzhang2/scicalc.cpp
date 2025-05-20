#include <algorithm>
#include <atomic>
#include <csignal>
#include <iostream>
#include <optional>
#include <random>

#include "expr.hpp"

std::atomic<bool> flag_int(false);

void handle_sigint(int) { flag_int = true; }

std::string sanitize_input(const std::string &input) {
    std::string ret;
    std::copy_if(input.begin(), input.end(), std::back_inserter(ret),
                 [](unsigned char c) { return c >= 32 && c <= 126; });
    return ret;
}

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
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(2, 9);

            std::vector<std::pair<int, int>> questions;
            std::vector<std::optional<int>> user_answers(10);

            // Generate all questions first
            for (int i = 0; i < 10; ++i) {
                questions.emplace_back(dist(gen), dist(gen));
            }

            for (int i = 0; i < 10;) {
                int a = questions[i].first;
                int b = questions[i].second;

                std::cout << "Question " << (i + 1) << ": " << a << " * " << b
                          << " = ";
                std::string answer_str;
                if (!safe_getline(answer_str, flag_int)) {
                    std::cout << "\nRestarting question..." << std::endl;
                    continue; // re-prompt the same question
                }

                try {
                    user_answers[i] = std::stoi(answer_str);
                    ++i;
                } catch (std::exception &ex) {
                    std::cerr << "Invalid input: " << ex.what()
                              << ". Please enter a number." << std::endl;
                }
            }

            // Display results after all answers are collected
            int correct_count = 0;
            std::cout << "\nExam Results:\n";
            for (int i = 0; i < 10; ++i) {
                int a = questions[i].first;
                int b = questions[i].second;
                int correct_answer = a * b;

                std::cout << "Question " << (i + 1) << ": " << a << " * " << b
                          << " = ";
                if (user_answers[i].has_value()) {
                    int user_answer = user_answers[i].value();
                    std::cout << user_answer;
                    if (user_answer == correct_answer) {
                        std::cout << " ✅" << std::endl;
                        ++correct_count;
                    } else {
                        std::cout << " ❌. Correct answer is " << correct_answer
                                  << std::endl;
                    }
                } else {
                    std::cout << "No answer provided." << std::endl;
                }
            }

            std::cout << "You got " << correct_count << " out of 10 correct."
                      << std::endl;
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
