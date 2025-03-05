#!/bin/bash

# Build and run script for poker_cfr_bot

# Set default values
BUILD_TYPE=${1:-Release}
BUILD_DIR="build"
ITERATIONS=1000
USE_MC=false
SAVE_FILE="strategy.dat"
LOAD_FILE=""
RUN_TEST=true

# Parse command-line arguments
for arg in "$@"; do
  case $arg in
    --debug)
      BUILD_TYPE="Debug"
      shift
      ;;
    --iterations=*)
      ITERATIONS="${arg#*=}"
      shift
      ;;
    --monte-carlo)
      USE_MC=true
      shift
      ;;
    --save=*)
      SAVE_FILE="${arg#*=}"
      shift
      ;;
    --load=*)
      LOAD_FILE="${arg#*=}"
      shift
      ;;
    --no-test)
      RUN_TEST=false
      shift
      ;;
    *)
      # Unknown option
      ;;
  esac
done

# Print build information
echo "Building poker_cfr_bot..."
echo "  Build type: $BUILD_TYPE"
echo "  Build directory: $BUILD_DIR"

# Create directories if they don't exist
mkdir -p $BUILD_DIR
mkdir -p data/strategies
mkdir -p data/abstractions

# Run the build script
echo "Running build script..."
./scripts/build.sh $BUILD_TYPE

# Check if build was successful
if [ $? -ne 0 ]; then
  echo "Build failed! Exiting."
  exit 1
fi

# Prepare run command
RUN_CMD="$BUILD_DIR/poker_cfr_bot --iterations $ITERATIONS"

if [ "$USE_MC" = true ]; then
  RUN_CMD="$RUN_CMD --monte-carlo"
  echo "Using Monte Carlo sampling"
fi

if [ ! -z "$SAVE_FILE" ]; then
  RUN_CMD="$RUN_CMD --save data/strategies/$SAVE_FILE"
  echo "Will save strategy to: data/strategies/$SAVE_FILE"
fi

if [ ! -z "$LOAD_FILE" ]; then
  RUN_CMD="$RUN_CMD --load data/strategies/$LOAD_FILE"
  echo "Will load strategy from: data/strategies/$LOAD_FILE"
fi

if [ "$RUN_TEST" = false ]; then
  RUN_CMD="$RUN_CMD --no-test"
  echo "Will skip test hand playthrough"
fi

# Run the program
echo "Running poker_cfr_bot with options: $RUN_CMD"
$RUN_CMD

# Check exit status
if [ $? -eq 0 ]; then
  echo "Run completed successfully."
else
  echo "Run failed with error code $?."
  exit 1
fi

echo "Done!"