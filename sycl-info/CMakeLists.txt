#[[  Copyright (C) Codeplay Software Limited.

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

find_package(Lyra REQUIRED)
find_package(nlohmann_json REQUIRED)

configure_file(config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/config.hpp)

add_executable(sycl-info)
target_sources(sycl-info PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
    cli_config.hpp
    impl_finder.hpp impl_finder.cpp
    impl_matchers.hpp impl_matchers.cpp
    sycl_info.cpp
    utility.hpp
)
target_include_directories(sycl-info PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(sycl-info PRIVATE
    Lyra::Lyra
    Codeplay::target-selector
    nlohmann_json
)

install(TARGETS sycl-info EXPORT sycl-info-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT sycl-info-base)

set(config_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/sycl-info)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/sycl-info/sycl-info-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

export(EXPORT sycl-info-targets
  FILE "${PROJECT_BINARY_DIR}/sycl-info/sycl-info-targets.cmake"
  NAMESPACE SYCL::)
configure_file(sycl-info-config.cmake
  "${PROJECT_BINARY_DIR}/sycl-info/sycl-info-config.cmake"
  COPYONLY)

install(EXPORT sycl-info-targets
  FILE sycl-info-targets.cmake
  NAMESPACE SYCL::
  DESTINATION ${config_package_location}
  COMPONENT sycl-info-devel)
install(
  FILES
    sycl-info-config.cmake
    "${PROJECT_BINARY_DIR}/sycl-info/sycl-info-config-version.cmake"
  DESTINATION ${config_package_location}
  COMPONENT sycl-info-devel)

install(FILES "${PROJECT_SOURCE_DIR}/LICENSES.TXT"
    DESTINATION share/licenses/sycl-info
    COMPONENT sycl-info-base)

cpack_add_component_group(sycl-info
    DISPLAY_NAME "sycl-info"
    DESCRIPTION "Prints metadata about available SYCL implementations"
    BOLD_TITLE
)

cpack_add_component(sycl-info-base
    DISPLAY "sycl-info"
    DESCRIPTION "The sycl-info tool"
    INSTALL_TYPES User
    DEPENDS target-selector-base
    GROUP sycl-info
)

cpack_add_component(sycl-info-devel
    DISPLAY "sycl-info development files"
    DESCRIPTION "Software development files for sycl-info"
    INSTALL_TYPES Developer
    DEPENDS sycl-info-base target-selector-devel
    GROUP sycl-info
)

get_target_property(target_selector_soversion target-selector SOVERSION)

set_component_debian_properties(COMPONENT sycl-info-base
    PACKAGE_DEPENDS "libtarget-selector${target_selector_soversion}"
    PACKAGE_NAME "sycl-info"
    FILE_NAME "sycl-info-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.deb"
)

set_component_debian_properties(COMPONENT sycl-info-devel
    PACKAGE_DEPENDS "libtarget-selector${target_selector_soversion}-dev"
    PACKAGE_NAME "sycl-info-dev"
    FILE_NAME "sycl-info-dev-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.deb"
)

set_component_rpm_properties(COMPONENT sycl-info-base
    PACKAGE_DEPENDS "libtarget-selector${target_selector_soversion}"
    PACKAGE_NAME "sycl-info"
    FILE_NAME "sycl-info-${PROJECT_VERSION}.${CMAKE_SYSTEM_PROCESSOR}.rpm"
)

set_component_rpm_properties(COMPONENT sycl-info-devel
    PACKAGE_DEPENDS "libtarget-selector${target_selector_soversion}-devel"
    PACKAGE_NAME "sycl-info-devel"
    FILE_NAME "sycl-info-devel-${PROJECT_VERSION}.${CMAKE_SYSTEM_PROCESSOR}.rpm"
)

if(BUILD_DOCS)
    add_subdirectory(doc)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
