//
//  Copyright (C) Codeplay Software Limited.
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
#include "config.hpp"

#include "cli_config.hpp"
#include "impl_matchers.hpp"
#include <cstdlib>
#include <stdexcept>

/// \brief Generates a configuration object.
/// \returns A sycl_info::cli_config object containing the user options.
///
sycl_info::cli_config make_config(int argc, const char** argv) noexcept {
  try {
    return sycl_info::cli_config{argc, argv};
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << '\n';
    std::exit(1);
  }
}

/// \brief Checks if the user has passed any command line options
/// \returns true if any command line options were given, false
/// otherwise
///
bool arguments_passed(int argc) {
  constexpr int noArguments = 1;
  if (argc == noArguments) {
    sycl_info::print_impls(std::cout);
    std::cout << std::flush;
    return false;
  }
  return true;
}

/// \brief Returns the SYCL implementations available
///
std::vector<nlohmann::json> get_sycl_info_impls(
    const sycl_info::cli_config& config) {
  if (config.hint()) {
    auto hint = config.get_hint();
    return sycl_info::get_impls(std::move(hint));
  }
  return sycl_info::get_impls();
}

std::pair<int, int> get_index_from_config(const std::string& config) {
  const auto sep = config.find(":");
  const auto last = config.size();
  const int platformIndex = std::stoi(config.substr(0, sep));
  const int deviceIndex = std::stoi(config.substr(sep + 1, last - sep));
  return {platformIndex, deviceIndex};
}

void process_impl(const sycl_info::cli_config& config) {
  // sycl_info outputs starts from 1...N
  const std::string requestedImpl = config.get_impl();

  const auto availableImpls = get_sycl_info_impls(config);
  const auto implIndexQuery =
      sycl_info::retrieve_index_for_impl(requestedImpl, availableImpls);
  if (implIndexQuery.first) {
    const int implIndex = implIndexQuery.second;
    if (config.config() && config.device_compiler_flags()) {
      sycl_info::print_config(implIndex, availableImpls,
                              get_index_from_config(config.get_config()),
                              config.get_target(), config.all(), std::cout);
    } else if (!config.config() && !config.device_compiler_flags()) {
      sycl_info::print_picked_impl(implIndex, availableImpls, config.all(),
                                   std::cout);
    }
  } else {
    // TBA error
  }
}

/// \brief Utility function that process the command line arguments passed
///
void process_cli(sycl_info::cli_config config) {
  if (config.hint() && !config.impl()) {
    sycl_info::print_impls(std::cout, config.get_hint());
  } else if (config.help()) {
    config.show_help(std::cout);
  } else if (config.impl()) {
    process_impl(config);
  }

  std::cout << std::flush;
}

int main(int argc, const char** argv) {
  if (arguments_passed(argc)) {
    process_cli(::make_config(argc, argv));
  }
  return 0;
}
