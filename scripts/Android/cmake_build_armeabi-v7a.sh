cmake .. -DCMAKE_TOOLCHAIN_FILE=../example/android/ndk/android-ndk-r20/build/cmake/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a -DANDROID_NATIVE_API_LEVEL=15 -DOPENSSL_ROOT_DIR=../example/android/openssl-1.1.1a-clang/ -DOPENSSL_CRYPTO_LIBRARY=../example/android/openssl-1.1.1a-clang/armeabi-v7a/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=../example/android/openssl-1.1.1a-clang/armeabi-v7a/lib/libssl.a -DOPENSSL_INCLUDE_DIR=../example/android/openssl-1.1.1a-clang/include/ -DCMAKE_INSTALL_PREFIX:PATH=../out/android/armeabi-v7a/ -DCMAKE_BUILD_TYPE=Release

cmake --build . --target install -j 4
