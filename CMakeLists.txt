cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# Detect if we're being included as a subproject.  If so, disable the tests.
if(DEFINED PROJECT_NAME)
  set(IS_SUBPROJECT ON)
else()
  set(IS_SUBPROJECT OFF)
endif()

project(refptr CXX)

include(FetchContent)

# Require the C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# TaggedUnion
FetchContent_Declare(
  tagged_union
  GIT_REPOSITORY git@github.com:tsmanner/cpp-tagged-union.git
  GIT_TAG        74577a4
)
FetchContent_GetProperties(tagged_union)
if (NOT tagged_union_POPULATED)
  FetchContent_Populate(tagged_union)
  add_subdirectory(${tagged_union_SOURCE_DIR} ${tagged_union_BINARY_DIR})
endif ()


if (NOT IS_SUBPROJECT)

# Catch2 unit testing framework.
  FetchContent_Declare(
    catch2
    GIT_REPOSITORY git@github.com:CatchOrg/Catch2.git
    GIT_TAG        v3.2.1
  )
  FetchContent_GetProperties(catch2)
  if (NOT catch2_POPULATED)
    FetchContent_Populate(catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
  endif ()

  add_executable(
    tests
    EXCLUDE_FROM_ALL
    ./tests/test_refptr.cpp
  )
  target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
  target_include_directories(
    tests
    PRIVATE
    ./include
    ${catch2_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/generated-includes
  )

endif()
