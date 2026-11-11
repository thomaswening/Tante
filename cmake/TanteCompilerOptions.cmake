# The compiler guardrails of ADR-0001 point 3
# (docs/adr/0001-cpp20-cmake-vcpkg-guardrails.md), applied to every target of
# Tante's own with tante_apply_guardrails(<target>).
#
# Warnings become errors through CMAKE_COMPILE_WARNING_AS_ERROR, which every
# preset in CMakePresets.json turns on. It is not hard-coded here, so a
# developer trying a newer compiler can turn it off on the command line
# (-DCMAKE_COMPILE_WARNING_AS_ERROR=OFF) without editing this file.

option(TANTE_SANITIZE
  "Build with AddressSanitizer, and UndefinedBehaviorSanitizer where the compiler has it"
  OFF)

function(tante_apply_guardrails target)
  # MSVC is a variable CMake sets to true when the compiler is Microsoft's.
  if(MSVC)
    # /W4: warning level 4 of 4, the strictest that is still usable. (/Wall
    #   also warns about the standard library's own headers.)
    # /permissive-: follow the standard instead of accepting old MSVC-only code.
    # /utf-8: source files and string literals are UTF-8 on every system.
    # /w15038: member initialisers out of declaration order, off even at /W4.
    # /external:W0: no warnings from third-party headers, which CMake passes
    #   as external (system) includes.
    target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /w15038 /external:W0)
  else()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow)
  endif()

  if(TANTE_SANITIZE)
    tante_apply_sanitizers(${target})
  endif()
endfunction()

function(tante_apply_sanitizers target)
  if(MSVC)
    # MSVC has AddressSanitizer only. /Zi gives the reports file and line
    # numbers; incremental linking does not work with it.
    target_compile_options(${target} PRIVATE /fsanitize=address /Zi)
    target_link_options(${target} PRIVATE /INCREMENTAL:NO)
    # MSVC's standard library marks the unused capacity of vectors, strings
    # and optionals for AddressSanitizer, and the linker refuses to mix such
    # code with code built without the sanitizer: the vcpkg libraries, Catch2
    # first. Turning the marks off costs only the check for reading the unused
    # capacity of a container; out-of-bounds access and use after free are
    # still caught.
    target_compile_definitions(${target} PRIVATE
      _DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION _DISABLE_OPTIONAL_ANNOTATION)
  else()
    # -fno-sanitize-recover=all: the first finding ends the program, so a test
    # fails instead of printing a report nobody reads.
    set(sanitizer_flags -fsanitize=address,undefined -fno-sanitize-recover=all)
    target_compile_options(${target} PRIVATE ${sanitizer_flags} -fno-omit-frame-pointer)
    target_link_options(${target} PRIVATE ${sanitizer_flags})
  endif()
endfunction()
