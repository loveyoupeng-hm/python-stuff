cmake_minimum_required(VERSION 3.31)

project(pyextension)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

find_package (Python3 REQUIRED COMPONENTS Development.Embed)

add_library(spam MODULE src/c/simple.c)
target_link_libraries(spam PRIVATE Python3::Python)

if(WIN32)
    set_target_properties(spam PROPERTIES SUFFIX ".pyd")
else()
    set_target_properties(spam PROPERTIES SUFFIX ".so")
endif()