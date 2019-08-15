cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(BOOST_ROOT "/usr/local/opt/boost/")
set(Boost_INCLUDE_DIR "/usr/local/opt/boost/include")
set(Boost_LIBRARY_DIR "/usr/local/opt/boost/lib")

#set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
#set(OPENSSL_CRYPTO_LIBRARY "/usr/local/opt/openssl/lib")
#set(OPENSSL_INCLUDE_DIR "/usr/local/opt/openssl/include")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS})

add_definitions (-DBOOST_TEST_DYN_LINK )


add_executable(TestExecutable mytest.cpp mytest.h)
target_link_libraries(TestExecutable
                      gputils
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})


add_test(NAME TestUtils COMMAND TestExecutable -l message)
#[[
add_executable(Logging Logging.cpp)
target_link_libraries(Logging
                      gputils
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

#add_test(NAME Logging COMMAND Logging -l message)


add_executable(Filesystem Filesystem.cpp)
target_link_libraries(Filesystem
                      gputils
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

#add_test(NAME Filesystem COMMAND Filesystem -l message)

add_executable(Timers Timers.cpp)
target_link_libraries(Timers
        gputils
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

#add_test(NAME Timers COMMAND Timers -l message)

add_executable(Streams Streams.cpp)
target_link_libraries(Streams
        gputils
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME Streams COMMAND Streams -l message)
]]

add_executable(Crypto Crypto.cpp)
target_link_libraries(Crypto
        gputils
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME Crypto COMMAND Crypto -l message)


