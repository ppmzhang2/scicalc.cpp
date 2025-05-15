set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# -----------------------------------------------------------------------------
# GTest
# -----------------------------------------------------------------------------
add_subdirectory(${SciCalc_SOURCE_DIR}/contrib/googletest-cmake)

# -----------------------------------------------------------------------------
# System and executable
# -----------------------------------------------------------------------------
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# Library List
# -----------------------------------------------------------------------------
set(MAIN_ALL_LIBS
    c
    dl
    Threads::Threads
)
set(TEST_ALL_LIBS
    c
    dl
    Threads::Threads
    gtest
    gtest_main
)

target_include_directories(${OUT_BIN_NAME} PRIVATE
    ${OpenCV_INC_DIR}
    ${ZLIBNG_INC_DIR}
    ${PNG_INC_DIR}
    ${EXIV2_INC_DIR}
    ${JSON_INC_DIR}
    ${SQLITE_INC_DIR}
    ${CSV_INC_DIR}
    "${SciCalc_SOURCE_DIR}/include"
    "${SciCalc_BINARY_DIR}/include" # Ensures config.h can be found
)
target_include_directories(${TEST_BIN_NAME} PRIVATE
    "${SciCalc_SOURCE_DIR}/include"
    "${SciCalc_BINARY_DIR}/include" # Ensures config.h can be found
)

target_compile_definitions(${TEST_BIN_NAME} PRIVATE
    -DGTEST_ACCESS
)

# Link libraries
target_link_libraries(${OUT_BIN_NAME} PRIVATE ${MAIN_ALL_LIBS})
target_link_libraries(${TEST_BIN_NAME} PRIVATE ${TEST_ALL_LIBS})
