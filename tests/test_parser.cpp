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

    parser::Raw raw1(srt1);
    parser::Raw raw2(srt2);
    parser::Raw raw3(srt3);
    parser::Raw raw4(srt4);

    EXPECT_EQ(ans1, raw1.parse(0)->eval());
    EXPECT_EQ(ans2, raw2.parse(0)->eval());
    EXPECT_EQ(ans3, raw3.parse(0)->eval());
    EXPECT_EQ(ans4, raw4.parse(0)->eval());
}
