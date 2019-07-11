
# Build engine

cd $TRAVIS_BUILD_DIR
mkdir -p build && cd build/
export CC="ccache gcc"
export CXX="ccache g++"
export CFLAGS="-m32"
export CXXFLAGS="-m32"
cmake \
	-DCMAKE_PREFIX_PATH=$TRAVIS_BUILD_DIR/sdl2/usr/local \
	-DXASH_DOWNLOAD_DEPENDENCIES=yes \
	-DXASH_STATIC=ON \
	-DXASH_DLL_LOADER=ON \
	-DXASH_VGUI=ON \
	-DMAINUI_USE_STB=ON \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo ../
make -j2 VERBOSE=1
cp engine/csmoe mainui/libxashmenu.so vgui_support/libvgui_support.so vgui_support/vgui.so .
cp $TRAVIS_BUILD_DIR/sdl2/usr/local/lib/$(readlink $TRAVIS_BUILD_DIR/sdl2/usr/local/lib/libSDL2-2.0.so.0) libSDL2-2.0.so.0
7z a -t7z $TRAVIS_BUILD_DIR/csmoe-linux.7z -m0=lzma2 -mx=9 -mfb=64 -md=32m -ms=on csmoe libSDL2-2.0.so.0 libvgui_support.so vgui.so libxashmenu.so
