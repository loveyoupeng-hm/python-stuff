cmake_minimum_required(VERSION 3.28)

project(PythonStuffTests)

enable_testing()

add_executable(${PROJECT_NAME} test.cpp)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${GOOGLE_TEST_SOURCE_DIR}/googletest/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})