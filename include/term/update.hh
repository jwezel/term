#pragma once

#include <utility>

#include "geometry.hh"
#include "text.hh"

namespace jwezel {

struct Update {

  Update(const Vector &position, Text text): position{position}, text{std::move(text)} {}

  explicit operator string() const {
    return string(position) + ": " + text.repr();
  }

  auto operator ==(const Update &other) const -> bool {
    return position == other.position and text == other.text;
  }

  auto operator !=(const Update &other) const -> bool {
    return !(*this == other);
  }

  auto operator <(const Update &other) const -> bool {
    return position < other.position;
  }

  auto operator >(const Update &other) const -> bool {
    return other < *this;
  }

  auto operator <=(const Update &other) const -> bool {
    return !(*this < other);
  }

  auto operator >=(const Update &other) const -> bool {
    return !(other < *this);
  }

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  Vector position;
  Text text;
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

using Updates = vector<Update>;

} // namespace jwezel
