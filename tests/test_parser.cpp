#include "expr.hpp"
#include <gtest/gtest.h>

TEST(EXPR, ParseRaw) {
    char str1[] = "2 + 3 - 4 * 5 - 6^2";
    char str2[] = "2 * (3 + 4) * 5 - 6 * 7";
    char str3[] = "(8 - 7 - (3 - 1)) * 3 - 2^(3+1)";
    char str4[] = "4*5+2^(3-1)^2 - 6*(7 + 1)/4 - 9";

    char str5[] = "ln4!";
    char str6[] = "3! - ln(5-1) + 7 / 3^2";
    char str7[] = "pi";

    float ep = 1e-3;
    float ans1 = -51;
    float ans2 = 28;
    float ans3 = -19;
    float ans4 = 15;
    float ans5 = 3.17805;
    float ans6 = 5.39148;
    float ans7 = 3.14159;

    EXPECT_EQ(ans1, expr::eval(str1));
    EXPECT_EQ(ans2, expr::eval(str2));
    EXPECT_EQ(ans3, expr::eval(str3));
    EXPECT_EQ(ans4, expr::eval(str4));
    EXPECT_NEAR(ans5, expr::eval(str5), ep);
    EXPECT_NEAR(ans6, expr::eval(str6), ep);
    EXPECT_NEAR(ans7, expr::eval(str7), ep);
}
