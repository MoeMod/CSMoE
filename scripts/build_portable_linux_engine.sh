
# Build engine

cd $TRAVIS_BUILD_DIR
mkdir -p build && cd build/
export CC="ccache gcc"
export CXX="ccache g++"
export CFLAGS="-m32"
export CXXFLAGS="-m32"
cmake \
	-DXASH_DOWNLOAD_DEPENDENCIES=yes \
	-DXASH_STATIC=ON \
	-DXASH_DLL_LOADER=ON \
	-DXASH_VGUI=ON \
	-DMAINUI_USE_STB=ON \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo ../
make -j4 VERBOSE=1