////////////////////////////////////////////////////////////////////////////////
// target_selector.cpp
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
#include "target_selector.h"
#include "color_scope.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

namespace target_selector {

/**
 * @brief A string that's returned when a function should return an invalid
 *        string instead of throwing an exception
 */
static constexpr const char* errorString = "(ERROR)";

/**
 * @brief Value of an invalid device type
 */
static constexpr cl_device_type invalidDeviceType = 0;

/**
 * @brief helper funtion that converts a string to lower case.
 * @note For Unicode: most C++ standard libraries will handle
 * Unicode characters correctly,this is more likely to be an
 * issue on embedded platforms where the STL can not support different locales.
 */
void inplace_lower(std::string& s) {
  std::transform(std::begin(s), std::end(s), std::begin(s), [](const char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  });
}

/**
 * @brief matches a set of platforms with the requested string/platform name
 */
bool match_platform(
    std::string requested, std::string platformName,
    const std::unordered_map<std::string, std::string>& platforms) {
  inplace_lower(requested);
  inplace_lower(platformName);

  if (is_target_any(requested) ||
      platformName.find(requested) != std::string::npos) {
    return true;
  }

  using elem_type = std::unordered_map<std::string, std::string>::value_type;
  auto result = std::find_if(
      std::begin(platforms), std::end(platforms),
      [&platformName](const elem_type platform) {
        return (platformName.find(platform.second) != std::string::npos);
      });
  return (result != std::end(platforms));
}

/**
 * @brief matches if the device is an accelerator, or gpu or cpu.
 */
cl_device_type match_device_type(std::string requested) {
  if (is_target_any(requested)) {
    return CL_DEVICE_TYPE_ALL;
  }
  inplace_lower(requested);
  if (requested == "gpu") {
    return CL_DEVICE_TYPE_GPU;
  }
  if (requested == "cpu") {
    return CL_DEVICE_TYPE_CPU;
  }
  if (requested == "accel") {
    return CL_DEVICE_TYPE_ACCELERATOR;
  }
  return invalidDeviceType;
}

std::string getenv_variable(const char* identifier) {
#if defined(_WIN32) && !defined(__GNUC__)
  size_t len = 1;
  errno_t envValue = getenv_s(&len, nullptr, 0, identifier);
  if (envValue != 0 || len == 0) {
    return std::string{};
  }
  auto result = std::string(len, 0);
  getenv_s(&len, &result[0], result.size(), identifier);
  return result;
#endif  //_WIN32

#ifdef __unix__
  const char* envValue = std::getenv(identifier);
  return (envValue) ? std::string{envValue} : std::string{};
#endif  //__linux__
}

/*!
  @brief Checks whether the device supports SPIR.
  @param device The device to get the info from
  @return Whether the device supports SPIR or not.
*/
bool has_spir(cl_device_id device) noexcept {
  auto extensions =
      get_info_from_opencl(device, CL_DEVICE_EXTENSIONS, clGetDeviceInfo);
  return (extensions.find("cl_khr_spir") != std::string::npos);
}

std::string get_target_from_environment_var() {
  return getenv_variable("SYCL_TARGET");
}

void parse_target(const std::string& target, std::string& vendor,
                  std::string& deviceType) {
  size_t colonLocation = target.find(':');
  if (colonLocation != std::string::npos) {
    vendor = target.substr(0, colonLocation);
    deviceType =
        target.substr(colonLocation + 1, target.length() - colonLocation);
  } else {
    vendor = target;  // does not contain a deviceType
    deviceType = "";
  }
  if (target.find(':', colonLocation + 1) != std::string::npos) {
    throw std::runtime_error("Requested target string has no valid format: " +
                             target);
  }
}

bool is_target_any(const std::string& target) {
  return (target.empty() || (target == "any") || (target == "*"));
}

void find_devices(
    std::vector<std::pair<cl_platform_id, cl_device_id>>& devicesFound,
    const std::string& reqVendor, const std::string& reqDeviceType,
    bool& usedVendorAsType,
    const std::unordered_map<std::string, std::string>& platforms, bool all) {
  ::cl_uint num_platforms = 0;

  target_selector_warn_on_cl_error(
      [&num_platforms]() {
        return clGetPlatformIDs(0, nullptr, &num_platforms);
      },
      "Unable to retrieve number of platforms.");

  if (num_platforms == 0) {
    // The ICD loader could not find any platforms
    return;
  }

  auto targetPlatforms = std::vector<cl_platform_id>(num_platforms);
  target_selector_warn_on_cl_error(
      [&num_platforms, &targetPlatforms]() {
        return clGetPlatformIDs(num_platforms, targetPlatforms.data(), nullptr);
      },
      "Unable to retrieve platforms.");

  auto get_platform_name = [](cl_platform_id platform) {
    size_t len = 0;

    auto result = target_selector_warn_on_cl_error(
        [&platform, &len]() {
          return clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, nullptr,
                                   &len);
        },
        "Unable to retrieve size of platform name");
    if (result != CL_SUCCESS) {
      return std::string{"N/A"};
    }

    auto platformName = std::string(len, '\0');
    result = target_selector_warn_on_cl_error(
        [&platform, &platformName, &len]() {
          return clGetPlatformInfo(platform, CL_PLATFORM_NAME, len,
                                   &platformName[0], nullptr);
        },
        "Unable to retrieve platform name");
    if (result != CL_SUCCESS) {
      return std::string{"N/A"};
    }

    return platformName;
  };

  auto get_device_type = [&platforms, &usedVendorAsType](
                             const std::string& reqVendor,
                             const std::string& reqDeviceType,
                             const std::string& platformName) {
    cl_device_type deviceType = invalidDeviceType;
    auto reqVendorStr = reqVendor;
    auto reqDeviceTypeStr = reqDeviceType;

    if (!is_target_any(reqVendorStr)) {
      // First try using the vendor string as the device type
      deviceType = match_device_type(reqVendorStr);
    }
    if (deviceType != invalidDeviceType) {
      // The vendor string was used as device type
      if (!is_target_any(reqDeviceTypeStr)) {
        target_selector_throw_error("Cannot specify device type twice: " +
                                    reqVendorStr + ":" + reqDeviceTypeStr);
      }
      reqDeviceTypeStr = reqVendorStr;
      reqVendorStr.clear();
      if (!usedVendorAsType) {
        usedVendorAsType = true;
      }
      return deviceType;
    }

    constexpr cl_device_type skipFlag = 0;
    if (!match_platform(reqVendorStr, platformName, platforms)) {
      return skipFlag;
    }

    deviceType = match_device_type(reqDeviceTypeStr);
    if (deviceType == invalidDeviceType) {
      target_selector_throw_error("Invalid device type: " + reqDeviceTypeStr);
    }
    return deviceType;
  };

  for (auto platform : targetPlatforms) {
    const auto platformName = get_platform_name(platform);

    constexpr size_t skipCurrentIteration = 0;
    const cl_device_type deviceType =
        get_device_type(reqVendor, reqDeviceType, platformName);
    if (deviceType == skipCurrentIteration) {
      continue;
    }

    ::cl_uint numDevices = 0;
    ::cl_int err = target_selector_warn_on_cl_error(
        [&platform, &deviceType, &numDevices]() {
          return clGetDeviceIDs(platform, deviceType, 0, nullptr, &numDevices);
        },
        std::string{": Unable to retrieve number of devices for platform " +
                    platformName});

    if (err == CL_DEVICE_NOT_FOUND || err != CL_SUCCESS) {
      continue;
    }

    auto devices = std::vector<cl_device_id>(numDevices);
    target_selector_warn_on_cl_error(
        [&platform, deviceType, &numDevices, &devices]() {
          return clGetDeviceIDs(platform, deviceType, numDevices,
                                devices.data(), nullptr);
        },
        std::string{"could not find platform: " + platformName});

    for (const auto device : devices) {
      if (all || has_spir(device)) {
        devicesFound.emplace_back(platform, device);
      }
    }
  }
}

void target_selector_warning(const std::string& message) {
#ifdef __linux__
  color_scope cs(color_code::red, std::cout);
#endif
  std::cout << message << '\n';
}

void target_selector_throw_error(const std::string& message) {
  target_selector_warning(message);
  std::stringstream stream;
  stream << "Sycl Info error: " << message;
  throw sycl_info_error(stream.str());
}

std::string trim_end(std::string str) {
  if (str.back() == ' ' || str.back() == '\0') {
    str.pop_back();
  }

  return str;
}

}  // namespace target_selector
