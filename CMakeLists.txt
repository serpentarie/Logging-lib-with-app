cmake_minimum_required(VERSION 3.10)
project(Logger CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(logger STATIC src/logger.cpp)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(logger_shared SHARED src/logger.cpp)
target_include_directories(logger_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(logg src/main.cpp)
target_link_libraries(logg PRIVATE logger)