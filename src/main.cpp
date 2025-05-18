#include <iostream>

#include "expr.hpp"

int main() {
    std::string input;

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, input);
        if (input == "exit")
            break;

        try {
            std::vector<char *> chrs = expr::split_str(input.c_str());
            std::vector<expr::Atom> atoms = expr::chrs2atoms(chrs);
            auto tokens = expr::atoms2tokens(atoms);
            auto zp = expr::tokens2chain(tokens, nullptr, true);
            float value = expr::eval(zp);
            std::cout << value << std::endl;
            expr::free_chrs(chrs);
        } catch (std::exception &ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}
