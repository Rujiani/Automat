cmake_minimum_required(VERSION 3.10.0)

project(Rujian_Regex)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_subdirectory(regex_compile)
add_library(regex my_regex.hpp my_regex.cpp)
target_compile_options(regex PRIVATE -g)
if (REGEX_ENABLE_TESTS)
add_compile_definitions(REGEX_ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(test)
endif()

add_executable(regex_main main.cpp)
target_link_libraries(regex_main regex regexTree regexToken DKA)
