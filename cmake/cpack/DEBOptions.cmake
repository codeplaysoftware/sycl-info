function(set_component_debian_properties)
    set(deb_single_value_properties
      DEBUGINFO_PACKAGE
      FILE_NAME
      PACKAGE_ARCHITECTURE
      PACKAGE_CONTROL_EXTRA
      PACKAGE_CONTROL_STRICT_PERMISSION
      PACKAGE_NAME
      PACKAGE_PRIORITY
      PACKAGE_SECTION
      PACKAGE_SHLIBDEPS
      PACKAGE_SOURCE
      PACKAGE_SUGGESTS
    )

    set(deb_multi_value_properties
      PACKAGE_BREAKS
      PACKAGE_CONFLICTS
      PACKAGE_DEPENDS
      PACKAGE_ENHANCES
      PACKAGE_PREDEPENDS
      PACKAGE_PROVIDES
      PACKAGE_RECOMMENDS
      PACKAGE_REPLACES
    )
    set(single_value_args COMPONENT ${deb_single_value_properties})
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${single_value_args}" "${deb_multi_value_properties}")

    string(TOUPPER ${ARG_COMPONENT} component)

    foreach(prop ${deb_single_value_properties} ${deb_multi_value_properties})
        if(DEFINED ARG_${prop})
            set(CPACK_DEBIAN_${component}_${prop} ${ARG_${prop}} CACHE INTERNAL "")
        endif()
    endforeach()
endfunction()

set(CPACK_DEBIAN_COMPRESSION_TYPE xz)
if(${CMAKE_BUILD_TYPE} MATCHES "(Debug|RelWithDebInfo)")
    set(CPACK_DEBIAN_DEBUGINFO_PACKAGE ON)
endif()
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON)

if(NOT DEFINED CPACK_DEB_COMPONENT_INSTALL)
    set(CPACK_DEB_COMPONENT_INSTALL ON)
endif()
