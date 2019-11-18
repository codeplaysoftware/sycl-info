#[[ Copyright Codeplay Software, Ltd.

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

include_guard()

#[[ Set the OpenCL version define as a COMPATIBLE_INTERFACE property.

 target_set_opencl_properties(
     TARGET <target>
     VERSION <version>
 )
 This sets an OPENCL_VERSION property and propagates it. If two versions
 are present, the higher one is picked and publicly added to
 the COMPILE_DEFINITIONS. This is BUILD_INTERFACE only.
#]]
function(target_set_opencl_properties)
    cmake_parse_arguments(TSOP "" "TARGET;VERSION" "" ${ARGN})
    set_property(TARGET ${TSOP_TARGET} APPEND PROPERTY
        COMPATIBLE_INTERFACE_NUMBER_MAX OPENCL_VERSION)
    set_target_properties(${TSOP_TARGET} PROPERTIES
        OPENCL_VERSION ${TSOP_VERSION}
        INTERFACE_OPENCL_VERSION ${TSOP_VERSION})
    target_compile_definitions(${TSOP_TARGET} PUBLIC
        $<BUILD_INTERFACE:CL_TARGET_OPENCL_VERSION=$<TARGET_PROPERTY:OPENCL_VERSION>>)
endfunction()
