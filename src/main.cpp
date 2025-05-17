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
            auto pairs = parser::str2atoms(input.c_str());
            auto tokens = parser::atoms2tokens(pairs);
            auto zp = parser::tokens2chain(tokens, nullptr, true);
            float value = parser::eval(zp);
            std::cout << value << std::endl;
        } catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}
