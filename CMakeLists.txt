cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(GPProto VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

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
        SET(STD14_FLAG /Qstd=c++14)
    else()
        SET(STD14_FLAG -std=c++14)
    endif()
    CHECK_CXX_COMPILER_FLAG(${STD14_FLAG} HAVE_STD14)
    if (NOT HAVE_STD14)
        string(REPLACE "c++14" "c++1y" STD14_FLAG "${STD14_FLAG}")
        CHECK_CXX_COMPILER_FLAG(${STD14_FLAG} HAVE_STD1Y)
        set(HAVE_STD14 ${HAVE_STD1Y})
    endif()
elseif (MSVC)
    set(HAVE_STD14 MSVC_VERSION>=1900)
endif()

if (NOT HAVE_STD14)
    message(FATAL_ERROR "No C++14 support in the compiler. Please upgrade the compiler.")
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STD14_FLAG}")
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
add_subdirectory(test)

set(GPCORE_SOURCE
        gp/proto/Context.h
        gp/proto/Context.cpp

        gp/proto/Core.cpp
        gp/proto/Core.h

        gp/proto/gp_client.h
        gp/proto/gp_client.cpp

        gp/proto/ClientSync.cpp
        gp/proto/ClientSync.h

        gp/net/ProtoError.h gp/proto/gp_client_data.h gp/net/Transport.cpp gp/net/Transport.h gp/proto/AuthKeyInfo.cpp gp/proto/AuthKeyInfo.h gp/proto/DatacenterSaltsetInfo.cpp gp/proto/DatacenterSaltsetInfo.h gp/proto/Request.cpp gp/proto/Request.h gp/proto/ProtoDelegate.h)

add_library(gpcore STATIC ${GPCORE_SOURCE})
target_include_directories(gpcore PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(gpcore SYSTEM PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(gpcore PUBLIC gputils gpnet PRIVATE OpenSSL::Crypto ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES})

set(GPCLIENT_SOURCE
        gp/proto/Proto.h
        gp/proto/Proto.cpp)

add_library(gpproto SHARED ${GPCLIENT_SOURCE})
target_include_directories(gpproto PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(gpproto PRIVATE gpcore)

add_library(GPStatic INTERFACE)
target_link_libraries(GPStatic INTERFACE gpproto)

install(TARGETS gpproto gpcore GPStatic EXPORT GPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(EXPORT GPTargets
        FILE GPTargets.cmake
        NAMESPACE gpproto::
        DESTINATION lib/cmake/gp)

install(FILES gp/proto/Proto.h DESTINATION include/gp/proto)
