function(set_component_rpm_properties)
    set(rpm_single_value_properties
      BUILD_SOURCE_DIRS_PREFIX
      DEBUGINFO_FILE_NAME
      DEBUGINFO_PACKAGE
      FILE_NAME
      PACKAGE_ARCHITECTURE
      PACKAGE_AUTOREQ
      PACKAGE_AUTOREQPROV
      PACKAGE_DESCRIPTION
      PACKAGE_GROUP
      PACKAGE_NAME
      PACKAGE_PREFIX
      PACKAGE_CONFLICTS
      PACKAGE_OBSOLETES
      PACKAGE_PROVIDES
      PACKAGE_REQUIRES
      PACKAGE_REQUIRES_POST
      PACKAGE_REQUIRES_POSTUN
      PACKAGE_REQUIRES_PRE
      PACKAGE_REQUIRES_PREUN
      PACKAGE_SUGGESTS
      PACKAGE_SUMMARY
      PACKAGE_URL
      POST_INSTALL_SCRIPT_FILE
      POST_UNINSTALL_SCRIPT_FILE
      PRE_INSTALL_SCRIPT_FILE
      PRE_UNINSTALL_SCRIPT_FILE
      INSTALL_PREFIX_RELOCATION
    )

    set(rpm_multi_value_properties
      USER_FILELIST
    )

    set(single_value_args COMPONENT ${rpm_single_value_properties})
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${single_value_args}" "${rpm_multi_value_properties}")

    string(TOUPPER ${ARG_COMPONENT} component)

    foreach(prop ${rpm_single_value_properties} ${rpm_multi_value_properties})
        if(DEFINED ARG_${prop})
            set(CPACK_RPM_${component}_${prop} ${ARG_${prop}} CACHE INTERNAL "")
        endif()
    endforeach()
endfunction()

set(CPACK_RPM_COMPRESSION_TYPE xz)
if(${CMAKE_BUILD_TYPE} MATCHES "(Debug|RelWithDebInfo)")
    set(CPACK_RPM_DEBUGINFO_PACKAGE ON)
endif()

if(NOT DEFINED CPACK_RPM_COMPONENT_INSTALL)
    set(CPACK_RPM_COMPONENT_INSTALL ON)
endif()
