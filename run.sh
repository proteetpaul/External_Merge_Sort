#!/bin/bash

ROOT_DIR=$(pwd)
BUILD_DIR=$ROOT_DIR/build

# Check if the build and bin directory exists, if not create it
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

cd $BUILD_DIR
cmake ..
make -j
./src/test