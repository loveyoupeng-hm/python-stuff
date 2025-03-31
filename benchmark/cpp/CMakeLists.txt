
enable_testing()

add_executable(benchmark_one2onequeue benchmark_one2onequeue.cpp)

target_include_directories(
    benchmark_one2onequeue
    PRIVATE
        ${benchmark_SOURCE_DIR}/include
)
target_compile_options(benchmark_one2onequeue PRIVATE -std=c++17)


target_link_libraries(
    benchmark_one2onequeue
    PRIVATE
        benchmark 
        one2onequeue
)