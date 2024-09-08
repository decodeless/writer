# Copyright (c) 2024 Pyarelal Knowles, MIT License

cmake_minimum_required(VERSION 3.20)

project(decodeless_writer)

set(CMAKE_CXX_STANDARD 20)

option(DECODELESS_SEARCH_DEPENDENCIES
       "Enable searching for dependencies in adjacent directories" ON)
option(DECODELESS_FETCH_DEPENDENCIES
       "Enable fetching dependencies with cmake FetchContent" OFF)

if(NOT TARGET decodeless::allocator)
  if(DECODELESS_SEARCH_DEPENDENCIES)
    option(DECODELESS_ALLOCATOR_SEARCH_PATH
           "${CMAKE_CURRENT_LIST_DIR}/../allocator")
    find_package(decodeless_allocator REQUIRED CONFIG PATHS
                 "${DECODELESS_ALLOCATOR_SEARCH_PATH}")
  elseif(DECODELESS_FETCH_DEPENDENCIES)
    include(FetchContent)
    FetchContent_Declare(
      decodeless_allocator
      GIT_REPOSITORY https://github.com/decodeless/allocator.git
      GIT_TAG 772bcb7e0c0f5bbdf3358395046e995a9d6c1ffb)
    FetchContent_MakeAvailable(decodeless_allocator)
  else()
    message(
      FATAL_ERROR
        "Dependency decodeless::allocator not found! You can enable searching with DECODELESS_SEARCH_DEPENDENCIES or downloading with DECODELESS_FETCH_DEPENDENCIES"
    )
  endif()
endif()

if(NOT TARGET decodeless::mappedfile)
  if(DECODELESS_SEARCH_DEPENDENCIES)
    option(DECODELESS_MAPPEDFILE_SEARCH_PATH
           "${CMAKE_CURRENT_LIST_DIR}/../mappedfile")
    find_package(decodeless_mappedfile REQUIRED CONFIG PATHS
                 "${DECODELESS_MAPPEDFILE_SEARCH_PATH}")
  elseif(DECODELESS_FETCH_DEPENDENCIES)
    include(FetchContent)
    FetchContent_Declare(
      decodeless_mappedfile
      GIT_REPOSITORY https://github.com/decodeless/mappedfile.git
      GIT_TAG bbd066e089210ac06063a9d4e278749b275fffe9)
    FetchContent_MakeAvailable(decodeless_mappedfile)
  else()
    message(
      FATAL_ERROR
        "Dependency decodeless::mappedfile not found! You can enable searching with DECODELESS_SEARCH_DEPENDENCIES or downloading with DECODELESS_FETCH_DEPENDENCIES"
    )
  endif()
endif()

add_library(decodeless_writer INTERFACE)
target_include_directories(decodeless_writer INTERFACE include)
target_link_libraries(decodeless_writer INTERFACE decodeless::allocator
                                                  decodeless::mappedfile)

add_library(decodeless::writer ALIAS decodeless_writer)

if(BUILD_TESTING)
  option(BUILD_DECODELESS_TESTING "Enable decodeless testing" ON)
  if(BUILD_DECODELESS_TESTING)
    enable_testing()
    add_subdirectory(test)
  endif()
endif()
