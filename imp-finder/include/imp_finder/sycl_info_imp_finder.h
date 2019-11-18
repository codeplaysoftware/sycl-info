////////////////////////////////////////////////////////////////////////////////
// sycl_info_imp_finder.h
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

#ifndef SYCL_INFO_IMP_FINDER_H
#define SYCL_INFO_IMP_FINDER_H

#include <CL/opencl.h>
#include <algorithm>
#include <fstream>
#include <impl_matchers/impl_matchers.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace sycl_info {

/// \brief Checks if the environmental variable "SYCL_INFO" is set
/// \returns true if it's set, false otherwise
///
bool is_sycl_env_set();

/// \brief Puts a path into the cache. First it loads the json specified by the
/// path and then puts its contents to the cache
/// \param a vector of the json representation of each sycl info file
///
void cache_path(std::vector<nlohmann::json>& cache, const std::string& path);

/// \brief Utility function that checks if a string ends with another string
/// \param two strings str and endsWith
/// \returns true if the string str ends with the string endsWith,
///  false otherwise
///
bool ends_with_suffix(const std::string& str, const std::string& suffix);

/// \brief Helper function that calls ends_with_suffix() with ".sycl"
/// \param a file name
/// \returns true if the file suffix is ".sycl", false otherwise
///
bool ends_with_sycl(const std::string& file);

/// \brief Constructs the full path given a folder, a platform depedent
/// separator and a file name
/// \param a path to a folder, a target separator (e.g. "/" for linux) and
/// the file name
/// \returns the full platform depedent path
///
std::string concat_path(const std::string& path, char separator,
                        const std::string& file);

/// \brief Linux/Windows implementation for finding .sycl files
/// \param
/// \returns a vector of the found implementations
///
std::vector<nlohmann::json> find_sycl_impls(
    const std::vector<std::string>& path);

/// \brief Utility function that dumps the found SYCL implementations to
/// a stream
/// \param a cache populated with the SYCL implementations and a stream object
///
void dump_impls(const std::vector<nlohmann::json>& implementations,
                std::ostream& out);

/// \brief Utility function that prints the found SYCL implementations
/// \param a stream to print and an optional hint
///
void print_impls(std::ostream& out, std::string hint = {});

/// \brief Utility function that retrieves the found SYCL implementation
/// \returns a vector of info for each implementation found
///
std::vector<nlohmann::json> get_impls(std::string hint = {});

/// \brief Helper function thas accesses the SYCL_VENDOR_PATH env variable
/// \returns a string with the path set in the SYCL_VENDOR_PATH env variable
///
std::string get_sycl_vendors_from_environment_var();
/// \brief Pretty print function for the --using command line option
///
void dump_picked_impl(
    const std::vector<std::pair<cl_platform_id, cl_device_id>>& devices,
    std::ostream& out);

/// \brief Picks a sycl-info implementation and displays
///
void print_picked_impl(std::ostream& out);

/// \brief Checks if the given id is inside the range of the container
/// \returns true if the id is inside the index range of impls
bool is_picked_index_valid(unsigned int id,
                           const std::vector<nlohmann::json>& impls) noexcept;

///\brief Utility template function that splits a comma seperated list of
/// basic_string<CharT, Traits> given a CharT token
/// \param a comma seperate basic_string<CharT, Traits> and a CharT separator
/// token
/// \returns a vector of basic_string<CharT, Traits>
///
template <class CharT, class Traits = std::char_traits<CharT>>
std::vector<std::basic_string<CharT, Traits>> split(
    std::basic_string<CharT, Traits> const& str, CharT const token) {
  using std::begin;
  using std::end;
  using std::vector;

  auto result = vector<std::basic_string<CharT, Traits>>{};
  for (auto first = begin(str); first != end(str);) {
    auto spliterator = std::find(first, end(str), token);

    // only consider sequences that we're not splitting on
    if (std::distance(first, spliterator) > 1) {
      result.emplace_back(first, spliterator);
    }

    first = std::move(spliterator);
    if (first != end(str)) {
      ++first;
    }
  }

  return result;
}

}  // namespace sycl_info

#endif
