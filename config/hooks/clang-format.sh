#!/bin/bash
#
#  Copyright Christopher Di Bella
# Copyright (C) Codeplay Software Limited.
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

format() {
   cd $1
   for i in `ls $1`; do
      if [[ -d $i ]]; then
         format `pwd`/$i
      elif [[ ${i: -4} == ".cpp" || ${i: -4} == ".cxx" || ${i: -3} == ".cc" || ${i: -2} == ".c" || \
              ${i: -4} == ".hpp" || ${i: -4} == ".hxx" || ${i: -3} == ".hh" || ${i: -2} == ".h" ]]
      then
         clang-format -i -style=file -- "$i"
      fi
   done
   cd ..
}

which clang-format
if [[ $? -ne 0 ]]; then
   echo "Could not find clang-format in PATH"
   exit 1
fi

format `pwd`/include
format `pwd`/source
format `pwd`/test

if [[ -z `git status --porcelain --untracked-files=no` ]]; then
   echo "No changes were identified after formatting: now pushing to remote."
   exit 0
else
   echo "Changes were identified after formatting: please commit these changes and re-try pushing."
   git status -v
   exit 1
fi
