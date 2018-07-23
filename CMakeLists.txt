cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(GPProto)

set(CMAKE_CXX_STANDARD 14)

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

add_subdirectory(gpnet)

add_subdirectory(gputils)