#
#  Copyright Morris Hafner
#  Copyright Christopher Di Bella
#  Copyright Codeplay Software, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from conans import ConanFile, CMake, RunEnvironment, tools
from conans.tools import SystemPackageTool
import os
import re


def get_version():
    try:
        with open("CMakeLists.txt") as cml:
            content = cml.read()
        version = re.search("project\\(sycl-info\\s+VERSION\\s+([^\\s\\)]*)",
                            content).group(1)
        return version.strip()
    except Exception as e:
        return None


class SYCLInfoConan(ConanFile):
    name = "sycl-info"
    description = "Print information about available SYCL implementations"
    author = "Codeplay Software, Ltd."
    license = "Apache-2.0"
    url = "https://github.com/codeplaysoftware/sycl-info"
    version = get_version()
    topics = ("cpp", "c++", "sycl")
    settings = ("os", "compiler", "arch", "build_type")
    generators = ("cmake", "virtualrunenv")
    options = {
        "build_docs": [False, True],
        "build_testing": [False, True],
        "clang_tidy": [False, True],
        "clang_tidy_werror": [False, True],
        "fPIC": [False, True],
        "shared": [False, True]
    }
    default_options = {
        "build_docs": False,
        "build_testing": False,
        "clang_tidy": False,
        "clang_tidy_werror": False,
        "fPIC": False,
        "khronos-opencl-icd-loader:shared": True,
        "shared": False
    }
    requires = "khronos-opencl-icd-loader/20190827@bincrafters/stable"
    build_requires = "lyra/1.1.0", "jsonformoderncpp/3.7.2@vthiery/stable"
    exports_sources = (
        ".clang*",
        ".gitignore",
        "cmake/*",
        "CMakeLists.txt",
        "imp-finder/*",
        "impl-matchers/*",
        "LICENSES.TXT",
        "README.md",
        "sycl-info-cpack.cmake",
        "sycl-info/*",
        "target-selector/*",
    )
    no_copy_source = True

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build_requirements(self):
        if self.options.build_testing:
            self.build_requires("doctest/2.3.4@bincrafters/stable")

        installer = SystemPackageTool()
        if self.options.build_docs:
            os_info = tools.OSInfo()
            if os_info.with_pacman:
                installer.install("ruby-ronn")
            if os_info.with_yum:
                installer.install("rubygem-ronn")
            if os_info.with_apt:
                installer.install("ruby-ronn")

    _cmake = None

    @property
    def cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.definitions.update({
                "BUILD_DOCS":
                self.options.build_docs,
                "BUILD_TESTING":
                self.options.build_testing,
                "SYCL_INFO_CLANG_TIDY":
                self.options.clang_tidy,
                "SYCL_INFO_CLANG_TIDY_WERROR":
                self.options.clang_tidy_werror,
            })
            self._cmake.configure()
        return self._cmake

    def build(self):
        self.cmake.build()

        if self.options.build_testing:
            env_build = RunEnvironment(self)
            with tools.environment_append(env_build.vars):
                self.cmake.test(output_on_failure=True)

    def package(self):
        self.cmake.install()
        self.copy("LICENSES.TXT",
                  dst=os.path.join(self.package_folder, "share", "licenses",
                                   "sycl-info"))

    def package_info(self):
        self.env_info.PATH.append(os.path.join(self.package_folder, "bin"))
        self.env_info.MANPATH.append(
            os.path.join(self.package_folder, "share", "man"))

    def package_id(self):
        del self.info.options.build_testing
        del self.info.options.clang_tidy
        del self.info.options.clang_tidy_werror
