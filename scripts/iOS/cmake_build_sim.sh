

cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/iOS.cmake -DPLATFORM=SIMULATOR64 -DENABLE_BITCODE=0 -DENABLE_VISIBILITY=1 -DDEPLOYMENT_TARGET=8 -DOPENSSL_ROOT_DIR=../example/ios/third_party/openssl -DOPENSSL_CRYPTO_LIBRARY=../example/ios/third_party/openssl/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=../example/ios/third_party/openssl/lib/libssl.a -DOPENSSL_INCLUDE_DIR=../example/ios/third_party/openssl/include/ -DCMAKE_INSTALL_PREFIX:PATH=../out/ios/sim/ -DCMAKE_BUILD_TYPE=Release

cmake --build . --target install -j 4
