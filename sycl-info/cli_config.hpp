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
#ifndef SYCL_INFO_CLI_CONFIG_HPP
#define SYCL_INFO_CLI_CONFIG_HPP

#include "config.hpp"

#include "utility.hpp"
#include <imp_finder/sycl_info_imp_finder.h>
#include <lyra/lyra.hpp>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sycl_info {
/// \brief Contains the sycl-info program configuration options.
///
class cli_config {
 public:
  /// \brief Establishes the invariants for an object of type cli_config.
  /// \param argc The number of user-specified arguments.
  /// \param argv A pointer to a C array of C strings
  /// \throws An object of type std::runtime_error if the parser fails to parse
  ///         command-line arguments, or one of the required command-line
  ///         arguments is missing.
  ///
  cli_config(int argc, const char** argv) {
    auto cli = make_cli();
    auto result = cli.parse(lyra::args(argc, argv));
    if (!result) {
      raise_error(result.errorMessage(), cli);
    }
  }

  /// \brief Returns the name of the running process.
  /// \returns A string containing the name of the process.
  ///
  SYCL_INFO_NODISCARD const std::string& process_name() const noexcept {
    return processName_;
  }

  /// \brief Returns whether or not the help option has been enabled.
  /// \returns true if the user has enabled help; false otherwise.
  ///
  SYCL_INFO_NODISCARD bool help() const noexcept { return help_; }

  /// \brief Writes the user manual to an output stream.
  /// \param o The stream to be written to.
  ///
  void show_help(std::ostream& out) noexcept {
    auto cli = make_cli();
    out << cli << '\n';
  }

  /// \brief Returns whether or not the user has requested verbose output.
  /// \returns true if the user has enabled verbose-mode; false otherwise.
  ///
  SYCL_INFO_NODISCARD bool verbose() const noexcept { return verbose_; }

  /// \brief Returns whether or not the user has requested to display all
  /// device/platform configurations, even those not supported.
  /// \returns true if the user has enabled verbose-mode; false otherwise.
  ///
  SYCL_INFO_NODISCARD bool all() const noexcept { return all_; }

  /// \brief Returns whether or not the user has requested output the
  ///        device-compiler's flags be output.
  /// \returns true if the user has requested device-compiler flags be output;
  ///          false otherwise.
  ///
  SYCL_INFO_NODISCARD bool device_compiler_flags() const noexcept {
    return deviceCompilerFlags_;
  }

  /// \brief Returns if the user has supplied a path hint for SYCL
  /// implementations
  /// \returns true if a --hint was given, false otherwise
  ///
  SYCL_INFO_NODISCARD bool hint() const noexcept { return !hint_.empty(); }

  /// \brief Returns the hint_ member
  /// \returns A path to the hint folder
  ///
  SYCL_INFO_NODISCARD const std::string& get_hint() const noexcept {
    return hint_;
  }

  /// \brief Returns if the user has provided an argument for the option --impl
  /// \returns True if the user passed a --impl, false otherwise
  ///
  SYCL_INFO_NODISCARD bool impl() const noexcept { return !impl_.empty(); }

  /// \brief Returns the member impl_
  /// \returns The value for the user-specified arg --impl
  ///
  SYCL_INFO_NODISCARD const std::string& get_impl() const noexcept {
    return impl_;
  }

  /// \brief Returns if the user has specified the option config_
  /// \returns True is config is not empty false otherwise
  ///
  SYCL_INFO_NODISCARD bool config() const noexcept { return !config_.empty(); }

  /// \brief Returns the member config_
  /// \returns the value for the user-specified arg --config
  ///
  SYCL_INFO_NODISCARD const std::string& get_config() const noexcept {
    return config_;
  }

  /// \brief Returns if the user has specified the member target_
  /// \returns True if the user has specified the option target_, false
  /// otherwise
  ///
  SYCL_INFO_NODISCARD bool target() const noexcept { return !target_.empty(); }

  /// \brief Returns the member target__
  /// \returns The target_ specified by the user
  ///
  SYCL_INFO_NODISCARD const std::string& get_target() const noexcept {
    return target_;
  }

 private:
  std::string processName_;
  bool help_{false};
  bool verbose_{false};
  bool all_{false};
  std::string target_;
  bool deviceCompilerFlags_{false};
  bool checkSupport_{false};
  std::string compilerTarget_;
  std::string hint_;
  std::string impl_;
  std::string config_;

  /// \brief Throws an exception with a reason that has a stable prefix and a
  ///        user-defined suffix.
  /// \param message The user-defined suffix.
  /// \throws Unconditionally throws an object of type std::runtime_error.
  ///
  template <class T>
  [[noreturn]] void raise_error(const std::string& message,
                                const T& cli) const {
    std::ostringstream out;
    out << processName_ << " command-line error: " << message << '\n' << cli;
    throw std::runtime_error{out.str()};
  }

  /// \brief Generates the CLI object.
  /// \returns A Lyra command-line object.
  ///
  auto make_cli() SYCL_INFO_NOEXCEPT_RETURN_DECLTYPE_AUTO(
      lyra::exe_name(processName_)  //
      | lyra::help(help_)           //
      | lyra::opt(verbose_)["-v"]["--verbose"](
            "Displays additional details for other options.")  //
      | lyra::opt(all_)["-a"]["--all"](
            "Displays all available device/platform configurations regardless "
            "of whether they are supported.")  //
      | lyra::opt(deviceCompilerFlags_)["--device-cflags"](
            "Outputs the flags required by the device compiler for s specified "
            "platform/device configuration.")  //
      | lyra::opt(hint_, "path_to_syclinfo")["--hint"](
            "Specifies a path to a directory containing .syclinfo files.")  //
      | lyra::opt(config_, "config")["--config"](
            "Selects a platform/device configuration from an impl.")  //
      | lyra::opt(target_, "target")["--target"](
            "Selects a SYCL back-end from a platform/device configuration.")  //
      | lyra::opt(impl_, "impl")["--impl"](
            "Selects a SYCL implementation and displays platform/device "
            "configurations (index starts at 1)."))
};
}  // namespace sycl_info

#endif  // SYCL_INFO_CLI_CONFIG_HPP
