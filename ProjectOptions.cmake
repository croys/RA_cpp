include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(RA_cpp_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(RA_cpp_setup_options)
  option(RA_cpp_ENABLE_HARDENING "Enable hardening" ON)
  option(RA_cpp_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    RA_cpp_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    RA_cpp_ENABLE_HARDENING
    OFF)

  RA_cpp_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR RA_cpp_PACKAGING_MAINTAINER_MODE)
    option(RA_cpp_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(RA_cpp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(RA_cpp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(RA_cpp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(RA_cpp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(RA_cpp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(RA_cpp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(RA_cpp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(RA_cpp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(RA_cpp_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(RA_cpp_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(RA_cpp_ENABLE_PCH "Enable precompiled headers" OFF)
    option(RA_cpp_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(RA_cpp_ENABLE_IPO "Enable IPO/LTO" ON)
    option(RA_cpp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(RA_cpp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(RA_cpp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(RA_cpp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(RA_cpp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(RA_cpp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(RA_cpp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(RA_cpp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(RA_cpp_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(RA_cpp_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(RA_cpp_ENABLE_PCH "Enable precompiled headers" OFF)
    option(RA_cpp_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      RA_cpp_ENABLE_IPO
      RA_cpp_WARNINGS_AS_ERRORS
      RA_cpp_ENABLE_USER_LINKER
      RA_cpp_ENABLE_SANITIZER_ADDRESS
      RA_cpp_ENABLE_SANITIZER_LEAK
      RA_cpp_ENABLE_SANITIZER_UNDEFINED
      RA_cpp_ENABLE_SANITIZER_THREAD
      RA_cpp_ENABLE_SANITIZER_MEMORY
      RA_cpp_ENABLE_UNITY_BUILD
      RA_cpp_ENABLE_CLANG_TIDY
      RA_cpp_ENABLE_CPPCHECK
      RA_cpp_ENABLE_COVERAGE
      RA_cpp_ENABLE_PCH
      RA_cpp_ENABLE_CACHE)
  endif()

  RA_cpp_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (RA_cpp_ENABLE_SANITIZER_ADDRESS OR RA_cpp_ENABLE_SANITIZER_THREAD OR RA_cpp_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(RA_cpp_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(RA_cpp_global_options)
  if(RA_cpp_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    RA_cpp_enable_ipo()
  endif()

  RA_cpp_supports_sanitizers()

  if(RA_cpp_ENABLE_HARDENING AND RA_cpp_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR RA_cpp_ENABLE_SANITIZER_UNDEFINED
       OR RA_cpp_ENABLE_SANITIZER_ADDRESS
       OR RA_cpp_ENABLE_SANITIZER_THREAD
       OR RA_cpp_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${RA_cpp_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${RA_cpp_ENABLE_SANITIZER_UNDEFINED}")
    RA_cpp_enable_hardening(RA_cpp_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(RA_cpp_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(RA_cpp_warnings INTERFACE)
  add_library(RA_cpp_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  RA_cpp_set_project_warnings(
    RA_cpp_warnings
    ${RA_cpp_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(RA_cpp_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(RA_cpp_options)
  endif()

  include(cmake/Sanitizers.cmake)
  RA_cpp_enable_sanitizers(
    RA_cpp_options
    ${RA_cpp_ENABLE_SANITIZER_ADDRESS}
    ${RA_cpp_ENABLE_SANITIZER_LEAK}
    ${RA_cpp_ENABLE_SANITIZER_UNDEFINED}
    ${RA_cpp_ENABLE_SANITIZER_THREAD}
    ${RA_cpp_ENABLE_SANITIZER_MEMORY})

  set_target_properties(RA_cpp_options PROPERTIES UNITY_BUILD ${RA_cpp_ENABLE_UNITY_BUILD})

  if(RA_cpp_ENABLE_PCH)
    target_precompile_headers(
      RA_cpp_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(RA_cpp_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    RA_cpp_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(RA_cpp_ENABLE_CLANG_TIDY)
    RA_cpp_enable_clang_tidy(RA_cpp_options ${RA_cpp_WARNINGS_AS_ERRORS})
  endif()

  if(RA_cpp_ENABLE_CPPCHECK)
    RA_cpp_enable_cppcheck(${RA_cpp_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(RA_cpp_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    RA_cpp_enable_coverage(RA_cpp_options)
  endif()

  if(RA_cpp_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(RA_cpp_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(RA_cpp_ENABLE_HARDENING AND NOT RA_cpp_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR RA_cpp_ENABLE_SANITIZER_UNDEFINED
       OR RA_cpp_ENABLE_SANITIZER_ADDRESS
       OR RA_cpp_ENABLE_SANITIZER_THREAD
       OR RA_cpp_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    RA_cpp_enable_hardening(RA_cpp_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
