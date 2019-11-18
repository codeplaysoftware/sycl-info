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

find_program(RONN_EXECUTABLE ronn)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ronn REQUIRED_VARS RONN_EXECUTABLE)

if(ronn_FOUND AND NOT TARGET ronn::ronn)
    add_executable(ronn::ronn IMPORTED)
    set_target_properties(ronn::ronn PROPERTIES
        IMPORTED_LOCATION ${RONN_EXECUTABLE}
    )
    mark_as_advanced(RONN_EXECUTABLE)
endif()

#[[ Generate documentation through ronn.
    ronn_generate(
        [TARGET <target>] [ALL]
        INPUTS <inputs...>
        OUTPUT <output>
        [ORGANIZATION <organization>]
        [MANUAL <manual>]
        [DATE <date>]
        [ROFF] [HTML] [FRAGMENT] [MARKDOWN]
    )
    Creates a custom command calling ronn on the specified inputs and output.
    Relative paths are interpreted relative to the current source directory.
#]]
function(ronn_generate)
    cmake_parse_arguments(PARSE_ARGV 0 RONN
        "ROFF;HTML;FRAGMENT;MARKDOWN;ALL"
        "ORGANIZATION;MANUAL;DATE;OUTPUT;TARGET"
        "INPUTS")
    if(NOT RONN_OUTPUT)
        message(FATAL_ERROR "Output filename must be specified")
    elseif(NOT RONN_INPUTS)
        message(FATAL_ERROR "ronn requires at least one input file")
    endif()

    if(RONN_ORGANIZATION)
        set(organization --organization ${RONN_ORGANIZATION})
    endif()
    if(RONN_MANUAL)
        set(manual --manual ${RONN_MANUAL})
    endif()
    if(RONN_DATE)
        set(date --date ${RONN_DATE})
    endif()
    if(RONN_ROFF)
        set(roff -r)
    endif()
    if(RONN_HTML)
        set(html -5)
    endif()
    if(RONN_FRAGMENT)
        set(fragment -f)
    endif()
    if(RONN_MARKDOWN)
        set(markdown --markdown)
    endif()

    add_custom_command(OUTPUT ${RONN_OUTPUT}
        COMMAND ronn::ronn --pipe
            ${organization} ${manual} ${date}
            ${roff} ${html} ${fragment} ${markdown}
            ${RONN_INPUTS} > ${RONN_OUTPUT}
        DEPENDS ${RONN_INPUTS}
        COMMENT "Generating documentation file ${RONN_OUTPUT}..."
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM COMMAND_EXPAND_LISTS
    )

    if(RONN_TARGET)
        if(RONN_ALL)
            set(all ALL)
        endif()
        add_custom_target(${RONN_TARGET} ${all}
            DEPENDS ${RONN_OUTPUT}
        )
    endif()
endfunction()
