#include <string>

#include "string.hh"
#include "utf8/checked.h"

namespace jwezel {

auto repr(const std::string_view &str) -> std::string {
  static const auto UnicodeControlPicturesStart{0x2400};
  std::string result{"\""};
  const auto required{str.size() + 3};
  if (result.capacity() < required) {
    result.reserve(required);
  }
  for (const auto &ch: str) {
    char32_t uch = ch < ' '? ch + UnicodeControlPicturesStart: ch;
    utf8::utf32to8(&uch, &uch + 1, back_inserter(result));
  }
  result.push_back('\"');
  return result;
}

} // namespace jwezel
