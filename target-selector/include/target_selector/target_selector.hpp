////////////////////////////////////////////////////////////////////////////////
// target_selector.h
//
// Copyright 2019 Codeplay Software, Ltd.
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

#ifndef UTIL_DEVICE_SELECTOR_H
#define UTIL_DEVICE_SELECTOR_H

#include "target_selector/config.hpp"

#include "color_scope.hpp"
#include <CL/opencl.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace target_selector {

/**
  @brief Exception thrown when there is an internal error in sycl_info
*/
struct sycl_info_error : public std::runtime_error {
  sycl_info_error(std::string message) : std::runtime_error(message) {}
};

/**
 * @brief Gets the value of the an environment variable.
 *
 * Uses std::getenv as a cross-platform way for retrieving environment
 * variables: https://en.cppreference.com/w/cpp/utility/program/getenv.
 */
TARGET_SELECTOR_EXPORT std::string getenv_variable(const char* identifier);

/**get_target_from_environment_var
 * @brief Returns the value of the environment variable SYCL_VENDOR_PATHS in an
 * std::string if it is set.
 * @return target String specifying the value of the environment variable
 * SYCL_VENDOR_PATHS
 */
TARGET_SELECTOR_EXPORT std::string get_target_from_environment_var();

/** parse_target.
 *   @brief Parses the target, either from the environment variable or via
 *   command line arg. vendor and deviceType parameters are set accordingly.
 *
 *   The SYCL_TARGET variable is interpreted as platform[:deviceType],
 *   whereas platform is the name of the platform vendor (e.g amd, intel)
 *   and deviceType is the OpenCL type of device (cpu, gpu, accel).
 *   No errors are reported at this stage if the vendor and/or deviceType are
 *   not valid, hence, the strings parsed from the environment variable may be
 *   reused.
 * @param target String specifying the target to be processed.
 * @param vendor String returning the vendor given by the environment variable.
 * @param deviceType String returning the type of the device given by the
 *                    environment variable.
 */
TARGET_SELECTOR_EXPORT void parse_target(const std::string& target,
                                         std::string& vendor,
                                         std::string& deviceType);

/** is_target_any.
 * @brief Checks whether the requested target can be any target
 * @param target String specifying the target to be processed.
 * @return True if target is empty string, "any", or "*"
 */
TARGET_SELECTOR_EXPORT bool is_target_any(const std::string& target);

/** find_Devices.
 *  @brief Finds all devices matching the given vendor and device type and
 *  returns them as a pair of (platform_id, device_id) in a vector.
 *  whereas platform is the name of the platform vendor (e.g amd, intel)
 *   and deviceType is the OpenCL type of device (cpu, gpu, accel).
 * @param devicesFound List of devices found by the routine
 * @param reqVendor Required name of the vendor to pick devices from
 * @param reqDeviceType Required type of device to select
 * @param all Whether all device should be searched, including non-SPIR ones
 * @param usedVendorAsType Set to true if reqVendor was used for the device
 *  type
 * @param platforms, the platforms to search for
 */
TARGET_SELECTOR_EXPORT void find_devices(
    std::vector<std::pair<cl_platform_id, cl_device_id>>& devicesFound,
    const std::string& reqVendor, const std::string& reqDeviceType,
    bool& usedVendorAsType,
    const std::unordered_map<std::string, std::string>& platforms,
    bool all = false);

/*!
  @brief Checks whether the device supports SPIR.
  @param device The device to get the info from
  @return Whether the device supports SPIR or not.
*/
TARGET_SELECTOR_EXPORT bool has_spir(cl_device_id device) noexcept;

/*
 *@brief Helper function boilerplate to generate a target_selector warning
 */
TARGET_SELECTOR_EXPORT void target_selector_warning(const std::string& message);

/*
 *@brief Helper function boilerplate to throw a target_selector error
 */
TARGET_SELECTOR_EXPORT void target_selector_throw_error(
    const std::string& message);

/*
 *@brief Helper function boilerplate that calls an OpenCL function and generate
 a warning *upon failure
 *@param a functor and a string message
 *@return the error code for the OpenCL function called and generate a warning
 with a the message passed if the call failed
 */
template <typename Callback>
::cl_int target_selector_warn_on_cl_error(Callback&& functor,
                                          const std::string& message) {
  ::cl_int err = functor();
  if (err != CL_SUCCESS) {
    std::stringstream stream;
    stream << "OpenCL error " << err << ": " << message;
    target_selector_warning(message);
  }
  return err;
}
/*
 *@brief Helper function boilerplate that calls an OpenCL function and throw an
 *error upon failure
 *@param a functor and a string message
 *@return the error code for the OpenCL function called or throw upon failure
 */
template <typename Callback>
::cl_int target_selector_throw_on_cl_error(Callback&& functor,
                                           const std::string& message) {
  ::cl_int err = functor();
  if (err != CL_SUCCESS) {
    std::stringstream stream;
    stream << "OpenCL error " << err << ": " << message;
    target_selector_warning(stream.str());
    throw sycl_info_error(stream.str());
  }
  return err;
}

/*!
  @brief Trips the end of a string by removing whitespaces, '\0'
  and
 */
TARGET_SELECTOR_EXPORT std::string trim_end(std::string str);

/*!
  @brief Boilerplate template that queries an OpenCL function
  @tparam IdType: the type of the id
  For example: cl_platform_id, cl_device_id, etc
  @tparam InfoType: the type of the information retrieved
  For example: cl_platform_info, cl_device_info
  @tparam Function: OpenCL function to query
  For example clGetPlatormInfo, clGetDeviceInfo.
  @param id: the unique identifier to query the OpenCL function Function
  @param attr: the information to query
  For example: CL_DEVICE_NAME
  @return The information specified with attr as a std::string. The error
  string otherwise
*/
template <typename IdType, typename InfoType, typename Function>
std::string get_info_from_opencl(IdType id, InfoType attr,
                                 Function fun) noexcept {
  size_t size = 0;

  auto result = target_selector_warn_on_cl_error(
      [&id, &attr, &size, &fun]() { return fun(id, attr, 0, nullptr, &size); },
      "Failed to retrieve the size.");
  if (result != CL_SUCCESS) {
    return "(ERROR)";
  }

  auto idString = std::string(size, ' ');

  result = target_selector_warn_on_cl_error(
      [&]() { return fun(id, attr, size, &idString[0], nullptr); },
      "Failed to retrieve the name");
  if (result != CL_SUCCESS) {
    return "(ERROR)";
  }

  return trim_end(std::move(idString));
}

}  // namespace target_selector
#endif  // UTIL_DEVICE_SELECTOR_H
