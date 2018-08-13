cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)

set(GPNET_SOURCE
        gp/net/NetworkSocket.cpp
        gp/net/NetworkSocket.h

        gp/net/os/posix/NetworkSocketPosix.cpp
        gp/net/os/posix/NetworkSocketPosix.h

        gp/net/os/windows/NetworkSocketWinsock.cpp
        gp/net/os/windows/NetworkSocketWinsock.h

        gp/net/Connection.h

        gp/net/ConnectionDelegate.h

        gp/net/TcpConnection.cpp
        gp/net/TcpConnection.h

        gp/net/NetworkSocketDelegate.h gp/net/NetworkAddress.cpp gp/net/NetworkAddress.h)

add_library(gpnet STATIC ${GPNET_SOURCE})
target_include_directories(gpnet PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(gpnet gputils)

install(TARGETS gpnet EXPORT GPTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)
