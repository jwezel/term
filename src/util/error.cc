#include "error.hh"
#include <string_view>

namespace jwezel {

Error::Error(std::string_view message):
message_{message}
{}

} // namespace jwezel
