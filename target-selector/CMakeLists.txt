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

find_package(OpenCL REQUIRED)

add_library(target-selector target_selector.cpp)
add_library(Codeplay::target-selector ALIAS target-selector)
target_set_opencl_properties(TARGET target-selector VERSION 120)

target_link_libraries(target-selector PUBLIC OpenCL::OpenCL)
target_compile_features(target-selector PUBLIC cxx_std_11)
target_include_directories(target-selector
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/target_selector
        ${CMAKE_CURRENT_BINARY_DIR}/include/target_selector
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
set_target_properties(target-selector PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

install(TARGETS target-selector EXPORT target-selector-targets
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT target-selector-base
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT target-selector-base
        NAMELINK_COMPONENT target-selector-devel
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT target-selector-devel)
install(DIRECTORY include
    DESTINATION .
    COMPONENT target-selector-devel
    FILES_MATCHING PATTERN "*.h"
                   PATTERN "*.hpp")

set(config_package_location "${CMAKE_INSTALL_LIBDIR}/cmake/target-selector")

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/target-selector/target-selector-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

export(EXPORT target-selector-targets
  FILE "${PROJECT_BINARY_DIR}/target-selector/target-selector-targets.cmake"
  NAMESPACE Codeplay::)
configure_file(target-selector-config.cmake
  "${PROJECT_BINARY_DIR}/target-selector/target-selector-config.cmake"
  COPYONLY)

install(EXPORT target-selector-targets
  FILE target-selector-targets.cmake
  NAMESPACE Codeplay::
  DESTINATION ${config_package_location}
  COMPONENT target-selector-devel)
install(
  FILES
    target-selector-config.cmake
    "${PROJECT_BINARY_DIR}/target-selector/target-selector-config-version.cmake"
  DESTINATION ${config_package_location}
  COMPONENT target-selector-devel)

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE.TXT"
    DESTINATION share/licenses/target-selector
    COMPONENT target-selector-base)

cpack_add_component_group(target-selector
    DISPLAY "target-selector"
    DESCRIPTION "The target-selector library"
)

cpack_add_component(target-selector-base
    DISPLAY "target-selector"
    DESCRIPTION "The target-selector library"
    INSTALL_TYPES User
    GROUP target-selector
)

cpack_add_component(target-selector-devel
    DISPLAY "target-selector development files"
    DESCRIPTION "Software development files for sycl-info"
    INSTALL_TYPES Developer
    DEPENDS target-selector-base
    GROUP target-selector
)

get_target_property(soversion target-selector SOVERSION)

set_component_debian_properties(COMPONENT target-selector-base
    PACKAGE_DEPENDS opencl-icd
    PACKAGE_NAME "libtarget-selector${soversion}"
    FILE_NAME "libtarget-selector${soversion}-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.deb"
)

set_component_debian_properties(COMPONENT target-selector-devel
    PACKAGE_DEPENDS "libtarget-selector${PROJECT_VERSION}" opencl-dev
    PACKAGE_NAME "libtarget-selector${soversion}-dev"
    FILE_NAME "libtarget-selector${soversion}-dev-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.deb"
)

set_component_rpm_properties(COMPONENT target-selector-base
    PACKAGE_DEPENDS ocl-icd
    PACKAGE_NAME "libtarget-selector${soversion}"
    FILE_NAME "libtarget-selector${soversion}-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.rpm"
)

set_component_rpm_properties(COMPONENT target-selector-devel
    PACKAGE_DEPENDS "libtarget-selector${PROJECT_VERSION}" ocl-icd-devel opencl-headers
    PACKAGE_NAME "libtarget-selector${soversion}-devel"
    FILE_NAME "libtarget-selector${soversion}-devel-${PROJECT_VERSION}.${CMAKE_SYSTEM_PROCESSOR}.rpm"
)

add_subdirectory(include)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
