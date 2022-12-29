#include <string>

#include "string.hh"
#include "utf8/checked.h"

namespace jwezel {

std::string repr(const std::string_view &str) {
  std::string result{"\""};
  result.reserve(str.size() + 2);
  for (const auto &c: str) {
    char32_t u = c < 32? c + 0x2400: c;
    utf8::utf32to8(&u, &u + 1, back_inserter(result));
  }
  result.push_back('\"');
  return result;
}

}
