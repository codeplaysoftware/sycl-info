////////////////////////////////////////////////////////////////////////////////
// impl-matchers.cpp
//
//   Copyright (C) Codeplay Software Limited.
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

#include "impl_matchers.hpp"

#include <fstream>
#include <target_selector/target_selector.hpp>

namespace sycl_info {

using_target_matcher::print_type using_target_matcher::from_json(
    const nlohmann::json& syclImp) {
  using_target_matcher::print_type result;

  using supported_config = select<selections::supported_configurations>;
  using plat_name = select<selections::plat_name>;
  using plat_vendor = select<selections::plat_vendor>;
  using dev_name = select<selections::dev_name>;
  using dev_vendor = select<selections::dev_vendor>;
  using drivers = select<selections::supported_drivers>;

  for (const auto& config : syclImp[supported_config::value]) {
    platform plat = {config[plat_name::value], config[plat_vendor::value], {}};
    device dev = {config[dev_name::value], config[dev_vendor::value], {}};
    std::copy(config[drivers::value].begin(), config[drivers::value].end(),
              std::back_inserter(dev.drivers));

    auto insertedPos = result.insert(std::move(plat));
    insertedPos.first->devices.insert(std::move(dev));
  }

  return result;
}

// =============================================================================
// Operations/steps                               | Cost
// -----------------------------------------------------------------------------
//                                                |
// 1. Find the common elements between the two    | O(n1 + n2), set n = n1 + n2
// sets(the syclinfo file and the current         | where n1 is the size of the
// available platforms in the system) and store   | the first set and n2 the
// them in a set C.                               | the size of the second
// (set_intersection on platforms)                |
//                                                |
// 2. For each platform element k in the common   | set_intersection() takes
// set C match its device with the systems.       | O(x + y) where x is the
// (set_intersection on devices). We also search  | number of devices available
// k in the current impls (as we will use it).    | in a platform in the common
// That search, is O(lgn2)                        | set C and y is the number of
//                                                | available devices for that
//                                                | given platform in the system
//                                                | Iterating over the set C
//                                                | takes O(min(n1, n2))
// -----------------------------------------------------------------------------
// Total runtime of the aglorithm: O((n1 + n2) + min(n1, n2) * (x + y)) =
//                                 O(min(n1, n2) * (lgn2 + x + y))
// =============================================================================
// If there are concerns about performance in the future consider to map this
// to a searching problem and use a hash table for quick O(1) lookups. This,
// solution requires writing a hash function for the structs platforms &&
// devices.

using_target_matcher::print_type using_target_matcher::match(
    const nlohmann::json& syclImpJson, const print_type& systemImp) {
  print_type syclImp = from_json(syclImpJson);
  print_type result;

  std::set_intersection(syclImp.begin(), syclImp.end(), systemImp.begin(),
                        systemImp.end(), std::inserter(result, result.begin()));
  // step 2
  for (auto& plat : result) {
    std::set<device> dev;
    auto it = systemImp.find(plat);  // O(lgn2) search
    std::set_intersection(plat.devices.begin(), plat.devices.end(),
                          it->devices.begin(), it->devices.end(),
                          std::inserter(dev, dev.begin()));
    plat.devices = std::move(dev);
  }

  return result;
}

void dump_picked_impl(const using_target_matcher::print_type& platforms,
                      std::ostream& out) noexcept {
  int platformIndex = 1;
  for (const auto& platform : platforms) {
    out << "====================================================\n";
    out << platformIndex << ". Platform name: " << platform.name;
    out << "\nPlatform vendor: " << platform.vendor << '\n';
    ++platformIndex;
    int deviceIndex = 1;
    for (const auto& device : platform.devices) {
      out << "  " << deviceIndex << ". Device name: " << device.name << '\n';
      out << "     Device vendor: " << device.vendor << '\n';
      out << "     Supported drivers: " << '\n';
      ++deviceIndex;
      for (const auto& driver : device.drivers) {
        out << driver << ' ';
      }
    }
    out << '\n';
  }
}

using_target_matcher::print_type to_print_type(
    const std::vector<std::pair<cl_platform_id, cl_device_id>>& devices) {
  using_target_matcher::print_type converted;

  for (const auto device : devices) {
    using namespace target_selector;
    using_target_matcher::platform plat;

    using_target_matcher::device dev;
    plat.name = trim_end(target_selector::get_info_from_opencl(
        device.first, CL_PLATFORM_NAME, clGetPlatformInfo));
    plat.vendor = trim_end(target_selector::get_info_from_opencl(
        device.first, CL_PLATFORM_VENDOR, clGetPlatformInfo));

    dev.name = trim_end(target_selector::get_info_from_opencl(
        device.second, CL_DEVICE_NAME, clGetDeviceInfo));
    dev.vendor = trim_end(target_selector::get_info_from_opencl(
        device.second, CL_DEVICE_VENDOR, clGetDeviceInfo));

    auto it = converted.insert(std::move(plat));
    it.first->devices.insert(std::move(dev));
  }

  return converted;
}

std::pair<bool, int> retrieve_index_for_impl(
    const std::string& chosenImpl,
    const std::vector<nlohmann::json>& impls) noexcept {
  std::pair<bool, int> implIndexQuery{false, -1};
  int implIndex = 1;
  for (const auto& impl : impls) {
    if (impl["name"].get<std::string>() == chosenImpl) {
      return std::pair<bool, int>{true, implIndex};
    }
    ++implIndex;
  }

  try {
    implIndex = std::stoi(chosenImpl, nullptr);

    // sycl_info outputs starts from 1...N
    const size_t last = impls.size();
    if ((1 <= implIndex) && (implIndex <= last)) {
      return std::pair<bool, int>{true, implIndex};
    }
  } catch (std::invalid_argument) {
    return std::pair<bool, int>{false, -1};
  }
}

using_target_matcher::print_type match_picked_impl(
    const unsigned int index, const std::vector<nlohmann::json>& impls,
    const bool displayAll) {
  auto devices = std::vector<std::pair<cl_platform_id, cl_device_id>>();
  auto platforms = std::unordered_map<std::string, std::string>{};

  bool all = true;
  target_selector::find_devices(devices, "*", "*", all, platforms, all);

  using_target_matcher::print_type devs = to_print_type(devices);
  if (displayAll) {
    return devs;
  } else {
    return using_target_matcher::match(impls[index - 1], devs);
  }
}

void print_picked_impl(const unsigned int index,
                       const std::vector<nlohmann::json>& impls,
                       const bool displayAll, std::ostream& out) {
  const auto result = match_picked_impl(index, impls, displayAll);
  dump_picked_impl(result, out);
}

bool is_config_index_valid(const using_target_matcher::print_type& impls,
                           const std::pair<int, int> configIndex) noexcept {
  // sycl_info outputs starts from 1...N
  constexpr int first = 1;
  const size_t last = impls.size();
  const bool isPlatValid =
      (first <= configIndex.first) && (configIndex.first <= last);
  if (isPlatValid) {
    auto platIt = impls.begin();
    std::advance(platIt, configIndex.first - 1);
    return (first <= configIndex.second) &&
           (configIndex.second <= platIt->devices.size());
  }
  return false;
}

config get_config(const unsigned int index,
                  const std::vector<nlohmann::json>& impls,
                  const std::pair<int, int> configIndex,
                  const bool displayAll) {
  // sycl_info outputs starts from 1...N
  const auto result = match_picked_impl(index, impls, displayAll);
  if (is_config_index_valid(result, configIndex)) {
    auto platIt = result.begin();
    std::advance(platIt, configIndex.first - 1);
    auto devIt = platIt->devices.begin();
    std::advance(devIt, configIndex.second - 1);
    return config{platIt->name, devIt->name};
  }

  return config{};
}

backend_info get_backend_from_target(
    nlohmann::json::const_iterator foundElement, const std::string& target) {
  using dev_flags = select<selections::dev_flags>;
  using supported_backend_targets =
      select<selections::supported_backend_targets>;

  using backend = select<selections::backend>;
  const auto& syclinfoBackends =
      (*foundElement)[supported_backend_targets::value];

  using elem_type = decltype(*begin(syclinfoBackends));

  auto pick_backend = [&target](const elem_type& elem) {
    return elem[backend::value] == target;
  };

  const auto foundBackend =
      find_if(begin(syclinfoBackends), end(syclinfoBackends), pick_backend);

  if (foundBackend != end(syclinfoBackends)) {
    return {(*foundBackend)[backend::value], (*foundBackend)[dev_flags::value]};
  }

  return backend_info{};
}

backend_info get_default_backend(
    const nlohmann::json::const_iterator foundElement) {
  using supported_backend = select<selections::supported_backend_targets>;
  using dev_flags = select<selections::dev_flags>;
  using backend = select<selections::backend>;

  constexpr int firstElement = 0;
  const auto elem = (*foundElement)[supported_backend::value][firstElement];
  return backend_info{elem[backend::value], elem[dev_flags::value]};
}

backend_info match_config_with_impls(const config& conf,
                                     const nlohmann::json& impl,
                                     const std::string& target) {
  using configs = select<selections::supported_configurations>;
  using platform = select<selections::plat_name>;
  using device = select<selections::dev_name>;

  using inner_type = decltype(impl[configs::value]);
  using std::begin;
  using std::end;
  const auto& supportedImpl = impl[configs::value];

  auto is_match = [&](const inner_type& elem) {
    return (elem[platform::value] == conf.platform) &&
           (elem[device::value] == conf.device);
  };

  auto foundElement =
      std::find_if(begin(supportedImpl), end(supportedImpl), is_match);

  if (foundElement != end(supportedImpl)) {
    return (!target.empty()) ? get_backend_from_target(foundElement, target)
                             : get_default_backend(foundElement);
  }

  return backend_info{};
}

void dump_config(const backend_info& info, std::ostream& out) noexcept {
  out << "Backend: " << info.backend << "\n"
      << "Device flags: " << info.deviceFlags << "\n";
}

void print_config(const unsigned int index,
                  const std::vector<nlohmann::json>& impls,
                  const std::pair<int, int> configIndex,
                  const std::string& target, const bool displayAll,
                  std::ostream& out) {
  const auto config = get_config(index, impls, configIndex, displayAll);
  if (!config.platform.empty()) {
    // sycl_info outputs starts from 1...N
    const auto info = match_config_with_impls(config, impls[index - 1], target);
    dump_config(info, out);
  }
}

}  // namespace sycl_info
