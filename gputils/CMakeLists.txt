cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)

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

        gp/utils/InputStream.h
        gp/utils/InputStream.cpp

        gp/utils/StreamSlice.cpp
        gp/utils/StreamSlice.h

        gp/utils/CustomStringConvertable.h

        gp/utils/OutputStream.cpp
        gp/utils/OutputStream.h

        gp/utils/OutputStreamException.cpp
        gp/utils/OutputStreamException.h

        gp/utils/Timer.cpp
        gp/utils/Timer.h

        gp/utils/DispatchQueuePool.cpp
        gp/utils/DispatchQueuePool.h)

add_library(gputils STATIC ${GPUTILS_SOURCE})
target_include_directories(gputils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install(TARGETS gputils EXPORT GPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)