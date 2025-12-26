cmake_minimum_required(VERSION 3.20)

enable_testing()

find_package(GTest REQUIRED)

add_executable(test_spsc test_spsc.cpp)
target_link_libraries(test_spsc PRIVATE itq GTest::gtest GTest::gtest_main pthread)
target_compile_options(test_spsc PRIVATE -Wall -Wextra -Wpedantic -O2)
add_test(NAME test_spsc COMMAND test_spsc)

add_executable(test_spmc test_spmc.cpp)
target_link_libraries(test_spmc PRIVATE itq GTest::gtest GTest::gtest_main pthread)
target_compile_options(test_spmc PRIVATE -Wall -Wextra -Wpedantic -O2)
add_test(NAME test_spmc COMMAND test_spmc)
