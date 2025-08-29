cmake_minimum_required(VERSION 3.10)
project(Logger CXX)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(logger STATIC src/logger.cpp)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(logger_shared SHARED src/logger.cpp)
target_include_directories(logger_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(logg src/main.cpp)
target_link_libraries(logg PRIVATE logger)

add_executable(stats_collector src/stats_collector.cpp)
target_link_libraries(stats_collector PRIVATE logger)

add_executable(test_sender test/test_sender.cpp)
target_link_libraries(test_sender PRIVATE logger)

find_package(GTest QUIET)
enable_testing()
add_executable(test_logger test/test_logger.cpp)
target_link_libraries(test_logger PRIVATE logger GTest::GTest GTest::Main)
target_include_directories(test_logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME LoggerTests COMMAND test_logger)
