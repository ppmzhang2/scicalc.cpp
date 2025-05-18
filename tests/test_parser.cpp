#include "expr.hpp"
#include <gtest/gtest.h>

TEST(EXPR, ParseRaw) {
    char srt1[] = "2 + 3 - 4 * 5 - 6^2";
    char srt2[] = "2 * (3 + 4) * 5 - 6 * 7";
    char srt3[] = "(8 - 7 - (3 - 1)) * 3 - 2^(3+1)";
    char srt4[] = "4*5+2^(3-1)^2 - 6*(7 + 1)/4 - 9";

    char srt5[] = "ln4!";
    char srt6[] = "3! - ln(5-1) + 7 / 3^2";

    float ep = 1e-3;
    float ans1 = -51;
    float ans2 = 28;
    float ans3 = -19;
    float ans4 = 15;
    float ans5 = 3.17805;
    float ans6 = 5.39148;

    std::vector<char *> chrs1 = expr::split_str(srt1);
    std::vector<char *> chrs2 = expr::split_str(srt2);
    std::vector<char *> chrs3 = expr::split_str(srt3);
    std::vector<char *> chrs4 = expr::split_str(srt4);
    std::vector<char *> chrs5 = expr::split_str(srt5);
    std::vector<char *> chrs6 = expr::split_str(srt6);

    std::vector<expr::Atom> atoms1 = expr::chrs2atoms(chrs1);
    std::vector<expr::Atom> atoms2 = expr::chrs2atoms(chrs2);
    std::vector<expr::Atom> atoms3 = expr::chrs2atoms(chrs3);
    std::vector<expr::Atom> atoms4 = expr::chrs2atoms(chrs4);
    std::vector<expr::Atom> atoms5 = expr::chrs2atoms(chrs5);
    std::vector<expr::Atom> atoms6 = expr::chrs2atoms(chrs6);

    auto tokens1 = expr::atoms2tokens(atoms1);
    auto tokens2 = expr::atoms2tokens(atoms2);
    auto tokens3 = expr::atoms2tokens(atoms3);
    auto tokens4 = expr::atoms2tokens(atoms4);
    auto tokens5 = expr::atoms2tokens(atoms5);
    auto tokens6 = expr::atoms2tokens(atoms6);

    auto zp1 = expr::tokens2chain(tokens1, nullptr, true);
    auto zp2 = expr::tokens2chain(tokens2, nullptr, true);
    auto zp3 = expr::tokens2chain(tokens3, nullptr, true);
    auto zp4 = expr::tokens2chain(tokens4, nullptr, true);
    auto zp5 = expr::tokens2chain(tokens5, nullptr, true);
    auto zp6 = expr::tokens2chain(tokens6, nullptr, true);

    expr::free_chrs(chrs1);
    expr::free_chrs(chrs2);
    expr::free_chrs(chrs3);
    expr::free_chrs(chrs4);
    expr::free_chrs(chrs5);
    expr::free_chrs(chrs6);

    EXPECT_EQ(ans1, expr::eval(zp1));
    EXPECT_EQ(ans2, expr::eval(zp2));
    EXPECT_EQ(ans3, expr::eval(zp3));
    EXPECT_EQ(ans4, expr::eval(zp4));
    EXPECT_NEAR(ans5, expr::eval(zp5), ep);
    EXPECT_NEAR(ans6, expr::eval(zp6), ep);
}
