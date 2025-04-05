
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "../../")

function(add_module name libs)
  add_library(${name} MODULE ${name}.c)
  target_link_libraries(${name} PRIVATE Python3::Python ${libs})

  if(WIN32)
    set_target_properties(${name} PROPERTIES SUFFIX ".pyd")
  else()
    set_target_properties(${name} PROPERTIES SUFFIX ".so")
  endif()
endfunction()

add_library(one2onequeue STATIC one2onequeue.c)
target_compile_definitions(one2onequeue PRIVATE _ATOMIC_OPERATORS_DEFINED)
add_library(coalescingringbuffer STATIC coalescingringbuffer.c)
target_compile_definitions(coalescingringbuffer PRIVATE _ATOMIC_OPERATORS_DEFINED)

add_module(spam "")
add_module(custom "")
add_module(message_queue one2onequeue)

