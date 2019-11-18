#!/bin/bash

set -e
set -x

pip install conan --upgrade
pip install conan_package_tools==0.30.2

conan user
