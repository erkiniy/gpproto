cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS})

add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable(TestExecutable mytest.cpp mytest.h)
target_link_libraries(TestExecutable
                      gputils
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})


add_test(NAME TestUtils COMMAND TestExecutable -l message)
#add_test(NAME TestUtils COMMAND ${CMAKE_COMMAND} --build . --target TestUtils --config $<CONFIGURATION>)

add_executable(Logging Logging.cpp)
target_link_libraries(Logging
                      gputils
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME Logging COMMAND Logging -l message)
#add_test(NAME Logging COMMAND ${CMAKE_COMMAND} --config $<CONFIGURATION>)