cmake_minimum_required(VERSION 3.28)

project(PythonStuffTests)

enable_testing()
include(GoogleTest)

function(add_unit_test name libs)
    add_executable(${name} ${name}.cpp)
    target_include_directories(
        ${name} 
        PRIVATE
            ${GOOGLE_TEST_SOURCE_DIR}/googletest/include
    )   

    target_link_libraries(
        ${name} 
        PRIVATE
            GTest::gtest_main
            ${libs}     
    )
    gtest_discover_tests(${name})
endfunction()

add_unit_test(test_one2onequeue one2onequeue)
add_unit_test(test_coalescingringbuffer coalescingringbuffer)




