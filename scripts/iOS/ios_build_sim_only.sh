
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

echo "Building for SIMULATOR"
prepare_dirs

cp cmake_build_sim.sh ../../build/
cd ../../build

./cmake_build_sim.sh

cd ../out/ios

rm -rf "$DEFAULT_INSTALL_PATH"
mkdir "$DEFAULT_INSTALL_PATH"

cp -R sim/ "$DEFAULT_INSTALL_PATH"
