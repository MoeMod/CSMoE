# Build engine

cd $TRAVIS_BUILD_DIR
mkdir -p win-build && cd win-build
cmake \
    -G "Visual Studio 15 2017 Win64" \
    -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release