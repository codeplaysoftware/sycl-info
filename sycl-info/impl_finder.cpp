////////////////////////////////////////////////////////////////////////////////
// sycl_info_imp_finder.cpp
//
// Copyright (C) Codeplay Software Limited.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "impl_finder.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <target_selector/target_selector.hpp>

using json = nlohmann::json;

// This is defined for both 32 and 64
#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#elif __unix__
#include <dirent.h>
#else
#error "Not a windows/POSIX environment"
#endif

namespace sycl_info {

bool is_sycl_env_set() {
  return !get_sycl_vendors_from_environment_var().empty();
}

void cache_path(std::vector<json>& cache, const std::string& path) {
  std::ifstream file{path};
  if (file) {
    json j;
    file >> j;
    cache.push_back(std::move(j));
  }
}

bool ends_with_suffix(const std::string& str, const std::string& suffix) {
  if (str.size() < suffix.size()) {
    return false;
  }

  auto result = std::mismatch(suffix.crbegin(), suffix.crend(), str.crbegin());

  return result.first == suffix.crend();
}

std::string concat_path(const std::string& path, char separator,
                        const std::string& file) {
  return std::string{path + separator + file};
}

bool ends_with_sycl(const std::string& file) {
  const auto extension = std::string{".syclinfo"};
  return ends_with_suffix(file, extension);
}

#ifdef __linux__
std::vector<json> find_sycl_impls(const std::vector<std::string>& paths) {
  auto cache = std::vector<json>{};
  const char separator = '/';
  for (const auto& path : paths) {
    auto customDeleter = [](DIR* ptr) { closedir(ptr); };
    auto dirDesc = std::unique_ptr<DIR, decltype(customDeleter)>(
        opendir(path.c_str()), customDeleter);
    struct dirent* dir;

    if (dirDesc) {
      while ((dir = readdir(dirDesc.get()))) {
        const auto file = std::string{dir->d_name};
        if (ends_with_sycl(file)) {
          cache_path(cache, concat_path(path, separator, file));
        }
      }
    }
  }
  return cache;
}
#endif  //__linux__

#ifdef _WIN32
std::vector<json> find_sycl_impls(const std::vector<std::string>& paths) {
  auto cache = std::vector<json>{};
  const char separator = '\\';
  for (const auto& path : paths) {
    WIN32_FIND_DATA data;

    // Find all files with a given extension
    auto customDeleter = [](HANDLE ptr) { FindClose(ptr); };

    // HANDLE is a typedef for PVOID which in respect is a typedef of void*
    // Therefore to access the type we need std::remove_pointer
    auto hFind = std::unique_ptr<std::remove_pointer<HANDLE>::type,
                                 decltype(customDeleter)>(
        FindFirstFileA((path + "\\*").c_str(), &data), customDeleter);

    if (hFind.get() != INVALID_HANDLE_VALUE) {
      do {
        const auto file = std::string{data.cFileName};
        if (ends_with_sycl(file)) {
          cache_path(cache, concat_path(path, separator, file));
        }
      } while (FindNextFile(hFind.get(), &data) != 0);
    }
  }
  return cache;
}
#endif  //_WIN32

void dump_impls(const std::vector<json>& implementations, std::ostream& out) {
  if (implementations.empty()) {
    out << "No SYCL implementation(s) available\n";
    return;
  }
  out << "SYCL implementation(s) available:\n\n";

  int i = 0;
  for (const auto& info : implementations) {
    out << ++i << ". ";
    out << info["name"] << " | " << info["version"] << " | " << info["vendor"]
        << "\n";
  }
  out << "\nTo select an implementation use command --impl with the index or "
         "name of the implementation.\n";
}

std::string get_sycl_vendors_from_environment_var() {
  return target_selector::getenv_variable("SYCL_VENDOR_PATHS");
}

std::vector<json> get_impls(std::string hint) {
  std::vector<std::string> paths{};
  if (is_sycl_env_set()) {
    const char semicolon = ';';
    auto envVarPaths =
        split<char>(get_sycl_vendors_from_environment_var(), semicolon);
    for (auto& path : envVarPaths) {
      paths.push_back(path);
    }
#ifdef _WIN32
    // We need to iterate on windows and remove end of string character '\0'
    // because the c-strings retrieved have the character '\0' appended at the
    // end
    for (auto& path : paths) {
      path.pop_back();
    }
#endif
  }
  if (!hint.empty()) {
    paths.push_back(std::move(hint));
  }

  if (!paths.empty()) {
    return find_sycl_impls(paths);

  } else {
    return std::vector<json>{};
  }
}

void print_impls(std::ostream& out, std::string hint) {
  auto implementations = get_impls(std::move(hint));
  dump_impls(implementations, out);
}
}  // namespace sycl_info
