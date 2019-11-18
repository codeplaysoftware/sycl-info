////////////////////////////////////////////////////////////////////////////////
// impl-matchers.h
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

#ifndef IMPL_MATCHERS_H
#define IMPL_MATCHERS_H

#include <CL/opencl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// This macro expands to the boilerplate of all the comparison operators
// The operator< must be defined first by the programmer. Any other
// operators are defined relatively to the operator<
#define SYCL_INFO_IMPL_MATCHERS_DEFINE_COMPARISON_OPERATORS(type)   \
  friend inline bool operator>(const type& lhs, const type& rhs) {  \
    return rhs < lhs;                                               \
  }                                                                 \
  friend inline bool operator==(const type& lhs, const type& rhs) { \
    return !(lhs < rhs) && !(rhs < lhs);                            \
  }                                                                 \
  friend inline bool operator!=(const type& lhs, const type& rhs) { \
    return !(lhs == rhs);                                           \
  }                                                                 \
  friend inline bool operator<=(const type& lhs, const type& rhs) { \
    return !(rhs < lhs);                                            \
  }                                                                 \
  friend inline bool operator>=(const type& lhs, const type& rhs) { \
    return !(lhs < rhs);                                            \
  }

namespace sycl_info {
struct using_target_matcher {
  // The layout we want to model/print
  //--------------------------------------
  // Platform name:         name
  // Platform vendor:       vendor
  //   device1:
  //     device_name:       name
  //     device_vendor:     vendor
  //     supported_drivers: 1.2, 1.3, etc
  //   device2:
  //     device_name:       name
  //     device_vendor:     vendor
  //     supported_drivers: 1.2, 1.3, etc
  //   ...
  //   ...
  //   deviceN:
  // Platform Name:         nameN
  // ...
  //--------------------------------------

  /// @brief A device consists of a name, a vendor and
  /// a vector of the supported drivers
  ///
  struct device {
    std::string name;
    std::string vendor;
    /// @brief This is necessarry as this struct will be stored in
    /// an std::set. Marking this mutable is important, because
    /// it allows us to modify this field without rebalancing
    /// the tree. Essentially, this field is ignored in the
    /// comparison function used by the data structure
    ///
    mutable std::vector<std::string> drivers;

    /// @brief The container of this is an std::set which requires
    /// strict weak ordering
    /// @see Strict weak ordering https://en.wikipedia.org/wiki/Weak_ordering
    /// @see std::set container requirements
    /// https://en.cppreference.com/w/cpp/container/set
    ///
    friend inline bool operator<(const device& lhs, const device& rhs) {
      return std::tie(lhs.name, lhs.vendor) < std::tie(rhs.name, rhs.vendor);
    }
    /// define the rest of the comparisson operators
    SYCL_INFO_IMPL_MATCHERS_DEFINE_COMPARISON_OPERATORS(device)
  };

  /// @brief A platform consists of a name, a vendor and
  /// a set of supported devices. Name and vendor,
  ///
  struct platform {
    std::string name;
    std::string vendor;
    /// @brief This is necessarry as this struct will be stored in
    /// an std::set. Marking this mutable is important, because
    /// it allows us to modify this field without rebalancing
    /// the tree. Essentially, this field is ignored in the
    /// comparison function used by the data structure
    ///
    mutable std::set<device> devices;

    /// @brief The container of this is an std::set which requires
    /// strict weak ordering
    /// @see Strict weak ordering https://en.wikipedia.org/wiki/Weak_ordering
    /// @see std::set container requirements
    /// https://en.cppreference.com/w/cpp/container/set
    ///
    friend inline bool operator<(const platform& lhs, const platform& rhs) {
      return std::tie(lhs.name, lhs.vendor) < std::tie(rhs.name, rhs.vendor);
    }
    /// define the rest of the comparisson operators
    SYCL_INFO_IMPL_MATCHERS_DEFINE_COMPARISON_OPERATORS(platform);
  };

  /// The type that we print
  using print_type = std::set<platform>;

  /// @brief Helper function the converts a json file to the internal
  /// representation print_type
  /// @param syclinfo json
  /// @return a print_type from the nlohmann::json
  ///
  static print_type from_json(const nlohmann::json& syclImp);

  /// @brief Finds the common platforms/devices between a sycl
  /// implementation and the system's available platforms/devices
  /// @param A syclinfo file, the current available hardware,
  /// that is, the systems platforms and devices retrieved
  /// with the target_selector
  /// @return a print_type with the matched results
  ///
  static print_type match(const nlohmann::json& syclImpJson,
                          const print_type& currentHardware);
};

/// @brief Helper function that converts a vector of platform ids and
/// and device ids into their respective string names
///
using_target_matcher::print_type to_print_type(
    const std::vector<std::pair<cl_platform_id, cl_device_id>>& devices);

/// @brief Pretty print function for the --using command line option
/// @param A result of type print_type retrieved by the match()
/// function and an ostream to print to
///
void dump_picked_impl(const using_target_matcher::print_type& devices,
                      std::ostream& out) noexcept;

/// @brief Retrieves the implementation index for a given input string
/// @param The user input string and the available implementations
/// @return A pair of bool and int, representing whether an implementation was
/// found, and if so the index of that implemetnation
///
std::pair<bool, int> retrieve_index_for_impl(
    const std::string& chosenImpl,
    const std::vector<nlohmann::json>& impls) noexcept;

/// @brief Matches the --impl index with the available implementations
/// @param The --impl index, a vector of the syclinfo implementations
/// to match
///
using_target_matcher::print_type match_picked_impl(
    const unsigned int index, const std::vector<nlohmann::json>& impls,
    const bool displayAll);

/// @brief Picks a sycl-info implementation and displays it
/// @param The --using-target index, a vector of the syclinfo
/// implementations and an ostream to print to
///
void print_picked_impl(const unsigned int index,
                       const std::vector<nlohmann::json>& impls,
                       const bool displayAll, std::ostream& out);

/// @brief Structure for platform/device pair to represent the --config option
///
struct config {
  std::string platform;
  std::string device;
};

/// @brief Gets the config from an implementation and an index
/// @return The config specified by the index
///
config get_config(const unsigned int index,
                  const std::vector<nlohmann::json>& impls,
                  const std::pair<int, int> configIndex, const bool displayAll);

/// @brief prints the config
///
void print_config(const unsigned int index,
                  const std::vector<nlohmann::json>& impls,
                  const std::pair<int, int> configIndex,
                  const std::string& target, const bool displayAll,
                  std::ostream& out);

/// @brief Checks if the user specified option config_ is a valid index
/// @return True if config_ is a valid index, false otherwise
bool is_config_index_valid(const using_target_matcher::print_type& impls,
                           const std::pair<int, int> configIndex) noexcept;

/// @brief Structure for platform/device pair to represent the backend
/// information of a specified --impl
struct backend_info {
  std::string backend;
  std::string deviceFlags;
};

/// @brief Returns the backend specified by the option --target
/// @param An iterator to the syclinfo file and the value of --target option
///
backend_info get_backend_from_target(
    nlohmann::json::const_iterator foundElement, const std::string& target);

/// @brief Returns the default backend for a target (first in the list by
/// default)
/// @param An iterator to the syclinfo file
///
backend_info get_default_backend(
    const nlohmann::json::const_iterator foundElement);

/// @brief Helper function that matches an implementation file with the
/// --config. Optinally, it filters by --target.
/// @return The backend specified by config filtered by the optional target
///
backend_info match_config_with_impls(const config& conf,
                                     const nlohmann::json& impl,
                                     const std::string& target);

/// @brief Dumps the backend information specified by --config to a stream
//
void dump_config(const backend_info& info, std::ostream& out) noexcept;

/// @brief Enum that represents the name of a field in the json file.
///
enum class selections {
  supported_configurations,
  plat_name,
  plat_vendor,
  dev_name,
  dev_flags,
  dev_vendor,
  supported_backend_targets,
  backend,
  supported_drivers
};

/// @brief Generic template that will be fully specialized for each value in
/// in the enum selections. The unspecialized base template is left undefined.
/// All in all, a field in the enum selections maps to a string value
/// representing the name of the respective field in the json file.
///
template <selections T>
struct select;

/// @brief Specialization for selections::supported_configurations
///
template <>
struct select<selections::supported_configurations> {
  static constexpr const char* value = "supported_configurations";
};

/// @brief Specialization for selections::plat_name
///
template <>
struct select<selections::plat_name> {
  static constexpr const char* value = "platform_name";
};

/// @brief Specialization for selections::plat_vendor
///
template <>
struct select<selections::plat_vendor> {
  static constexpr const char* value = "platform_vendor";
};

/// @brief Specialization for selections::device_name
///
template <>
struct select<selections::dev_name> {
  static constexpr const char* value = "device_name";
};

/// @brief Specialization for selections::device_flags
///
template <>
struct select<selections::dev_flags> {
  static constexpr const char* value = "device_flags";
};

/// @brief Specialization for selections::device_vendor
///
template <>
struct select<selections::dev_vendor> {
  static constexpr const char* value = "device_vendor";
};

/// @brief Specialization for selections::supported_backend_targets
///
template <>
struct select<selections::supported_backend_targets> {
  static constexpr const char* value = "supported_backend_targets";
};

/// @brief Specialization for selections::backend_target
///
template <>
struct select<selections::backend> {
  static constexpr const char* value = "backend_target";
};

/// @brief Specialization for selections::supported_drivers
///
template <>
struct select<selections::supported_drivers> {
  static constexpr const char* value = "supported_drivers";
};

}  // namespace sycl_info
#undef SYCL_INFO_IMPL_MATCHERS_DEFINE_COMPARISON_OPERATORS
#endif
