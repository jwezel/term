#pragma once

#include "geometry.hh"
#include "text.hh"

namespace jwezel {

struct Update {
  Vector position;
  Text text;

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
    return make_tuple(position.x, position.y) < make_tuple(other.position.x, other.position.y);
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
