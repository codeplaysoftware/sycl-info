configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(GenerateExportHeader)
generate_export_header(target-selector
    EXPORT_FILE_NAME export.h
)

target_compile_definitions(target-selector PRIVATE
    $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,STATIC_LIBRARY>:TARGET_SELECTOR_STATIC_DEFINE>
    target_selector_EXPORTS
)
target_sources(target-selector PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/color_scope.h
    ${CMAKE_CURRENT_SOURCE_DIR}/target_selector.h
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/export.h
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/config.h
        ${CMAKE_CURRENT_BINARY_DIR}/export.h
    DESTINATION
        include/target_selector
    COMPONENT
        target-selector-devel
)
