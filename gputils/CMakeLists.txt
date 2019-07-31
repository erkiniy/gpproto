cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

if (NOT ZLIB_FOUND)
    find_package(ZLIB REQUIRED)
endif()

# cmake -D-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl/ -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include ..

if (ZLIB_FOUND)
    set(GP_HAVE_ZLIB 1)
    message(STATUS "Found ZLIB: ${ZLIB_INCLUDE_DIR} ${ZLIB_LIBRARIES}")

    if (NOT OPENSSL_FOUND)
        find_package(OpenSSL REQUIRED)
    endif()

    if (OPENSSL_FOUND)
        set(GP_HAVE_OPENSSL 1)
        message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR} ${OPENSSL_CRYPTO_LIBRARY}")
    endif()
endif()

set(GPUTILS_SOURCE
        gp/utils/DispatchQueue.h
        gp/utils/DispatchQueue.cpp

        gp/utils/Semaphore.h

        gp/utils/os/darwin/GPLogWrapperDarwin.h
        gp/utils/os/darwin/GPLogWrapperDarwin.cpp

        gp/utils/Logging.h
        gp/utils/Logging.cpp

        gp/utils/filesystem/File.h
        gp/utils/filesystem/FileInfo.h
        gp/utils/filesystem/FileSystem.h
        gp/utils/filesystem/FileInfo.cpp

        gp/utils/filesystem/NativeFile.cpp
        gp/utils/filesystem/NativeFile.h
        gp/utils/filesystem/NativeFileSystem.cpp
        gp/utils/filesystem/NativeFileSystem.h

        gp/utils/StringUtils.cpp
        gp/utils/StringUtils.h

        gp/utils/ByteOrder.h

        gp/utils/OutputStream.h
        gp/utils/OutputStream.cpp

        gp/utils/StreamSlice.cpp
        gp/utils/StreamSlice.h

        gp/utils/CustomStringConvertable.h

        gp/utils/InputStream.cpp
        gp/utils/InputStream.h

        gp/utils/InputStreamException.h

        gp/utils/Timer.cpp
        gp/utils/Timer.h

        gp/utils/DispatchQueuePool.cpp
        gp/utils/DispatchQueuePool.h

        gp/utils/Crypto.cpp
        gp/utils/Crypto.h

        gp/utils/Common.cpp
        gp/utils/Common.h

        gp/utils/Random.cpp
        gp/utils/Random.h

        gp/utils/BigNum.cpp
        gp/utils/BigNum.h

        gp/utils/Runloop.h
        gp/utils/Runloop.cpp

        gp/utils/third_party/nlohmann/json.hpp

        gp/utils/SecureKeychain.cpp
        gp/utils/SecureKeychain.h)

add_library(gputils STATIC ${GPUTILS_SOURCE})

target_include_directories(gputils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

    if (OPENSSL_FOUND)
    target_link_libraries(gputils PRIVATE ${OPENSSL_CRYPTO_LIBRARY} ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES})
    target_include_directories(gputils SYSTEM PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()

if (ZLIB_FOUND)
    target_link_libraries(gputils PRIVATE ${ZLIB_LIBRARIES})
    target_include_directories(gputils SYSTEM PRIVATE ${ZLIB_INCLUDE_DIR})
endif()

install(TARGETS gputils EXPORT GPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)