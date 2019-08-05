
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

lipo -create os/lib/libgpclient.dylib sim/lib/libgpclient.dylib -output universal/lib/libgpclient.dylib

