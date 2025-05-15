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
            parser::Raw raw(input.data());
            auto expr = raw.parse(0);

            std::shared_ptr<parser::Expr> assign_expr;

            float value = expr->eval();
            std::cout << value << std::endl;
        } catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}
