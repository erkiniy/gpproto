cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)
#cmake .. -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include -DCMAKE_INSTALL_PREFIX:PATH=../example/gpSwift/gp -DCMAKE_BUILD_TYPE=Debug

#cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/iOS.cmake -DIOS_PLATFORM=SIMULATOR -DOPENSSL_ROOT_DIR=../example/ios/third_party/openssl -DOPENSSL_CRYPTO_LIBRARY=../example/ios/third_party/openssl/lib/libcrypto.a -DOPENSSL_SSL_LIBRARY=../example/ios/third_party/openssl/lib/libssl.a -DOPENSSL_INCLUDE_DIR=../example/ios/third_party/openssl/include/ -DCMAKE_INSTALL_PREFIX:PATH=../example/ios/GPProtoImpl/gp -DCMAKE_BUILD_TYPE=Debug

#/Users/jalol/CLionProjects/gpproto/example/ios/third_party/openssl

#install_name_tool -id "@rpath/libgpclient.dylib" libgpclient.dylib

#https://github.com/x2on/OpenSSL-for-iPhone

project(GPProto VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Prevent in-source build
get_filename_component(GP_REAL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
get_filename_component(GP_REAL_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" REALPATH)

if (GP_REAL_BINARY_DIR STREQUAL GP_REAL_SOURCE_DIR)
    message("  Out-of-source build should be used to build GPProto.")
    message("  You need to remove the files already created by CMake and")
    message("  rerun CMake from a new directory:")
    message("  rm -rf CMakeFiles CMakeCache.txt")
    message("  mkdir build")
    message("  cd build")
    message("  cmake ..")
    message(FATAL_ERROR "In-source build failed.")
endif()

message("Compiling with ${CMAKE_CXX_COMPILER_ID}")

if (POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(GCC 1)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    set(CLANG 1)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Intel)
    set(INTEL 1)
elseif (NOT MSVC)
    message(FATAL_ERROR "Compiler isn't supported")
endif()

include(CheckCXXCompilerFlag)

if (GCC OR CLANG OR INTEL)
    if (WIN32 AND INTEL)
        SET(STD17_FLAG /Qstd=c++17)
    else()
        SET(STD17_FLAG -std=c++17)
    endif()
    CHECK_CXX_COMPILER_FLAG(${STD17_FLAG} HAVE_STD17)
    if (NOT HAVE_STD17)
        string(REPLACE "c++17" "c++1z" STD17_FLAG "${STD17_FLAG}")
        CHECK_CXX_COMPILER_FLAG(${STD17_FLAG} HAVE_STD1Z)
        set(HAVE_STD17 ${HAVE_STD1Z})
    endif()
elseif (MSVC)
    set(HAVE_STD17 MSVC_VERSION>=1900)
endif()

if (NOT HAVE_STD17)
    message(FATAL_ERROR "No C++17 support in the compiler. Please upgrade the compiler.")
endif()


if (NOT ANDROID) # _FILE_OFFSET_BITS is broken in ndk r15 and r15b and doesn't work prior to Android 7.0
    add_definitions(-D_FILE_OFFSET_BITS=64)
endif()

if (NOT DEFINED CMAKE_MODULE_PATH)
    set(CMAKE_MODULE_PATH "")
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" "${CMAKE_MODULE_PATH}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (THREADS_FOUND)
    message(STATUS "Found Threads")
endif()

if (THREADS_HAVE_PTHREAD_ARG)
    message(STATUS "setting CXX flags with pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
#set(CCACHE_FOUND 0)
if (CCACHE_FOUND)
    message(STATUS "Found ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
else()
    message(STATUS "Could NOT find ccache")
endif()

if (MSVC)
    if (CMAKE_CXX_FLAGS_DEBUG MATCHES "/RTC1")
        string(REPLACE "/RTC1" " " CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    endif()
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /W4 /wd4100 /wd4127 /wd4324 /wd4505 /wd4702")
elseif (CLANG OR GCC)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STD14_FLAG} -fno-omit-frame-pointer -fno-exceptions -fno-rtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STD17_FLAG}")
    if (APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-dead_strip,-x,-S")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--exclude-libs,ALL")
    endif()

    if (MEMPROF)
        CHECK_CXX_COMPILER_FLAG(-no-pie CXX_NO_PIE_FLAG)
        if (CXX_NO_PIE_FLAG)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -no-pie")
        elseif (APPLE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_pie")
        endif()
    endif()
elseif (INTEL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STD17_FLAG}")
endif()

include(AddCXXCompilerFlag)
if (NOT MSVC)
    # add_cxx_compiler_flag("-Wall")
    # add_cxx_compiler_flag("-Wextra")
    #add_cxx_compiler_flag("-Wimplicit-fallthrough=2")
    #add_cxx_compiler_flag("-Wpointer-arith")
    # add_cxx_compiler_flag("-Wcast-qual")
    #add_cxx_compiler_flag("-Wsign-compare")
    #add_cxx_compiler_flag("-Wduplicated-branches")
    #add_cxx_compiler_flag("-Wduplicated-cond")
    #add_cxx_compiler_flag("-Walloc-zero")
    # add_cxx_compiler_flag("-Wlogical-op")
    #add_cxx_compiler_flag("-Wno-tautological-compare")
    #add_cxx_compiler_flag("-Wpointer-arith")
    #add_cxx_compiler_flag("-Wvla")
    #add_cxx_compiler_flag("-Wnon-virtual-dtor")
    #add_cxx_compiler_flag("-Wno-unused-parameter")
    #add_cxx_compiler_flag("-Wconversion")
    # add_cxx_compiler_flag("-Wno-sign-conversion")
    #add_cxx_compiler_flag("-Wc++14-compat-pedantic")
    # add_cxx_compiler_flag("-Qunused-arguments")
    # add_cxx_compiler_flag("-Wodr")
    #add_cxx_compiler_flag("-flto-odr-type-merging")

    #  add_cxx_compiler_flag("-Werror")

    #  add_cxx_compiler_flag("-Wcast-align")

    #std::int32_t <-> int and off_t <-> std::size_t/std::int64_t
    #  add_cxx_compiler_flag("-Wuseless-cast")

    #external headers like openssl
    #  add_cxx_compiler_flag("-Wzero-as-null-pointer-constant")
endif()

if (NOT OPENSSL_FOUND)
    find_package(OpenSSL)
endif()
if (OPENSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR} ${OPENSSL_LIBRARIES}")
endif()

enable_testing()

add_subdirectory(gpnet)
add_subdirectory(gputils)
#add_subdirectory(test)

set(GPCORE_SOURCE
        gp/proto/Proto.h
        gp/proto/Proto.cpp

        gp/proto/Context.h
        gp/proto/Context.cpp

        gp/proto/Core.cpp
        gp/proto/Core.h

        gp/proto/ClientSync.cpp
        gp/proto/ClientSync.h

        gp/network/ProtoError.h

        gp/network/Transport.h

        gp/proto/AuthKeyInfo.cpp
        gp/proto/AuthKeyInfo.h

        gp/proto/DatacenterSaltsetInfo.cpp
        gp/proto/DatacenterSaltsetInfo.h

        gp/proto/Request.cpp
        gp/proto/Request.h

        gp/proto/ProtoDelegate.h

        gp/proto/DatacenterAddress.h

        gp/proto/Session.cpp
        gp/proto/Session.h

        gp/network/TransportDelegate.h
        gp/network/TcpTransport.cpp
        gp/network/TcpTransport.h

        gp/network/IncomingMessage.h
        gp/network/OutgoingMessage.h

        gp/network/TcpTransportContext.cpp
        gp/network/TcpTransportContext.h

        gp/network/TcpTransportContextDelegate.h

        gp/network/TransportScheme.cpp
        gp/network/TransportScheme.h

        gp/proto/ContextChangeDelegate.h

        gp/proto/MessageEncryptionKey.cpp
        gp/proto/MessageEncryptionKey.h

        gp/proto/ProtoInternalMessage.h

        gp/proto/InternalParser.h

        gp/proto/TimeFixContext.h

        gp/proto/MessageService.h

        gp/proto/MessageTransaction.h

        gp/proto/RequestMessageService.cpp
        gp/proto/RequestMessageService.h

        gp/proto/TimeSyncMessageService.cpp
        gp/proto/TimeSyncMessageService.h

        gp/proto/TransportTransaction.h
        gp/proto/PreparedMessage.h

        gp/proto/DatacenterAuthAction.h
        gp/proto/DatacenterAuthAction.cpp

        gp/proto/DatacenterAuthMessageService.cpp
        gp/proto/DatacenterAuthMessageService.h

        gp/network/Transport.cpp

        gp/proto/RequestContext.cpp
        gp/proto/RequestContext.h

        gp/proto/UpdateMessageService.cpp
        gp/proto/UpdateMessageService.h

        gp/proto/JsonAdapter.cpp
        gp/proto/JsonAdapter.h)

add_library(gpcore STATIC ${GPCORE_SOURCE})
target_include_directories(gpcore PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(gpcore SYSTEM PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(gpcore PUBLIC gputils gpnet PRIVATE ${OPENSSL_CRYPTO_LIBRARY} ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES})

set(GPCLIENT_SOURCE
        gp/proto/gp_client.h
        gp/proto/gp_client.cpp
        gp/proto/gp_client_data.h)

add_library(gpclient SHARED ${GPCLIENT_SOURCE})
target_include_directories(gpclient PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(gpclient PRIVATE gpcore)

add_library(GPStatic INTERFACE)
target_link_libraries(GPStatic INTERFACE gpclient)

add_executable(clientInternal gp/proto/clientExecutable.cpp)
target_link_libraries(clientInternal gpclient)

install(TARGETS gpclient gpcore GPStatic EXPORT GPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(EXPORT GPTargets
        FILE GPTargets.cmake
        NAMESPACE gpproto::
        DESTINATION lib/cmake/gp)

install(FILES gp/proto/gp_client.h gp/proto/gp_client_data.h DESTINATION include/gp/proto)
