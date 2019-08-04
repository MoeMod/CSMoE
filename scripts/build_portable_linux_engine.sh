# Build engine

cd $TRAVIS_BUILD_DIR
mkdir -p build && cd build/
export CC="ccache gcc"
export CXX="ccache g++"
cmake \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
make -j4 VERBOSE=1