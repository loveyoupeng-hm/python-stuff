cmake_minimum_required(VERSION 3.31)


project(pyextension)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_MODULE_PREFIX "")
set(CMAKE_SHARED_MODULE_PREFIX "")


set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(CPM_SOURCE_CACHE ${LIB_DIR}/cpm)
set(CMAKE_C_COMPILER_VERSION 13)

include(cmake/cpm.cmake)


set(THREADS_PREFER_PTHREAD_FLAG ON)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

find_package (Python3 REQUIRED COMPONENTS Development.Embed)
find_package(Threads REQUIRED)
enable_testing()

CPMAddPackage(
    NAME GOOGLE_TEST
    GITHUB_REPOSITORY google/googletest
    VERSION 1.16.0
    SOURCE_DIR ${LIB_DIR}/googletest
    OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"

)

CPMAddPackage(
    NAME benchmark
    GITHUB_REPOSITORY google/benchmark
    VERSION 1.9.2
    SOURCE_DIR ${LIB_DIR}/benchmark
    OPTIONS "BENCHMARK_ENABLE_TESTING Off"
)
if(benchmark_ADDED)
  set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
endif()

include_directories("src/c/includes")

add_subdirectory(src/c)
add_subdirectory(tests/c/)
add_subdirectory(benchmark/cpp/)
