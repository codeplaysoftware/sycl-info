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

find_package(ronn REQUIRED)

if(UNIX OR MINGW)
    ronn_generate(TARGET sycl-info-doc-man ALL
        INPUTS sycl-info.1.md
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sycl-info.1
        ORGANIZATION "Codeplay Software"
        MANUAL "SYCL Programmer's Manual"
        ROFF
    )

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sycl-info.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/
        COMPONENT sycl-info-base)
endif()

ronn_generate(TARGET sycl-info-doc-html ALL
    INPUTS sycl-info.1.md
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sycl-info.1.html
    ORGANIZATION "Codeplay Software"
    MANUAL "SYCL Programmer's Manual"
    HTML
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sycl-info.1.html
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/
    COMPONENT sycl-info-base)
