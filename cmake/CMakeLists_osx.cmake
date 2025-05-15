set(CMAKE_OSX_DEPLOYMENT_TARGET "14.4" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=14.4")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=14.4")

# -----------------------------------------------------------------------------
# GTest
# -----------------------------------------------------------------------------
add_subdirectory(${SciCalc_SOURCE_DIR}/contrib/googletest-cmake)

# -----------------------------------------------------------------------------
# System and executable
# -----------------------------------------------------------------------------
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_library(LIBICONV NAMES iconv)

# -----------------------------------------------------------------------------
# Library List
# -----------------------------------------------------------------------------
set(MAIN_ALL_LIBS
    "-framework OpenCL"
    "-framework Accelerate"
    Threads::Threads
)
set(TEST_ALL_LIBS
    "-framework OpenCL"
    "-framework Accelerate"
    Threads::Threads
    gtest
    gtest_main
)

target_include_directories(${OUT_BIN_NAME} PRIVATE
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
