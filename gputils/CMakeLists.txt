cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)

set(GPUTILS_SOURCE
        gp/utils/DispatchQueue.h
        gp/utils/DispatchQueue.cpp

        gp/utils/Semaphore.h

        gp/utils/os/darwin/GPLogWrapperDarwin.h
        gp/utils/os/darwin/GPLogWrapperDarwin.cpp

        gp/utils/Logging.h
        gp/utils/Logging.cpp gp/utils/File.h gp/utils/FileInfo.h)

add_library(gputils STATIC ${GPUTILS_SOURCE})
target_include_directories(gputils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install(TARGETS gputils EXPORT GPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)