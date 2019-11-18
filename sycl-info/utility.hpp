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
#ifndef SYCL_INFO_UTILITY_HPP
#define SYCL_INFO_UTILITY_HPP

#include "config.hpp"

/// \brief Makes it possible to have auto-return types in C++11.
/// \note This macro takes a single expression: it cannot take statements. If
///       you need to have multiple statements, you will need to use an
///       immediately-invoked lambda expression (and capture `this` if it is a
///       member function).
///
#define SYCL_INFO_NOEXCEPT_RETURN_DECLTYPE_AUTO(...) \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { return __VA_ARGS__; }

#endif  // SYCL_INFO_UTILITY_HPP
