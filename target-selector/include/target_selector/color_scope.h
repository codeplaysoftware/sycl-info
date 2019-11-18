////////////////////////////////////////////////////////////////////////////////
// color_scope.h
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

#ifndef COLOR_SCOPE_H
#define COLOR_SCOPE_H

#include "target_selector/config.h"

#include <ostream>

#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace target_selector {

/**
 @brief enum class that represents color codes in a *nix terminal
 */
enum class color_code {
  normal = 0,
  red = 31,
  green = 32,
  yellow = 33,
  cyan = 36
};

/**
 @brief overload operator << to support color_code
 */
inline std::ostream& operator<<(std::ostream& out, const color_code c) {
  return out << static_cast<int>(c);
}

#ifdef __linux__
/**
 @brief Print the color_code with color 'c' on the terminal.
        Only supported on linux.
 */
inline void color(color_code c, std::ostream& out) {
#if !defined(_WIN32)
  if (isatty(1) && isatty(2)) {
    out << "\33[" << c << 'm';
  }
#endif
}

/**
 @brief wrapper that pretty prints on construction/destruction the color passed
*/
struct color_scope {
  std::ostream& m_out;
  color_scope(color_code c, std::ostream& out) : m_out(out) { color(c, m_out); }
  ~color_scope() { color(color_code::normal, m_out); }
};

#endif  //__linux__
}  // namespace target_selector
#endif  // COLOR_SCOPE_H
