#!/bin/bash

# Build script for poker_cfr_bot

# Set default build type
BUILD_TYPE=${1:-Release}
BUILD_DIR="build"
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Print build information
echo "Building poker_cfr_bot..."
echo "  Build type: $BUILD_TYPE"
echo "  Build directory: $BUILD_DIR"
echo "  Using $NUM_CORES cores for compilation"

# Create build directory if it doesn't exist
mkdir -p $BUILD_DIR

# Configure CMake
echo "Configuring CMake..."
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build the project
echo "Building project..."
cmake --build . -- -j$NUM_CORES

# Check build status
if [ $? -eq 0 ]; then
    echo "Build successful!"
    
    # Run tests if requested
    if [ "$2" == "test" ]; then
        echo "Running tests..."
        ctest -V
    fi
else
    echo "Build failed!"
    exit 1
fi

cd ..

echo "Done!"