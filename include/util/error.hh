#pragma once

#include <string>
#include <string_view>

namespace jwezel {

using std::string, std::string_view;

struct Error {
  explicit Error(string_view message);

  Error(const Error &) = default;

  Error(Error &&) = delete;

  auto operator=(const Error &) -> Error & = default;

  auto operator=(Error &&) -> Error & = delete;

  virtual ~Error() = default;

  virtual void report(string_view /*message*/) {}

  private:
  string message_;
};

} // namespace jwezel
