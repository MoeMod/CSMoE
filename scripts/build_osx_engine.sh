#!/bin/bash

# Build engine

cd $TRAVIS_BUILD_DIR
mkdir -p osx-build && cd osx-build
cmake \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
make -j 4