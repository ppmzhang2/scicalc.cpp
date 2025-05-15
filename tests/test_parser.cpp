#include "parser.hpp"
#include <gtest/gtest.h>

TEST(PARSER, ParseRaw) {
    char srt1[] = "2 + 3 - 4 * 5 - 6^2";
    char srt2[] = "2 * (3 + 4) * 5 - 6 * 7";
    float ans1 = -51;
    float ans2 = 28;

    parser::Raw raw1(srt1);
    parser::Raw raw2(srt2);

    EXPECT_EQ(ans1, raw1.parse(0)->eval());
    EXPECT_EQ(ans2, raw2.parse(0)->eval());
}
