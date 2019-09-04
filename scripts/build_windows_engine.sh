# Build engine

cd $TRAVIS_BUILD_DIR
mkdir -p win-build && cd win-build
cmake \
    -G "NMake Makefiles" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
nmake