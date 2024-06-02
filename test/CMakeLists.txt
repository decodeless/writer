# Copyright (c) 2024 Pyarelal Knowles, MIT License

cmake_minimum_required(VERSION 3.20)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

if(NOT TARGET decodeless::offset_ptr)
  if(DECODELESS_SEARCH_DEPENDENCIES)
    find_package(decodeless_offset_ptr REQUIRED CONFIG PATHS
                 "${CMAKE_CURRENT_LIST_DIR}/../../offset_ptr")
  elseif(DECODELESS_FETCH_DEPENDENCIES)
    include(FetchContent)
    FetchContent_Declare(
      decodeless_offset_ptr
      GIT_REPOSITORY https://github.com/decodeless/offset_ptr.git
      GIT_TAG 1f87a9d7a8be23b90c06124014f286df91562006)
    FetchContent_MakeAvailable(decodeless_offset_ptr)
  else()
    message(
      FATAL_ERROR
        "Dependency decodeless::offset_ptr not found! You can enable searching with DECODELESS_SEARCH_DEPENDENCIES or downloading with DECODELESS_FETCH_DEPENDENCIES"
    )
  endif()
endif()

# Unit tests
add_executable(${PROJECT_NAME}_tests src/writer.cpp)
target_link_libraries(${PROJECT_NAME}_tests decodeless::writer
                      decodeless::offset_ptr gtest_main gmock_main)

# TODO: presets?
# https://stackoverflow.com/questions/45955272/modern-way-to-set-compiler-flags-in-cross-platform-cmake-project
if(MSVC)
  target_compile_options(${PROJECT_NAME}_tests PRIVATE /W4 /WX)
  target_compile_definitions(${PROJECT_NAME}_tests PRIVATE WIN32_LEAN_AND_MEAN=1
                                                           NOMINMAX)
else()
  target_compile_options(${PROJECT_NAME}_tests PRIVATE -Wall -Wextra -Wpedantic
                                                       -Werror)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
      ${PROJECT_NAME}_tests
      PRIVATE $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG>
              $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG_BACKTRACE>)
    try_compile(
      HAS_STDCXX_LIBBACKTRACE SOURCE_FROM_CONTENT
      stdc++_libbacktrace_test.cpp "int main() { return 0; }"
      LINK_LIBRARIES stdc++_libbacktrace)
    if(HAS_STDCXX_LIBBACKTRACE)
      target_link_libraries(${PROJECT_NAME}_tests
                            $<$<CONFIG:Debug>:stdc++_libbacktrace>)
    endif()
  endif()

  # Ignore "may be used uninitialized" in MemoryMap(MemoryMap&& other) noexcept
  # I'm like 90% sure this is just a gcc bug. gcc 10, 12 and clang have no
  # issues
  if(CMAKE_COMPILER_IS_GNUCC
     AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11
     AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
    target_compile_options(${PROJECT_NAME}_tests
                           PRIVATE -Wno-error=maybe-uninitialized)
  endif()
endif()

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
