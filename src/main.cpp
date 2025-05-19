#include <atomic>
#include <csignal>
#include <iostream>
#include <random>

#include "expr.hpp"

std::atomic<bool> interrupted(false);

void handle_sigint(int) { interrupted = true; }

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

        if (!std::getline(std::cin, input)) {
            if (interrupted) {
                // Clear error state and consume partial input
                std::cin.clear();
                std::cout << std::endl;
                interrupted = false;
                continue;
            } else {
                std::cout << "\nExiting..." << std::endl;
                break;
            }
        }

        if (interrupted) {
            // Discard this line (it may contain ^C or be invalid)
            interrupted = false;
            continue;
        }

        if (input == "exit")
            break;

        else if (input == "exam") {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(1, 9);

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
                if (!std::getline(std::cin, answer_str)) {
                    if (interrupted) {
                        std::cin.clear();
                        std::cout << "\nInterrupted. Restarting question..."
                                  << std::endl;
                        interrupted = false;
                        continue; // re-prompt the same question
                    } else {
                        std::cout << "\nInput error. Exiting exam..."
                                  << std::endl;
                        break;
                    }
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
                        std::cout << " -> Correct!" << std::endl;
                        ++correct_count;
                    } else {
                        std::cout << " -> Wrong. Correct answer is "
                                  << correct_answer << std::endl;
                    }
                } else {
                    std::cout << "No answer provided." << std::endl;
                }
            }

            std::cout << "You got " << correct_count << " out of 10 correct."
                      << std::endl;
            continue;
        }

        try {
            float ans = expr::eval(input.c_str());
            std::cout << ans << std::endl;
        } catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}
