configure_file(config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/config.hpp)

include(GenerateExportHeader)
generate_export_header(target-selector
    EXPORT_FILE_NAME export.hpp
)

target_compile_definitions(target-selector PRIVATE
    $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,STATIC_LIBRARY>:TARGET_SELECTOR_STATIC_DEFINE>
    target_selector_EXPORTS
)
target_sources(target-selector PRIVATE
    color_scope.hpp
    target_selector.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/export.hpp
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/config.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/export.hpp
    DESTINATION
        include/target_selector
    COMPONENT
        target-selector-devel
)
