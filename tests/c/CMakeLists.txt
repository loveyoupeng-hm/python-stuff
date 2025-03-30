cmake_minimum_required(VERSION 3.28)

project(PythonStuffTests)

enable_testing()

add_executable(test_one2onequeue test_one2onequeue.cpp)

target_include_directories(
    test_one2onequeue
    PRIVATE
        ${GOOGLE_TEST_SOURCE_DIR}/googletest/include
)
target_compile_options(test_one2onequeue PRIVATE -std=c++17)


target_link_libraries(
    test_one2onequeue
    PRIVATE
        GTest::gtest_main
        one2onequeue
)

include(GoogleTest)
gtest_discover_tests(test_one2onequeue)