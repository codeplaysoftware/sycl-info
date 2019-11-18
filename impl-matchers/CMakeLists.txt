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

add_library(impl-matchers OBJECT include/impl_matchers/impl_matchers.h impl-matchers.cpp)
add_library(Codeplay::impl-matchers ALIAS impl-matchers)

set_target_properties(impl-matchers PROPERTIES OPENCL_VERSION 120)
target_compile_features(impl-matchers PUBLIC cxx_std_11)
target_include_directories(impl-matchers
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/impl_matchers
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
set_target_properties(impl-matchers PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

target_link_libraries(impl-matchers PUBLIC nlohmann_json
                                    PRIVATE Codeplay::target-selector)
