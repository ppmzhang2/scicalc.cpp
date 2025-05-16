#include <iostream>

#include "parser.hpp"

int main() {
    std::string input;

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, input);
        if (input == "exit")
            break;

        try {
            auto pairs = parser::str2pairs(input.c_str());
            auto tokens = parser::pairs2tokens(pairs);
            std::reverse(tokens.begin(), tokens.end());
            auto expr = parser::parse(tokens, 0);

            float value = expr->eval();
            std::cout << value << std::endl;
        } catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}
