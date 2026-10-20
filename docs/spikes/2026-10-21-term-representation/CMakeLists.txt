# Spike for ADR-0005; see main.cpp.
cmake_minimum_required(VERSION 3.28)
project(tante_spike_term_representation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(SPIKE_SANITIZE "Build with sanitizers" OFF)

# The module is a shared library, so the engine cannot inline its code, nor it the engine's.
add_library(spike_module SHARED module.cpp)
target_compile_definitions(spike_module PRIVATE SPIKE_MODULE_BUILD)
set_target_properties(spike_module PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)

add_executable(spike main.cpp)
target_link_libraries(spike PRIVATE spike_module)

foreach(t spike spike_module)
  if(MSVC)
    target_compile_options(${t} PRIVATE /W4 /WX /permissive- /utf-8)
    target_compile_definitions(${t} PRIVATE _CRT_SECURE_NO_WARNINGS)
  else()
    target_compile_options(${t} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror)
  endif()
  if(SPIKE_SANITIZE)
    if(MSVC)
      target_compile_options(${t} PRIVATE /fsanitize=address /Zi)
      target_link_options(${t} PRIVATE /INCREMENTAL:NO)
    else()
      target_compile_options(${t} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
      target_link_options(${t} PRIVATE -fsanitize=address,undefined)
    endif()
  endif()
endforeach()

if(SPIKE_SANITIZE)
  target_compile_definitions(spike PRIVATE SPIKE_SANITIZE)
endif()
