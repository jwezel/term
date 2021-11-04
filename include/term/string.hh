///
/// @defgroup   STRING string
///
/// This file defines supplemental string functions.
///
/// @author     J
/// @date       2021-01-02
///

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>
#include <algorithm>
#include <assert.h>
#include <exception>
#include <iostream>

#include <fmt/core.h>

#include "text.hh"

namespace jwezel {

template<typename T>
std::string str(const T &v) {
  return string(v);
}

}
