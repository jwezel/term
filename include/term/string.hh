///
/// @defgroup   STRING string
///
/// This file defines supplemental string functions.
///
/// @author     J
/// @date       2021-01-02
///

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>

namespace jwezel {

template<typename T>
auto str(const T &v) -> std::string {
  return string(v);
}

auto repr(const std::string_view &str) -> std::string;

} // namespace jwezel

