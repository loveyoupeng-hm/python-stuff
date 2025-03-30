cmake_minimum_required(VERSION 3.31)

project(pyextension)


set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CPM_SOURCE_CACHE ${LIB_DIR}/cpm)

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
    VERSION 1.15.2
    SOURCE_DIR ${LIB_DIR}/googletest
    OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"

)


include_directories("src/c/includes")


function(add_module name)
  add_library(${name} MODULE src/c/${name}.c)
  target_link_libraries(${name} PRIVATE Python3::Python)
  if(WIN32)
    set_target_properties(${name} PROPERTIES SUFFIX ".pyd")
  else()
    set_target_properties(${name} PROPERTIES SUFFIX ".so")
  endif()
endfunction(add_module name path)

add_module(spam)
add_module(custom)
add_module(message_queue)

add_subdirectory(tests/c/)