add_executable(test test.cpp ../Machines/A_Moore.cpp)

find_package(gtest)

target_link_libraries(test gtest gtest_main)
target_link_libraries(test Mealy Moore)

add_compile_options(test -std=c++)