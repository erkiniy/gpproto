cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(GPProto VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

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

enable_testing()

add_subdirectory(gpnet)

add_subdirectory(gputils)

add_subdirectory(test)
