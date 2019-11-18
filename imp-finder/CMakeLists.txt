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

find_package(nlohmann_json REQUIRED)

add_library(imp-finder OBJECT sycl_info_imp_finder.cpp)
add_library(Codeplay::imp-finder ALIAS imp-finder)

set_target_properties(imp-finder PROPERTIES OPENCL_VERSION 120)
target_compile_features(imp-finder PUBLIC cxx_std_11)
target_include_directories(imp-finder
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/imp_finder
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_link_libraries(imp-finder PUBLIC
    Codeplay::target-selector
    Codeplay::impl-matchers
    nlohmann_json::nlohmann_json)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
