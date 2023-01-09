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

  # {fmt} formatting library
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY git@github.com:fmtlib/fmt.git
    GIT_TAG        9.1.0
  )
  FetchContent_GetProperties(fmt)
  if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
  endif ()

  add_executable(tests EXCLUDE_FROM_ALL ./tests/test_refptr.cpp)
  target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
  target_include_directories(tests PRIVATE ./include)

  # Benchmarking binaries
  set(BENCH_COMPILE_OPTIONS "-O2")

  add_executable(bench-refptr EXCLUDE_FROM_ALL ./benchmarks/refptr.cpp)
  target_link_libraries(bench-refptr PRIVATE fmt::fmt)
  target_include_directories(bench-refptr PRIVATE ./include)
  target_compile_options(bench-refptr PRIVATE ${BENCH_COMPILE_OPTIONS})

  add_executable(bench-shared_ptr EXCLUDE_FROM_ALL ./benchmarks/shared_ptr.cpp)
  target_link_libraries(bench-shared_ptr PRIVATE fmt::fmt)
  target_include_directories(bench-shared_ptr PRIVATE ./include)
  target_compile_options(bench-shared_ptr PRIVATE ${BENCH_COMPILE_OPTIONS})

  add_executable(bench-ptr EXCLUDE_FROM_ALL ./benchmarks/ptr.cpp)
  target_link_libraries(bench-ptr PRIVATE fmt::fmt)
  target_include_directories(bench-ptr PRIVATE ./include)
  target_compile_options(bench-ptr PRIVATE ${BENCH_COMPILE_OPTIONS})


endif()
