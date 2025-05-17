#include "parser.hpp"
#include <gtest/gtest.h>

TEST(PARSER, ParseRaw) {
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

    auto pairs1 = parser::str2atoms(srt1);
    auto pairs2 = parser::str2atoms(srt2);
    auto pairs3 = parser::str2atoms(srt3);
    auto pairs4 = parser::str2atoms(srt4);
    auto pairs5 = parser::str2atoms(srt5);
    auto pairs6 = parser::str2atoms(srt6);

    auto tokens1 = parser::atoms2tokens(pairs1);
    auto tokens2 = parser::atoms2tokens(pairs2);
    auto tokens3 = parser::atoms2tokens(pairs3);
    auto tokens4 = parser::atoms2tokens(pairs4);
    auto tokens5 = parser::atoms2tokens(pairs5);
    auto tokens6 = parser::atoms2tokens(pairs6);

    auto zp1 = parser::tokens2chain(tokens1, nullptr, true);
    auto zp2 = parser::tokens2chain(tokens2, nullptr, true);
    auto zp3 = parser::tokens2chain(tokens3, nullptr, true);
    auto zp4 = parser::tokens2chain(tokens4, nullptr, true);
    auto zp5 = parser::tokens2chain(tokens5, nullptr, true);
    auto zp6 = parser::tokens2chain(tokens6, nullptr, true);

    EXPECT_EQ(ans1, parser::eval(zp1));
    EXPECT_EQ(ans2, parser::eval(zp2));
    EXPECT_EQ(ans3, parser::eval(zp3));
    EXPECT_EQ(ans4, parser::eval(zp4));
    EXPECT_NEAR(ans5, parser::eval(zp5), ep);
    EXPECT_NEAR(ans6, parser::eval(zp6), ep);
}
