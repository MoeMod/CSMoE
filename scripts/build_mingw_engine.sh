# Build engine

cd $TRAVIS_BUILD_DIR

mkdir -p mingw-build && cd mingw-build
export CC="ccache i686-w64-mingw32-gcc"
export CXX="ccache i686-w64-mingw32-g++"
export CFLAGS="-static-libgcc -no-pthread"
export CXXFLAGS="-static-libgcc -static-libstdc++"

cmake \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
make -j4 VERBOSE=1