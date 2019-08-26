cmake .. -DCMAKE_TOOLCHAIN_FILE=../example/android/ndk/android-ndk-r20/build/cmake/android.toolchain.cmake -DANDROID_ABI=x86_64 -DANDROID_NATIVE_API_LEVEL=15 -DOPENSSL_ROOT_DIR=../example/android/openssl-1.1.1a-clang/ -DOPENSSL_CRYPTO_LIBRARY=../example/android/openssl-1.1.1a-clang/x86_64/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=../example/android/openssl-1.1.1a-clang/x86_64/lib/libssl.a -DOPENSSL_INCLUDE_DIR=../example/android/openssl-1.1.1a-clang/include/ -DCMAKE_INSTALL_PREFIX:PATH=../out/android/x86_64/ -DCMAKE_BUILD_TYPE=Release

cmake --build . --target install -j 4
