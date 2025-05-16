#include "parser.hpp"
#include <gtest/gtest.h>

TEST(PARSER, ParseRaw) {
    char srt1[] = "2 + 3 - 4 * 5 - 6^2";
    char srt2[] = "2 * (3 + 4) * 5 - 6 * 7";
    char srt3[] = "(8 - 7 - (3 - 1)) * 3 - 2^(3+1)";
    char srt4[] = "4*5+2^(3-1)^2 - 6*(7 + 1)/4 - 9";
    float ans1 = -51;
    float ans2 = 28;
    float ans3 = -19;
    float ans4 = 15;

    auto pairs1 = parser::str2pairs(srt1);
    auto pairs2 = parser::str2pairs(srt2);
    auto pairs3 = parser::str2pairs(srt3);
    auto pairs4 = parser::str2pairs(srt4);


    auto tokens1 = parser::pairs2tokens(pairs1);
    auto tokens2 = parser::pairs2tokens(pairs2);
    auto tokens3 = parser::pairs2tokens(pairs3);
    auto tokens4 = parser::pairs2tokens(pairs4);

    std::reverse(tokens1.begin(), tokens1.end());
    std::reverse(tokens2.begin(), tokens2.end());
    std::reverse(tokens3.begin(), tokens3.end());
    std::reverse(tokens4.begin(), tokens4.end());

    auto expr1 = parser::parse(tokens1, 0);
    auto expr2 = parser::parse(tokens2, 0);
    auto expr3 = parser::parse(tokens3, 0);
    auto expr4 = parser::parse(tokens4, 0);

    EXPECT_EQ(ans1, expr1->eval());
    EXPECT_EQ(ans2, expr2->eval());
    EXPECT_EQ(ans3, expr3->eval());
    EXPECT_EQ(ans4, expr4->eval());
}
