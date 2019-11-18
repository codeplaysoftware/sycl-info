#[[ Copyright (C) Codeplay Software Limited.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
#]]

cmake_minimum_required(VERSION 3.13.0)
project(sycl-info
    VERSION 0.1
    DESCRIPTION "Prints metadata about available SYCL implementations"
    HOMEPAGE_URL "https://github.com/codeplaysoftware/sycl-info"
    LANGUAGES CXX
)

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpack")
include(CMakePackageConfigHelpers)
include(CPackComponent)
include(CTest)
include(CheckIPOSupported)
include(GNUInstallDirs)
include(OpenCLCompatibleInterface)

include(DEBOptions)
include(RPMOptions)
include(WIXOptions)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

check_ipo_supported(RESULT ipo_is_supported)
if(ipo_is_supported AND BUILD_SHARED_LIBS)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

option(BUILD_DOCS "Build the documentation" OFF)
option(BUILD_TESTING "Build the unit tests" OFF)
option(SYCL_INFO_CLANG_TIDY "Enable clang-tidy on the build" OFF)
option(SYCL_INFO_CLANG_TIDY_WERROR "Treat certain clang-tidy checks as errors" OFF)

if(SYCL_INFO_CLANG_TIDY)
    find_package(clang-tidy REQUIRED)

    set(checks
        bugprone-*
        -cert-dcl21-cpp
        -cert-dcl59-cpp
        clang-analyzer-*
        -clang-diagnostic-missing-prototypes
        -clang-diagnostic-unused-command-line-argument
        cppcoreguildelines-*
        google-*
        -google-build-namespaces
        -google-build-using-namespace
        -google-runtime-int
        -google-runtime-references
        misc-*
        modernize-*
        performance-*
        portability-*
        readability-*
        -readability-named-parameter
        -readability-uppercase-literal-suffix
    )

    if(SYCL_INFO_CLANG_TIDY_WERROR)
        set(warnings_as_errors WARNINGS_AS_ERRORS ${checks})
    endif()

    enable_clang_tidy(CURRENT_SCOPE CHECKS ${checks} ${warnings_as_errors})
endif()

add_subdirectory(target-selector)
add_subdirectory(impl-matchers)
add_subdirectory(imp-finder)
add_subdirectory(sycl-info)

include(sycl-info-cpack.cmake)
