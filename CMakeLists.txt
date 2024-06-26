# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.11)

include(FetchContent)

project(gemma)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FetchContent_Declare(highway GIT_REPOSITORY https://github.com/google/highway.git GIT_TAG da250571a45826b21eebbddc1e50d0c1137dee5f)
FetchContent_MakeAvailable(highway)

## Note: absl needs to be installed by sentencepiece. This will only happen if
## cmake is invoked with -DSPM_ENABLE_SHARED=OFF and -DSPM_ABSL_PROVIDER=module
FetchContent_Declare(sentencepiece GIT_REPOSITORY https://github.com/google/sentencepiece GIT_TAG 53de76561cfc149d3c01037f0595669ad32a5e7c)
FetchContent_MakeAvailable(sentencepiece)

set(SOURCES
  gemma.cc
  compression/blob_store.cc
  compression/blob_store.h
  compression/compress.h
  compression/compress-inl.h
  compression/nuq.h
  compression/nuq-inl.h
  compression/sfp.h
  compression/sfp-inl.h
  compression/test_util.h
  util/app.h
  util/args.h
  )

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Allowable types for WEIGHT_TYPE:
# float - slow, not recommended
# hwy::bfloat16_t - bfloat16 as implemented by https://github.com/google/highway
# SfpStream - 8-bit switched floating point (recommended)
# NuqStream - experimental, work-in-progress
option(WEIGHT_TYPE "Set weight type" "")

if (WEIGHT_TYPE)
  add_definitions(-DGEMMA_WEIGHT_T=${WEIGHT_TYPE})
endif()

# Executable Target

add_executable(gemma run.cc)
target_sources(gemma PRIVATE ${SOURCES})
set_property(TARGET gemma PROPERTY CXX_STANDARD 17)
target_link_libraries(gemma hwy hwy_contrib sentencepiece)
target_include_directories(gemma PRIVATE ./)
FetchContent_GetProperties(sentencepiece)
target_include_directories(gemma PRIVATE ${sentencepiece_SOURCE_DIR})
target_compile_definitions(gemma PRIVATE $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS NOMINMAX>)
target_compile_options(gemma PRIVATE $<$<PLATFORM_ID:Windows>:-Wno-deprecated-declarations>)

## Library Target

add_library(libgemma ${SOURCES})
set_property(TARGET libgemma PROPERTY CXX_STANDARD 17)
set_target_properties(libgemma PROPERTIES PREFIX "")
set_property(TARGET libgemma PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(libgemma PUBLIC ./)
target_link_libraries(libgemma hwy hwy_contrib sentencepiece)
target_include_directories(libgemma PUBLIC ${sentencepiece_SOURCE_DIR})
target_compile_definitions(libgemma PRIVATE $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS NOMINMAX>)
target_compile_options(libgemma PRIVATE $<$<PLATFORM_ID:Windows>:-Wno-deprecated-declarations>)

set(GEMMA_ENABLE_TESTS OFF CACHE BOOL "Enable Gemma tests")
if (GEMMA_ENABLE_TESTS)

set(GEMMA_TEST_FILES
  ops_test.cc
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

foreach (TESTFILE IN LISTS GEMMA_TEST_FILES)
  # The TESTNAME is the name without the extension or directory.
  get_filename_component(TESTNAME ${TESTFILE} NAME_WE)
  add_executable(${TESTNAME} ${TESTFILE})
  # Test all targets, not just the best/baseline. This changes the default
  # policy to all-attainable; note that setting -DHWY_COMPILE_* directly can
  # cause compile errors because only one may be set, and other CMakeLists.txt
  # that include us may set them.
  target_compile_options(${TESTNAME} PRIVATE -DHWY_IS_TEST=1)

  target_link_libraries(${TESTNAME} PRIVATE libgemma GTest::gtest_main hwy hwy_contrib hwy_test)

  gtest_discover_tests(${TESTNAME})
endforeach ()
endif()  # GEMMA_ENABLE_TESTS

## Tools

add_executable(compress_weights compress_weights.cc)
target_link_libraries(compress_weights libgemma hwy hwy_contrib)
