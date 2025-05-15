# shamelessly borrowed from chdb
set(GTEST_SRC_DIR "${SciCalc_SOURCE_DIR}/contrib/googletest/googletest")

add_library(gtest "${GTEST_SRC_DIR}/src/gtest-all.cc")
target_include_directories(gtest SYSTEM PUBLIC "${GTEST_SRC_DIR}/include")
target_include_directories(gtest PRIVATE "${GTEST_SRC_DIR}")

add_library(gtest_main "${GTEST_SRC_DIR}/src/gtest_main.cc")
target_include_directories(gtest_main SYSTEM PUBLIC "${GTEST_SRC_DIR}/include")
