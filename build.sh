#!/bin/sh

# Function to display usage instructions
usage() {
    echo "Usage: $0 [debug|release]"
    exit 1
}

# Check if the first argument is supplied
if [ -z "$1" ]; then
    usage
fi

# Determine the build mode based on the argument
case "$1" in
    debug)
        CONFIG_PRESET="ux-debug"
        BUILD_PRESET="ux-debug-build"
        ;;
    release)
        CONFIG_PRESET="ux-release"
        BUILD_PRESET="ux-release-build"
        ;;
    *)
        usage
        ;;
esac

# Create the build directory if it doesn't exist
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

find_llvm_dir() {
    for dir in /usr/lib/llvm-*; do
        if [ -d "$dir" ]; then
            echo "$dir"
            return
        fi
    done
    echo ""
}

# Add LLVM to the library path and link flags
LLVM_DIR=$(find_llvm_dir)
if [ -n "$LLVM_DIR" ]; then
    export LD_LIBRARY_PATH="${LLVM_DIR}/lib:$LD_LIBRARY_PATH"
    export LDFLAGS="-L${LLVM_DIR}/lib"
fi

# Configure and build the project using the appropriate presets
cmake --preset=$CONFIG_PRESET
cmake --build --preset=$BUILD_PRESET

echo "Build complete in $1 mode!"
