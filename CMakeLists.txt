cmake_minimum_required(VERSION 3.27)
project(SciCalc VERSION 1.0.0)

# Specify output binary names
set(OUT_BIN_NAME "scicalc")
set(TEST_BIN_NAME "scicalc_test")

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Specify the compilers
set(CMAKE_C_COMPILER ${CC})
set(CMAKE_CXX_COMPILER ${CXX})

# Force CMake to use ld.lld, and set linker flags
set(CMAKE_LINKER "${LD}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=lld")

# -----------------------------------------------------------------------------
# Main executable
# -----------------------------------------------------------------------------
# define sources and headers
set(SOURCES
    "${SciCalc_SOURCE_DIR}/src/main.cpp"
    "${SciCalc_SOURCE_DIR}/src/parser.cpp"
)
set(HEADERS
    "${SciCalc_SOURCE_DIR}/include/parser.hpp"
)

# Create the executable
add_executable(${OUT_BIN_NAME}
    ${SOURCES}
    ${HEADERS}
)
# Set the runtime output directory to be inside the build directory
set_target_properties(${OUT_BIN_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SciCalc_BINARY_DIR}/bin"
)

# Configure the file into the build directory
configure_file(
    "${SciCalc_SOURCE_DIR}/include/config.h.in"
    "${SciCalc_BINARY_DIR}/include/config.h"
)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
enable_testing()
add_executable(${TEST_BIN_NAME}
    "${SciCalc_SOURCE_DIR}/src/parser.cpp"
    "${SciCalc_SOURCE_DIR}/tests/test_main.cpp"
)
add_test(NAME ${TEST_BIN_NAME} COMMAND ${TEST_BIN_NAME})

# -----------------------------------------------------------------------------
# Detect the operating system and architecture
# -----------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND
   CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    message(STATUS "Building for macOS arm64")
    include("./cmake/CMakeLists_osx.cmake")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
       CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    message(STATUS "Building for Linux x86-64")
    include("./cmake/CMakeLists_x64.cmake")
else()
    message(FATAL_ERROR
        "Unsupported operating system or architecture: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Additional common configuration (if any) can be added here
