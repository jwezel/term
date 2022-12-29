#pragma once

#include "geometry.hh"
#include "text.hh"

namespace jwezel {

struct Update {
  Vector position;
  Text text;

  Update(const Vector &position, const Text &text): position{position}, text{text} {}

  operator string() const {
    return string(position) + ": " + text.repr();
  }

  bool operator ==(const Update &other) const {
    return position == other.position and text == other.text;
  }

  bool operator !=(const Update &other) const {
    return !(*this == other);
  }

  bool operator <(const Update &other) const {
    return position < other.position;
  }

  bool operator >(const Update &other) const {
    return other < *this;
  }

  bool operator <=(const Update &other) const {
    return !(*this < other);
  }

  bool operator >=(const Update &other) const {
    return !(other < *this);
  }
};

using Updates = vector<Update>;

} // namespace
