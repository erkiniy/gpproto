
DEFAULT_INSTALL_PATH="/Users/jaloliddinerkiniy/Projects/Android/LibClient/libclient/src/main"
DEFAULT_LIBS_PATH="$DEFAULT_INSTALL_PATH/jniLibs"

echo $DEFAULT_LIBS_PATH

function clear_build() {
    echo "Clear build directory"
    rm -rf ../../build
    mkdir ../../build
}

function prepare_dirs() {
    echo "Preparing directories"
    clear_build
}

prepare_jni() {
    echo "Preparing JNI files"
    rm -rf gp

    mkdir gp && cd gp
    mkdir proto

    cd ..

    cp ../../gp/proto/gp_client_data.h gp/proto/gp_client_data.h
    cp ../../gp/proto/gp_client.h gp/proto/gp_client.h

    rm -rf org

    mkdir org && cd org
    mkdir gp && cd gp
    mkdir libclient

    cd ../..

    swig -java -package org.gp.libclient -outdir org/gp/libclient/ swig_module_libclient.i

    cp swig_module_libclient_wrap.c ../../gp/proto/jni/swig_module_libclient_wrap.c
}

prepare_jni
echo "Installing JNI files"
cp -Rv org "$DEFAULT_INSTALL_PATH/java"

echo "Building for arm64-v8a"
prepare_dirs

cp cmake_build_arm64-v8a.sh ../../build/
cd ../../build

./cmake_build_arm64-v8a.sh

cd ../scripts/Android/

echo "Building for armeabi-v7a"
prepare_dirs

cp cmake_build_armeabi-v7a.sh ../../build/
cd ../../build

./cmake_build_armeabi-v7a.sh

cd ../scripts/Android/

echo "Building for x86_64"
prepare_dirs

cp cmake_build_x86_64.sh ../../build/
cd ../../build

./cmake_build_x86_64.sh

cd ../scripts/Android/

echo "Building for x86"
prepare_dirs

cp cmake_build_x86.sh ../../build/
cd ../../build

./cmake_build_x86.sh

echo "Finished building for Android"

cd ../out/android

rm -rf "$DEFAULT_LIBS_PATH"
mkdir "$DEFAULT_LIBS_PATH"

echo "Installing arm64-v8a"
ARM_64_V8A_DIR="$DEFAULT_LIBS_PATH/arm64-v8a/"
mkdir ARM_64_V8A_DIR
cp -Rv arm64-v8a/lib/ "$ARM_64_V8A_DIR"

echo "Installing armeabi-v7a"
ARMEABI_V7A_DIR="$DEFAULT_LIBS_PATH/armeabi-v7a/"
mkdir ARMEABI_V7A_DIR
cp -Rv armeabi-v7a/lib/ "$ARMEABI_V7A_DIR"

echo "Installing x86_64"
X86_64_DIR="$DEFAULT_LIBS_PATH/x86_64/"
mkdir X86_64_DIR
cp -Rv x86_64/lib/ "$X86_64_DIR"

echo "Installing x86"
X86_DIR="$DEFAULT_LIBS_PATH/x86/"
mkdir X86_DIR
cp -Rv x86/lib/ "$X86_DIR"


