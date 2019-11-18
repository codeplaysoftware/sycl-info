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

# The Clang CMake package is completely broken on Ubuntu
# Look for clang-tidy ourselves instead until we switch it over to Conan
set(clang_tidy_executable_names clang-tidy clang-tidy-3.9)
foreach(i RANGE 4 9)
    list(APPEND clang_tidy_executable_names clang-tidy-${i})
endforeach()

find_program(CLANG_TIDY_EXECUTABLE NAMES ${clang_tidy_executable_names})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(clang-tidy REQUIRED_VARS CLANG_TIDY_EXECUTABLE)

if(clang-tidy_FOUND AND NOT TARGET Clang::clang-tidy)
    add_executable(Clang::clang-tidy IMPORTED GLOBAL)
    set_target_properties(Clang::clang-tidy PROPERTIES
        IMPORTED_LOCATION ${CLANG_TIDY_EXECUTABLE})
    mark_as_advanced(CLANG_TIDY_EXECUTABLE)
endif()

#[[ Enables clang-tidy globally or on a target.
  enable_clang_tidy(
      CHECKS <checks>
      WARNINGS_AS_ERRORS <werror-checks>
      [TARGET <target>]
      [CURRENT_SCOPE]
  )
  Either TARGET <target> or CURRENT_SCOPE must be passed.
  CURRENT_SCOPE sets CMAKE_<LANG>_CLANG_TIDY in the scope of the caller.
#]]
function(enable_clang_tidy)
    cmake_parse_arguments(ECT "CURRENT_SCOPE" "TARGET" "CHECKS;WARNINGS_AS_ERRORS" ${ARGN})
    if(NOT ECT_CURRENT_SCOPE AND NOT ECT_TARGET)
        message(FATAL_ERROR "enable_clang_tidy() must be called on a TARGET or with the CURRENT_SCOPE keyword")
    endif()

    list(JOIN ECT_CHECKS "," checks)
    list(JOIN ECT_WARNINGS_AS_ERRORS "," warnings_as_errors)

    find_package(clang-tidy REQUIRED)
    set(cmdline ${CLANG_TIDY_EXECUTABLE}
        -p=${CMAKE_BINARY_DIR}
        --checks=${checks}
    )
    if(ECT_WARNINGS_AS_ERRORS)
        list(APPEND cmdline --warnings-as-errors=${warnings_as_errors})
    endif()

    if(ECT_TARGET)
        set_target_properties(${ECT_TARGET} PROPERTIES
            C_CLANG_TIDY "${cmdline}"
            CXX_CLANG_TIDY "${cmdline}"
        )
    endif()
    
    if(ECT_CURRENT_SCOPE)
        set(CMAKE_C_CLANG_TIDY "${cmdline}" PARENT_SCOPE)
        set(CMAKE_CXX_CLANG_TIDY "${cmdline}" PARENT_SCOPE)
    endif()
endfunction()
