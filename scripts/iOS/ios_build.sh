
DEFAULT_INSTALL_PATH="/Users/jaloliddinerkiniy/Desktop/GapMessenger/Submodules/libProto/LibProto/gp"

function clear_build() {
    echo "Clear build directory"
    rm -rf ../../build
    mkdir ../../build
}

function prepare_dirs() {
    echo "Preparing directories"
    clear_build
}

echo "Building for iOS"
prepare_dirs

cp cmake_build_os.sh ../../build/
cd ../../build

./cmake_build_os.sh

cd ../scripts/iOS/

echo "Building for SIMULATOR"
prepare_dirs

cp cmake_build_sim.sh ../../build/
cd ../../build

./cmake_build_sim.sh

cd ../out/ios

rm -rf universal
mkdir universal
mkdir universal/lib

cp -R os/include universal/

lipo -create os/lib/libgpclientstatic.a sim/lib/libgpclientstatic.a -output universal/lib/libgpclientstatic.a

lipo -create os/lib/libgpcore.a sim/lib/libgpcore.a -output universal/lib/libgpcore.a

lipo -create os/lib/libgpnet.a sim/lib/libgpnet.a -output universal/lib/libgpnet.a

lipo -create os/lib/libgputils.a sim/lib/libgputils.a -output universal/lib/libgputils.a

cp os/lib/libcrypto.a universal/lib/libcrypto.a

libtool -static -o universal/lib/libgpclient.a universal/lib/libgpclientstatic.a universal/lib/libgpcore.a universal/lib/libgputils.a universal/lib/libgpnet.a universal/lib/libcrypto.a

#lipo -create os/lib/libgpclient.dylib sim/lib/libgpclient.dylib -output universal/lib/libgpclient.dylib


rm -rf "$DEFAULT_INSTALL_PATH"
mkdir "$DEFAULT_INSTALL_PATH"

cp -R universal/ "$DEFAULT_INSTALL_PATH"
